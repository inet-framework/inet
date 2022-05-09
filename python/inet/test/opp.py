import builtins
import glob
import logging
import signal
import subprocess

from inet.simulation.project import *
from inet.test.task import *

logger = logging.getLogger(__name__)

class OppTestTask(TestTask):
    def __init__(self, simulation_project, working_directory, test_file_name, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project
        self.working_directory = working_directory
        self.test_file_name = test_file_name

    def get_parameters_string(self, **kwargs):
        return self.test_file_name

    def run_protected(self, capture_output=True, **kwargs):
        executable = "sh"
        args = [executable, "runtest", self.test_file_name]
        subprocess_result = subprocess.run(args, cwd=self.working_directory, capture_output=capture_output, env=self.simulation_project.get_env())
        stdout = subprocess_result.stdout.decode("utf-8")
        match = re.search(r"Aggregate result: (\w+)", stdout)
        if subprocess_result.returncode == signal.SIGINT.value or subprocess_result.returncode == -signal.SIGINT.value:
            return self.task_result_class(self, result="CANCEL", reason="Cancel by user")
        elif match and subprocess_result.returncode == 0:
            return self.task_result_class(self, result=match.group(1))
        else:
            return self.task_result_class(self, result="FAIL", reason=f"Non-zero exit code: {subprocess_result.returncode}")

def get_opp_test_tasks(test_folder, simulation_project=default_project, filter=".*", full_match=False, **kwargs):
    def create_test_task(test_file_name):
        return OppTestTask(simulation_project, simulation_project.get_full_path(test_folder), os.path.basename(test_file_name), task_result_class=TestTaskResult, **kwargs)
    test_file_names = list(builtins.filter(lambda test_file_name: matches_filter(test_file_name, filter, None, full_match),
                                           glob.glob(os.path.join(simulation_project.get_full_path(test_folder), "*.test"))))
    test_tasks = list(map(create_test_task, test_file_names))
    return MultipleTestTasks(tasks=test_tasks, multiple_task_results_class=MultipleTestTaskResults, **kwargs)

def run_opp_tests(test_folder, **kwargs):
    multiple_test_tasks = get_opp_test_tasks(test_folder, **kwargs)
    return multiple_test_tasks.run(**kwargs)
