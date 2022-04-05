import logging

from inet.simulation import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

class SanitizerTestTask(SimulationTestTask):
    def run_protected(self, output_stream=sys.stdout, **kwargs):
        simulation_task_result = self.simulation_task.run_protected(output_stream=output_stream, **kwargs)
        stderr = simulation_task_result.subprocess_result.stderr.decode("utf-8")
        task_result = super().check_simulation_task_result(simulation_task_result, **kwargs)
        match = re.search("SUMMARY: (.*)", stderr)
        if match:
            task_result.reason = re.sub(" in", "", match.group(1))
        return task_result

def get_sanitizer_test_tasks(mode="sanitize", cpu_time_limit="1s", **kwargs):
    return get_simulation_test_tasks(mode=mode, cpu_time_limit=cpu_time_limit, name="sanitizer test", simulation_test_task_class=SanitizerTestTask, **kwargs)

def run_sanitizer_tests(**kwargs):
    multiple_test_tasks = get_sanitizer_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)
