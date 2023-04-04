"""
Provides abstractions for simulation test tasks and their results.

Please note that undocumented features are not supposed to be used by the user.
"""

import datetime
import logging
import sqlalchemy
import time

from omnetpp.common import *
from omnetpp.simulation.task import *
from omnetpp.simulation.config import *
from omnetpp.test.task import *

_logger = logging.getLogger(__name__)

class SimulationTestTaskResult(TestTaskResult):
    """
    Represents a simulation test task result that is collected when a simulation test task is run.

    Please note that undocumented features are not supposed to be called by the user.
    """

    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TestTaskResult.id"), primary_key=True)

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

    def store(self, database_session):
        stored_simulation_test_task_result = super().store(database_session)
        stored_simulation_test_task_result.simulation_task_result = self.simulation_task_result.ensure_stored(database_session)
        return stored_simulation_test_task_result

add_orm_mapper("SimulationTestTaskResult", lambda: mapper_registry.map_imperatively(
    SimulationTestTaskResult,
    sqlalchemy.Table("SimulationTestTaskResult",
                     mapper_registry.metadata,
                     SimulationTestTaskResult.id,
                     sqlalchemy.Column("simulation_task_result_id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TaskResult.id"))),
    properties={"id": sqlalchemy.orm.column_property(SimulationTestTaskResult.id, TestTaskResult.id, TaskResult.id),
                "simulation_task_result": sqlalchemy.orm.relationship("TaskResult", remote_side="TaskResult.id", foreign_keys="SimulationTestTaskResult.simulation_task_result_id", uselist=False)},
    inherits=TestTaskResult,
    polymorphic_identity="SimulationTestTaskResult"))

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
    """
    Represents a simulation test task that can be run (and re-run) without providing additional parameters.

    Please note that undocumented features are not supposed to be called by the user.
    """

    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TestTask.id"), primary_key=True)

    def __init__(self, simulation_task=None, task_result_class=SimulationTestTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_task = simulation_task

    def get_hash(self, **kwargs):
        return self.simulation_task.get_hash(**kwargs)

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

    def store(self, database_session):
        stored_simulation_test_task = super().store(database_session)
        stored_simulation_test_task.simulation_task = self.simulation_task.ensure_stored(database_session)
        return stored_simulation_test_task

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        result = "PASS" if simulation_task_result.result == "DONE" else simulation_task_result.result
        expected_result = "PASS" if simulation_task_result.expected_result == "DONE" else simulation_task_result.expected_result
        return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result=result, expected_result=expected_result, reason=simulation_task_result.reason)

add_orm_mapper("SimulationTestTask", lambda: mapper_registry.map_imperatively(
    SimulationTestTask,
    sqlalchemy.Table("SimulationTestTask",
                     mapper_registry.metadata,
                     SimulationTestTask.id,
                     sqlalchemy.Column("simulation_task_id", sqlalchemy.Integer, sqlalchemy.ForeignKey("Task.id"))),
    properties={"id": sqlalchemy.orm.column_property(SimulationTestTask.id, TestTask.id, Task.id),
                "simulation_task": sqlalchemy.orm.relationship("SimulationTask", remote_side="Task.id", foreign_keys="SimulationTestTask.simulation_task_id", uselist=False)},
    inherits=TestTask,
    polymorphic_identity="SimulationTestTask"))

class MultipleSimulationTestTasks(MultipleTestTasks):
    def __init__(self, build=True, simulation_project=None, **kwargs):
        super().__init__(build=build, simulation_project=simulation_project, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.build = build
        self.simulation_project = simulation_project

    def get_description(self):
        return self.simulation_project.get_name() + " " + super().get_description()

    def run(self, **kwargs):
        if self.build:
            build_project(**dict(kwargs, simulation_project=self.simulation_project))
        return super().run(**kwargs)

def get_simulation_test_tasks(simulation_test_task_class=SimulationTestTask, multiple_simulation_test_tasks_class=MultipleSimulationTestTasks, **kwargs):
    multiple_simulation_tasks = get_simulation_tasks(**kwargs)
    test_tasks = list(map(lambda simulation_task: simulation_test_task_class(simulation_task=simulation_task, **kwargs), multiple_simulation_tasks.tasks))
    return multiple_simulation_test_tasks_class(tasks=test_tasks, **dict(kwargs, simulation_project=multiple_simulation_tasks.simulation_project))

def run_simulation_tests(**kwargs):
    multiple_test_tasks = get_test_tasks(**kwargs)
    return multiple_test_tasks.run()

class SimulationUpdateTaskResult(UpdateTaskResult):
    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("UpdateTaskResult.id"), primary_key=True)

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

    def store(self, database_session):
        stored_simulation_update_task_result = super().store(database_session)
        stored_simulation_update_task_result.simulation_task_result = self.simulation_task_result.ensure_stored(database_session)
        return stored_simulation_update_task_result

add_orm_mapper("SimulationUpdateTaskResult", lambda: mapper_registry.map_imperatively(
    SimulationUpdateTaskResult,
    sqlalchemy.Table("SimulationUpdateTaskResult",
                     mapper_registry.metadata,
                     SimulationUpdateTaskResult.id,
                     sqlalchemy.Column("simulation_task_result_id", sqlalchemy.Integer, sqlalchemy.ForeignKey("TaskResult.id"))),
    properties={"id": sqlalchemy.orm.column_property(SimulationUpdateTaskResult.id, UpdateTaskResult.id, TaskResult.id),
                "simulation_task_result": sqlalchemy.orm.relationship("TaskResult", remote_side="TaskResult.id", foreign_keys="SimulationUpdateTaskResult.simulation_task_result_id", uselist=False)},
    inherits=UpdateTaskResult,
    polymorphic_identity="SimulationUpdateTaskResult"))

class SimulationUpdateTask(UpdateTask):
    id = sqlalchemy.Column("id", sqlalchemy.Integer, sqlalchemy.ForeignKey("UpdateTask.id"), primary_key=True)

    def __init__(self, simulation_task=None, task_result_class=SimulationUpdateTaskResult, **kwargs):
        super().__init__(task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_task = simulation_task

    def get_hash(self, **kwargs):
        return self.simulation_task.get_hash(**kwargs)

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

    def store(self, database_session):
        stored_simulation_update_task = super().store(database_session)
        stored_simulation_update_task.simulation_task = self.simulation_task.ensure_stored(database_session)
        return stored_simulation_update_task

add_orm_mapper("SimulationUpdateTask", lambda: mapper_registry.map_imperatively(
    SimulationUpdateTask,
    sqlalchemy.Table("SimulationUpdateTask",
                     mapper_registry.metadata,
                     SimulationUpdateTask.id,
                     sqlalchemy.Column("simulation_task_id", sqlalchemy.Integer, sqlalchemy.ForeignKey("Task.id"))),
    properties={"id": sqlalchemy.orm.column_property(SimulationUpdateTask.id, UpdateTask.id, Task.id),
                "simulation_task": sqlalchemy.orm.relationship("SimulationTask", remote_side="Task.id", foreign_keys="SimulationUpdateTask.simulation_task_id", uselist=False)},
    inherits=UpdateTask,
    polymorphic_identity="SimulationUpdateTask"))

class MultipleSimulationUpdateTasks(MultipleUpdateTasks):
    def __init__(self, simulation_project=None, **kwargs):
        super().__init__(simulation_project=simulation_project, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project

    def get_description(self):
        return self.simulation_project.get_name() + " " + super().get_description()
