#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <sstream>
#include <signal.h>
#include "c_testcase.h"

using namespace std;

#define TEST_CASE_STATUS_PASSED 0
#define TEST_CASE_STATUS_SKIPPED 32
#define TEST_CASE_STATUS_FAILED 16
#define TEST_CASE_STATUS_SETUP_FAILED 17
#define TEST_CASE_STATUS_TEARDOWN_FAILED 18

#define MAX_TESTCASE 64

struct TestCase {
    const char* name;
    test_case func;
};

static TestCase test_cases[MAX_TESTCASE];
static int test_case_total = 0;

static interactive_func interactive = NULL;
static context_func setup = NULL;
static context_func teardown = NULL;

int _add_test_case(const char* name, test_case func) {
    if (test_case_total == MAX_TESTCASE) {
        fprintf(stderr, "too many test case\n");
        exit(1);
    } 
    TestCase* tc = &(test_cases[test_case_total++]);
    tc->name = name;
    tc->func = func;
    return 0;
}

int _set_interactive(interactive_func func) {
    interactive = func;
    return 0;
}

int _set_setup(context_func func) {
    setup = func;
    return 0;
}

int _set_teardown(context_func func) {
    teardown = func;
    return 0;
}

static __inline int get_tty_col(int fd) {
    struct winsize size;
    ioctl(fd, TIOCGWINSZ,&size);
    return size.ws_col;
}

static __inline void print_separator(char lc) {
    int size = get_tty_col(STDOUT_FILENO);
    for (int i = 0; i < size; ++i) {
        putchar(lc);
    }
}

static __inline void print_separator_ex(char lc, const char* str, const char* color) {
    int size = get_tty_col(STDOUT_FILENO);
    int len = strlen(str);
    printf("\033[0m%s", color); // 设置颜色
    if(len > size) {
        printf("%s\n", str);
    } else {
        int pad = (size - len - 2) / 2;
        for(int i = 0; i < pad; i++) {
            putchar(lc);
        }
        printf(" %s ", str);
        for(int i = 0; i < pad; i++) {
            putchar(lc);
        }
        if((size - len) % 2) putchar(lc);
        putchar('\n');
    }
    printf("\033[0m"); // 重置颜色
}

static int collect_testcase() {
    for (int i = 0; i < test_case_total; ++i) {
        printf(test_cases[i].name);
        putchar(' ');
    }
    putchar('\n');
    return 0;
}

static int unittest_testcase(const char* name) {
    TestCase* tc = NULL;
    if (*name >= '0' && *name <= '9') {
        int id = atoi(name);
        if (id >= 0 && id < test_case_total) {
            tc = &(test_cases[id]);
        }
    } else {
        for (int i = 0; i < test_case_total; ++i) {
            if (strcmp(test_cases[i].name, name) == 0) {
                tc = &(test_cases[i]);
                break;
            }
        }
    }
    if (tc == NULL) {
        fprintf(stderr, "test case %s not found\n", name);
        return 1;
    }
    if (setup && setup(tc->name)) {
        return TEST_CASE_STATUS_SETUP_FAILED;
    }
    int ret = tc->func();
    if (teardown && teardown(tc->name)) {
        return TEST_CASE_STATUS_TEARDOWN_FAILED;
    }

    if (ret == SKIP_RET_NUMBER) {
        return TEST_CASE_STATUS_SKIPPED;
    } else if (ret != 0) {
        return TEST_CASE_STATUS_FAILED;
    } else {
        return 0;
    }
}

int main(int argc, const char** argv) {
    // bool verbose = false;
    // bool capture_output = true;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-i") == 0 || strcmp(arg, "--interactive") == 0) {
            if (interactive)
                return interactive(argc, argv);
            else {
                cout << "interactive mode is not supported" << endl;
                return 1;
            }
        }
        else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--collect") == 0) {
            return collect_testcase();
        }
        else if (strcmp(arg, "-u") == 0 || strcmp(arg, "--unittest") == 0) {
            const char* name = NULL;
            if (i + 1 < argc) {
                name = argv[++i];
            } else {
                cout << "--unittest require an argument" << endl;
                return 2;
            }
            return unittest_testcase(name);
        }
        // else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0) {
        //     verbose = true;
        // }
        // else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--no-capture-output") == 0) {
        //     capture_output = false;
        // }
    }

    int total = test_case_total;
    int passed = 0;
    int failed = 0;
    int skipped = 0;

    for (int i = 0; i < total; ++i) {
        print_separator('=');;
        TestCase* tc = &test_cases[i];
        if (setup && setup(tc->name)) {
            cout << "\033[0m\033[1;31msetup \"" << tc->name << "\" failed\033[0m" << endl;
            failed++;
            continue;
        }
        cout << "running " << tc->name << endl;
        int ret = tc->func();
        if (teardown && teardown(tc->name)) {
            cout << "\033[0m\033[1;31mteardown \"" << tc->name << "\" failed\033[0m" << endl;
            failed++;
            continue;
        }
        if (ret == SKIP_RET_NUMBER) {
            cout << "\033[0m\033[1;33mtest case \"" << tc->name << "\" skipped\033[0m" << endl;
            skipped++;
        } else if (ret != 0) {
            cout << "\033[0m\033[1;31mtest case \"" << tc->name << "\" failed\033[0m" << endl;
            failed++;
        } else {
            cout << "\033[0m\033[1;32mtest case \"" << tc->name << "\" passed\033[0m" << endl;
            passed++;
        }
    }
    
    stringstream ss;
    ss << "total: " << total << ", passed: " << passed << ", failed: " << failed << ", skipped: " << skipped;
    string sum = ss.str();

    const char* color;
    if (failed)
        color = "\033[1;31m";
    else if (skipped)
        color = "\033[1;33m";
    else
        color = "\033[1;32m";

    print_separator_ex('=', sum.c_str(), color);
    return 0;
}