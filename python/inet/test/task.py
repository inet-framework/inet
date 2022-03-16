import logging

from inet.common import *

logger = logging.getLogger(__name__)

class AssertionResult:
    def __init__(self, check, result):
        self.check = check
        self.result = result

class TestTaskResult(TaskResult):
    def __init__(self, task, result=None, bool_result=None, assertion_results=None, possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task, result or ("PASS" if bool_result else "FAIL"), possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.assertion_results = assertion_results

class MultipleTestTaskResults(MultipleTaskResults):
    def __init__(self, multiple_tasks, results, possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(multiple_tasks, results, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)

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

class MultipleTestTasks(MultipleTasks):
    def __init__(self, tasks, name="test", multiple_task_results_class=MultipleTestTaskResults, **kwargs):
        super().__init__(tasks, name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)

class UpdateTaskResult(TaskResult):
    def __init__(self, task, result, possible_results=["KEEP", "SKIP", "CANCEL", "INSERT", "UPDATE", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task, result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)

class MultipleUpdateTaskResults(MultipleTaskResults):
    def __init__(self, multiple_tasks, results, possible_results=["KEEP", "SKIP", "CANCEL", "INSERT", "UPDATE", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(multiple_tasks, results, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)

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

class MultipleUpdateTasks(MultipleTasks):
    def __init__(self, tasks, multiple_task_results_class=MultipleUpdateTaskResults, **kwargs):
        super().__init__(tasks, multiple_task_results_class=multiple_task_results_class, **kwargs)
