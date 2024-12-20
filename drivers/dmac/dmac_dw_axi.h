/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DMAC_DW_AXI_H
#define DMAC_DW_AXI_H

#include "dmac.h"

#define DMAC_AXI_COMMON_REG     0x0
#define DMAC_AXI_CH1_REG        0x100
#define DMAC_AXI_CH2_REG        0x200

#define DMAC_AXI_ID                     0x0
#define DMAC_AXI_COMP_VER               0x08
#define DMAC_AXI_CFG                    0x10
#define DMAC_AXI_CH_EN                  0x18
#define DMAC_AXI_INTR_STATUS            0x30
#define DMAC_AXI_COMMON_INTR_CLEAR      0x38
#define DMAC_AXI_COMMON_STATUS_ENABLE   0x40
#define DMAC_AXI_COMMON_SIGNAL_ENABLE   0x48
#define DMAC_AXI_COMMON_INTR_STATUS     0x50
#define DMAC_AXI_RST_REG                0x58

#define DMAC_AXI_CH_SAR                 0x0
#define DMAC_AXI_CH_DAR                 0x08
#define DMAC_AXI_CH_BLOCK_TS            0x10
#define DMAC_AXI_CH_CTL                 0x18
#define DMAC_AXI_CH_CTL_HI              0x1c
#define DMAC_AXI_CH_CFG                 0x20
#define DMAC_AXI_CH_LLP                 0x28
#define DMAC_AXI_CH_STATUS              0x30
#define DMAC_AXI_CH_SWHSSRC_REQ         0x38
#define DMAC_AXI_CH_SWHSDST_REQ         0x40
#define DMAC_AXI_CH_BLK_TFR_RESUME_REQ  0x48
#define DMAC_AXI_CH_AXI_ID              0x50
#define DMAC_AXI_CH_AXI_QOS             0x58
#define DMAC_AXI_CH_SSTAT               0x60
#define DMAC_AXI_CH_DSTAT               0x68
#define DMAC_AXI_CH_STATAR              0x70
#define DMAC_AXI_CH_DSTATAR             0x78
#define DMAC_AXI_CH_INTR_STATUS_ENABLE  0x80
#define DMAC_AXI_CH_INTR_STATUS         0x88
#define DMAC_AXI_CH_INTR_SIGNAL_ENABLE  0x90
#define DMAC_AXI_CH_INTR_CLEAR          0x98

#define DMAC_AXI0_COMMON_ID                      DMAC_AXI_COMMON_REG + DMAC_AXI_ID
#define DMAC_AXI0_COMMON_COMP_VER                DMAC_AXI_COMMON_REG + DMAC_AXI_COMP_VER
#define DMAC_AXI0_COMMON_CFG                     DMAC_AXI_COMMON_REG + DMAC_AXI_CFG
#define DMAC_AXI0_COMMON_CH_EN                   DMAC_AXI_COMMON_REG + DMAC_AXI_CH_EN
#define DMAC_AXI0_COMMON_INTR_STATUS             DMAC_AXI_COMMON_REG + DMAC_AXI_INTR_STATUS
#define DMAC_AXI0_COMMON_COMMON_INTR_CLEAR       DMAC_AXI_COMMON_REG + DMAC_AXI_COMMON_INTR_CLEAR
#define DMAC_AXI0_COMMON_COMMON_STATUS_ENABLE    DMAC_AXI_COMMON_REG + DMAC_AXI_COMMON_STATUS_ENABLE
#define DMAC_AXI0_COMMON_COMMON_SIGNAL_ENABLE    DMAC_AXI_COMMON_REG + DMAC_AXI_COMMON_SIGNAL_ENABLE
#define DMAC_AXI0_COMMON_COMMON_INTR_STATUS      DMAC_AXI_COMMON_REG + DMAC_AXI_COMMON_INTR_STATUS
#define DMAC_AXI0_COMMON_RST_REG                 DMAC_AXI_COMMON_REG + DMAC_AXI_RST_REG

