# See LICENSE for license details.

#*****************************************************************************
# fcvt.S
#-----------------------------------------------------------------------------
#
# Test fcvt.h.{wu|w|lu|l}, fcvt.h.d, and fcvt.d.h instructions.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .text
.globl fcvt_test
.type fcvt_test, @function

fcvt_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  TEST_INT_FP_OP_H( 2,  fcvt.h.w,                   2.0,  2);
  TEST_INT_FP_OP_H( 3,  fcvt.h.w,                  -2.0, -2);

  TEST_INT_FP_OP_H( 4, fcvt.h.wu,                   2.0,  2);
  TEST_INT_FP_OP_H( 5, fcvt.h.wu,               0h:7c00, -2);

#if __riscv_xlen >= 64
  TEST_INT_FP_OP_H( 6,  fcvt.h.l,                   2.0,  2);
  TEST_INT_FP_OP_H( 7,  fcvt.h.l,                  -2.0, -2);

  TEST_INT_FP_OP_H( 8, fcvt.h.lu,                   2.0,  2);
  TEST_INT_FP_OP_H( 9, fcvt.h.lu,               0h:7c00, -2);
#endif
  
  TEST_FCVT_H_S( 10, -1.5, -1.5)

#if __riscv_xlen >= 64
  TEST_FCVT_H_D( 11, -1.5, -1.5)
#endif
  
  TEST_PASSFAIL(t6)

.end
