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
.local value0
value0:   .float 3.5
.local value1
value1:   .float 4.2
.local value2
value2:   .double 3.141592614159261
.local value3
value3:   .double 4.5

.section .text
.globl fleq_test
.type fleq_test, @function

fleq_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------
  SAV_REG(t6);

test_1:
  li  gp,  1;
  la  t0,  value0;     // load value0 address to t0
  flw ft0, 0(t0);      // load value0 value to ft0
  la  t1,  value1;     // load value1 address to t0
  flw ft1, 0(t1);      // load value1 value to ft0

  fleq.s  t2,  ft0, ft1; 
  beqz    t2,  fail;       // if t1=0, jump to fail

test_2:
  li  gp,  2;
  la  t0,  value2; 
  fld ft0, 0(t0);
  la  t1,  value3;
  fld ft1, 0(t1);

  fleq.d  t2,  ft0, ft1;   
  beqz    t2,  fail;       


  TEST_PASSFAIL(t6)

.end


