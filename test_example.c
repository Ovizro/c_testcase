#include <stdio.h>
#include "c_testcase.h"

SETUP {
    printf("Running setup...\n");
    return 0;
}

TEARDOWN {
    printf("Running teardown...\n");
    return 0;
}

TEST_CASE(test1) {
    printf("Hello world!\n");
    END_TEST;
}

TEST_CASE(test2) {
    assert(1 + 1 == 2);
    SKIP_TEST;
}
