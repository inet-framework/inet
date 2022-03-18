import logging

from inet.test.fingerprint import *
from inet.test.statistical import *

logger = logging.getLogger(__name__)

def run_regression_tests(**kwargs):
    logger.info("Running regression tests")
    return run_fingerprint_tests(**kwargs)
    #run_statistical_tests(**kwargs)
