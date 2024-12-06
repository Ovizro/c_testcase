#!/usr/bin/python3

import os
import sys
from io import StringIO
from argparse import ArgumentParser
from enum import IntEnum
from glob import glob
from subprocess import run
from time import time
from traceback import print_exception
from typing import List, NoReturn, Optional


__version__ = "0.1.2"


if sys.stdout.isatty():
    TTY_COLOR_RED = "\033[31m"
    TTY_COLOR_RED_BOLD = "\033[1;31m"
    TTY_COLOR_GREEN = "\033[32m"
    TTY_COLOR_GREEN_BOLD = "\033[1;32m"
    TTY_COLOR_YELLOW = "\033[33m"
    TTY_COLOR_YELLOW_BOLD = "\033[1;33m"
    TTY_COLOR_CYAN_BOLD = "\033[1;36m"
    TTY_COLOR_CLEAR = "\033[0m"

    TTY_COLUMNS_SIZE = os.get_terminal_size().columns
else:
    TTY_COLOR_RED = ""
    TTY_COLOR_RED_BOLD = ""
    TTY_COLOR_GREEN = ""
    TTY_COLOR_GREEN_BOLD = ""
    TTY_COLOR_YELLOW = ""
    TTY_COLOR_YELLOW_BOLD = ""
    TTY_COLOR_CYAN_BOLD = ""
    TTY_COLOR_CLEAR = ""

    TTY_COLUMNS_SIZE = 80


def print_separator_ex(lc: str, title: str, color: str) -> None:
    len_str = len(title)
    
    print(f"{TTY_COLOR_CLEAR}{color}", end='')  # 设置颜色样式
    if len_str + 2 > TTY_COLUMNS_SIZE:
        print(title)
    else:
        print(f" {title} ".center(TTY_COLUMNS_SIZE, lc))
    print(TTY_COLOR_CLEAR, end='')  # 重置颜色为默认


class CTestCaseStatus(IntEnum):
    NOT_RUN = -1
    PASSED = 0
    ERROR = 1
    FAILED = 16
    SKIPPED = 32
    SETUP_FAILED = 17
    TEARDOWN_FAILED = 18


class CTestCaseCounter:
    __slots__ = ["total_count", "passed", "error", "failed", "skipped"]
    
    def __init__(self) -> None:
        self.total_count = 0
        self.passed: 'set[CTestCase]' = set()
        self.error: 'set[CTestCase]' = set()
        self.failed: 'set[CTestCase]' = set()
        self.skipped: 'set[CTestCase]' = set()
    
    def update(self, test_case: "CTestCase") -> None:
        self.total_count += 1
        if test_case.status == CTestCaseStatus.PASSED:
            self.passed.add(test_case)
        elif test_case.status == CTestCaseStatus.ERROR:
            self.error.add(test_case)
        elif test_case.status == CTestCaseStatus.SKIPPED:
            self.skipped.add(test_case)
        elif test_case.status in [CTestCaseStatus.FAILED, CTestCaseStatus.SETUP_FAILED, CTestCaseStatus.TEARDOWN_FAILED]:
            self.failed.add(test_case)
        else:
            raise ValueError(f"{test_case.status} is not a valid status for counter")
    
    def clone(self) -> "CTestCaseCounter":
        counter = CTestCaseCounter()
        counter.total_count = self.total_count
        counter.passed = self.passed
        counter.error = self.error
        counter.failed = self.failed
        counter.skipped = self.skipped
        return counter
    
    def __add__(self, other: "CTestCaseCounter") -> "CTestCaseCounter":
        counter = self.clone()
        counter += other
        return counter

    def __iadd__(self, other: "CTestCaseCounter") -> "CTestCaseCounter":
        self.total_count += other.total_count
        self.passed.update(other.passed)
        self.error.update(other.error)
        self.failed.update(other.failed)
        self.skipped.update(other.skipped)
        return self

    @property
    def status(self) -> CTestCaseStatus:
        if self.error:
            return CTestCaseStatus.ERROR
        elif self.failed:
            return CTestCaseStatus.FAILED
        elif self.skipped:
            return CTestCaseStatus.SKIPPED
        elif self.passed:
            return CTestCaseStatus.PASSED
        else:
            return CTestCaseStatus.NOT_RUN
    
    def __repr__(self) -> str:
        return f"<counter total: {self.total_count}, passed: {len(self.passed)}, error: {len(self.error)}, failed: {len(self.failed)}, skipped: {len(self.skipped)}>"

    def __str__(self) -> str:
        ss = StringIO()
        ss.write(f"total: {self.total_count}")
        if self.passed:
            ss.write(f", passed: {len(self.passed)}")
        elif self.skipped:
            ss.write(f", skipped: {len(self.skipped)}")
        elif self.failed:
            ss.write(f", failed: {len(self.failed)}")
        elif self.error:
            ss.write(f", error: {len(self.error)}")
        return ss.getvalue()


