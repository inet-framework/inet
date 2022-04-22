import logging

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

logger = logging.getLogger(__name__)

class PacketTestTask(TestTask):
    def __init__(self, simulation_project, name="packet test", task_result_class=TestTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.name

    def run_protected(self, **kwargs):
        executable = "./runtest"
        working_directory = self.simulation_project.get_full_path("tests/packet")
        args = [executable, "-s"]
        logger.debug(args)
        subprocess_result = subprocess.run(args, cwd=working_directory, capture_output=True, env=self.simulation_project.get_env())
        stdout = subprocess_result.stdout.decode("utf-8")
        match = re.search(r"Packet unit test: (\w+)", stdout)
        return self.task_result_class(self, result=match.group(1) if match and subprocess_result.returncode == 0 else "FAIL")

def get_packet_test_tasks(filter=None, working_directory_filter=None, ini_file_filter=None, config_filter=None, run_filter=None, **kwargs):
    if filter or working_directory_filter or ini_file_filter or config_filter or run_filter:
        packet_test_tasks = []
    else:
        packet_test_tasks = [PacketTestTask(inet_project)]
    return MultipleTestTasks(tasks=packet_test_tasks, name="packet test", **kwargs)

def get_queueing_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/queueing", name="queueing test", **kwargs)

def get_protocol_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/protocol", name="protocol test", **kwargs)

def get_module_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/module", name="module test", **kwargs)

def get_unit_test_tasks(**kwargs):
    return get_opp_test_tasks("tests/unit", name="unit test", **kwargs)

def get_all_test_tasks(**kwargs):
    test_task_functions = [get_smoke_test_tasks,
                           get_sanitizer_test_tasks,
                           get_fingerprint_test_tasks,
                           get_statistical_test_tasks,
                           get_validation_test_tasks,
                           #get_speed_test_tasks,
                           #get_feature_test_tasks,
                           get_packet_test_tasks,
                           get_queueing_test_tasks,
                           get_protocol_test_tasks,
                           get_module_test_tasks,
                           get_unit_test_tasks,
                           get_chart_test_tasks]
    test_tasks = []
    for test_task_function in test_task_functions:
        multiple_test_tasks = test_task_function(**kwargs)
        if multiple_test_tasks.tasks:
            test_tasks.append(multiple_test_tasks)
    return MultipleTestTasks(test_tasks, name="test group", **dict(kwargs, concurrent=False))

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
