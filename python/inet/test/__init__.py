import logging

logging.basicConfig()
logging.getLogger().setLevel(logging.WARN)

from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.leak import *
from inet.test.regression import *
from inet.test.run import *
from inet.test.smoke import *
from inet.test.speed import *
from inet.test.statistical import *
from inet.test.validation import *

def run_all_tests(**kwargs):
    run_smoke_tests(**kwargs)
    run_regression_tests(**kwargs)
    run_validation_tests(**kwargs)
    run_leak_tests(**kwargs)
    run_speed_tests(**kwargs)
    run_feature_tests(**kwargs)