class CTestCase:
    __slots__ = ["id", "name", "status", "file", "result", "error_info"]

    def __init__(self, id: int, name: str, file: "CTestCaseFile") -> None:
        self.id = id
        self.name = name
        self.file = file
        self.status = CTestCaseStatus.NOT_RUN
        self.result = None
        self.error_info = None
    
    def run(self, *, verbose: bool = False, capture: bool = True) -> CTestCaseStatus:
        try:
            sys.stdout.flush()
            sys.stderr.flush()
            self.result = run([self.file.path, "--unittest", str(self.id)], capture_output=capture)
        except Exception:
            self.status = CTestCaseStatus.ERROR
            self.error_info = sys.exc_info()
            if not capture:
                print(TTY_COLOR_RED)
                print_exception(*self.error_info)
                print(TTY_COLOR_CLEAR)
        except KeyboardInterrupt:
            self.status = CTestCaseStatus.ERROR
            self.error_info = sys.exc_info()
        else:
            code = self.result.returncode
            if code in CTestCaseStatus.__members__.values():
                self.status = CTestCaseStatus(code)
            else:
                self.status = CTestCaseStatus.ERROR
        if verbose:
            self.print_status_verbose()
        else:
            self.print_status()
        return self.status

    def print_status(self) -> None:
        if self.status == CTestCaseStatus.PASSED:
            print(f"{TTY_COLOR_GREEN}.{TTY_COLOR_CLEAR}", end='')
        elif self.status in [CTestCaseStatus.FAILED, CTestCaseStatus.SETUP_FAILED, CTestCaseStatus.TEARDOWN_FAILED]:
            print(f"{TTY_COLOR_RED}F{TTY_COLOR_CLEAR}", end='')
        elif self.status == CTestCaseStatus.ERROR:
            print(f"{TTY_COLOR_RED}E{TTY_COLOR_CLEAR}", end='')
        elif self.status == CTestCaseStatus.SKIPPED:
            print(f"{TTY_COLOR_YELLOW}s{TTY_COLOR_CLEAR}", end='')
        else:
            raise ValueError(f"invalid test case status: {self.status}")
    
    def print_status_verbose(self) -> None:
        if self.status == CTestCaseStatus.PASSED:
            print(f"{self.name} {TTY_COLOR_GREEN}PASSED{TTY_COLOR_CLEAR}")
        elif self.status == CTestCaseStatus.FAILED:
            print(f"{self.name} {TTY_COLOR_RED}FAILED{TTY_COLOR_CLEAR}")
        elif self.status == CTestCaseStatus.ERROR:
            print(f"{self.name} {TTY_COLOR_RED}ERROR{TTY_COLOR_CLEAR}")
        elif self.status == CTestCaseStatus.SETUP_FAILED:
            print(f"{self.name} {TTY_COLOR_RED}SETUP FAILED{TTY_COLOR_CLEAR}")
        elif self.status == CTestCaseStatus.TEARDOWN_FAILED:
            print(f"{self.name} {TTY_COLOR_RED}TEARDOWN FAILED{TTY_COLOR_CLEAR}")
        elif self.status == CTestCaseStatus.SKIPPED:
            print(f"{self.name} {TTY_COLOR_YELLOW}SKIPPED{TTY_COLOR_CLEAR}")
        else:
            raise ValueError(f"invalid test case status: {self.status}")
    
    def report(self) -> Optional[str]:
        if self.status == CTestCaseStatus.PASSED:
            return
        elif self.status == CTestCaseStatus.SKIPPED:
            return f"{TTY_COLOR_YELLOW}SKIPPED{TTY_COLOR_CLEAR} {self.name}"
        elif self.status == CTestCaseStatus.ERROR:
            if self.error_info:
                print_separator_ex('_', self.name, TTY_COLOR_RED)
                print(TTY_COLOR_RED, end='')
                print_exception(*self.error_info)
                print(TTY_COLOR_CLEAR, end='')
                assert self.error_info[0]
                if str(self.error_info[1]):
                    error = f"{self.error_info[0].__name__}: {self.error_info[1]}"
                else:
                    error = f"{self.error_info[0].__name__}"
                return f"{TTY_COLOR_RED}ERROR{TTY_COLOR_CLEAR} {self.name} - {error}"
            else:
                assert self.result
                if self.result.stderr:
                    print_separator_ex('_', self.name, TTY_COLOR_RED)
                    print(TTY_COLOR_RED, end='')
                    print(self.result.stderr.decode("utf-8"), end='')
                    print(TTY_COLOR_CLEAR, end='')
                if self.result.stdout:
                    print_separator_ex('-', "Captured stdout", '')
                    print(self.result.stdout.decode("utf-8"), end='')
                return f"{TTY_COLOR_RED}ERROR{TTY_COLOR_CLEAR} {self.name} - RuntimeError ({self.result.returncode})"
        elif self.status in [CTestCaseStatus.FAILED, CTestCaseStatus.SETUP_FAILED, CTestCaseStatus.TEARDOWN_FAILED]:
            assert self.result
            if self.result.stderr:
                print_separator_ex('_', self.name, TTY_COLOR_RED)
                print(TTY_COLOR_RED, end='')
                print(self.result.stderr.decode("utf-8"), end='')
                print(TTY_COLOR_CLEAR, end='')
            if self.result.stdout:
                if self.result.stderr:
                    print_separator_ex('-', "Captured stdout", '')
                else:
                    print_separator_ex('_', self.name, '')
                print(self.result.stdout.decode("utf-8", "replace"), end='')
            if self.status == CTestCaseStatus.FAILED:
                return f"{TTY_COLOR_RED}FAILED{TTY_COLOR_CLEAR} {self.name}"
            elif self.status == CTestCaseStatus.SETUP_FAILED:
                return f"{TTY_COLOR_RED}FAILED{TTY_COLOR_CLEAR} {self.name} - SetupError"
            else:
                return f"{TTY_COLOR_RED}FAILED{TTY_COLOR_CLEAR} {self.name} - TeardownError"
        else:
            raise ValueError(f"invalid test case status: {self.status}")


