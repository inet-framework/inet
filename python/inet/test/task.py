import logging

from inet.common import *

logger = logging.getLogger(__name__)

class AssertionResult:
    def __init__(self, check, result):
        self.check = check
        self.result = result

class TestTaskResult(TaskResult):
    def __init__(self, task=None, result=None, expected_result="PASS", bool_result=None, assertion_results=None, possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task=task, result=result or ("PASS" if bool_result or bool_result is None else "FAIL"), expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.assertion_results = assertion_results

class MultipleTestTaskResults(MultipleTaskResults):
    def __init__(self, multiple_tasks=None, results=[], expected_result="PASS", possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(multiple_tasks=multiple_tasks, results=results, expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def is_all_results_pass(self):
        return self.num_expected["PASS"] == len(self.results)

    def get_pass_results(self, exclude_expected=True):
        return self.filter_results(result_filter="PASS", exclude_expected_result_filter="PASS" if exclude_expected else None)

    def get_skip_results(self, exclude_expected=True):
        return self.filter_results(result_filter="SKIP", exclude_expected_result_filter="SKIP" if exclude_expected else None)

    def get_fail_results(self, exclude_expected=True):
        return self.filter_results(result_filter="FAIL", exclude_expected_result_filter="FAIL" if exclude_expected else None)

class TestTask(Task):
    def __init__(self, name="test", task_result_class=TestTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        return self.task_result_class(self, result="PASS", reason="Test completed")

class MultipleTestTasks(MultipleTasks):
    def __init__(self, name="test", multiple_task_results_class=MultipleTestTaskResults, **kwargs):
        super().__init__(name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

class UpdateTaskResult(TaskResult):
    def __init__(self, task=None, result="KEEP", expected_result="KEEP", possible_results=["KEEP", "SKIP", "CANCEL", "INSERT", "UPDATE", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task=task, result=result, expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

class MultipleUpdateTaskResults(MultipleTaskResults):
    def __init__(self, multiple_tasks=None, results=[], expected_result="KEEP", possible_results=["KEEP", "SKIP", "CANCEL", "INSERT", "UPDATE", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(multiple_tasks=multiple_tasks, results=results, expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def is_all_results_keep(self):
        return self.num_expected["KEEP"] == len(self.results)

    def get_keep_results(self, exclude_expected=True):
        return self.filter_results(result_filter="KEEP", exclude_expected_result_filter="KEEP" if exclude_expected else None)

    def get_skip_results(self, exclude_expected=True):
        return self.filter_results(result_filter="SKIP", exclude_expected_result_filter="SKIP" if exclude_expected else None)

    def get_insert_results(self, exclude_expected=True):
        return self.filter_results(result_filter="INSERT", exclude_expected_result_filter="INSERT" if exclude_expected else None)

    def get_update_results(self, exclude_expected=True):
        return self.filter_results(result_filter="UPDATE", exclude_expected_result_filter="UPDATE" if exclude_expected else None)

class UpdateTask(Task):
    def __init__(self, task_result_class=UpdateTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        return self.task_result_class(self, result="KEEP", reason="Update completed")

class MultipleUpdateTasks(MultipleTasks):
    def __init__(self, tasks=[], multiple_task_results_class=MultipleUpdateTaskResults, **kwargs):
        super().__init__(tasks=tasks, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

def run_test_task_tests():
    return run_task_tests(multiple_tasks_class=MultipleTestTasks, task_class=TestTask, expected_result="PASS")

def run_update_task_tests():
    return run_task_tests(multiple_tasks_class=MultipleUpdateTasks, task_class=UpdateTask, expected_result="KEEP")
