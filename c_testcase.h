#ifndef _INCLUDE_C_TESTCASE_
#define _INCLUDE_C_TESTCASE_

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

#define SKIP_RET_NUMBER (*((const int*)"SKIP"))
#define SKIP_TEST do { return SKIP_RET_NUMBER; } while (0)
#define END_TEST do { return 0; } while (0)

#define TEST_CASE(name)                                     \
    int name ();                                            \
    int _tc_tmp_ ## name = _add_test_case(# name, name);    \
    int name ()
#define INTERACTIVE                                                 \
    int interactive_impl(int argc, const char** argv);              \
    int _tc_s_tmp_interactive = _set_interactive(interactive_impl); \
    int interactive_impl(int argc, const char** argv)
#define SETUP                                       \
    int setup_impl(const char* test_case_name);     \
    int _tc_s_tmp_setup = _set_setup(setup_impl);   \
    int setup_impl(const char* test_case_name)
#define TEARDOWN                                            \
    int teardown_impl(const char* test_case_name);          \
    int _tc_s_tmp_teardown = _set_teardown(teardown_impl);  \
    int teardown_impl(const char* test_case_name)

#undef assert
#define assert(expr) do {                           \
    if (!(expr)) {                                  \
        printf("assert failed: " #expr "\n");       \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        return 1;                                   \
    }                                               \
} while (0)

typedef int (*test_case)();
typedef int (*interactive_func)(int argc, const char** argv);
typedef int (*context_func)(const char* name);

int _add_test_case(const char* name, test_case func);
int _set_interactive(interactive_func func);
int _set_setup(context_func func);
int _set_teardown(context_func func);


#ifdef __cplusplus
}
#endif

#endif
