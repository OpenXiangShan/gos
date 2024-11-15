# See LICENSE for license details.

#*****************************************************************************
# zacas.S
#-----------------------------------------------------------------------------
#
# Test zacas.h instructions.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .text
.globl zacas_test
.type zacas_test, @function

zacas_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  #------------------------------------------------------------- 
  li a2, 0x11
  li a0, 0x33
  la a1, w_value0

  amocas.w.aqrl a2,a0,(a1)
  #             rd, rs2, rs1 
  lw a3,0(a1)
  bne a3, a0, fail
 
  #------------------------------------------------------------- 
  li a2, 0x12
  li a0, 0x33
  la a1, w_value1

  amocas.w.aqrl a2,a0,(a1)
  #             rd, rs2, rs1 
  lw a3,0(a1)
  beq a3, a0, fail
 
  #------------------------------------------------------------- 
  li t0, 0x12345678
  li a2, 0
  or a2, a2, t0
  li t1, 0x12345678
  slli t1,t1,32
  or a2, a2, t1

  li t0, 0x66666666
  li a0, 0
  or a0, a0, t0
  li t1, 0x66666666
  slli t1,t1,32
  or a0, a0, t1

  la a1, d_value0

  amocas.d.aqrl a2,a0,(a1)
  #              rd rs2   rs1 

  ld a3,0(a1)
  bne a3, a0, fail
  #------------------------------------------------------------- 
  li t0, 0x12345670
  li a2, 0
  or a2, a2, t0
  li t1, 0x12345678
  slli t1,t1,32
  or a2, a2, t1

  li t0, 0x66666666
  li a0, 0
  or a0, a0, t0
  li t1, 0x66666666
  slli t1,t1,32
  or a0, a0, t1

  la a1, d_value1

  amocas.d.aqrl a2,a0,(a1)
  #              rd rs2   rs1 

  ld a3,0(a1)
  beq a3, a0, fail
  
  j pass

  TEST_PASSFAIL(t6)

.align 8
w_value0:
.word  0x11
.align 8
w_value1:
.word  0x11

.align 8
d_value0:
.dword  0x1234567812345678
.align 8
d_value1:
.dword  0x1234567812345678

.end


