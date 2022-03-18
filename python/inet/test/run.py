import glob
import logging

from inet.common import *
from inet.simulation.project import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

def _run_test(test_run, output_stream=sys.stdout, **kwargs):
    test_result = test_run.run(output_stream=output_stream, **kwargs)
    test_result.print_result(complete_error_message=False, output_stream=output_stream)
    return test_result

class TestRun:
    def __init__(self, **kwargs):
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        self.cancel = cancel

#    def print_run(self, index, count):
#        print(("[" + str(index + 1) + "/" + str(count) + "] " if index is not None and count is not None else "") + "Running " + self.get_parameters_string(**kwargs), end=print_end, file=output_stream)

class MultipleTestRuns:
    def __init__(self, test_runs, concurrent=True, **kwargs):
        self.test_runs = test_runs
        self.concurrent = concurrent

    def __repr__(self):
        return repr(self)

    def run(self, concurrent=None, build=True, **kwargs):
        if concurrent is None:
            concurrent = self.concurrent
        logger.info("Running tests " + str(kwargs))
        start_time = time.time()
        test_results = map_sequentially_or_concurrently(self.test_runs, _run_test, concurrent=concurrent, **kwargs)
        end_time = time.time()
        return MultipleTestResults(self, test_results, elapsed_wall_time=end_time - start_time)

class AssertionResult:
    def __init__(self):
        pass

class TestResult:
    def __init__(self, test_run, result=None, bool_result=None, expected_result="PASS", reason=None, **kwargs):
        self.test_run = test_run
        self.result = result or ("PASS" if bool_result else "FAIL")
        self.expected_result = expected_result
        self.expected = expected_result == self.result
        self.reason = reason
        self.color = get_result_color(self.result)

    def __repr__(self):
        return "Test result: " + self.get_description()

    def get_description(self, complete_error_message=True, include_simulation_parameters=False):
        return (self.test_run.get_parameters_string() + " " if include_simulation_parameters else "") + \
               self.color + self.result + COLOR_RESET + \
               ((COLOR_YELLOW + " (unexpected)" + COLOR_RESET) if not self.expected and self.expected_result != "PASS" else "") + \
               ((COLOR_GREEN + " (expected)" + COLOR_RESET) if self.expected and self.expected_result != "PASS" else "") + \
               (" (" + self.reason + ")" if self.reason else "")

    def print_result(self, complete_error_message=True, output_stream=sys.stdout):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

    def rerun(self, **kwargs):
        return self.test_run.run(**kwargs)

