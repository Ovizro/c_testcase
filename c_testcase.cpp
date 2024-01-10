#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <sstream>
#include "c_testcase.h"

using namespace std;

struct TestCase {
    const char* name;
    test_case func;
};

#define MAX_TESTCASE 64

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
    int size = get_tty_col(STDIN_FILENO);
    for (int i = 0; i < size; ++i) {
        putchar(lc);
    }
}

void print_separator_ex(char lc, const char* str, const char* color) {
    int size = get_tty_col(STDIN_FILENO);
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

int main(int argc, const char** argv) {
    if (argc > 1 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0)) {
        if (interactive)
            return interactive(argc, argv);
        else {
            cout << "interactive mode is not supported" << endl;
            return 1;
        }
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
