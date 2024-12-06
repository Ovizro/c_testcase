#ifndef _INCLUDE_C_TESTCASE_
#define _INCLUDE_C_TESTCASE_

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
#include <iostream>

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
#ifdef __cplusplus
#define assert(expr) do {                           \
    if (!(static_cast <bool> (expr))) {             \
        ::std::cout << "assert failed: " #expr "\n" << ::std::endl;    \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_eq(expr1, expr2) do {                \
    if ((expr1) != (expr2)) {                       \
        ::std::cout << "assert failed: " #expr1 " == " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_ne(expr1, expr2) do {                \
    if ((expr1) == (expr2)) {                       \
        ::std::cout << "assert failed: " #expr1 " != " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_gt(expr1, expr2) do {                \
    if ((expr1) <= (expr2)) {                       \
        ::std::cout << "assert failed: " #expr1 " > " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_ls(expr1, expr2) do {                \
    if ((expr1) >= (expr2)) {                       \
        ::std::cout << "assert failed: " #expr1 " < " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_ge(expr1, expr2) do {                \
    if ((expr1) < (expr2)) {                        \
        ::std::cout << "assert failed: " #expr1 " >= " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_le(expr1, expr2) do {                \
    if ((expr1) > (expr2)) {                        \
        ::std::cout << "assert failed: " #expr1 " <= " #expr2 "\n" << ::std::endl;    \
        ::std::cout << "\t#0: " << (expr1) << ::std::endl;   \
        ::std::cout << "\t#1: " << (expr2) << ::std::endl;   \
        ::std::cout << "file: \"" __FILE__ "\", line " << __LINE__ << ", in " << __ASSERT_FUNCTION << "\n" << ::std::endl;\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#else
#define assert(expr) do {                           \
    if (!((bool)(expr))) {                          \
        printf("assert failed: " #expr "\n");       \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#endif
#define assert_i32_eq(expr1, expr2) do {            \
    if ((int32_t)(expr1) != (uint32_t)(expr2)) {                       \
        printf("assert failed: " #expr1 " == " #expr2 "\n");    \
        printf("\t\t%d\t!=\t%d\n", (int32_t)(expr1), (int32_t)(expr2)); \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_i32_ne(expr1, expr2) do {                \
    if ((int32_t)(expr1) == (int32_t)(expr2)) {                       \
        printf("assert failed: " #expr1 " != " #expr2 "\n");    \
        printf("\t\t%d\t==\t%d\n", (int32_t)(expr1), (int32_t)(expr2));     \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_i64_eq(expr1, expr2) do {            \
    if ((expr1) != (expr2)) {                       \
        printf("assert failed: " #expr1 " == " #expr2 "\n");    \
        printf("\t\t%lld\t!=\t%lld\n", (int64_t)(expr1), (int64_t)(expr2)); \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_i64_ne(expr1, expr2) do {            \
    if ((expr1) == (expr2)) {                       \
        printf("assert failed: " #expr1 " != " #expr2 "\n");    \
        printf("\t\t%lld\t==\t%lld\n", (int64_t)(expr1), (int64_t)(expr2)); \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_str_eq(expr1, expr2) do {            \
    if (strcmp((expr1), (expr2)) != 0) {            \
        printf("assert failed: " #expr1 " == " #expr2 "\n");    \
        printf("\t#0: %s\n", (expr1));              \
        printf("\t#1: %s\n", (expr2));              \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_str_ne(expr1, expr2) do {            \
    if (strcmp((expr1), (expr2)) == 0) {            \
        printf("assert failed: " #expr1 " != " #expr2 "\n");    \
        printf("\t#0: %s\n", (expr1));              \
        printf("\t#1: %s\n", (expr2));              \
        printf("file: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                   \
    }                                               \
} while (0)
#define assert_mem_eq(expr1, expr2, size) do {                      \
    if (memcmp((expr1), (expr2), (size)) != 0) {                    \
        printf("assertion failed: %s == %s\n", #expr1, #expr2);     \
        printf("\t#0: ");                                           \
        for (size_t i = 0; i < (size); i++) {                          \
            printf("%02X", ((uint8_t*)(expr1))[i]);                 \
        }                                                           \
        printf("\n\t#1: ");                                         \
        for (size_t i = 0; i < (size); i++) {                          \
            printf("%02X", ((uint8_t*)(expr2))[i]);                 \
        }                                                           \
        printf("\nfile: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                         \
    }                                                               \
} while (0)
#define assert_mem_ne(expr1, expr2, size) do {                      \
    if (memcmp((expr1), (expr2), (size)) == 0) {                    \
        printf("assertion failed: %s != %s\n", #expr1, #expr2);     \
        printf("\t#0: ");                                           \
        for (int i = 0; i < (size); i++) {                          \
            printf("%02X", ((uint8_t*)(expr1))[i]);                 \
        }                                                           \
        printf("\n\t#1: ");                                         \
        for (int i = 0; i < (size); i++) {                          \
            printf("%02X", ((uint8_t*)(expr2))[i]);                 \
        }                                                           \
        printf("\nfile: \"%s\", line %d, in %s\n", __FILE__, __LINE__, __ASSERT_FUNCTION);\
        test_case_abort(1);                                         \
    }                                                               \
} while (0)


typedef int (*test_case)();
typedef int (*interactive_func)(int argc, const char** argv);
typedef int (*context_func)(const char* name);

int _add_test_case(const char* name, test_case func);
int _set_interactive(interactive_func func);
int _set_setup(context_func func);
int _set_teardown(context_func func);
[[noreturn]] void test_case_abort(int exit_code);

#ifdef __cplusplus
}
#endif

#endif
