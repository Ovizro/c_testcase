#include <signal.h>
#include <setjmp.h>
#include <iostream>
#include <string>
#include <sstream>
#include <atomic>
#include <thread>
#include "c_testcase.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

#define TEST_CASE_STATUS_PASSED 0
#define TEST_CASE_STATUS_SKIPPED 32
#define TEST_CASE_STATUS_FAILED 16
#define TEST_CASE_STATUS_SETUP_FAILED 17
#define TEST_CASE_STATUS_TEARDOWN_FAILED 18

#define MAX_TESTCASE 64

#define DEFAULT_TTY_COL_SIZE 80

struct TestCase {
    const char* name;
    test_case func;
};

static TestCase test_cases[MAX_TESTCASE];
static int test_case_total = 0;

static interactive_func interactive = NULL;
static context_func setup = NULL;
static context_func teardown = NULL;

static atomic_bool testcase_running;
static jmp_buf testcase_env;
static std::thread::id testcase_thread_id;
static int testcase_exit_code = 0;

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

[[noreturn]] void test_case_abort(int exit_code) {
    auto tid = std::this_thread::get_id();
    if (!testcase_running.load(std::memory_order_acquire) || tid != testcase_thread_id) {
        exit(exit_code);
    }
    testcase_exit_code = exit_code;
    longjmp(testcase_env, 1);
}

static inline int get_tty_col(int fd) {
#ifdef _WIN32
    // Windows
    HANDLE hConsole = (HANDLE)_get_osfhandle(fd);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return DEFAULT_TTY_COL_SIZE; // 错误处理
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return DEFAULT_TTY_COL_SIZE; // 错误处理
    }
    return csbi.srWindow.Right - csbi.srWindow.Left + 1; // 计算列数
#else
    // POSIX (Linux, macOS, etc.)
    struct winsize size;
    if (ioctl(fd, TIOCGWINSZ, &size) == -1) {
        return DEFAULT_TTY_COL_SIZE; // 错误处理
    }
    return size.ws_col; // 返回列数
#endif
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
        puts(test_cases[i].name);
    }
    return 0;
}

static TestCase* get_test_case(const char* name) {
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
    return tc;
}

static int run_test_case_func(test_case func) {
    bool running = false;
    if (!testcase_running.compare_exchange_strong(running, true, std::memory_order_acq_rel)) {
        cerr << "test case is running" << endl;
        return 1;
    }
    if (setjmp(testcase_env)) {
        return testcase_exit_code;
    }
    testcase_thread_id = std::this_thread::get_id();
    int ret = func();
    running = true;
    if (!testcase_running.compare_exchange_strong(running, false, std::memory_order_acq_rel)) {
        cerr << "test case is not running" << endl;
        return 1;
    }
    return ret;
}

static int unittest_testcase(TestCase* tc) {
    assert(tc != NULL);
    if (setup && setup(tc->name)) {
        return TEST_CASE_STATUS_SETUP_FAILED;
    }
    int ret = run_test_case_func(tc->func);
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
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            printf("usage: %s [-c] [-u NAME] [-h]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  -i, --interactive: run in interactive mode.\n");
            printf("  -c, --collect: list all test cases.\n");
            printf("  -u, --unittest: run a single test case.\n");
            printf("  -h, --help: show the help text.\n");
            return 0;
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
            TestCase* tc = get_test_case(name);
            if (tc == NULL) {
                cout << "test case " << name << " not found" << endl;
                return 1;
            }
            return unittest_testcase(tc);
        } else {
            if (interactive)
                return interactive(argc, argv);
            else {
                cout << "unknown argument '" << arg << "'" << endl;
                return 1;
            }
        }
    }

    int total = test_case_total;
    int passed = 0;
    int failed = 0;
    int skipped = 0;

    for (int i = 0; i < total; ++i) {
        print_separator('-');
        TestCase* tc = &test_cases[i];
        cout << "running " << tc->name << endl;
        switch (unittest_testcase(tc)) {
            case 0:
                cout << "\033[0m\033[1;32mtest case \"" << tc->name << "\" passed\033[0m" << endl;
                passed++;
            break;
            case TEST_CASE_STATUS_SKIPPED:
                cout << "\033[0m\033[1;33mtest case \"" << tc->name << "\" skipped\033[0m" << endl;
                skipped++;
            break;
            case TEST_CASE_STATUS_SETUP_FAILED:
                cout << "\033[0m\033[1;31msetup \"" << tc->name << "\" failed\033[0m" << endl;
                failed++;
            break;
            case TEST_CASE_STATUS_TEARDOWN_FAILED:
                cout << "\033[0m\033[1;31mteardown \"" << tc->name << "\" failed\033[0m" << endl;
                failed++;
            break;
            default:
                cout << "\033[0m\033[1;31mtest case \"" << tc->name << "\" failed\033[0m" << endl;
                failed++;
            break;
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