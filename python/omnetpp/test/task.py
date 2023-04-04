"""
Provides abstractions for generic test tasks and their results.

Please note that undocumented features are not supposed to be used by the user.
"""

import logging
import sqlalchemy

from omnetpp.common import *

_logger = logging.getLogger(__name__)

class AssertionResult:
    def __init__(self, check, result):
        self.check = check
        self.result = result

class TestTaskResult(TaskResult):
    """
    Represents a test task result that is created when a test task is run.
    """

    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TaskResult.id"), primary_key=True)

    def __init__(self, task=None, result=None, expected_result="PASS", bool_result=None, assertion_results=None, possible_results=["PASS", "SKIP", "CANCEL", "FAIL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task=task, result=result or ("PASS" if bool_result or bool_result is None else "FAIL"), expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.assertion_results = assertion_results

add_orm_mapper("TestTaskResult", lambda: mapper_registry.map_imperatively(
    TestTaskResult,
    sqlalchemy.Table("TestTaskResult",
                     mapper_registry.metadata,
                     TestTaskResult.id),
    properties={"id": sqlalchemy.orm.column_property(TestTaskResult.id, TaskResult.id)},
    inherits=TaskResult,
    polymorphic_identity="TestTaskResult"))

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

    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("Task.id"), primary_key=True)

    def __init__(self, name="test", task_result_class=TestTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        return self.task_result_class(self, result="PASS", reason="Test completed")

add_orm_mapper("TestTask", lambda: mapper_registry.map_imperatively(
    TestTask,
    sqlalchemy.Table("TestTask",
                     mapper_registry.metadata,
                     TestTask.id),
    properties={"id": sqlalchemy.orm.column_property(TestTask.id, Task.id)},
    inherits=Task,
    polymorphic_identity="TestTask"))

class MultipleTestTasks(MultipleTasks):
    """
    Represents multiple test tasks that can be run together.
    """

    def __init__(self, name="test", multiple_task_results_class=MultipleTestTaskResults, **kwargs):
        super().__init__(name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

class UpdateTaskResult(TaskResult):
    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TaskResult.id"), primary_key=True)

    def __init__(self, task=None, result="KEEP", expected_result="KEEP", possible_results=["KEEP", "SKIP", "CANCEL", "INSERT", "UPDATE", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_YELLOW, COLOR_YELLOW, COLOR_RED], **kwargs):
        super().__init__(task=task, result=result, expected_result=expected_result, possible_results=possible_results, possible_result_colors=possible_result_colors, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

add_orm_mapper("UpdateTaskResult", lambda: mapper_registry.map_imperatively(
    UpdateTaskResult,
    sqlalchemy.Table("UpdateTaskResult",
                     mapper_registry.metadata,
                     UpdateTaskResult.id),
    properties={"id": sqlalchemy.orm.column_property(UpdateTaskResult.id, TaskResult.id)},
    inherits=TaskResult,
    polymorphic_identity="UpdateTaskResult"))

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
    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("Task.id"), primary_key=True)

    def __init__(self, task_result_class=UpdateTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        return self.task_result_class(self, result="KEEP", reason="Update completed")

add_orm_mapper("UpdateTask", lambda: mapper_registry.map_imperatively(
    UpdateTask,
    sqlalchemy.Table("UpdateTask",
                     mapper_registry.metadata,
                     UpdateTask.id),
    properties={"id": sqlalchemy.orm.column_property(UpdateTask.id, Task.id)},
    inherits=Task,
    polymorphic_identity="UpdateTask"))

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
