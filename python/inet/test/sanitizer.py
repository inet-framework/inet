import logging

from inet.simulation import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

class SanitizerTestTask(SimulationTestTask):
    def run_protected(self, output_stream=sys.stdout, **kwargs):
        simulation_task_result = self.simulation_task.run_protected(output_stream=output_stream, **kwargs)
        stderr = simulation_task_result.subprocess_result.stderr.decode("utf-8") if simulation_task_result.subprocess_result.stderr else ""
        test_task_result = super().check_simulation_task_result(simulation_task_result, **kwargs)
        match = re.search(r"SUMMARY: (.*)", stderr)
        if match:
            test_task_result.reason = re.sub(r" in", "", match.group(1))
            # TODO isn't there a better way?
            if test_task_result.result == "PASS":
                test_task_result.result = "FAIL"
                test_task_result.expected = False
                test_task_result.color = COLOR_YELLOW
        return test_task_result

def get_sanitizer_test_tasks(mode="sanitize", cpu_time_limit="1s", run=0, **kwargs):
    return get_simulation_test_tasks(mode=mode, cpu_time_limit=cpu_time_limit, name="sanitizer test", simulation_test_task_class=SanitizerTestTask, run=run, **kwargs)

def run_sanitizer_tests(**kwargs):
    multiple_test_tasks = get_sanitizer_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)
