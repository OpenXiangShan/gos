#ifndef __IOMMU_LIB_H__
#define __IOMMU_LIB_H__

enum ERROR_CODE {
	ERROR_NONE = 0,
	ERROR_INVALID_PARAM,
	ERROR_OOM,
};

enum riscv_iommu_dc_fsc_atp_modes {
	RISCV_IOMMU_DC_FSC_MODE_BARE = 0,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV32 = 8,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV39 = 8,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV48 = 9,
	RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV57 = 10,
};

enum riscv_iommu_ddtp_modes {
	DDTP_MODE_OFF,
	DDTP_MODE_BARE,
	DDTP_MODE_1LEVEL,
	DDTP_MODE_2LEVEL,
	DDTP_MODE_3LEVEL
};

enum riscv_iommu_pdtp_modes {
	PDTP_MODE_BARE,
	PDTP_MODE_1LEVEL,
	PDTP_MODE_2LEVEL,
	PDTP_MODE_3LEVEL
};

struct rv_iommu_lib_ops {
	int (*print)(const char *fmt, ...);
};

struct rv_iommu_pgtable_setup_struct {
	unsigned long cap;
	int sv_mode;
	int ddt_mode;
	int pdt_mode;
	int fstage_en;
	int gstage_en;
	int pasid_en;
	int dpe_en;
	int devid;
	int pasid;
	int gscid;
	int pscid;
};

int rv_iommu_lib_init(unsigned long base, unsigned int len,
		      struct rv_iommu_lib_ops *ops);
struct rv_iommu *rv_iommu_pgtable_setup_init(struct
					     rv_iommu_pgtable_setup_struct
					     *info, int *err_code);
void *rv_iommu_pgtable_setup(struct rv_iommu *iommu, int *err_code);
int rv_iommu_page_mapping(struct rv_iommu *iommu,
			  unsigned long iova,
			  unsigned long gpa,
			  unsigned long hpa, unsigned int size);
void rv_iommu_show_pte_addr(struct rv_iommu *iommu);

#endif
