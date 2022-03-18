import datetime
import logging
import time

from inet.common import *
from inet.simulation.run import *
from inet.simulation.config import *
from inet.test.run import *

logger = logging.getLogger(__name__)

def _run_simulation_test(test_run, output_stream=sys.stdout, **kwargs):
    test_result = test_run.run(output_stream=output_stream, **kwargs)
    test_result.print_result(complete_error_message=False, output_stream=output_stream)
    return test_result

class SimulationTestRun(TestRun):
    def __init__(self, simulation_run, check_test_function, **kwargs):
        super().__init__(**kwargs)
        self.simulation_run = simulation_run
        self.check_test_function = check_test_function

    def set_cancel(self, cancel):
        self.simulation_run.set_cancel(cancel)

    def create_cancel_result(self, simulation_result):
        return SimulationTestResult(self, simulation_result, result="CANCEL", reason="Cancel by user")

    def run(self, sim_time_limit=None, cancel=False, output_stream=sys.stdout, **kwargs):
        if cancel or self.cancel:
            print("Running " + self.simulation_run.get_simulation_parameters_string(sim_time_limit=sim_time_limit, **kwargs), end=" ", file=output_stream)
            return self.create_cancel_result(None)
        else:
            simulation_result = self.simulation_run.run_simulation(sim_time_limit=sim_time_limit, output_stream=output_stream, **kwargs)
            if simulation_result.result != "CANCEL":
                return self.check_test_function(self, simulation_result, **kwargs)
            else:
                return self.create_cancel_result(simulation_result)

class MultipleSimulationTestRuns(MultipleTestRuns):
    def __init__(self, multiple_simulation_runs, test_runs, **kwargs):
        super().__init__(test_runs, **kwargs)
        self.multiple_simulation_runs = multiple_simulation_runs

    def run(self, simulation_project=None, concurrent=None, build=True, **kwargs):
        if concurrent is None:
            concurrent = self.multiple_simulation_runs.concurrent
        if build:
            build_project(simulation_project or self.multiple_simulation_runs.simulation_project, **kwargs)
        logger.info("Running tests " + str(kwargs))
        start_time = time.time()
        test_results = map_sequentially_or_concurrently(self.test_runs, self.multiple_simulation_runs.run_simulation_function, concurrent=concurrent, **kwargs)
        end_time = time.time()
        flattened_test_results = flatten(map(lambda test_result: test_result.get_test_results(), test_results))
        simulation_results = list(map(lambda test_result: test_result.simulation_result, flattened_test_results))
        return MultipleSimulationTestResults(self, flattened_test_results, elapsed_wall_time=end_time - start_time)

class SimulationTestResult(TestResult):
    def __init__(self, test_run, simulation_result, **kwargs):
        super().__init__(test_run, **kwargs)
        self.simulation_result = simulation_result

    def get_subprocess_result(self):
        return self.simulation_result.subprocess_result if self.simulation_result else None

    def get_test_results(self):
        return [self]

    def get_description(self, complete_error_message=True, include_simulation_parameters=False):
        return (self.test_run.simulation_run.get_simulation_parameters_string() + " " if include_simulation_parameters else "") + \
               self.color + self.result + COLOR_RESET + \
               ((COLOR_YELLOW + " (unexpected)" + COLOR_RESET) if not self.expected and self.expected_result != "PASS" else "") + \
               ((COLOR_GREEN + " (expected)" + COLOR_RESET) if self.expected and self.expected_result != "PASS" else "") + \
               (" " + (self.simulation_result.error_message + " -- in module " + self.simulation_result.error_module if complete_error_message else self.simulation_result.error_message) if self.simulation_result and self.simulation_result.error_message and self.simulation_result.error_module else "") + \
               (" (" + self.reason + ")" if self.reason else "")

class MultipleSimulationTestResults(MultipleTestResults):
    def get_test_results(self):
        return self.test_results

    def filter(self, result_filter=None, exclude_result_filter=None, expected_result_filter=None, exclude_expected_result_filter=None, exclude_expected_test_result=False, exclude_error_message_filter=None, error_message_filter=None, full_match=True):
        def matches_test_result(test_result):
            return (not exclude_expected_test_result or test_result.expected_result != test_result.result) and \
                   matches_filter(test_result.result, result_filter, exclude_result_filter, full_match) and \
                   matches_filter(test_result.expected_result, expected_result_filter, exclude_expected_result_filter, full_match) and \
                   matches_filter(test_result.simulation_result.error_message if test_result.simulation_result else None, error_message_filter, exclude_error_message_filter, full_match)
        test_results = list(filter(matches_test_result, self.test_results))
        test_runs = list(map(lambda test_result: test_result.test_run, test_results))
        simulation_runs = list(map(lambda test_run: test_run.simulation_run, test_runs))
        orignial_multiple_simulation_runs = self.multiple_test_runs.multiple_simulation_runs
        multiple_simulation_runs = MultipleSimulationRuns(orignial_multiple_simulation_runs.simulation_project, simulation_runs, concurrent=orignial_multiple_simulation_runs.concurrent, run_simulation_function=orignial_multiple_simulation_runs.run_simulation_function)
        multiple_test_runs = self.multiple_test_runs.__class__(multiple_simulation_runs, test_runs)
        return MultipleSimulationTestResults(multiple_test_runs, test_results)

def check_test(test_run, simulation_result, **kwargs):
    if test_run.simulation_run.cancel or simulation_result.result == "CANCEL":
        return SimulationTestResult(test_run, simulation_result, result="CANCEL", reason="Cancel by user")
    else:
        return SimulationTestResult(test_run, simulation_result, bool_result=simulation_result.subprocess_result.returncode == 0, expected_result="PASS")

def get_tests(run_test_function=_run_simulation_test, check_test_function=check_test, **kwargs):
    multiple_simulation_runs = get_simulations(run_simulation_function=run_test_function, **kwargs)
    test_runs = list(map(lambda simulation_run: SimulationTestRun(simulation_run, check_test_function, **kwargs), multiple_simulation_runs.simulation_runs))
    return MultipleSimulationTestRuns(multiple_simulation_runs, test_runs)

def run_tests(**kwargs):
    multiple_test_runs = None
    try:
        logger.info("Running tests")
        multiple_test_runs = get_tests(**kwargs)
        return multiple_test_runs.run(**kwargs)
    except KeyboardInterrupt:
        test_results = list(map(lambda test_run: SimulationTestResult(test_run, None, result="CANCEL", reason="Cancel by user"), multiple_test_runs.test_runs)) if multiple_test_runs else []
        return MultipleSimulationTestResults(multiple_test_runs, test_results)
