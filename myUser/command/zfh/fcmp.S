# See LICENSE for license details.

#*****************************************************************************
# fcmp.S
#-----------------------------------------------------------------------------
#
# Test f{eq|lt|le}.h instructions.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .text
.globl fcmp_test
.type fcmp_test, @function

fcmp_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  TEST_FP_CMP_OP_H( 2, feq.h, 0x00, 1, -1.36, -1.36)
  TEST_FP_CMP_OP_H( 3, fle.h, 0x00, 1, -1.36, -1.36)
  TEST_FP_CMP_OP_H( 4, flt.h, 0x00, 0, -1.36, -1.36)

  TEST_FP_CMP_OP_H( 5, feq.h, 0x00, 0, -1.37, -1.36)
  TEST_FP_CMP_OP_H( 6, fle.h, 0x00, 1, -1.37, -1.36)
  TEST_FP_CMP_OP_H( 7, flt.h, 0x00, 1, -1.37, -1.36)

  TEST_PASSFAIL(t6)

.end
