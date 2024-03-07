#ifndef RISCV_IOMMU_H
#define RISCV_IOMMU_H

#define DDTP_PPN_MASK      (0x3FFFFFFFFFFC00ULL)
#define DC_FSC_MODE_MASK   (~0x0FFFFFFFFFFFFFFFULL)
#define ATP_PPN_MASK       (0xFFFFFFFFFFFULL)
#define IOHGATP_GSCID_MASK (0xFFFF00000000000ULL)
#define DC_TA_PSCID_MASK   (0xFFFFF000ULL)

#define IOMMU_CAPABILITIES_OFFSET   0x0
#define IOMMU_FCTL_OFFSET           0x8
#define IOMMU_DDTP_OFFSET           0x10
#define IOMMU_CQB_OFFSET            0x18
#define IOMMU_CQH_OFFSET            0x20
#define IOMMU_CQT_OFFSET            0x24
#define IOMMU_FQB_OFFSET            0x28
#define IOMMU_FQH_OFFSET            0x30
#define IOMMU_FQT_OFFSET            0x34
#define IOMMU_CQCSR_OFFSET          0x48
#define IOMMU_FQCSR_OFFSET          0x4c
#define IOMMU_IPSR_OFFSET           0x54
#define IOMMU_IOCOUNTOVF_OFFSET     0x58
#define IOMMU_IOCOUNTINH_OFFSET     0x5c
#define IOMMU_IOHPMCYCLES_OFFSET    0x60
#define IOMMU_IOHPMCTR_OFFSET       0x68
#define IOMMU_IOHPMEVT_OFFSET       0x160
#define IOMMU_TR_REQ_IOVA_OFFSET    0x258
#define IOMMU_TR_REQ_CTL_OFFSET     0x260
#define IOMMU_TR_RESPONSE_OFFSET    0x268
#define IOMMU_ICVEC_OFFSET          0x2f8
#define IOMMU_MSI_ADDR_0_OFFSET     0x300
#define IOMMU_MSI_DATA_0_OFFSET     0x308
#define IOMMU_MSI_VEC_CTL_0_OFFSET  0x30c
#define IOMMU_MSI_ADDR_1_OFFSET     0x310
#define IOMMU_MSI_DATA_1_OFFSET     0x318
#define IOMMU_MSI_VEC_CTL_1_OFFSET  0x31c
#define IOMMU_MSI_ADDR_2_OFFSET     0x320
#define IOMMU_MSI_DATA_2_OFFSET     0x328
#define IOMMU_MSI_VEC_CTL_2_OFFSET  0x32c
#define IOMMU_MSI_ADDR_3_OFFSET     0x330
#define IOMMU_MSI_DATA_3_OFFSET     0x338
#define IOMMU_MSI_VEC_CTL_3_OFFSET  0x33c
#define IOMMU_MSI_ADDR_4_OFFSET     0x340
#define IOMMU_MSI_DATA_4_OFFSET     0x348
#define IOMMU_MSI_VEC_CTL_4_OFFSET  0x34c
#define IOMMU_MSI_ADDR_5_OFFSET     0x350
#define IOMMU_MSI_DATA_5_OFFSET     0x358
#define IOMMU_MSI_VEC_CTL_5_OFFSET  0x35c
#define IOMMU_MSI_ADDR_6_OFFSET     0x360
#define IOMMU_MSI_DATA_6_OFFSET     0x368
#define IOMMU_MSI_VEC_CTL_6_OFFSET  0x36c
#define IOMMU_MSI_ADDR_7_OFFSET     0x370
#define IOMMU_MSI_DATA_7_OFFSET     0x378
#define IOMMU_MSI_VEC_CTL_7_OFFSET  0x37c
#define IOMMU_MSI_ADDR_8_OFFSET     0x380
#define IOMMU_MSI_DATA_8_OFFSET     0x388
#define IOMMU_MSI_VEC_CTL_8_OFFSET  0x38c
#define IOMMU_MSI_ADDR_9_OFFSET     0x390
#define IOMMU_MSI_DATA_9_OFFSET     0x398
#define IOMMU_MSI_VEC_CTL_9_OFFSET  0x39c
#define IOMMU_MSI_ADDR_10_OFFSET    0x3a0
#define IOMMU_MSI_DATA_10_OFFSET    0x3a8
#define IOMMU_MSI_VEC_CTL_10_OFFSET 0x3ac
#define IOMMU_MSI_ADDR_11_OFFSET    0x3b0
#define IOMMU_MSI_DATA_11_OFFSET    0x3b8
#define IOMMU_MSI_VEC_CTL_11_OFFSET 0x3bc
#define IOMMU_MSI_ADDR_12_OFFSET    0x3c0
#define IOMMU_MSI_DATA_12_OFFSET    0x3c8
#define IOMMU_MSI_VEC_CTL_12_OFFSET 0x3cc
#define IOMMU_MSI_ADDR_13_OFFSET    0x3d0
#define IOMMU_MSI_DATA_13_OFFSET    0x3d8
#define IOMMU_MSI_VEC_CTL_13_OFFSET 0x3dc
#define IOMMU_MSI_ADDR_14_OFFSET    0x3e0
#define IOMMU_MSI_DATA_14_OFFSET    0x3e8
#define IOMMU_MSI_VEC_CTL_14_OFFSET 0x3ec
#define IOMMU_MSI_ADDR_15_OFFSET    0x3f0
#define IOMMU_MSI_DATA_15_OFFSET    0x3f8
#define IOMMU_MSI_VEC_CTL_15_OFFSET 0x3fc