class CTestCaseFile:
    __slots__ = ["path", "test_cases", "collect_result", "collect_error_info", "counter"]

    def __init__(self, path: str) -> None:
        self.path = path
        self.test_cases: List[CTestCase] = []
        self.collect_result = None
        self.collect_error_info = None
    
    def collect(self) -> int:
        try:
            result = run([self.path, "--collect"], capture_output=True)
        except Exception:
            self.collect_error_info = sys.exc_info()
            return 0
        
        if result.returncode != 0:
            self.collect_result = result
            return 0
        for id, name in enumerate(result.stdout.decode("ascii").split()):
            self.test_cases.append(CTestCase(id, name, self))
        return len(self.test_cases)
    
    def run(self, verbose: bool = False, capture: bool = True) -> CTestCaseCounter:
        counter = CTestCaseCounter()
        if not verbose:
            print(self.path, end=' ')
        for i in self.test_cases:
            if verbose:
                print(self.path, end='::')
            i.run(verbose=verbose, capture=capture)
            counter.update(i)
        if not verbose:
            print()
        return counter
    
    def report(self) -> List[str]:
        if self.collect_result is not None:
            print_separator_ex('_', f"ERROR collecting {self.path}", TTY_COLOR_RED_BOLD)
            print(TTY_COLOR_RED, end='')
            print(self.collect_result.stderr.decode())
            print(TTY_COLOR_CLEAR, end='')
            if self.collect_result.stdout:
                print_separator_ex('-', "Captured stdout", '')
                print(self.collect_result.stdout.decode())
            return [f"ERROR {self.path} - CollectError ({self.collect_result.returncode})"]
        elif self.collect_error_info is not None:
            assert self.collect_error_info[0]
            print_separator_ex('_', f"ERROR collecting {self.path}", TTY_COLOR_RED_BOLD)
            print(TTY_COLOR_RED, end='')
            print_exception(*self.collect_error_info)
            print(TTY_COLOR_CLEAR, end='')
            return [f"ERROR{TTY_COLOR_CLEAR} {self.path} - {self.collect_error_info[0].__name__}: {self.collect_error_info[1]}"]
        return list(filter(None, (i.report() for i in self.test_cases)))
    
    @property
    def error(self) -> bool:
        return self.collect_result is not None or self.collect_error_info is not None


