import glob
import logging

from inet.common import *
from inet.simulation.project import *
from inet.test.run import *

logger = logging.getLogger(__name__)

class OppTestRun(TestRun):
    def __init__(self, working_directory, test_file_name, **kwargs):
        self.working_directory = working_directory
        self.test_file_name = test_file_name

    def get_parameters_string(self, **kwargs):
        return self.test_file_name

    def run(self, cancel=False, index=None, count=None, print_end=" ", output_stream=sys.stdout, keyboard_interrupt_handler=None, **kwargs):
        executable = "sh"
        args = [executable, "runtest", self.test_file_name]
        print(("[" + str(index + 1) + "/" + str(count) + "] " if index is not None and count is not None else "") + "Running " + self.get_parameters_string(**kwargs), end=print_end, file=output_stream)
        logger.debug(args)
        if cancel or self.cancel:
            return TestResult(self, None)
        else:
            try:
                with EnabledKeyboardInterrupts(keyboard_interrupt_handler):
                    subprocess_result = subprocess.run(args, cwd=self.working_directory, capture_output=True)
                    stdout = subprocess_result.stdout.decode("utf-8")
                    match = re.match("Aggregate result: (.*?)", stdout)
                    result = None
                    if match:
                        result = match.group(1)
                    return TestResult(self, result=result, bool_result=subprocess_result.returncode == 0)
            except KeyboardInterrupt:
                return TestResult(self, None)

def run_opp_tests(test_folder, simulation_project=default_project, filter=".*", full_match=False, **kwargs):
    multiple_test_runs = None
    try:
        test_file_names = list(builtins.filter(lambda test_file_name: matches_filter(test_file_name, filter, None, full_match), glob.glob(os.path.join(simulation_project.get_full_path(test_folder), "*.test"))))
        test_runs = list(map(lambda test_file_name: OppTestRun(simulation_project.get_full_path(test_folder), os.path.basename(test_file_name), **kwargs), test_file_names))
        multiple_test_runs = MultipleTestRuns(test_runs, **kwargs)
        return multiple_test_runs.run(simulation_project=simulation_project, **kwargs)
    except KeyboardInterrupt:
        test_results = list(map(lambda test_run: TestResult(test_run, None, result="CANCEL", reason="Cancel by user"), multiple_test_runs.test_runs)) if multiple_test_runs else []
        return MultipleTestResults(multiple_test_runs, test_results)
