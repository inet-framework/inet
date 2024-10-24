import argparse
import logging
import sys

logger = logging.getLogger(__name__)

from inet.simulation.project import *
from inet.simulation.task import *
from inet.test import *

def parse_arguments(task_name):
    description = "Runs all " + task_name + " in the enclosing project recursively from the current working directory"
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("-l", "--log-level", choices=["ERROR", "WARN", "INFO", "DEBUG"], default="WARN", help="Verbose output mode")
    parser.add_argument('--concurrent', action='store_true', help="Concurrent execution")
    parser.add_argument('--no-concurrent', dest='concurrent', action='store_false')
    parser.add_argument("--dry-run", action="store_true", help="Display what would be done but doesn't actually do anything")
    parser.add_argument("-m", "--mode", choices=["debug", "release"], help="Specifies the build mode of the executable")
    parser.add_argument('--build', action='store_true', help="Build executable")
    parser.add_argument('--no-build', dest='build', action='store_false')
    parser.add_argument("-u", "--user-interface", choices=["Cmdenv", "Qtenv"], default="Cmdenv", help="User interface")
    parser.add_argument("-t", "--sim-time-limit", default=None, help="Simulation time limit")
    parser.add_argument("-T", "--cpu-time-limit", default=None, help="CPU time limit")
    parser.add_argument('--start', default=None, help="First task index")
    parser.add_argument('--end', default=None, help="Last task index")
    parser.add_argument("-f", "--filter", default=None, help="Filter")
    parser.add_argument("--exclude-filter", default=None, help="Exclude filter")
    parser.add_argument("-w", "--working-directory-filter", default=None, help="Working directory filter")
    parser.add_argument("--exclude-working-directory-filter", default=None, help="Exclude working directory filter")
    parser.add_argument("-i", "--ini-file-filter", default=None, help="INI file filter")
    parser.add_argument("--exclude-ini-file-filter", default=None, help="Exclude INI file filter")
    parser.add_argument("-c", "--config-filter", default=None, help="Config filter")
    parser.add_argument("--exclude-config-filter", default=None, help="Exclude config filter")
    parser.set_defaults(concurrent=True, build=True)
    return parser.parse_args(sys.argv[1:])

def process_arguments(task):
    args = parse_arguments(task)
    logger = logging.getLogger()
    logger.setLevel(args.log_level)
    handler = logging.StreamHandler()
    handler.setLevel(args.log_level)
    handler.setFormatter(ColoredLoggingFormatter())
    logger.handlers = []
    logger.addHandler(handler)
    kwargs = {k: v for k, v in vars(args).items() if v is not None}
    kwargs["working_directory_filter"] = args.working_directory_filter or os.path.relpath(os.getcwd(), os.path.realpath(inet_project.get_full_path(".")))
    kwargs["working_directory_filter"] = re.sub(r"(.*)/$", "\\1", kwargs["working_directory_filter"])
    return kwargs

def run_main(main_function, task_name):
    try:
        result = main_function(**process_arguments(task_name))
        print(result)
        sys.exit(0 if (result is None or result.is_all_results_expected()) else 1)
    except KeyboardInterrupt:
        logger.warn("Program interrupted by user")

def run_simulations_main():
    run_main(run_simulations, "simulations")

def run_chart_tests_main():
    run_main(run_chart_tests, "chart tests")

def run_fingerprint_tests_main():
    run_main(run_fingerprint_tests, "fingerprint tests")

def run_sanitizer_tests_main():
    run_main(run_sanitizer_tests, "sanitizer tests")

def run_module_tests_main():
    run_main(run_module_tests, "module tests")

def run_packet_tests_main():
    run_main(run_packet_tests, "packet tests")

def run_protocol_tests_main():
    run_main(run_protocol_tests, "protocol tests")

def run_queueing_tests_main():
    run_main(run_queueing_tests, "queueing tests")

def run_smoke_tests_main():
    run_main(run_smoke_tests, "smoke tests")

def run_speed_tests_main():
    run_main(run_speed_tests, "speed tests")

def run_statistical_tests_main():
    run_main(run_statistical_tests, "statistical tests")

def run_unit_tests_main():
    run_main(run_unit_tests, "unit tests")

def run_validation_tests_main():
    run_main(run_validation_tests, "validation tests")

def run_all_tests_main():
    run_main(run_all_tests, "all tests")

def run_release_tests_main():
    run_main(run_release_tests, "release tests")
