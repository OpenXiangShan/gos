#include <asm/type.h>
#include <asm/mmio.h>
#include <device.h>
#include <print.h>
#include <mm.h>
#include <string.h>
#include "iommu.h"
#include "dma_mapping.h"
#include "io_pgtable.h"
#include "queue.h"

static struct riscv_iommu iommu;

static struct riscv_iommu_dc *riscv_iommu_get_dc(struct riscv_iommu *iommu,
						 int dev_id)
{
	int depth = iommu->ddt_mode - DDTP_MODE_1LEVEL;
	char ddi_bits[3] = { 0 };
	unsigned long *ddtp = NULL, ddt;
	int base_format = iommu->cap & RISCV_IOMMU_CAP_MSI_FLAT;

	if (iommu->ddt_mode == DDTP_MODE_OFF ||
	    iommu->ddt_mode == DDTP_MODE_BARE ||
	    iommu->ddt_mode > DDTP_MODE_3LEVEL)
		return NULL;
	/*
	 * Device id partitioning for base format:
	 * DDI[0]: bits 0 - 6   (1st level) (7 bits)
	 * DDI[1]: bits 7 - 15  (2nd level) (9 bits)
	 * DDI[2]: bits 16 - 23 (3rd level) (8 bits)
	 *
	 * For extended format:
	 * DDI[0]: bits 0 - 5   (1st level) (6 bits)
	 * DDI[1]: bits 6 - 14  (2nd level) (9 bits)
	 * DDI[2]: bits 15 - 23 (3rd level) (9 bits)
	 */
	if (base_format) {
		ddi_bits[0] = 7;
		ddi_bits[1] = 7 + 9;
		ddi_bits[2] = 7 + 9 + 8;
	} else {
		ddi_bits[0] = 6;
		ddi_bits[1] = 6 + 9;
		ddi_bits[2] = 6 + 9 + 9;
	}

	if (dev_id >= (1 << ddi_bits[depth]))
		return NULL;

	for (ddtp = (unsigned long *)iommu->ddtp; depth-- > 0;) {
		const int split = ddi_bits[depth];
		/*
		 * Each non-leaf node is 64bits wide and on each level
		 * nodes are indexed by DDI[depth].
		 */
		ddtp += (dev_id >> split) & 0x1FF;
		ddt = *ddtp;
		if (ddt & RISCV_IOMMU_DDTE_VALID) {
			ddtp = (unsigned long *)ppn_to_phys(ddt);
		} else {
			unsigned long new = (unsigned long)mm_alloc(4096);
			if (!new) {
				print("Out of memory -- %s %d\n", __FUNCTION__,
				      __LINE__);
				return NULL;
			}
			ddt = phys_to_ppn(new) | RISCV_IOMMU_DDTE_VALID;
			*ddtp = ddt;
			ddtp = (unsigned long *)new;
		}
	}

	/*
	   find dc in last ddtp.
	   DC is 4*64bit in base format and 8*64bit in extended format
	 */
	ddtp += (dev_id & ((64 << base_format) - 1)) << (3 - base_format);

	return (struct riscv_iommu_dc *)ddtp;
}

static int riscv_iommu_enable(struct riscv_iommu *iommu)
{
	unsigned long ddtp;

	switch (iommu->ddt_mode) {
	case DDTP_MODE_OFF:
	case DDTP_MODE_BARE:
		iommu->ddtp = 0;
		break;
	case DDTP_MODE_1LEVEL:
	case DDTP_MODE_2LEVEL:
	case DDTP_MODE_3LEVEL:
		iommu->ddtp = mm_alloc(4096);
		ddtp =
		    (((unsigned long)iommu->ddtp >> 2) & DDTP_PPN_MASK) |
		    iommu->ddt_mode;
		break;
	default:
		return -1;
	}

	writeq(iommu->base + IOMMU_DDTP_OFFSET, ddtp);

	return 0;
}

