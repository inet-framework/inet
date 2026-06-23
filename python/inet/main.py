"""
Command-line entry points for the INET-specific tests that opp_repl does not provide.

The generic run/build/test commands are provided by the opp_repl tooling itself
(``opp_run_*``, ``opp_build_project``, ``opp_repl``). This module only wraps the
INET-specific test types -- validation, packet, protocol, queueing, module and unit
tests -- and the INET-flavored aggregate runners (all, release), reusing opp_repl's
argument parsing and task-running machinery.

Each entry point loads the bundled ``.opp`` project descriptors (so the INET project
is registered and auto-selected from the working directory) and then delegates to
:py:func:`opp_repl.main.run_tasks_main`.
"""

from opp_repl import load_opp_file
from opp_repl.main import run_tasks_main

from inet.test import *

__sphinx_mock__ = True # ignore this module in documentation

def _run_inet_tasks_main(main_function, task_name):
    load_opp_file("@opp")
    run_tasks_main(main_function, task_name)

def run_validation_tests_main():
    _run_inet_tasks_main(run_validation_tests, "validation tests")

def run_packet_tests_main():
    _run_inet_tasks_main(run_packet_tests, "packet tests")

def run_protocol_tests_main():
    _run_inet_tasks_main(run_protocol_tests, "protocol tests")

def run_queueing_tests_main():
    _run_inet_tasks_main(run_queueing_tests, "queueing tests")

def run_module_tests_main():
    _run_inet_tasks_main(run_module_tests, "module tests")

def run_unit_tests_main():
    _run_inet_tasks_main(run_unit_tests, "unit tests")

def run_all_tests_main():
    _run_inet_tasks_main(run_all_tests, "all tests")

def run_release_tests_main():
    _run_inet_tasks_main(run_release_tests, "release tests")
