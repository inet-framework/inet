import argparse
import logging
import sys

logger = logging.getLogger(__name__)

from omnetpp.main import *

from inet.simulation.project import *
from inet.test import *

def run_chart_tests_main():
    run_tasks_main(run_chart_tests, "chart tests")

def run_sanitizer_tests_main():
    run_tasks_main(run_sanitizer_tests, "sanitizer tests")

def run_module_tests_main():
    run_tasks_main(run_module_tests, "module tests")

def run_packet_tests_main():
    run_tasks_main(run_packet_tests, "packet tests")

def run_protocol_tests_main():
    run_tasks_main(run_protocol_tests, "protocol tests")

def run_queueing_tests_main():
    run_tasks_main(run_queueing_tests, "queueing tests")

def run_speed_tests_main():
    run_tasks_main(run_speed_tests, "speed tests")

def run_statistical_tests_main():
    run_tasks_main(run_statistical_tests, "statistical tests")

def run_unit_tests_main():
    run_tasks_main(run_unit_tests, "unit tests")

def run_validation_tests_main():
    run_tasks_main(run_validation_tests, "validation tests")

def run_all_tests_main():
    run_tasks_main(run_all_tests, "tests")