def report_collect_error(start_time: float, *error_files: CTestCaseFile) -> NoReturn:
    print_separator_ex('=', "ERRORS", '')
    summary = []
    for i in error_files:
        summary.extend(i.report())

    print_separator_ex('=', "short test summary info", TTY_COLOR_CYAN_BOLD)
    for i in summary:
        print(i)
    print_separator_ex('!', f"Interrupted: {len(error_files)} error during collection", '')
    cur_time = time()
    print_separator_ex('=', f"{len(summary)} error in {cur_time - start_time:.2f}s", TTY_COLOR_RED_BOLD)
    sys.exit(1)


def report_no_ran(start_time: float) -> NoReturn:
    cur_time = time()
    print_separator_ex('=', f"no tests ran in {cur_time - start_time:.2f}s", TTY_COLOR_YELLOW)
    sys.exit()

def report(start_time: float, counter: CTestCaseCounter, *, show_capture: bool = True) -> NoReturn:
    cur_time = time()
    summary = []
    if counter.error:
        if show_capture:
            print_separator_ex('=', "ERRORS", '')
        for i in counter.error:
            summary.append(i.report())
    if counter.failed:
        if show_capture:
            print_separator_ex('=', "FAILURES", TTY_COLOR_RED)
        for i in counter.failed:
            summary.append(i.report())
    if counter.skipped:
        for i in counter.skipped:
            summary.append(i.report())
    
    if summary:
        print_separator_ex('=', "short test summary info", TTY_COLOR_CYAN_BOLD)
        for i in summary:
            print(i)
    
    if counter.status in [CTestCaseStatus.FAILED, CTestCaseStatus.ERROR]:
        color = TTY_COLOR_RED_BOLD
    elif counter.status == CTestCaseStatus.SKIPPED:
        color = TTY_COLOR_YELLOW_BOLD
    else:
        color = TTY_COLOR_GREEN_BOLD
    
    print_separator_ex('=', f"{counter} in {cur_time - start_time:.2f}s", color)
    if counter.status in [CTestCaseStatus.FAILED, CTestCaseStatus.ERROR]:
        sys.exit(1)
    sys.exit()


if __name__ == "__main__":
    parser = ArgumentParser("unittest", description="Run unit tests")
    parser.add_argument("path", nargs='+', help="path to the test directory or file", default="./test_*")
    parser.add_argument("-V", "--version", action="version", version=__version__)
    parser.add_argument("-v", "--verbose", action="store_true", help="verbose output")
    parser.add_argument("-s", "--no-capture", action="store_false", help="capture stdout and stderr")

    namespace = parser.parse_args()

    print_separator_ex("=", "test session starts", "")
    print(f"platform: {sys.platform} -- Python {sys.version.split(' ')[0]}, c_unittest {__version__}")
    print(f"rootdir: {os.getcwd()}")

    files: List[CTestCaseFile] = []
    total = 0
    error_files = []
    start_time = time()

    paths = []
    for p in namespace.path:
        if '*' in p:
            paths.extend(glob(p, recursive=True))
        elif os.path.isfile(p):
            paths.append(p)
    for p in paths:
        f = CTestCaseFile(p)
        total += f.collect()
        if f.error:
            error_files.append(f)
        else:
            files.append(f)
    
    if error_files:
        print(f"collected {total} items / {len(error_files)} error\n")
        report_collect_error(start_time, *error_files)
    else:
        print(f"collected {total} items\n")
    
    if total == 0:
        report_no_ran(start_time)
    
    counter = CTestCaseCounter()
    for f in files:
        counter += f.run(verbose=namespace.verbose, capture=namespace.no_capture)
    
    report(start_time, counter, show_capture=namespace.no_capture)