static unsigned long riscv_iommu_atp(struct riscv_iommu_device *iommu)
{
	unsigned long atp = iommu->mode & DC_FSC_MODE_MASK;
	void *pgdp = iommu->g_stage_enabled ? iommu->pgdp_gstage : iommu->pgdp;

	if (iommu->mode != RISCV_IOMMU_DC_FSC_MODE_BARE)
		atp |= ((unsigned long)pgdp >> PAGE_SHIFT) & ATP_PPN_MASK;
	if (iommu->g_stage_enabled)
		atp |= iommu->pscid & IOHGATP_GSCID_MASK;

	return atp;
}

static int riscv_iommu_gstage_finalize(struct riscv_iommu_device *iommu_dev)
{
	if (!iommu_dev->g_stage_enabled)
		return -1;

	iommu_dev->dc->fsc = riscv_iommu_atp(iommu_dev);

	return 0;
}

static int riscv_iommu_map_pages(struct device *dev, unsigned long iova,
				 void *addr, unsigned int size, int gstage)
{
	int ret = 0;
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu.priv_data;

	if (gstage) {
		if (!iommu_dev->g_stage_enabled) {
			print("%s -- g_stage_enabled is NULL\n", __FUNCTION__);
			return -1;
		}

		ret = riscv_gstage_map_pages(iommu_dev, iova, addr, size, 0);
	} else
		ret = riscv_fstage_map_pages(iommu_dev, iova, addr, size, 0);

	if (ret)
		return ret;

	if (gstage)
		riscv_iommu_gstage_finalize(iommu_dev);

	return ret;
}

static void *riscv_iommu_alloc(struct device *dev, unsigned long iova,
			       unsigned int size, unsigned int gfp)
{
	void *addr = NULL;
	static struct riscv_iommu_dc *dc;

	dc = riscv_iommu_get_dc(&iommu, dev->iommu.dev_id);
	if (!dc) {
		print("riscv iommu get dc failed\n");
		return NULL;
	}

	addr = mm_alloc(size);
	if (!addr) {
		print("%s - %s -- Out of memory\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (riscv_iommu_map_pages(dev, iova, addr, size, 0)) {
		print("riscv map pages failed\n");
		return NULL;
	}

	return addr;
}

static int riscv_iommu_finalize(struct device *dev, int pscid)
{
	unsigned long val = 0;
	struct iommu_group *gp;
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu.priv_data;
	struct riscv_iommu_dc *dc;
	int pgd_size;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n", __FUNCTION__);
		return -1;
	}

	dc = iommu_dev->dc;
	if (!dc) {
		print("%s -- dc is NULL\n", __FUNCTION__);
		return -1;
	}

	gp = dma_mapping_find_iommu_group(dev);
	if (!gp) {
		print("%s -- can not find iommu group\n", __FUNCTION__);
		return -1;
	}

	if (!gp->pgdp) {
		if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV32)
			pgd_size = PAGE_SIZE;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV39)
			pgd_size = PAGE_SIZE;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV48)
			pgd_size = PAGE_SIZE;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV57)
			pgd_size = PAGE_SIZE;
		else {
			print("%s -- unsupported pt mode\n", __FUNCTION__);
			return -1;
		}

		gp->pgdp = mm_alloc(pgd_size);
		if (!gp->pgdp) {
			print("%s -- Out of memory size:%d\n", __FUNCTION__,
			      pgd_size);
			return -1;
		}
	}

	if (!gp->pgdp_gstage) {
		if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV32)
			pgd_size = PAGE_SIZE * 4;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV39)
			pgd_size = PAGE_SIZE * 4;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV48)
			pgd_size = PAGE_SIZE * 4;
		else if (iommu_dev->mode == RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV57)
			pgd_size = PAGE_SIZE * 4;
		else {
			print("%s -- unsupported pt mode\n", __FUNCTION__);
			return -1;
		}

		gp->pgdp_gstage = mm_alloc(pgd_size);
		if (!gp->pgdp_gstage) {
			print("%s -- Out of memory size:%d\n", __FUNCTION__,
			      pgd_size);
			return -1;
		}
	}

	iommu_dev->pgdp = gp->pgdp;
	iommu_dev->pgdp_gstage = gp->pgdp_gstage;
	iommu_dev->pscid = pscid;

	if (iommu_dev->g_stage_enabled) {
		iommu_dev->dc->ta = 0;
		iommu_dev->dc->fsc = 0;
		iommu_dev->dc->iohgatp = riscv_iommu_atp(iommu_dev);
	} else {
		if (iommu_dev->pasid_enabled) {

		} else {
			val = DC_TA_PSCID_MASK & iommu_dev->pscid;
			iommu_dev->dc->ta = val;
			iommu_dev->dc->fsc = riscv_iommu_atp(iommu_dev);
		}
	}

	val = IOMMU_DC_TC_V;
	dc->tc = val;

	return 0;
}

