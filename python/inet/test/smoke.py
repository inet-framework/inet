"""
This module provides functionality for smoke testing of multiple simulations.

The main function is :py:func:`run_smoke_tests`. It allows running multiple smoke tests matching the
provided filter criteria. Smoke tests check if simulations run without crashing and terminate properly.
"""

import logging

from inet.simulation import *
from inet.test.simulation import *

_logger = logging.getLogger(__name__)

class SmokeTestTask(SimulationTestTask):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        # TODO check simulation_task_result.elapsed_cpu_time
        return super().check_simulation_task_result(simulation_task_result, **kwargs)

def get_smoke_test_tasks(cpu_time_limit="1s", run_number=0, **kwargs):
    """
    Returns multiple smoke test tasks matching the provided filter criteria. The returned tasks can be run by calling
    the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict): The filter criteria parameters are inherited from the :py:meth:`get_simulation_tasks <inet.simulation.task.get_simulation_tasks>` method.

    Returns (:py:class:`MultipleSimulationTestTasks <inet.test.simulation.MultipleSimulationTestTasks>`):
        an object that contains a list of :py:class:`SimulationTestTask <inet.test.simulation.SimulationTestTask>` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    return get_simulation_test_tasks(cpu_time_limit=cpu_time_limit, run_number=run_number, name="smoke test", simulation_test_task_class=SmokeTestTask, **kwargs)

def run_smoke_tests(**kwargs):
    """
    Runs one or more smoke tests that match the provided filter criteria.

    Parameters:
        kwargs (dict): The filter criteria parameters are inherited from the :py:func:`get_smoke_test_tasks` function.

    Returns (:py:class:`MultipleTestTaskResults <inet.test.task.MultipleTestTaskResults>`):
        an object that contains a list of :py:class:`SimulationTestTaskResult <inet.test.simulation.SimulationTestTaskResult>` objects. Each object describes the result of running one test task.
    """
    multiple_test_tasks = get_smoke_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)
