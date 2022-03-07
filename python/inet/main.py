import argparse
import logging
import sys

from inet.simulation.project import *
from inet.simulation.run import *
from inet.test import *
from inet.test.fingerprint.run import *
from inet.test.smoke import *

def parse_arguments(task):
    description = "Runs all " + task + " in the enclosing project recursively from the current working directory"
    parser = argparse.ArgumentParser(description=description)
    # TODO not all of these arguments are meaningful for each cases
    #      it must be carefully checked and updated accordingly
    #
    # parser.add_argument("-f", "--fpfilter", default=list(), action="append", metavar="ingredient", help="fingerprint ingredient filter (expected values after the "/" character)")
    # parser.add_argument("-F", "--excludefpfilter", default=list(), action="append", metavar="ingredient", help="fingerprint ingredient excluding filter (excluded values after the "/" character)")
    # parser.add_argument("-m", "--match", action="append", metavar="regex", help="Line filter: a line (more precisely, workingdir+SPACE+args) must match any of the regular expressions in order for that test case to be run")
    # parser.add_argument("-x", "--exclude", action="append", metavar="regex", help="Negative line filter: a line (more precisely, workingdir+SPACE+args) must NOT match any of the regular expressions in order for that test case to be run")
    # parser.add_argument("-t", "--threads", type=int, default=defaultNumThreads, help="number of parallel threads (default: number of CPUs, currently "+str(defaultNumThreads)+")")
    # parser.add_argument("-r", "--repeat", type=int, default=1, help="number of repeating each test (default: 1)")
    # parser.add_argument("-e", "--executable", default="inet", help="Determines which binary to execute (e.g. opp_run_dbg, opp_run_release) if the command column in the CSV file does not specify one.")
    # parser.add_argument("-C", "--directory", help="Change to DIRECTORY before executing the tests. Working dirs in the CSV files are relative to this.")
    # parser.add_argument("-d", "--debug", action="store_true", help="Run debug executables: use the debug version of the executable (appends _dbg to the executable name)")
    # parser.add_argument("-s", "--release", action="store_true", help="Run release executables: use the release version of the executable (appends _release to the executable name)")
    # parser.add_argument("-q", "--disable_outfile", dest="writeOutfile", action="store_false", help="disable the writing standard output of simulations to result directory")
    # parser.add_argument("-c", "--calculator", default=defaultFingerprintCalculator, help="Set the fingerprintcalculator-class. Empty value means the original OmNET++ fingerprint calculator")
    # parser.add_argument("-a", "--oppargs", action="append", metavar="oppargs", nargs=argparse.REMAINDER, help="extra opp_run arguments until the end of the line")
    # parser.add_argument("-n", type=int, default=1, help="Split the selected test cases into n sets, and only run one of these sets")
    # parser.add_argument("-i", type=int, default=0, help="Which set of test cases to run [0..n-1]")
    parser.add_argument("-l", "--log-level", choices=["ERROR", "WARN", "INFO", "DEBUG"], default="WARN", help="Verbose output mode")
    parser.add_argument("--concurrent", default=True, action=argparse.BooleanOptionalAction, help="Concurrent execution")
    parser.add_argument("--dry-run", default=False, action=argparse.BooleanOptionalAction, help="Display what would be done but doesn't actually do anything")
    parser.add_argument("-m", "--mode", choices=["debug", "release"], help="Specifies the build mode of the executable")
    parser.add_argument("--build", default=True, action=argparse.BooleanOptionalAction, help="Build executable")
    parser.add_argument("-u", "--user-interface", choices=["Cmdenv", "Qtenv"], default="Cmdenv", help="User interface")
    parser.add_argument("-t", "--sim-time-limit", default=None, help="Simulation time limit")
    parser.add_argument("-T", "--cpu-time-limit", default=None, help="CPU time limit")
    parser.add_argument("-f", "--filter", default=None, help="Filter")
    parser.add_argument("--exclude-filter", default=None, help="Exclude filter")
    parser.add_argument("-w", "--working-directory-filter", default=None, help="Working directory filter")
    parser.add_argument("--exclude-working-directory-filter", default=None, help="Exclude working directory filter")
    parser.add_argument("-i", "--ini-file-filter", default=None, help="INI file filter")
    parser.add_argument("--exclude-ini-file-filter", default=None, help="Exclude INI file filter")
    parser.add_argument("-c", "--config-filter", default=None, help="Config filter")
    parser.add_argument("--exclude-config-filter", default=None, help="Exclude config filter")
    return parser.parse_args(sys.argv[1:])

def process_arguments(task):
    args = parse_arguments(task)
    logging.getLogger().setLevel(args.log_level)
    kwargs = {k: v for k, v in vars(args).items() if v is not None}
    kwargs["working_directory_filter"] = args.working_directory_filter or os.path.relpath(os.getcwd(), inet_project.get_full_path("."))
    kwargs["working_directory_filter"] = re.sub("(.*)/$", "\\1", kwargs["working_directory_filter"])
    return kwargs

def run_main(main_function, task_name):
    try:
        result = main_function(**process_arguments(task_name))
        print(result)
        sys.exit(0 if (result is None or
                       (hasattr(result, "is_all_done") and result.is_all_done()) or
                       (hasattr(result, "is_all_pass") and result.is_all_pass()))
                   else 1)
    except KeyboardInterrupt:
        print("Program interrupted by user")

def run_simulations_main():
    run_main(run_simulations, "simulations")

def run_leak_tests_main():
    run_main(run_leak_tests, "leak tests")

def run_fingerprint_tests_main():
    run_main(run_fingerprint_tests, "fingerprint tests")

def run_regression_tests_main():
    run_main(run_regression_tests, "regression tests")

def run_smoke_tests_main():
    run_main(run_smoke_tests, "smoke tests")

def run_speed_tests_main():
    run_main(run_speed_tests, "speed tests")

def run_statistical_tests_main():
    run_main(run_statistical_tests, "statistical tests")

def run_validation_tests_main():
    run_main(run_validation_tests, "validation tests")

def run_all_tests_main():
    run_main(run_all_tests, "tests")
