import logging

from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.leak import *
from inet.test.opp import *
from inet.test.regression import *
from inet.test.simulation import *
from inet.test.smoke import *
from inet.test.speed import *
from inet.test.statistical import *
from inet.test.validation import *

logger = logging.getLogger(__name__)

def run_packet_tests(**kwargs):
    # TODO
    pass

def run_queueing_tests(**kwargs):
    return run_opp_tests("tests/queueing", **kwargs)

def run_protocol_tests(**kwargs):
    return run_opp_tests("tests/protocol", **kwargs)

def run_module_tests(**kwargs):
    return run_opp_tests("tests/module", **kwargs)

def run_unit_tests(**kwargs):
    return run_opp_tests("tests/unit", **kwargs)

def run_all_tests(**kwargs):
    test_functions = [run_smoke_tests,
                      run_regression_tests,
                      run_validation_tests,
                      #run_leak_tests,
                      #run_speed_tests,
                      #run_feature_tests,
                      run_packet_tests,
                      run_queueing_tests,
                      run_protocol_tests,
                      run_module_tests,
                      run_unit_tests,
                      run_chart_tests]
    test_results = []
    for test_function in test_functions:
        multiple_test_results = test_function(**kwargs)
        print(multiple_test_results)
        test_results = test_results + multiple_test_results.test_results
    return MultipleTestResults(None, test_results)