static void *riscv_iommu_probe_device(struct device *dev)
{
	struct riscv_iommu_device *iommu_dev;

	iommu_dev = mm_alloc(sizeof(struct riscv_iommu_device));
	if (!iommu_dev) {
		print("Out of memory -- %s\n", __FUNCTION__);
		return NULL;
	}
	memset((char *)iommu_dev, 0, sizeof(struct riscv_iommu_device));

	iommu_dev->dc = riscv_iommu_get_dc(&iommu, dev->iommu.dev_id);

	iommu_dev->mode = iommu.pg_mode;

	return iommu_dev;
}

unsigned long riscv_iommu_iova_to_phys(struct device *dev, unsigned long iova)
{
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu.priv_data;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n", __FUNCTION__);
		return 0;
	}

	return riscv_iova_to_phys(iommu_dev, iova);
}

static unsigned long riscv_iommu_iova_to_phys_with_devid(int dev_id,
							 unsigned long iova)
{
	struct riscv_iommu_dc *dc;
	unsigned long *ddtp, ddt, *pgdp = NULL, *pte;
	int depth = iommu.ddt_mode - DDTP_MODE_1LEVEL;
	char ddi_bits[3] = { 0 };
	int base_format = iommu.cap & RISCV_IOMMU_CAP_MSI_FLAT;

	if (base_format) {
		ddi_bits[0] = 7;
		ddi_bits[1] = 7 + 9;
		ddi_bits[2] = 7 + 9 + 8;
	} else {
		ddi_bits[0] = 6;
		ddi_bits[1] = 6 + 9;
		ddi_bits[2] = 6 + 9 + 9;
	}

	if (dev_id >= (1 << ddi_bits[depth]))
		return NULL;

	for (ddtp = (unsigned long *)iommu.ddtp; depth-- > 0;) {
		const int shift = ddi_bits[depth];

		ddtp += (dev_id >> shift) & 0x1FF;
		ddt = *ddtp;
		if (ddt & RISCV_IOMMU_DDTE_VALID)
			ddtp = (unsigned long *)ppn_to_phys(ddt);
		else
			return NULL;
	}
	ddtp += (dev_id & ((64 << base_format) - 1)) << (3 - base_format);

	dc = (struct riscv_iommu_dc *)ddtp;
	if (!dc)
		return NULL;

	if (dc->fsc)
		pgdp = (unsigned long *)(dc->fsc << PAGE_SHIFT);

	if (!pgdp)
		return NULL;

	pte = riscv_iommu_pt_walk_fetch(pgdp, iova, PGDIR_SHIFT, 1);
	if (!pte) {
		print("%s -- pt walk fetch pte is NULL\n", __FUNCTION__);
		return 0;
	}
	if (!pmd_present(*pte)) {
		print("%s -- pte entry is not persent\n", __FUNCTION__);
		return 0;
	}

	return (pfn_to_phys(pte_pfn(*pte)) | (iova & (PAGE_SIZE - 1)));
}

