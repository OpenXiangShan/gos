#ifndef __RESULT_H_
#define __RESULT_H_

#include "print.h"

#define TEST_PASS note_test_result(__FUNCTION__, __LINE__, 0)
#define TEST_FAIL note_test_result(__FUNCTION__, __LINE__, 1)

static __inline__ int note_test_result(const char *func, const int line,
                                        int result) {

  print("f:%s l:%d TEST %s\n", func, line, result == 0 ? "PASS" : "FAIL");
  asm("beqz %0, pass\n"
      "j fail\n"

      "pass:\n"
      "    li a0, 0\n"
      "    .word 0x5006b\n"
      " j finish\n"
      "fail:\n"
      "    li a0, 1\n"
      "    .word 0x5006b\n"
      "finish:\n"
      :
      : "r"(result));

	return result;
}
#endif
