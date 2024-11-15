# See LICENSE for license details.

#*****************************************************************************
# sh1add.S
#-----------------------------------------------------------------------------
#
# Test sh1add instruction.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .data
.local result0
 result0: .float 0.3125
.local result1
 result1: .double 0.3125

.section .text
.globl fli_test
.type fli_test, @function

fli_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------
  SAV_REG(t6);

test_1:
  li gp, 1;
  la t0, result0;       // load result address to t0
  flw ft1, 0(t0);      // load result value to ft1
  fli.s ft0, 0.3125;   // load 0.3124 to ft0
  feq.s t1, ft0, ft1;  // if ft0==ft1 : t1 =1; else: t1=0
  beqz t1, fail;       // if t1=0, jump to fail

test_2:
  li gp, 2;
  la t0, result1;
  fld ft1, 0(t0);
  fli.d ft0, 0.3125;
  feq.d t1, ft0, ft1;
  beqz t1, fail;

  TEST_PASSFAIL(t6)

.end


