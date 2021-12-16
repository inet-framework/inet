from inet.common import *
from inet.simulation.run import *
from inet.simulation.config import *

def _run_test(test_run, **kwargs):
    return test_run.run(**kwargs)

class TestRun:
    def __init__(self, simulation_run, check_test_function, **kwargs):
        self.simulation_run = simulation_run
        self.check_test_function = check_test_function
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def run(self, sim_time_limit=None, cancel=False, **kwargs):
        if cancel or self.cancel:
            return TestResult(self, None, result="CANCEL", reason="Cancel by user")
        else:
            simulation_result = self.simulation_run.run_simulation(print_end=" ", sim_time_limit=sim_time_limit, **kwargs)
            test_result = self.check_test_function(self, simulation_result, **kwargs)
            print(test_result.get_description())
            return test_result

class MultipleTestRuns:
    def __init__(self, multiple_simulation_runs, test_runs, **kwargs):
        self.multiple_simulation_runs = multiple_simulation_runs
        self.test_runs = test_runs

    def __repr__(self):
        return repr(self)

    def run(self, **kwargs):
        test_results = self.multiple_simulation_runs.run_simulation_runs(self.test_runs, **kwargs)
        flattened_test_results = flatten(map(lambda test_result: test_result.get_test_results(), test_results))
        simulation_results = list(map(lambda test_result: test_result.simulation_result, flattened_test_results))
        return MultipleTestResults(self, flattened_test_results)

class AssertionResult:
    def __init__(self):
        pass

class TestResult:
    def __init__(self, test_run, simulation_result, result=None, bool_result=None, expected_result="PASS", reason=None, **kwargs):
        self.test_run = test_run
        self.simulation_result = simulation_result
        self.result = result or ("PASS" if bool_result else "FAIL")
        self.expected_result = expected_result
        self.expected = expected_result == self.result
        self.reason = reason
        if self.result == "PASS":
            color = COLOR_GREEN
        elif self.result == "SKIP":
            color = COLOR_CYAN
        elif self.result == "CANCEL":
            color = COLOR_CYAN
        elif self.result == "FAIL":
            color = COLOR_YELLOW
        elif self.result == "ERROR":
            color = COLOR_RED
        else:
            raise("Unknown test result: " + result)
        self.color = color

    def __repr__(self):
        return "Test result: " + self.get_description()

    def get_subprocess_result(self):
        return self.simulation_result.subprocess_result if self.simulation_result else None

    def get_test_results(self):
        return [self]

    def get_description(self, include_simulation_parameters=False):
        return (self.test_run.simulation_run.get_simulation_parameters_string() + " " if include_simulation_parameters else "") + \
               self.color + self.result + COLOR_RESET + \
               ((COLOR_YELLOW + " (unexpected)" + COLOR_RESET) if not self.expected and self.expected_result != "PASS" else "") + \
               ((COLOR_GREEN + " (expected)" + COLOR_RESET) if self.expected and self.expected_result != "PASS" else "") + \
               (" " + self.simulation_result.error_message if self.simulation_result and self.simulation_result.error_message else "") + \
               (" (" + self.reason + ")" if self.reason else "")

    def rerun(self, **kwargs):
        return self.test_run.run(**kwargs)

class MultipleTestResults:
    def __init__(self, multiple_test_runs, test_results, **kwargs):
        self.multiple_test_runs = multiple_test_runs
        self.test_results = test_results
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

    def __repr__(self):
        if len(self.test_results) == 1:
            return self.test_results[0].__repr__()
        else:
            return ("" if self.is_all_pass() else self.get_details(include_simulation_parameters=True) + "\n\n") + "Test summary: " + self.get_summary()

    def get_test_results(self):
        return self.test_results

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
            return ", ".join(texts)

    def get_details(self, separator="\n  ", **kwargs):
        texts = []
        for test_result in filter(lambda test_result: test_result.result == "FAIL", self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: test_result.result == "SKIP", self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: test_result.result == "CANCEL", self.test_results):
            texts.append(test_result.get_description(**kwargs))
        for test_result in filter(lambda test_result: test_result.result == "ERROR", self.test_results):
            texts.append(test_result.get_description(**kwargs))
        return "  " + separator.join(texts)

    def rerun(self, result=None, **kwargs):
        return self.multiple_test_runs.run(**kwargs)

    def filter(self, result_filter=None, full_match=True):
        test_results = list(filter(lambda test_result: re.search(result_filter if full_match else ".*" + result_filter + ".*", test_result.result), self.test_results))
        test_runs = list(map(lambda test_result: test_result.test_run, test_results))
        simulation_runs = list(map(lambda test_run: test_run.simulation_run, test_runs))
        orignial_multiple_simulation_runs = self.multiple_test_runs.multiple_simulation_runs
        multiple_simulation_runs = MultipleSimulationRuns(simulation_runs, concurrent=orignial_multiple_simulation_runs.concurrent, run_simulation_function=orignial_multiple_simulation_runs.run_simulation_function)
        multiple_test_runs = MultipleTestRuns(multiple_simulation_runs, test_runs)
        return MultipleTestResults(multiple_test_runs, test_results)

def check_return_code(simulation_result):
    return simulation_result.subprocess_result.returncode == 0

def check_test(test_run, simulation_result, **kwargs):
    if simulation_result.result == "CANCEL":
        return TestResult(test_run, simulation_result, result="CANCEL", reason="Cancel by user")
    else:
        return TestResult(test_run, simulation_result, bool_result=check_return_code(simulation_result), expected_result="PASS")

def get_tests(run_test_function=_run_test, check_test_function=check_test, **kwargs):
    multiple_simulation_runs = get_simulations(run_simulation_function=run_test_function, **kwargs)
    test_runs = list(map(lambda simulation_run: TestRun(simulation_run, check_test_function, **kwargs), multiple_simulation_runs.simulation_runs))
    return MultipleTestRuns(multiple_simulation_runs, test_runs)

def run_tests(**kwargs):
    logger.info("Running tests")
    multiple_test_runs = get_tests(**kwargs)
    return multiple_test_runs.run(**kwargs)
