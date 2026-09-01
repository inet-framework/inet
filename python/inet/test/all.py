import logging

from inet.project.inet import *
from inet.test.chart import *
from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.opp import *
from inet.test.sanitizer import *
from inet.test.simulation import *
from inet.test.smoke import *
from inet.test.speed import *
from inet.test.statistical import *
from inet.test.validation import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class PacketTestTask(TestTask):
    def __init__(self, simulation_project=inet_project, name="packet test", task_result_class=TestTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.name

    def run_protected(self, **kwargs):
        working_directory = self.simulation_project.get_full_path("tests/packet")
        args = ["make", "-s", "MODE=debug", "-j", str(multiprocessing.cpu_count())]
        run_command_with_logging(args, cwd=working_directory, error_message=f"Build {self.simulation_project.get_name()} failed")
        args = [f"./packet_test_dbg", "-s", "-u", "Cmdenv", "-c", "UnitTest"]
        subprocess_result = run_command_with_logging(args, cwd=working_directory, env=self.simulation_project.get_env())
        return self.task_result_class(self, result="PASS" if subprocess_result.returncode == 0 else "FAIL")

def get_packet_test_tasks(filter=None, working_directory_filter=None, ini_file_filter=None, config_filter=None, run_filter=None, **kwargs):
    if filter or (working_directory_filter and not os.path.abspath("tests/packet").startswith(os.path.abspath(working_directory_filter))) or ini_file_filter or config_filter or run_filter:
        packet_test_tasks = []
    else:
        packet_test_tasks = [PacketTestTask(**kwargs)]
    return MultipleTestTasks(tasks=packet_test_tasks, name="packet test", **dict(kwargs, concurrent=False))

def get_queueing_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/queueing", name="queueing test", **kwargs)

def get_protocol_test_tasks(simulation_project=None, **kwargs):
    """
    Returns the protocol test tasks of every suite under tests/protocol.

    Each direct subfolder that holds .test files at any depth is one suite, for example
    arp, element, ethernet, ipv4, ipv6, self, tcp or wifi. The suite folder is the root a
    test is built and run from, so a test may sit in a subfolder of it and still reach the
    files the whole suite shares. A new subfolder needs no registration here. Select one
    suite with the working directory filter, for example:

        inet_run_protocol_tests -w self

    A suite links the protocol test framework library when its tests compile against it.
    """
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    protocol_folder = "tests/protocol"
    working_directory_filter = kwargs.get("working_directory_filter", None)
    exclude_working_directory_filter = kwargs.get("exclude_working_directory_filter", None)
    full_match = kwargs.get("full_match", False)
    test_tasks = []
    for folder in sorted(glob.glob(os.path.join(simulation_project.get_full_path(protocol_folder), "*"))):
        # a suite may group its tests in subfolders, so look for them at any depth, but
        # never inside the generated work folder
        test_file_names = [test_file_name
                           for test_file_name in glob.glob(os.path.join(folder, "**", "*.test"), recursive=True)
                           if "/work/" not in test_file_name]
        if not test_file_names:
            continue
        suite_name = os.path.basename(folder)
        test_folder = protocol_folder + "/" + suite_name
        if not matches_filter(test_folder, working_directory_filter, exclude_working_directory_filter, full_match):
            continue
        uses_framework_library = any('#include "ProtocolTest.h"' in read_file(test_file_name)
                                     for test_file_name in test_file_names)
        multiple_test_tasks = get_opp_test_tasks(test_folder, simulation_project=simulation_project,
                                                 lib_folder=protocol_folder + "/lib" if uses_framework_library else None,
                                                 lib_name="protocoltest",
                                                 **dict(kwargs, name=f"protocol {suite_name} test", pass_keyboard_interrupt=True))
        if multiple_test_tasks.tasks:
            test_tasks.append(multiple_test_tasks)
    return MultipleTestTasks(tasks=test_tasks, **dict(kwargs, name="protocol test", start=None, end=None, concurrent=False))

def get_module_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/module", name="module test", **kwargs)

def get_unit_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/unit", name="unit test", **kwargs)

def get_all_test_tasks(**kwargs):
    test_task_functions = [
                           get_chart_test_tasks,
                           get_feature_test_tasks,
                           get_fingerprint_test_tasks,
                           get_module_test_tasks,
                           get_packet_test_tasks,
                           get_protocol_test_tasks,
                           get_queueing_test_tasks,
                           get_sanitizer_test_tasks,
                           get_smoke_test_tasks,
                           get_speed_test_tasks,
                           get_statistical_test_tasks,
                           get_unit_test_tasks,
                           get_validation_test_tasks
                          ]
    test_tasks = []
    for test_task_function in test_task_functions:
        multiple_test_tasks = test_task_function(**dict(kwargs, pass_keyboard_interrupt=True))
        if multiple_test_tasks.tasks:
            test_tasks.append(multiple_test_tasks)
    return MultipleTestTasks(tasks=test_tasks, **dict(kwargs, name="test group", start=None, end=None, concurrent=False))

def run_packet_tests(**kwargs):
    return get_packet_test_tasks(**kwargs).run(**kwargs)

def run_queueing_tests(**kwargs):
    return get_queueing_test_tasks(**kwargs).run(**kwargs)

def run_protocol_tests(**kwargs):
    return get_protocol_test_tasks(**kwargs).run(**kwargs)

def run_module_tests(**kwargs):
    return get_module_test_tasks(**kwargs).run(**kwargs)

def run_unit_tests(**kwargs):
    return get_unit_test_tasks(**kwargs).run(**kwargs)

def run_all_tests(**kwargs):
    return get_all_test_tasks(**kwargs).run(**kwargs)
