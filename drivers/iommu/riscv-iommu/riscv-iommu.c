#include "asm/type.h"
#include "asm/mmio.h"
#include "iommu.h"
#include "device.h"
#include "string.h"
#include "vmap.h"
#include "print.h"
#include "riscv-iommu.h"
#include "io_pgtable.h"
#include "mm.h"
#include "queue.h"

extern int mmu_is_on;

static unsigned long riscv_iommu_atp(struct riscv_iommu_device *iommu)
{
	unsigned long atp = ((unsigned long)iommu->mode << 60) & DC_FSC_MODE_MASK;
	void *pgdp = iommu->g_stage_enabled ? iommu->pgdp_gstage : iommu->pgdp;
	unsigned long pgdp_pa;

	if (mmu_is_on)
		pgdp_pa = virt_to_phy(pgdp);
	else
		pgdp_pa = (unsigned long)pgdp;

	if (iommu->mode != RISCV_IOMMU_DC_FSC_MODE_BARE)
		atp |= ((unsigned long)pgdp_pa >> PAGE_SHIFT) & ATP_PPN_MASK;
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
		unsigned long ddtp_pa;
		/*
		 * Each non-leaf node is 64bits wide and on each level
		 * nodes are indexed by DDI[depth].
		 */
		ddtp += (dev_id >> split) & 0x1FF;
		ddt = *ddtp;
		if (ddt & RISCV_IOMMU_DDTE_VALID) {
			ddtp = (unsigned long *)phy_to_virt(ppn_to_phys(ddt));
		} else {
			unsigned long new = (unsigned long)mm_alloc(4096);
			if (!new) {
				print("Error -- riscv-iommu: Alloc pte failed\n");
				return NULL;
			}
			memset((char *)new, 0, 4096);
			if (mmu_is_on)
				ddtp_pa = virt_to_phy(new);
			else
				ddtp_pa = new;
			ddt = phys_to_ppn(ddtp_pa) | RISCV_IOMMU_DDTE_VALID;
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
	unsigned long ddtp_pa;

	switch (iommu->ddt_mode) {
	case DDTP_MODE_OFF:
	case DDTP_MODE_BARE:
		iommu->ddtp = 0;
		break;
	case DDTP_MODE_1LEVEL:
	case DDTP_MODE_2LEVEL:
	case DDTP_MODE_3LEVEL:
		iommu->ddtp = mm_alloc(4096);
		memset(iommu->ddtp, 0 ,4096);
		if (mmu_is_on)
			ddtp_pa = virt_to_phy(iommu->ddtp);
		else
			ddtp_pa = (unsigned long)iommu->ddtp;
		ddtp =
		    ((ddtp_pa >> 2) & DDTP_PPN_MASK) | iommu->ddt_mode;
		break;
	default:
		return -1;
	}

	writeq(iommu->base + IOMMU_DDTP_OFFSET, ddtp);

	return 0;
}

static unsigned long riscv_iommu_walk_pt(struct device *dev, unsigned long iova, int gstage)
{
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu->priv;
	unsigned addr;

	if (gstage) {
		if (!iommu_dev->g_stage_enabled) {
			print("Error -- riscv-iommu: G_stage_enabled is NULL\n");
			return -1;
		}

		addr = riscv_iommu_gstage_io_walk_pt(iommu_dev, iova);
	} else
		addr = riscv_iommu_fstage_io_walk_pt(iommu_dev, iova);

	return addr;
}

static int riscv_iommu_page_mapping(void *pgdp, unsigned long iova, void *addr,
				    unsigned int size, int gfp)
{
	return riscv_iommu_io_page_mapping(pgdp, iova, addr, size, gfp);
}

static int riscv_iommu_map_pages(struct device *dev, unsigned long iova,
				 void *addr, unsigned int size, int gstage)
{
	int ret = 0;
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu->priv;

	if (gstage) {
		if (!iommu_dev->g_stage_enabled) {
			print("Error -- riscv-iommu: G_stage_enabled is NULL\n");
			return -1;
		}

		ret = riscv_iommu_gstage_io_map_pages(iommu_dev, iova, addr, size, 0);
	} else
		ret = riscv_iommu_fstage_io_map_pages(iommu_dev, iova, addr, size, 0);

	if (ret)
		return ret;

	if (gstage)
		riscv_iommu_gstage_finalize(iommu_dev);

	return ret;
}

static int riscv_iommu_map_msi_addr(struct device *dev, unsigned long msi_pa,
				    unsigned long msi_iova, int len)
{

	return 0;
}

static void *riscv_iommu_alloc(struct device *dev, unsigned long iova,
			       unsigned int size, unsigned int gfp)
{
	void *addr = NULL;
	struct iommu *iommu = dev->iommu;
	struct riscv_iommu *riscv_iommu;
	static struct riscv_iommu_dc *dc;

	if (!iommu)
		return NULL;

	riscv_iommu = to_riscv_iommu(iommu);

	dc = riscv_iommu_get_dc(riscv_iommu, dev->dev_id);
	if (!dc) {
		print("riscv iommu: get dc failed\n");
		return NULL;
	}

	addr = (void *)virt_to_phy(mm_alloc_align(PAGE_SIZE, PAGE_SIZE));
	if (!addr) {
		print("Error -- riscv-iommu: alloc failed\n");
		return NULL;
	}

	if (riscv_iommu_map_pages(dev, iova, addr, size, 0)) {
		print("Error -- riscv iommu: map pages failed\n");
		return NULL;
	}

	return addr;
}

static void riscv_iommu_enable_gstage(struct device *dev, int gstage)
{
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu->priv;

	iommu_dev->g_stage_enabled = gstage;
}

static int riscv_iommu_probe_device(struct device *dev)
{
	struct iommu *iommu = dev->iommu;
	struct riscv_iommu_device *iommu_dev;
	struct riscv_iommu *riscv_iommu = to_riscv_iommu(iommu);

	iommu_dev = (struct riscv_iommu_device *)mm_alloc(sizeof(struct riscv_iommu_device));
	if (!iommu_dev) {
		print("Error -- riscv-iommu: Alloc iommu_dev failed\n");
		return -1;
	}
	memset((char *)iommu_dev, 0, sizeof(struct riscv_iommu_device));

	iommu_dev->dc = riscv_iommu_get_dc(riscv_iommu, dev->dev_id);
	iommu_dev->mode = riscv_iommu->pg_mode;

	iommu->priv = iommu_dev;

	return 0;
}

static int riscv_iommu_finalize(struct device *dev, int pscid)
{
	unsigned long val = 0;
	struct iommu_group *gp;
	struct riscv_iommu_device *iommu_dev =
	    (struct riscv_iommu_device *)dev->iommu->priv;
	struct riscv_iommu_dc *dc;
	struct riscv_iommu_msi_pte *msi_pte;
	int pgd_size;
	int i;

	if (!iommu_dev) {
		print("Error -- riscv-iommu: Iommu_dev is NULL\n");
		return -1;
	}

	dc = iommu_dev->dc;
	if (!dc) {
		print("Error -- riscv-iommu: DC is NULL\n");
		return -1;
	}

	gp = iommu_get_group(dev);
	if (!gp) {
		print("Error -- riscv-iommu: Can not find iommu group\n");
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
			print("Error -- riscv-iommu: unsupported pt mode\n");
			return -1;
		}

		gp->pgdp = mm_alloc(pgd_size);
		if (!gp->pgdp) {
			print("Error -- riscv-iommu: Out of memory size:%d\n", pgd_size);
			return -1;
		}
	}

	if (iommu_dev->g_stage_enabled) {
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
				print("Error -- riscv-iommu: Unsupported pt mode\n");
				return -1;
			}

			gp->pgdp_gstage = mm_alloc(pgd_size);
			if (!gp->pgdp_gstage) {
				print("Error -- riscv-iommu: Out of memory size:%d\n",
				      pgd_size);
				return -1;
			}
		}
	}

	if (!gp->msi_root) {
		gp->msi_root = (struct riscv_iommu_msi_pte *)mm_alloc(PAGE_SIZE);
		if (!gp->msi_root) {
			print("Warning -- riscv-iommu: Alloc msi root failed\n");
		}
		for (i = 0; i < 256; i++) {
			msi_pte = &gp->msi_root[i];
			msi_pte->pte = IOMMU_MSI_PTE_V | IOMMU_MSI_PTE_M;
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

static struct iommu_ops riscv_iommu_ops = {
	.alloc = riscv_iommu_alloc,
	.map_pages = riscv_iommu_map_pages,
	.page_mapping = riscv_iommu_page_mapping,
	.map_msi_addr = riscv_iommu_map_msi_addr,
	.walk_pt = riscv_iommu_walk_pt,
	.probe_device = riscv_iommu_probe_device,
	.finalize = riscv_iommu_finalize,
	.enable_gstage = riscv_iommu_enable_gstage,
};

int riscv_iommu_init(struct device *dev, void *data)
{
	struct riscv_iommu *riscv_iommu;
	struct riscv_iommu_priv_data *priv =
	    (struct riscv_iommu_priv_data *)data;

	riscv_iommu = (struct riscv_iommu *)mm_alloc(sizeof(struct riscv_iommu));
	if (!riscv_iommu)
		return -1;
	memset((char *)riscv_iommu, 0, sizeof(struct riscv_iommu));

	riscv_iommu->base = (unsigned long)ioremap((void *)dev->base, dev->len, 0);
	strcpy(riscv_iommu->iommu.name, dev->compatible);
	riscv_iommu->cmdq_len = priv->cmdq_len;
	riscv_iommu->fltq_len = priv->fltq_len;
	riscv_iommu->priq_len = priv->priq_len;
	riscv_iommu->cmdq_irq = priv->cmdq_irq;
	riscv_iommu->fltq_irq = priv->fltq_irq;
	riscv_iommu->priq_irq = priv->priq_irq;
	riscv_iommu->ddt_mode = priv->ddt_mode;
	riscv_iommu->pg_mode = priv->pg_mode;
	print
	    ("riscv iommu: base:0x%x, cmdq_len:%d, fltq_len:%d, priq_len:%d, ddt_mode:%d\n",
	     dev->base, riscv_iommu->cmdq_len, riscv_iommu->fltq_len,
	     riscv_iommu->priq_len, riscv_iommu->ddt_mode);

	// read iommu capabilities
	riscv_iommu->cap = readq(riscv_iommu->base + IOMMU_CAPABILITIES_OFFSET);

	// Clear any pending interrupt flag
	writel(riscv_iommu->base + IOMMU_IPSR_OFFSET,
	       CIP_MASK | FIP_MASK | PMIP_MASK | PIP_MASK);

	riscv_iommu_enable(riscv_iommu);

	riscv_iommu->iommu.ops = &riscv_iommu_ops;

	iommu_register(&riscv_iommu->iommu);

	return 0;
}

DRIVER_REGISTER(riscv_iommu, riscv_iommu_init, "riscv,iommu");