void riscv_iommu_enable_gstage(struct device *dev, int gstage)
{
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu.priv_data;

	iommu_dev->g_stage_enabled = gstage;
}

unsigned long riscv_iommu_iova_to_phys_in_two_stage(struct device *dev,
						    unsigned long iova)
{
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu.priv_data;
	unsigned long gpa = 0;

	if (!iommu_dev) {
		print("%s -- iommu_dev is NULL\n", __FUNCTION__);
		return 0;
	}

	if (!iommu_dev->g_stage_enabled) {
		print("%s -- g_stage_enabled is NULL\n", __FUNCTION__);
		return 0;
	}
	//first stage page walk (iova --> gpa)
	gpa = riscv_iova_to_phys(iommu_dev, iova);

	//gstage page walk
	return riscv_iova_to_phys_gstage(iommu_dev, gpa);
}

unsigned long riscv_iommu_iova_to_phys_in_two_stage_with_devid(int dev_id,
							       unsigned long
							       iova)
{
	return NULL;
}

static struct dma_map_ops riscv_iommu_mapping_ops = {
	.alloc = riscv_iommu_alloc,
	.map_pages = riscv_iommu_map_pages,
	.probe_device = riscv_iommu_probe_device,
	.finalize = riscv_iommu_finalize,
	.iova_to_phys = riscv_iommu_iova_to_phys,
	.iova_to_phys_with_devid = riscv_iommu_iova_to_phys_with_devid,
	.enable_gstage = riscv_iommu_enable_gstage,
	.iova_to_phys_in_two_stage = riscv_iommu_iova_to_phys_in_two_stage,
	.iova_to_phys_in_two_stage_with_devid =
	    riscv_iommu_iova_to_phys_in_two_stage_with_devid,
};

int riscv_iommu_init(struct device *dev, void *data)
{
	struct riscv_iommu_priv_data *priv =
	    (struct riscv_iommu_priv_data *)data;

	print("%s %d base: 0x%x, len: %d, irq: %d\n", __FUNCTION__, __LINE__,
	      dev->start, dev->len, dev->irqs[0]);

	memset((char *)&iommu, 0, sizeof(struct riscv_iommu));

	iommu.base = dev->start;
	iommu.cmdq_len = priv->cmdq_len;
	iommu.fltq_len = priv->fltq_len;
	iommu.priq_len = priv->priq_len;
	iommu.cmdq_irq = priv->cmdq_irq;
	iommu.fltq_irq = priv->fltq_irq;
	iommu.priq_irq = priv->priq_irq;
	iommu.ddt_mode = priv->ddt_mode;
	iommu.pg_mode = priv->pg_mode;
	print
	    ("%s -- base:0x%x, cmdq_len:%d, fltq_len:%d, priq_len:%d, ddt_mode:%d\n",
	     __FUNCTION__, iommu.base, iommu.cmdq_len, iommu.fltq_len,
	     iommu.priq_len, iommu.ddt_mode);

	// read iommu capabilities
	iommu.cap = readq(iommu.base + IOMMU_CAPABILITIES_OFFSET);

	// Clear any pending interrupt flag
	writel(iommu.base + IOMMU_IPSR_OFFSET,
	       CIP_MASK | FIP_MASK | PMIP_MASK | PIP_MASK);

	// Init queue   
	if (riscv_iommu_cmdq_init(&iommu)) {
		print("riscv iommu cmdq init failed\n");
		return -1;
	}
	if (riscv_iommu_fltq_init(&iommu)) {
		print("riscv iommu fltq init failed\n");
		return -1;
	}
	if (riscv_iommu_priq_init(&iommu)) {
		print("riscv iommu priq init failed\n");
		return -1;
	}

	riscv_iommu_enable(&iommu);

	set_dma_mapping_ops(&riscv_iommu_mapping_ops);

	print("riscv_iommu_init success\n");

	return 0;
}

DRIVER_REGISTER(riscv_iommu, riscv_iommu_init, "riscv,iommu");
