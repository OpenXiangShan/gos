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
value0:   .float 3.7
.local result
result:   .float 4.0
.local value2
value2:   .double 3.741592614159261
.local result1
result1:   .double 4.0

.section .text
.globl fround_test
.type fround_test, @function

fround_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------
  SAV_REG(t6);

test_1:
  li  gp,  1;
  la  t0,  value0;     // load value0 address to t0
  flw ft0, 0(t0);      // load value0 value to ft0
  la  t2,  result;
  flw ft2, 0(t2);

  fround.s ft1, ft0;        //save data to f1
  feq.s   t1,  ft2, ft1;   // if ft3==ft2 : t1 =1; else: t1=0
  beqz    t1,  fail;       // if t1=0, jump to fail

test_2:
  li  gp,  2;
  la  t0,  value2;
  fld ft0, 0(t0);     
  la  t2,  result1;
  fld ft2, 0(t2);

  fround.d ft1, ft0;
  feq.d   t1,  ft2, ft1;
  beqz    t1,  fail;   

  TEST_PASSFAIL(t6)
 
.end

