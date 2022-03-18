import logging

from inet.common import *
from inet.documentation.chart import *
from inet.test.run import *

logger = logging.getLogger(__name__)

class ChartTestRun(TestRun):
    def __init__(self, simulation_project, analysis_file_name, chart_name, **kwargs):
        self.simulation_project = simulation_project
        self.analysis_file_name = analysis_file_name
        self.chart_name = chart_name

    def get_parameters_string(self, **kwargs):
        return self.analysis_file_name + ": " + self.chart_name

    def run(self, cancel=False, index=None, count=None, print_end=" ", output_stream=sys.stdout, keyboard_interrupt_handler=None, **kwargs):
        print(("[" + str(index + 1) + "/" + str(count) + "] " if index is not None and count is not None else "") + "Running " + self.get_parameters_string(**kwargs), end=print_end, file=output_stream)
        if cancel or self.cancel:
            return TestResult(self, None)
        else:
            try:
                with EnabledKeyboardInterrupts(keyboard_interrupt_handler):
                    workspace = omnetpp.scave.analysis.Workspace(omnetpp.scave.analysis.Workspace.find_workspace(get_workspace_path(".")), [])
                    analysis = omnetpp.scave.analysis.load_anf_file(self.simulation_project.get_full_path(self.analysis_file_name))
                    for chart in analysis.collect_charts():
                        if chart.name == self.chart_name:
                            folder = os.path.dirname(self.simulation_project.get_full_path(self.analysis_file_name))
                            analysis.export_image(chart, folder, workspace, format="png", dpi=150, target_folder="doc/media")
                            return TestResult(self, result="PASS")
                return TestResult(self, result="ERROR")
            except KeyboardInterrupt:
                return TestResult(self, None)
            except Exception as e:
                return TestResult(self, result="FAIL", reason=str(e))

def run_chart_tests(simulation_project=default_project, pool_class=multiprocessing.Pool, **kwargs):
    multiple_test_runs = None
    try:
        test_runs = []
        for analysis_file_name in get_analysis_files(simulation_project=simulation_project, pool_class=pool_class, **kwargs):
            analysis = omnetpp.scave.analysis.load_anf_file(simulation_project.get_full_path(analysis_file_name))
            for chart in analysis.collect_charts():
                test_runs.append(ChartTestRun(simulation_project, analysis_file_name, chart.name))
        multiple_test_runs = MultipleTestRuns(test_runs, simulation_project=simulation_project, pool_class=pool_class, **kwargs)
        return multiple_test_runs.run(simulation_project=simulation_project, pool_class=pool_class, **kwargs)
    except KeyboardInterrupt:
        test_results = list(map(lambda test_run: TestResult(test_run, None, result="CANCEL", reason="Cancel by user"), multiple_test_runs.test_runs)) if multiple_test_runs else []
        return MultipleTestResults(multiple_test_runs, test_results)
