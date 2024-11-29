"""
This module provides functionality for running multiple tests using the :command:`opp_test` command.

The main function is :py:func:`run_opp_tests`. It allows running multiple tests matching the provided
filter criteria.
"""

import builtins
import glob
import logging
import signal
import subprocess

from inet.simulation.project import *
from inet.test.task import *

_logger = logging.getLogger(__name__)

class OppTestTask(TestTask):
    def __init__(self, simulation_project, working_directory, test_file_name, mode="debug", **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project
        self.working_directory = working_directory
        self.test_file_name = test_file_name
        self.mode = mode

    def get_parameters_string(self, **kwargs):
        return self.test_file_name

    def run_protected(self, **kwargs):
        binary_suffix = "_dbg" if self.mode == "debug" else ""
        test_binary_name = re.sub(r"\.test", "", self.test_file_name)
        test_directory = os.path.join(self.working_directory, f"work/{test_binary_name}")
        has_lib = os.path.exists(os.path.join(self.working_directory, "lib"))
        os.makedirs(test_directory, exist_ok=True)
        args = ["opp_test", "gen", "-v", self.test_file_name]
        subprocess_result = run_command_with_logging(args, cwd=self.working_directory, env=self.simulation_project.get_env())
        if subprocess_result.returncode != 0:
            return self.task_result_class(self, result="ERROR", stderr=subprocess_result.stderr)
        args = ["opp_makemake", "-f", "--deep", f"-lINET{binary_suffix}", "-L../../../../src", *([f"-ltest{binary_suffix}", "-L../../lib"] if has_lib else []), "-P", test_directory, "-I../../../../src", "-I../../lib"]
        subprocess_result = run_command_with_logging(args, cwd=test_directory, env=self.simulation_project.get_env())
        if subprocess_result.returncode != 0:
            return self.task_result_class(self, result="ERROR", stderr=subprocess_result.stderr)
        args = ["make", f"MODE={self.mode}", "-j", str(multiprocessing.cpu_count())]
        subprocess_result = run_command_with_logging(args, cwd=test_directory, env=self.simulation_project.get_env())
        if subprocess_result.returncode != 0:
            return self.task_result_class(self, result="ERROR", stderr=subprocess_result.stderr)
        args = ["opp_test", "run", "-v", "-p", f"{test_binary_name}/{test_binary_name}{binary_suffix}", self.test_file_name, "-a", "--check-signals=false", "-lINET", "-n", f"../../../../src:.:{'../../lib' if has_lib else ''}"]
        subprocess_result = run_command_with_logging(args, cwd=self.working_directory, env=self.simulation_project.get_env())
        stdout = subprocess_result.stdout
        stderr = subprocess_result.stderr
        match = re.search(r"Aggregate result: (\w+)", stdout)
        if match:
            result = match.group(1)
            return self.task_result_class(self, result=match.group(1), stdout=stdout, stderr=stderr)
        elif subprocess_result.returncode == signal.SIGINT.value or subprocess_result.returncode == -signal.SIGINT.value:
            return self.task_result_class(self, result="CANCEL", reason="Cancel by user")
        else:
            return self.task_result_class(self, result="FAIL", reason=f"Non-zero exit code: {subprocess_result.returncode}", stdout=stdout, stderr=stderr)

def get_opp_test_tasks(test_folder, simulation_project=None, filter=".*", full_match=False, **kwargs):
    """
    Returns multiple opp test tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            TODO

    Returns (:py:class:`MultipleTestTasks`):
        an object that contains a list of :py:class:`OppTestTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    def create_test_task(test_file_name):
        return OppTestTask(simulation_project, simulation_project.get_full_path(test_folder), os.path.basename(test_file_name), task_result_class=TestTaskResult, **dict(kwargs, pass_keyboard_interrupt=True))
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    test_file_names = list(builtins.filter(lambda test_file_name: matches_filter(test_file_name, filter, None, full_match),
                                           glob.glob(os.path.join(simulation_project.get_full_path(test_folder), "*.test"))))
    test_tasks = list(map(create_test_task, test_file_names))
    return MultipleOppTestTasks(tasks=test_tasks, simulation_project=simulation_project, test_folder=test_folder, multiple_task_results_class=MultipleTestTaskResults, **kwargs)

class MultipleOppTestTasks(MultipleTestTasks):
    def __init__(self, simulation_project=None, test_folder=None, mode="debug", **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project
        self.test_folder = test_folder
        self.mode = mode

    def run_protected(self, **kwargs):
        test_directory = self.simulation_project.get_full_path(self.test_folder)
        lib_directory = os.path.join(test_directory, "lib")
        if os.path.exists(lib_directory):
            args = ["make", f"MODE={self.mode}", "-j", str(multiprocessing.cpu_count())]
            subprocess_result = run_command_with_logging(args, cwd=lib_directory, env=self.simulation_project.get_env())
            if subprocess_result.returncode != 0:
                raise Exception("Cannot build lib")
        return super().run_protected(**kwargs)

def run_opp_tests(test_folder, **kwargs):
    """
    Runs one or more tests using the :command:`opp_test` command that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_opp_test_tasks` function.

    Returns (:py:class:`MultipleTestTaskResults`):
        an object that contains a list of :py:class:`TestTaskResult` objects. Each object describes the result of running one test task.
    """
    multiple_test_tasks = get_opp_test_tasks(test_folder, **kwargs)
    return multiple_test_tasks.run(**kwargs)