class MultipleTestResults:
    def __init__(self, multiple_test_runs, test_results, elapsed_wall_time=None, **kwargs):
        self.multiple_test_runs = multiple_test_runs
        self.test_results = test_results
        self.elapsed_wall_time = elapsed_wall_time
        self.num_different_results = 0
        self.num_pass_expected = self.count_results("PASS", True)
        self.num_pass_unexpected = self.count_results("PASS", False)
        self.num_skip_expected = self.count_results("SKIP", True)
        self.num_skip_unexpected = self.count_results("SKIP", False)
        self.num_cancel_expected = self.count_results("CANCEL", True)
        self.num_cancel_unexpected = self.count_results("CANCEL", False)
        self.num_fail_expected = self.count_results("FAIL", True)
        self.num_fail_unexpected = self.count_results("FAIL", False)
        self.num_error_expected = self.count_results("ERROR", True)
        self.num_error_unexpected = self.count_results("ERROR", False)
        self.total_result = ("ERROR" if self.num_error_unexpected != 0 else "FAIL") if self.num_fail_unexpected != 0 else ("CANCEL" if self.num_cancel_unexpected != 0 else "PASS")
        self.total_color = get_result_color(self.total_result)

    def __repr__(self):
        if len(self.test_results) == 1:
            return self.test_results[0].__repr__()
        else:
            details = self.get_details(exclude_test_result_filter="SKIP|CANCEL", exclude_expected_test_result=True, include_simulation_parameters=True)
            return ("" if details.strip() == "" else "\nDetails:\n" +details + "\n\n") + \
                   "Multiple test results: " + self.total_color + self.total_result + COLOR_RESET + ", " + \
                   "summary: " + self.get_summary()

    def print_result(self, output_stream=sys.stdout, **kwargs):
        print(self.get_summary(), file=output_stream)
        for test_result in self.test_results:
            if test_result.result != "PASS":
                print("  ", end="", file=output_stream)
                test_result.print_result(output_stream=output_stream, **kwargs)

    def is_all_pass(self):
        return self.num_pass_expected == len(self.test_results)

    def count_results(self, result, expected):
        num = sum(e.result == result and e.expected == expected for e in self.test_results)
        if num != 0:
            self.num_different_results += 1
        return num

    def get_result_class_texts(self, result, color, num_expected, num_unexpected):
        texts = []
        if num_expected != 0:
            texts.append(color + str(num_expected) + " " + result + (COLOR_GREEN + " (expected)" + COLOR_RESET if result != "PASS" else "") + COLOR_RESET)
        if num_unexpected != 0:
            texts.append(color + str(num_unexpected) + " " + result + (COLOR_YELLOW + " (unexpected)" + COLOR_RESET if result == "PASS" else "") + COLOR_RESET)
        return texts

    def get_summary(self):
        if len(self.test_results) == 1:
            return self.test_results[0].get_description()
        else:
            texts = []
            if self.num_different_results != 1:
                texts.append(str(len(self.test_results)) + " TOTAL")
            texts += self.get_result_class_texts("PASS", COLOR_GREEN, self.num_pass_expected, self.num_pass_unexpected)
            texts += self.get_result_class_texts("SKIP", COLOR_CYAN, self.num_skip_expected, self.num_skip_unexpected)
            texts += self.get_result_class_texts("CANCEL", COLOR_CYAN, self.num_cancel_expected, self.num_cancel_unexpected)
            texts += self.get_result_class_texts("FAIL", COLOR_YELLOW, self.num_fail_expected, self.num_fail_unexpected)
            texts += self.get_result_class_texts("ERROR", COLOR_RED, self.num_error_expected, self.num_error_unexpected)
            return ", ".join(texts) + (" in " + str(datetime.timedelta(seconds=self.elapsed_wall_time)) if self.elapsed_wall_time else "")

    def get_details(self, separator="\n  ", test_result_filter=None, exclude_test_result_filter=None, exclude_expected_test_result=False, **kwargs):
        texts = []
        def matches_test_result(test_result, result):
            return test_result.result == result and \
                   (not exclude_expected_test_result or test_result.expected_result != test_result.result) and \
                   matches_filter(result, test_result_filter, exclude_test_result_filter, True)
        for test_result in filter(lambda test_result: matches_test_result(test_result, "PASS"), self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: matches_test_result(test_result, "SKIP"), self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: matches_test_result(test_result, "CANCEL"), self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: matches_test_result(test_result, "FAIL"), self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: matches_test_result(test_result, "ERROR"), self.test_results):
            texts.append(test_result.get_description(**kwargs))
        return "  " + separator.join(texts)

    def get_unique_error_messages(self, length=None):
        def process_error_message(test_result):
            error_message = test_result.simulation_result.error_message if test_result else None
            return error_message[0:length] if error_message else None
        return list(set(map(process_error_message, self.test_results)))

    def rerun(self, result=None, **kwargs):
        return self.multiple_test_runs.run(**kwargs)

    def get_passes(self, exclude_expected_passes=True):
        return self.filter(result_filter="PASS", exclude_expected_result_filter="PASS" if exclude_expected_passes else None)

    def get_skips(self, exclude_expected_fails=True):
        return self.filter(result_filter="SKIP", exclude_expected_result_filter="SKIP" if exclude_expected_fails else None)

    def get_cancels(self, exclude_expected_fails=True):
        return self.filter(result_filter="CANCEL", exclude_expected_result_filter="CANCEL" if exclude_expected_fails else None)

    def get_fails(self, exclude_expected_fails=True):
        return self.filter(result_filter="FAIL", exclude_expected_result_filter="FAIL" if exclude_expected_fails else None)

    def get_errors(self, exclude_expected_errors=True):
        return self.filter(result_filter="ERROR", exclude_expected_result_filter="ERROR" if exclude_expected_errors else None)

    def get_unexpected(self):
        return self.filter(exclude_result_filter="SKIP|CANCEL", exclude_expected_test_result=True)

def get_result_color(result):
    if result == "PASS":
        return COLOR_GREEN
    elif result == "SKIP":
        return COLOR_CYAN
    elif result == "CANCEL":
        return COLOR_CYAN
    elif result == "FAIL":
        return COLOR_YELLOW
    elif result == "ERROR":
        return COLOR_RED
    else:
        raise("Unknown result: " + result)