#define CQ_LOG2SZ_1     (5 )
// Mask for cqb.PPN (cqb[53:10])
#define CQB_PPN_MASK    (0x3FFFFFFFFFFC00ULL)
// Mask for CQ PPN (cqb[55:12])
#define CQ_PPN_MASK        (0xFFFFFFFFFFF000ULL)

#define FQ_LOG2SZ_1     (5 )
// Mask for cqb.PPN (fqb[53:10])
#define FQB_PPN_MASK    (0x3FFFFFFFFFFC00ULL)
// Mask for CQ PPN (cqb[55:12])
#define FQ_PPN_MASK     (0xFFFFFFFFFFF000ULL)

/* Translation control fields */
#define IOMMU_DC_TC_V             (1ULL << 0)
#define IOMMU_DC_TC_EN_ATS        (1ULL << 1)
#define IOMMU_DC_TC_EN_PRI        (1ULL << 2)
#define IOMMU_DC_TC_T2GPA         (1ULL << 3)
#define IOMMU_DC_TC_DTF           (1ULL << 4)
#define IOMMU_DC_TC_PDTV          (1ULL << 5)
#define IOMMU_DC_TC_PRPR          (1ULL << 6)
#define IOMMU_DC_TC_GADE          (1ULL << 7)
#define IOMMU_DC_TC_SADE          (1ULL << 8)
#define IOMMU_DC_TC_DPE           (1ULL << 9)
#define IOMMU_DC_TC_SBE           (1ULL << 10)
#define IOMMU_DC_TC_SXL           (1ULL << 11)

// cqcsr masks
#define CQCSR_CQEN          (1UL << 0)
#define CQCSR_CIE           (1UL << 1)
#define CQCSR_CQMF          (1UL << 8)
#define CQCSR_CMD_TO        (1UL << 9)
#define CQCSR_CMD_ILL       (1UL << 10)
#define CQCSR_FENCE_W_IP    (1UL << 11)
#define CQCSR_CQON          (1UL << 16)
#define CQCSR_BUSY          (1UL << 17)

// fqcsr masks
#define FQCSR_FQEN      (1ULL << 0 )
#define FQCSR_FIE       (1ULL << 1 )
#define FQCSR_FQMF      (1ULL << 8 )
#define FQCSR_FQOF      (1ULL << 9 )
#define FQCSR_FQON      (1ULL << 16)
#define FQCSR_BUSY      (1ULL << 17)

// Interrupt pending bits
#define CIP_MASK            (1UL << 0)
#define FIP_MASK            (1UL << 1)
#define PMIP_MASK           (1UL << 2)
#define PIP_MASK            (1UL << 3)

#define RISCV_IOMMU_CAP_MSI_FLAT (1UL << 22)
#define RISCV_IOMMU_DDTE_VALID  (1UL << 0)

/* RISC-V IOMMU PPN <> PHYS address conversions, PHYS <=> PPN[53:10] */
#define phys_to_ppn(pa)  (((pa) >> 2) & (((1ULL << 44) - 1) << 10))
#define ppn_to_phys(pn)  (((pn) << 2) & (((1ULL << 44) - 1) << 12))

enum riscv_iommu_ddtp_modes {
	DDTP_MODE_OFF,
	DDTP_MODE_BARE,
	DDTP_MODE_1LEVEL,
	DDTP_MODE_2LEVEL,
	DDTP_MODE_3LEVEL
};

enum riscv_iommu_dc_fsc_atp_modes {
	RISCV_IOMMU_DC_FSC_MODE_BARE = 0,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV32 = 8,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV39 = 8,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV48 = 9,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV57 = 10,
	RISCV_IOMMU_DC_FSC_PDTP_MODE_PD8 = 1,
	RISCV_IOMMU_DC_FSC_PDTP_MODE_PD17 = 2,
	RISCV_IOMMU_DC_FSC_PDTP_MODE_PD20 = 3
};

struct riscv_iommu_queue {
	void *base;
	int irq;
	unsigned int cqb;
	unsigned int cqcsr;
};

struct riscv_iommu_priv_data {
	int cmdq_len;
	int fltq_len;
	int priq_len;
	int cmdq_irq;
	int fltq_irq;
	int priq_irq;
	int ddt_mode;
	int pg_mode;
};

struct riscv_iommu_dc {
	unsigned long tc;
	unsigned long iohgatp;
	unsigned long ta;
	unsigned long fsc;
	unsigned long msiptp;
	unsigned long msi_addr_mask;
	unsigned long msi_addr_pattern;
	unsigned long _reserved;
};

struct riscv_iommu_device {
	struct riscv_iommu_dc *dc;
	unsigned pscid;
	int g_stage_enabled;
	int pasid_enabled;
	int mode;
	void *pgdp;
};

struct riscv_iommu {
	unsigned long base;
	int cmdq_len;
	int fltq_len;
	int priq_len;
	int cmdq_irq;
	int fltq_irq;
	int priq_irq;
	struct riscv_iommu_queue cmdq;
	struct riscv_iommu_queue fltq;
	struct riscv_iommu_queue priq;
	int ddt_mode;
	void *ddtp;
	unsigned long cap;
	int pg_mode;
};

#endif
