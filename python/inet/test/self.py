import argparse
import logging
import os
import re
import sys

from inet.simulation import *
from inet.test import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

# TODO
# running simulations
# running smoke tests
# running fingerprint tests
#
# dimensions:
#  - current project vs selected project
#  - debug vs release
#  - build vs no-build
#  - include filter and exclude filter (working directory, ini file, config, run number)
#  - sim time limit, cpu time limit
#  - sequential vs. concurrent
#  - localhost vs. cluster
#  - native environment vs specific nix environment

class SimulationSelfTestTask(TestTask):
    def __init__(self, simulation_project, action="Running simulation self test", print_run_start_separately=False, **kwargs):
        super().__init__(simulation_project=simulation_project, action=action, print_run_start_separately=print_run_start_separately, **kwargs)
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.simulation_project.get_name()

    def run_protected(self, **kwargs):
        simulation_task_results = run_simulations(**self.kwargs)
        return TaskResult(task=self, result=simulation_task_results.result)

class SmokeTestSelfTestTask(TestTask):
    def __init__(self, simulation_project, action="Running smoke test self test", print_run_start_separately=False, **kwargs):
        super().__init__(simulation_project=simulation_project, action=action, print_run_start_separately=print_run_start_separately, **kwargs)
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.simulation_project.get_name()

    def run_protected(self, **kwargs):
        smoke_test_task_results = run_smoke_tests(**self.kwargs)
        return TestTaskResult(task=self, result=smoke_test_task_results.result)

class FingerprintTestSelfTestTask(TestTask):
    def __init__(self, simulation_project, action="Running fingerprint test self test", print_run_start_separately=False, **kwargs):
        super().__init__(simulation_project=simulation_project, action=action, print_run_start_separately=print_run_start_separately, **kwargs)
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.simulation_project.get_name()

    def run_protected(self, **kwargs):
        update_correct_fingerprint_task_results = update_correct_fingerprints(**self.kwargs)
        fingerprint_test_task_results = run_fingerprint_tests(**self.kwargs)
        repeated_update_correct_fingerprint_task_results = update_correct_fingerprints(**self.kwargs)
        repeated_fingerprint_test_task_results = run_fingerprint_tests(**self.kwargs)
        return TestTaskResult(task=self, result=fingerprint_test_task_results.result)

class SimulationProjectSelfTestTasks(MultipleTestTasks):
    def __init__(self, simulation_project, name="simulation project self test", print_run_start_separately=False, **kwargs):
        super().__init__(tasks = [SimulationSelfTestTask(simulation_project=simulation_project, concurrent=True, **kwargs),
                                  SmokeTestSelfTestTask(simulation_project=simulation_project, concurrent=True, **kwargs),
                                  FingerprintTestSelfTestTask(simulation_project=simulation_project, concurrent=False, **kwargs)],
                         simulation_project=simulation_project, name=name, print_run_start_separately=print_run_start_separately, concurrent=False, **kwargs)
        self.simulation_project = simulation_project

class MultipleSelfTestTasks(MultipleTestTasks):
    def __init__(self, name="self test", print_run_start_separately=False, **kwargs):
        super().__init__(tasks=[SimulationProjectSelfTestTasks(simulation_project=get_simulation_project("aloha", None), sim_time_limit="90min", **kwargs),
                                SimulationProjectSelfTestTasks(simulation_project=get_simulation_project("tictoc", None), sim_time_limit="1s", **kwargs)],
                         name=name, print_run_start_separately=print_run_start_separately, concurrent=False, **kwargs)

def parse_arguments():
    description = "Runs the self test on the OMNeT++ sample projects."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("-m", "--mode", choices=["debug", "release"], help="specifies the build mode of the projects")
    parser.add_argument("-f", "--filter", default=None, help="includes simulations that match the specified generic filter")
    parser.add_argument("--exclude-filter", default=None, help="exclude simulations that match the specified generic filter")
    parser.add_argument("-r", "--run-filter", default=None, help="includes simulations having the specified run numbers")
    parser.add_argument("--exclude-run-filter", default=None, help="exclude simulations having the specified run numbers")
    parser.add_argument("-l", "--log-level", choices=["ERROR", "WARN", "INFO", "DEBUG"], default="WARN", help="specifies the log level for the root logging category")
    return parser.parse_args(sys.argv[1:])

def process_arguments(args):
    kwargs = {k: v for k, v in vars(args).items() if v is not None}
    return kwargs

def define_self_test_projects():
    define_simulation_project("aloha", folder_environment_variable="__omnetpp_root_dir", folder="samples/aloha")
    define_simulation_project("tictoc", folder_environment_variable="__omnetpp_root_dir", folder="samples/tictoc")

def run_self_tests_main():
    args = parse_arguments()
    kwargs = process_arguments(args)
    initialize_logging(args.log_level)
    define_self_test_projects()
    self_test_task_results = MultipleSelfTestTasks(**kwargs).run(**kwargs)
    print(self_test_task_results)
    sys.exit(0 if (self_test_task_results is None or self_test_task_results.is_all_results_expected()) else 1)
