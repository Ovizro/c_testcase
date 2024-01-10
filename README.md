# c_testcase

This is a simple C test case framework that provides a way to organize and run tests. It defines macros for defining test cases, setting up and tearing down the test environment, and skipping tests. The framework uses a custom assert macro for assertion checking within tests.

## Usage

To use this framework, follow these steps:

Include the testcase.h header file in your test source file:

```cpp
#include "testcase.h"
```
Define your test cases using the TEST_CASE macro. Each test case should have a function name that follows the test_case() function type definition:

```cpp
TEST_CASE(test_addition){
    int result = 1 + 2;
    assert(result == 3);
    END_TEST;
}
```

Every test case must end with the END_TEST macro or `return 0`. If a non-zero return value is returned from a test case, the test case will be marked as failed.

If you want to skip a test case, use the SKIP_TEST macro:

```cpp
TEST_CASE(test_addition_skip){
    // ... do something
    SKIP_TEST;
}
```

(Optional) You can define an interactive function for command-line interaction:

```cpp
INTERACTIVE {
    printf("Hello, world!\n");
    return 0;
}
```

> If the command line parameter -i or --interactive is passed in, interactive mode will be entered.

(Optional) Define setup and teardown functions for your test case:
```cpp
SETUP {
    printf("Running setup...\n");
    return 0;
}

TEARDOWN {
    printf("Running teardown...\n");
    return 0;
}
```

> The main function does not need to be defined, it is already defined in c_testcase.cpp. It automatically collects all test cases and executes them.

Compile and run your test executable:

```bash
mkdir -p bin/
g++ test_yourtest.c c_testcase.cpp -lstdc++ -g -Og -Wall -o bin/test_yourtest
```

A shell script is provided for running tests in ./bin/test_*:

```bash
./unittest.sh
```
