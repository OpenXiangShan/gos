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

#ifndef __RISCV_IOMMU_DATA_H
#define __RISCV_IOMMU_DATA_H

struct riscv_iommu_data {
	int cmdq_len;
	int fltq_len;
	int priq_len;
	int cmdq_irq;
	int fltq_irq;
	int priq_irq;
	int ddt_mode;
	int pg_mode;
};

#endif
