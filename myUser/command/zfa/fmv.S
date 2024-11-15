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
value:   .double 3.7
.local result
result:   .dword 0x400921FB54442D18

.section .text
.globl fmv_test
.type fmv_test, @function

fmv_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------
  SAV_REG(t6);

test_1:
  li  gp,  1;
  la  t0,  value;     // load value0 address to t0
  fld ft0, 0(t0);      // load value0 value to ft0
  la  t1,  result;
  ld  t2,  0(t1);

  fmv.x.d    t1, ft0;        //save data to f1
  bne        t1, t2, fail;       // if t1=0, jump to fail

  TEST_PASSFAIL(t6)
 
.end


