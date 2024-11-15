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
value1:   .float 2.5
.local result
result:   .float 3.5
.local value2
value2:   .double 3.141592614159261
.local value3
value3:   .double 2.5
.local result1
result1:   .double 3.141592614159261 

.section .text
.globl fmaxm_test
.type fmaxm_test, @function

fmaxm_test:
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
  la  t2,  result;
  flw ft2, 0(t2);

  fmaxm.s ft3, ft0, ft1;   //save max data to ft3
  feq.s   t1,  ft2, ft3;   // if ft3==ft2 : t1 =1; else: t1=0
  beqz    t1,  fail;       // if t1=0, jump to fail

test_2:
  li  gp,  2;
  la  t0,  value2;
  fld ft0, 0(t0);     
  la  t1,  value3;
  fld ft1, 0(t1);     
  la  t2,  result1;
  fld ft2, 0(t2);

  fmaxm.d ft3, ft0, ft1;
  feq.d   t1,  ft2, ft3;
  beqz    t1,  fail;

  TEST_PASSFAIL(t6)

.end

