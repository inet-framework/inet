"""
This module provides abstractions for generic test tasks and their results.
"""

import logging
import sys
import time

from inet.common import *

_logger = logging.getLogger(__name__)

class AssertionResult:
    def __init__(self, check, result):
        self.check = check
        self.result = result

class TestTaskResult(TaskResult):
    """
    Represents a test task result that is created when a test task is run.
    """

    def __init__(self, task=None, result=None, expected_result="PASS", bool_result=None, assertion_results=None, possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task=task, result=result or ("PASS" if bool_result or bool_result is None else "FAIL"), expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.assertion_results = assertion_results

class MultipleTestTaskResults(MultipleTaskResults):
    """
    Represents multiple test task results that are created when multiple test tasks are run.
    """

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
    """
    Represents a self-contained test task that can be run without additional parameters.
    """

    def __init__(self, name="test", task_result_class=TestTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        return self.task_result_class(self, result="PASS", reason="Test completed")

class MultipleTestTasks(MultipleTasks):
    """
    Represents multiple test tasks that can be run together.
    """

    def __init__(self, name="test", multiple_task_results_class=MultipleTestTaskResults, **kwargs):
        super().__init__(name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

class TaskTestTask(TestTask):
    def __init__(self, tested_task=None, **kwargs):
        super().__init__(**kwargs)
        assert(tested_task is not None)
        self.tested_task = tested_task

    def count_tasks(self):
        return self.tested_task.count_tasks() + 1

    def get_description(self):
        return self.name

    def run(self, context=None, progress=None, index=0, count=1, output_stream=sys.stdout, **kwargs):
        context = extend_task_context(context, self.name, index, count)
        progress = progress or TaskProgress(self.count_tasks())
        elements = [e for e in [progress.get_string(**kwargs), context.get_string(**kwargs), "▶", self.get_description()] if e != ""]
        print(" ".join(elements), file=output_stream)
        if self.cancel:
            task_results = list(map(lambda task: task.task_result_class(task=task, result="CANCEL", reason="Cancel by user"), self.tasks))
            task_result = self.multiple_task_results_class(multiple_tasks=self, results=task_results)
        else:
            try:
                start_time = time.time()
                task_result = self.run_protected(context=context, progress=progress, output_stream=output_stream, **kwargs)
                end_time = time.time()
                task_result.elapsed_wall_time = end_time - start_time
            except KeyboardInterrupt:
                task_result = task.task_result_class(task=task, result="CANCEL", reason="Cancel by user")
        progress = progress.increment_num_finished()
        elements = [e for e in [progress.get_string(**kwargs), context.get_string(**kwargs), "◉", self.get_description(), task_result.get_description()] if e != ""]
        print(" ".join(elements), file=output_stream)
        return task_result

    def run_protected(self, **kwargs):
        start_time = time.time()
        tested_task_result = self.tested_task.run(**kwargs)
        end_time = time.time()
        tested_task_result.elapsed_wall_time = end_time - start_time
        if tested_task_result.result == "DONE":
            return self.check_task_result(tested_task_result=tested_task_result, **kwargs)
        else:
            return self.task_result_class(task=self, tested_task_result=tested_task_result, result=tested_task_result.result, expected_result=tested_task_result.expected_result, expected=tested_task_result.expected, reason=tested_task_result.reason, error_message="simulation exited with error")

    def check_task_result(self, **kwargs):
        return self.task_result_class(task=self, result="PASS")

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
