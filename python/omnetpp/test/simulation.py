import datetime
import logging
import time

from inet.common import *
from inet.simulation.task import *
from inet.simulation.config import *
from inet.test.task import *

logger = logging.getLogger(__name__)

class SimulationTestTaskResult(TestTaskResult):
    def __init__(self, simulation_task_result=None, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_task_result = simulation_task_result

    def get_error_message(self, **kwargs):
        return (self.simulation_task_result.get_error_message(**kwargs) + ", " if self.simulation_task_result else "") + \
               super().get_error_message(**kwargs)

    def get_subprocess_result(self):
        return self.simulation_task_result.subprocess_result if self.simulation_task_result else None

    def get_test_results(self):
        return [self]

class MultipleSimulationTestTaskResults(MultipleTestTaskResults):
    def get_test_results(self):
        return self.results

    def filter(self, result_filter=None, exclude_result_filter=None, expected_result_filter=None, exclude_expected_result_filter=None, exclude_expected_test_result=False, exclude_error_message_filter=None, error_message_filter=None, full_match=True):
        def matches_test_result(test_result):
            return (not exclude_expected_test_result or test_result.expected_result != test_result.result) and \
                   matches_filter(test_result.result, result_filter, exclude_result_filter, full_match) and \
                   matches_filter(test_result.expected_result, expected_result_filter, exclude_expected_result_filter, full_match) and \
                   matches_filter(test_result.simulation_task_result.error_message if test_result.simulation_task_result else None, error_message_filter, exclude_error_message_filter, full_match)
        test_results = list(filter(matches_test_result, self.results))
        test_tasks = list(map(lambda test_result: test_result.test_task, test_results))
        simulation_tasks = list(map(lambda test_task: test_task.simulation_run, test_tasks))
        orignial_multiple_simulation_tasks = self.multiple_test_tasks.multiple_simulation_tasks
        multiple_simulation_tasks = MultipleSimulationTasks(simulation_project=orignial_multiple_simulation_tasks.simulation_project, tasks=simulation_tasks, concurrent=orignial_multiple_simulation_tasks.concurrent)
        multiple_test_tasks = self.multiple_test_tasks.__class__(multiple_simulation_tasks, test_tasks)
        return MultipleSimulationTestTaskResults(multiple_test_tasks, test_results)

class SimulationTestTask(TestTask):
    def __init__(self, simulation_task=None, task_result_class=SimulationTestTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_task = simulation_task

    def set_cancel(self, cancel):
        super().set_cancel(cancel)
        self.simulation_task.set_cancel(cancel)

    def get_parameters_string(self, **kwargs):
        return self.simulation_task.get_parameters_string(**kwargs)

    def run_protected(self, output_stream=sys.stdout, **kwargs):
        simulation_config = self.simulation_task.simulation_config
        if simulation_config.user_interface and simulation_config.user_interface != self.simulation_task.user_interface:
            return self.task_result_class(task=self, result="SKIP", expected_result="SKIP", reason="Requires different user interface")
        else:
            simulation_task_result = self.simulation_task.run_protected(output_stream=output_stream, **kwargs)
            if simulation_task_result.result == "DONE":
                return self.check_simulation_task_result(simulation_task_result=simulation_task_result, **kwargs)
            else:
                return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result=simulation_task_result.result, expected_result=simulation_task_result.expected_result, expected=simulation_task_result.expected, reason=simulation_task_result.reason, error_message="simulation exited with error")

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        result = "PASS" if simulation_task_result.result == "DONE" else simulation_task_result.result
        expected_result = "PASS" if simulation_task_result.expected_result == "DONE" else simulation_task_result.expected_result
        return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result=result, expected_result=expected_result, reason=simulation_task_result.reason)

class MultipleSimulationTestTasks(MultipleTestTasks):
    def __init__(self, build=True, simulation_project=None, **kwargs):
        super().__init__(build=build, simulation_project=simulation_project, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.build = build
        self.simulation_project = simulation_project

    def run(self, **kwargs):
        if self.build:
            build_project(simulation_project=self.simulation_project, **kwargs)
        return super().run(**kwargs)
    #     test_results = super().run(**kwargs)
    #     flattened_test_results = flatten(map(lambda test_result: test_result.get_test_results(), test_results))
    #     simulation_task_results = list(map(lambda test_result: test_result.simulation_task_result, flattened_test_results))
    #     return MultipleSimulationTestTaskResults(self, flattened_test_results, elapsed_wall_time=end_time - start_time)

def get_simulation_test_tasks(simulation_test_task_class=SimulationTestTask, multiple_simulation_test_tasks_class=MultipleSimulationTestTasks, **kwargs):
    multiple_simulation_tasks = get_simulation_tasks(**kwargs)
    test_tasks = list(map(lambda simulation_task: simulation_test_task_class(simulation_task=simulation_task, **kwargs), multiple_simulation_tasks.tasks))
    return multiple_simulation_test_tasks_class(tasks=test_tasks, simulation_project=multiple_simulation_tasks.simulation_project, **kwargs)

def run_simulation_tests(**kwargs):
    multiple_test_tasks = get_test_tasks(**kwargs)
    return multiple_test_tasks.run()
