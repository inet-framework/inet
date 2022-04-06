import logging

from inet.simulation import *
from inet.test.simulation import *
from inet.test.task import *

logger = logging.getLogger(__name__)

class SpeedTestTask(SimulationTestTask):
    def __init__(self, simulation_task, **kwargs):
        super().__init__(simulation_task, **kwargs)

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        # TODO check simulation_task_result.elapsed_cpu_time
        return super().check_simulation_task_result(simulation_task_result, **kwargs)

def get_speed_test_tasks(cpu_time_limit="1s", **kwargs):
    return get_simulation_test_tasks(cpu_time_limit=cpu_time_limit, name="speed test", simulation_test_task_class=SpeedTestTask, **kwargs)

def run_speed_tests(**kwargs):
    multiple_test_tasks = get_speed_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)
