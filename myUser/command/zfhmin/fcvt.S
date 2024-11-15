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
.globl min_fcvt_test
.type min_fcvt_test, @function

min_fcvt_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  
  TEST_FCVT_H_S( 10, -1.5, -1.5)
  
  TEST_PASSFAIL(t6)

.end
