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
.local value
value:   .double 3.741592614159261
.local result
result:   .word 3

.section .text
.globl fcvtmod_test
.type fcvtmod_test, @function

fcvtmod_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------
  SAV_REG(t6);

test_1:
  li  gp,  1;
  la  t0,  value;     // load value0 address to t0
  fld ft0, 0(t0);      // load value0 value to ft0
  la  t1,  result;
  lw  t2,  0(t1); 

  fcvtmod.w.d   t1, ft0, rtz;        //save data to f1
  bne          t1, t2, fail;       // if t1=0, jump to fail

  TEST_PASSFAIL(t6)

.end


