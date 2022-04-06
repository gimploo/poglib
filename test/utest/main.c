#include "../../utest.h"


UTEST_BEGIN(test01, "2")
    const char *i = "2";
    UTEST_SET_OUTPUT(i);
UTEST_END

UTEST_BEGIN(test02, "2")
    const char *i = "1";
    UTEST_SET_OUTPUT(i);
UTEST_END

