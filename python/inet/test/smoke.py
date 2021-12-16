import logging

from inet.simulation import *
from inet.test.run import *

logger = logging.getLogger(__name__)

def check_smoke_test(test_run, simulation_result, **kwargs):
    # TODO check simulation_result.elapsed_cpu_time
    return check_test(test_run, simulation_result, **kwargs)

def get_smoke_tests(cpu_time_limit="3s", **kwargs):
    return get_tests(check_test_function=check_smoke_test, cpu_time_limit=cpu_time_limit, **kwargs)

def run_smoke_tests(**kwargs):
    logger.info("Running smoke tests")
    multiple_test_runs = get_smoke_tests(**kwargs)
    return multiple_test_runs.run()
