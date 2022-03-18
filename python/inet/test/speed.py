import logging

from inet.simulation import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

def check_running_time(simulation_result, **kwargs):
    # TODO
    return TestResult(result="PASS")

def run_speed_test(simulation_config, **kwargs):
    simulation_result = run_simulation_config(simulation_config, print_end=" ", **kwargs)
    test_result = check_running_time(simulation_result, **kwargs)
    print(test_result.get_description())
    return test_result

def run_speed_tests(**kwargs):
    logger.info("Running speed tests")
    return run_tests(run_speed_test, **kwargs)
