import logging

from inet.simulation import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

def check_statistical_results(simulation_result, **kwargs):
    # TODO
    return TestResult(result="PASS")

def run_statistical_test(simulation_config, **kwargs):
    simulation_result = run_simulation_config(simulation_config, print_end=" ", **kwargs)
    test_result = check_statistical_results(simulation_result, **kwargs)
    print(test_result.get_description())
    return test_result

def run_statistical_tests(**kwargs):
    logger.info("Running statistical tests")