#define DMAC_AXI0_CH1_SAR                        DMAC_AXI_CH1_REG + DMAC_AXI_CH_SAR
#define DMAC_AXI0_CH1_DAR                        DMAC_AXI_CH1_REG + DMAC_AXI_CH_DAR
#define DMAC_AXI0_CH1_BLOCK_TS                   DMAC_AXI_CH1_REG + DMAC_AXI_CH_BLOCK_TS
#define DMAC_AXI0_CH1_CTL                        DMAC_AXI_CH1_REG + DMAC_AXI_CH_CTL
#define DMAC_AXI0_CH1_CTL_HI                     DMAC_AXI_CH1_REG + DMAC_AXI_CH_CTL_HI
#define DMAC_AXI0_CH1_CFG                        DMAC_AXI_CH1_REG + DMAC_AXI_CH_CFG
#define DMAC_AXI0_CH1_LLP                        DMAC_AXI_CH1_REG + DMAC_AXI_CH_LLP
#define DMAC_AXI0_CH1_STATUS                     DMAC_AXI_CH1_REG + DMAC_AXI_CH_STATUS
#define DMAC_AXI0_CH1_SWHSSRC_REQ                DMAC_AXI_CH1_REG + DMAC_AXI_CH_SWHSSRC_REQ
#define DMAC_AXI0_CH1_SWHSDST_REQ                DMAC_AXI_CH1_REG + DMAC_AXI_CH_SWHSDST_REQ
#define DMAC_AXI0_CH1_BLK_TFR_RESUME_REQ         DMAC_AXI_CH1_REG + DMAC_AXI_CH_BLK_TFR_RESUME_REQ
#define DMAC_AXI0_CH1_AXI_ID                     DMAC_AXI_CH1_REG + DMAC_AXI_CH_AXI_ID
#define DMAC_AXI0_CH1_AXI_QOS                    DMAC_AXI_CH1_REG + DMAC_AXI_CH_AXI_QOS
#define DMAC_AXI0_CH1_SSTAT                      DMAC_AXI_CH1_REG + DMAC_AXI_CH_SSTAT
#define DMAC_AXI0_CH1_DSTAT                      DMAC_AXI_CH1_REG + DMAC_AXI_CH_DSTAT
#define DMAC_AXI0_CH1_STATAR                     DMAC_AXI_CH1_REG + DMAC_AXI_CH_STATAR
#define DMAC_AXI0_CH1_DSTATAR                    DMAC_AXI_CH1_REG + DMAC_AXI_CH_DSTATAR
#define DMAC_AXI0_CH1_INTR_STATUS_ENABLE         DMAC_AXI_CH1_REG + DMAC_AXI_CH_INTR_STATUS_ENABLE
#define DMAC_AXI0_CH1_INTR_STATUS                DMAC_AXI_CH1_REG + DMAC_AXI_CH_INTR_STATUS
#define DMAC_AXI0_CH1_INTR_SIGNAL_ENABLE         DMAC_AXI_CH1_REG + DMAC_AXI_CH_INTR_SIGNAL_ENABLE
#define DMAC_AXI0_CH1_INTR_CLEAR                 DMAC_AXI_CH1_REG + DMAC_AXI_CH_INTR_CLEAR

#define DMAC_AXI0_CH2_SAR                        DMAC_AXI_CH2_REG + DMAC_AXI_CH_SAR
#define DMAC_AXI0_CH2_DAR                        DMAC_AXI_CH2_REG + DMAC_AXI_CH_DAR
#define DMAC_AXI0_CH2_BLOCK_TS                   DMAC_AXI_CH2_REG + DMAC_AXI_CH_BLOCK_TS
#define DMAC_AXI0_CH2_CTL                        DMAC_AXI_CH2_REG + DMAC_AXI_CH_CTL
#define DMAC_AXI0_CH2_CFG                        DMAC_AXI_CH2_REG + DMAC_AXI_CH_CFG
#define DMAC_AXI0_CH2_LLP                        DMAC_AXI_CH2_REG + DMAC_AXI_CH_LLP
#define DMAC_AXI0_CH2_STATUS                     DMAC_AXI_CH2_REG + DMAC_AXI_CH_STATUS
#define DMAC_AXI0_CH2_SWHSSRC_REQ                DMAC_AXI_CH2_REG + DMAC_AXI_CH_SWHSSRC_REQ
#define DMAC_AXI0_CH2_SWHSDST_REQ                DMAC_AXI_CH2_REG + DMAC_AXI_CH_SWHSDST_REQ
#define DMAC_AXI0_CH2_BLK_TFR_RESUME_REQ         DMAC_AXI_CH2_REG + DMAC_AXI_CH_BLK_TFR_RESUME_REQ
#define DMAC_AXI0_CH2_AXI_ID                     DMAC_AXI_CH2_REG + DMAC_AXI_CH_AXI_ID
#define DMAC_AXI0_CH2_AXI_QOS                    DMAC_AXI_CH2_REG + DMAC_AXI_CH_AXI_QOS
#define DMAC_AXI0_CH2_SSTAT                      DMAC_AXI_CH2_REG + DMAC_AXI_CH_SSTAT
#define DMAC_AXI0_CH2_DSTAT                      DMAC_AXI_CH2_REG + DMAC_AXI_CH_DSTAT
#define DMAC_AXI0_CH2_STATAR                     DMAC_AXI_CH2_REG + DMAC_AXI_CH_STATAR
#define DMAC_AXI0_CH2_DSTATAR                    DMAC_AXI_CH2_REG + DMAC_AXI_CH_DSTATAR
#define DMAC_AXI0_CH2_INTR_STATUS_ENABLE         DMAC_AXI_CH2_REG + DMAC_AXI_CH_INTR_STATUS_ENABLE
#define DMAC_AXI0_CH2_INTR_STATUS                DMAC_AXI_CH2_REG + DMAC_AXI_CH_INTR_STATUS
#define DMAC_AXI0_CH2_INTR_SIGNAL_ENABLE         DMAC_AXI_CH2_REG + DMAC_AXI_CH_INTR_SIGNAL_ENABLE
#define DMAC_AXI0_CH2_INTR_CLEAR                 DMAC_AXI_CH2_REG + DMAC_AXI_CH_INTR_CLEAR

struct dw_dmac_priv_info {
	unsigned int dma_width;
        unsigned int src_addr_inc;
        unsigned int des_addr_inc;
        unsigned int src_width;
        unsigned int des_width;
        unsigned int src_burstsize;
        unsigned int des_burstsize;
        unsigned int burst_len;
};

struct dmac_dw_axi {
	struct dmac_device dmac;
	unsigned long base;
	int done;
};

#endif
