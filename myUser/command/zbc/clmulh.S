# See LICENSE for license details.

#*****************************************************************************
# clmulh.S
#-----------------------------------------------------------------------------
#
# Test clmulh instruction.
#

#include "riscv_test.h"
#include "test_macros.h"

.section .text
.globl clmulh_test
.type clmulh_test, @function

clmulh_test:
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  SAV_REG(t6);
  TEST_RR_OP(32,  clmulh, 0x0000000000001200, 0x0000000000007e00, 0x6db6db6db6db6db7 );
  TEST_RR_OP(33,  clmulh, 0x0000000000001240, 0x0000000000007fc0, 0x6db6db6db6db6db7 );

  TEST_RR_OP( 2,  clmulh, 0x00000000, 0x00000000, 0x00000000 );
  TEST_RR_OP( 3,  clmulh, 0x00000000, 0x00000001, 0x00000001 );
  TEST_RR_OP( 4,  clmulh, 0x00000000, 0x00000003, 0x00000007 );

  TEST_RR_OP( 5,  clmulh, 0x0000000000000000, 0x0000000000000000, 0xffffffffffff8000 );
  TEST_RR_OP( 6,  clmulh, 0x0000000000000000, 0xffffffff80000000, 0x00000000 );
  TEST_RR_OP( 7,  clmulh, 0x555555557fffd555, 0xffffffff80000000, 0xffffffffffff8000 );

  TEST_RR_OP(30,  clmulh, 0x00000000000133cd, 0xaaaaaaaaaaaaaaab, 0x000000000002fe7d );
  TEST_RR_OP(31,  clmulh, 0x00000000000133cd, 0x000000000002fe7d, 0xaaaaaaaaaaaaaaab );

  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  TEST_RR_SRC1_EQ_DEST( 8, clmulh, 0, 13, 11 );
  TEST_RR_SRC2_EQ_DEST( 9, clmulh, 0, 14, 11 );
  TEST_RR_SRC12_EQ_DEST( 10, clmulh, 0, 13 );

  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  TEST_RR_DEST_BYPASS( 11, 0, clmulh, 0, 13, 11 );
  TEST_RR_DEST_BYPASS( 12, 1, clmulh, 0, 14, 11 );
  TEST_RR_DEST_BYPASS( 13, 2, clmulh, 0, 15, 11 );

  TEST_RR_SRC12_BYPASS( 14, 0, 0, clmulh, 0, 13, 11 );
  TEST_RR_SRC12_BYPASS( 15, 0, 1, clmulh, 0, 14, 11 );
  TEST_RR_SRC12_BYPASS( 16, 0, 2, clmulh, 0, 15, 11 );
  TEST_RR_SRC12_BYPASS( 17, 1, 0, clmulh, 0, 13, 11 );
  TEST_RR_SRC12_BYPASS( 18, 1, 1, clmulh, 0, 14, 11 );
  TEST_RR_SRC12_BYPASS( 19, 2, 0, clmulh, 0, 15, 11 );

  TEST_RR_SRC21_BYPASS( 20, 0, 0, clmulh, 0, 13, 11 );
  TEST_RR_SRC21_BYPASS( 21, 0, 1, clmulh, 0, 14, 11 );
  TEST_RR_SRC21_BYPASS( 22, 0, 2, clmulh, 0, 15, 11 );
  TEST_RR_SRC21_BYPASS( 23, 1, 0, clmulh, 0, 13, 11 );
  TEST_RR_SRC21_BYPASS( 24, 1, 1, clmulh, 0, 14, 11 );
  TEST_RR_SRC21_BYPASS( 25, 2, 0, clmulh, 0, 15, 11 );

  TEST_RR_ZEROSRC1( 26, clmulh, 0, 31 );
  TEST_RR_ZEROSRC2( 27, clmulh, 0, 32 );
  TEST_RR_ZEROSRC12( 28, clmulh, 0 );
  TEST_RR_ZERODEST( 29, clmulh, 33, 34 );

  TEST_PASSFAIL(t6)

.end
