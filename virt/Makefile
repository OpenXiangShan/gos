# Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2 or later, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

obj-$(CONFIG_VIRT) += machine.o
obj-$(CONFIG_VIRT_ENABLE_TIMER) += clint_emulator.o
obj-$(CONFIG_VIRT_ENABLE_AIA) += imsic_emulator.o
obj-$(CONFIG_VIRT) += memory_emulator.o
obj-$(CONFIG_VIRT) += uart_emulator.o
obj-$(CONFIG_VIRT) += memory_test_emulator.o
obj-$(CONFIG_VIRT) += scheduler_emulator.o
obj-$(CONFIG_VIRT_ENABLE_PCI_GENERIC) += pci_generic_emulator.o
obj-$(CONFIG_VIRT) += vcpu_insn.o
obj-$(CONFIG_VIRT_ENABLE_TIMER) += vcpu_timer.o
obj-$(CONFIG_VIRT) += virt_vm_exit.o
obj-$(CONFIG_VIRT) += gstage_pgtable.o
obj-$(CONFIG_VIRT_ENABLE_AIA) += vcpu_aia.o
obj-$(CONFIG_VIRT) += vcpu_sbi.o
obj-$(CONFIG_VIRT) += virt.o
obj-$(CONFIG_VIRT) += cpu_tlb_s.o
obj-$(CONFIG_VIRT) += vcpu_switch_s.o
obj-$(CONFIG_VIRT_DEVICE_PASSTHROUGH) += vcpu_pt_remapping.o
