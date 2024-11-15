# See LICENSE for license details.

#*****************************************************************************
# fdiv.S
#-----------------------------------------------------------------------------
#
# Test f{div|sqrt}.h instructions.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .text
.globl fdiv_test
.type fdiv_test, @function

fdiv_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  li a0, 0
  li a1, 0
  li a2, 0
  li a3, 0
  fsgnj.s ft0, ft0, ft0
  fsgnj.s ft1, ft1, ft1
  fsgnj.s ft2, ft2, ft2
  fsgnj.s ft3, ft3, ft3

  TEST_FP_OP2_H(112,  fdiv.h, 17, 1.1557273520668288, 3.14159265, 2.71828182 );
  TEST_FP_OP2_H(113,  fdiv.h, 1,-0.9991093838555584,      -1234,     1235.1 );
  TEST_FP_OP2_H(114,  fdiv.h, 0,         3.14159265, 3.14159265,        1.0 );

  TEST_FP_OP1_H(115,  fsqrt.h, 1, 1.7724538498928541, 3.14159265 );
  TEST_FP_OP1_H(116,  fsqrt.h, 0,                100,      10000 );

  TEST_FP_OP1_H_DWORD_RESULT(117,  fsqrt.h, 0x10, 0x00007e00, -1.0 );

  TEST_FP_OP1_H(118,  fsqrt.h, 1, 13.076696, 171.0);


  TEST_PASSFAIL(t6)

.end
