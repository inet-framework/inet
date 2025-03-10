"""
This module provides functionality for chart testing of multiple simulations.

The main function is :py:func:`run_chart_tests`. It allows running multiple chart tests matching the
provided filter criteria. Chart tests check if charts of the result analysis are the same as the saved
baseline charts. The baseline charts can be found in the media folder of the simulation project. For
the INET Framework the media folder can be found at https://github.com/inet-framework/media in a separate
GitHub repository.
"""

import filecmp
import io
import logging
import matplotlib
import numpy
import importlib.util

import omnetpp
import omnetpp.scave
import omnetpp.scave.analysis

from inet.common import *
from inet.documentation.chart import *
from inet.test.statistical import *
from inet.test.task import *

_logger = logging.getLogger(__name__)


def sewar_rmse(a, b):
    assert a.shape == b.shape, "Supplied images have different sizes " + \
    str(a.shape) + " and " + str(b.shape)
    if len(a.shape) == 2:
        a = a[:,:,numpy.newaxis]
        b = b[:,:,numpy.newaxis]
    a = a.astype(numpy.float64)
    b = b.astype(numpy.float64)
    return numpy.sqrt(numpy.mean((a.astype(numpy.float64)-b.astype(numpy.float64))**2))

class ChartTestTask(TestTask):
    def __init__(self, analysis_file_name, id, chart_name, simulation_project=None, name="chart test", **kwargs):
        super().__init__(name=name, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.analysis_file_name = analysis_file_name
        self.id = id
        self.chart_name = chart_name
        self.simulation_project = simulation_project

    def get_parameters_string(self, **kwargs):
        return self.analysis_file_name + ": " + self.chart_name

    def run_protected(self, keep_charts=True, output_stream=sys.stdout, **kwargs):
        workspace = omnetpp.scave.analysis.Workspace(get_workspace_path("."), [])
        analysis = omnetpp.scave.analysis.load_anf_file(self.simulation_project.get_full_path(self.analysis_file_name))
        for chart in analysis.collect_charts():
            if chart.id == self.id:
                image_export_filename = chart.properties["image_export_filename"]
                if image_export_filename is None or image_export_filename == "":
                    return self.task_result_class(self, result="SKIP", expected_result="SKIP", reason="Chart file name is not specified")
                folder = os.path.dirname(self.simulation_project.get_full_path(self.analysis_file_name))
                file_name = analysis.export_image(chart, folder, workspace, format="png", dpi=150, target_folder=self.simulation_project.media_folder, filename=image_export_filename + "-new")
                new_file_name = os.path.join(folder, file_name)
                old_file_name = os.path.join(folder, re.sub(r"-new\.png$", ".png", file_name))
                diff_file_name = os.path.join(folder, re.sub(r"-new\.png$", "-diff.png", file_name))
                if os.path.exists(diff_file_name):
                    os.remove(diff_file_name)
                if os.path.exists(old_file_name):
                    new_image = matplotlib.image.imread(new_file_name)
                    old_image = matplotlib.image.imread(old_file_name)
                    if old_image.shape != new_image.shape:
                        return self.task_result_class(self, result="FAIL", reason="Supplied images have different sizes" + str(old_image.shape) + " and " + str(new_image.shape))
                    metric = sewar_rmse(old_image, new_image)
                    if metric == 0 or not keep_charts:
                        os.remove(new_file_name)
                    else:
                        image_diff = numpy.abs(new_image - old_image)
                        matplotlib.image.imsave(diff_file_name, image_diff[:, :, :3])
                    result = "PASS" if metric == 0 else "FAIL"
                    reason = "Metric: " + str(metric) if result == "FAIL" else None
                    return self.task_result_class(self, result=result, reason=reason)
                else:
                    return self.task_result_class(self, result="FAIL", reason="Baseline chart not found")
        return self.task_result_class(self, result="ERROR", reason="Chart not found")

def get_chart_test_sim_time_limit(simulation_config, run=0):
    return simulation_config.sim_time_limit

class MultipleChartTestTasks(MultipleTestTasks):
    def __init__(self, multiple_simulation_tasks=None, name="chart test", multiple_task_results_class=MultipleTestTaskResults, **kwargs):
        super().__init__(name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.multiple_simulation_tasks = multiple_simulation_tasks

    def run_protected(self, **kwargs):
        multiple_simulation_task_results = self.multiple_simulation_tasks.run_protected(**kwargs)
        # avoid reusing the processes from the process pool because matplotlib can generate different images due to tight layout
        return super().run_protected(**kwargs, maxtasksperchild=1)

def get_chart_test_tasks(simulation_project=None, run_simulations=True, filter="showcases", working_directory_filter=None, chart_filter=None, exclude_chart_filter=None, **kwargs):
    """
    Returns multiple chart test tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            TODO

    Returns (:py:class:`MultipleTestTasks`):
        an object that contains a list of :py:class:`ChartTestTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    test_tasks = []
    simulation_tasks = []
    for analysis_file_name in get_analysis_files(simulation_project=simulation_project, filter=filter or working_directory_filter, **kwargs):
        analysis = omnetpp.scave.analysis.load_anf_file(simulation_project.get_full_path(analysis_file_name))
        for chart in analysis.collect_charts():
            if matches_filter(chart.name, chart_filter, exclude_chart_filter, False):
                folder = os.path.dirname(simulation_project.get_full_path(analysis_file_name))
                working_directory = os.path.relpath(folder, simulation_project.get_full_path("."))
                multiple_simulation_tasks = get_simulation_tasks(simulation_project=simulation_project, working_directory_filter=working_directory, sim_time_limit=get_chart_test_sim_time_limit, **kwargs)
                if run_simulations:
                    for simulation_task in multiple_simulation_tasks.tasks:
                        if not list(builtins.filter(lambda element: element.simulation_config == simulation_task.simulation_config and element.run_number == simulation_task.run_number, simulation_tasks)):
                            simulation_tasks.append(simulation_task)
                if multiple_simulation_tasks.tasks:
                    test_tasks.append(ChartTestTask(simulation_project=simulation_project, analysis_file_name=analysis_file_name, id=chart.id, chart_name=chart.name, task_result_class=TestTaskResult))
    return MultipleChartTestTasks(tasks=test_tasks, multiple_simulation_tasks=MultipleSimulationTasks(tasks=simulation_tasks, simulation_project=simulation_project, **kwargs), **dict(kwargs, scheduler="process"))

def run_chart_tests(**kwargs):
    """
    Runs one or more chart tests that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_chart_test_tasks` function.

    Returns (:py:class:`MultipleTestTaskResults`):
        an object that contains a list of :py:class:`TestTaskResult` objects. Each object describes the result of running one test task.
    """
    multiple_chart_test_tasks = get_chart_test_tasks(**kwargs)
    return multiple_chart_test_tasks.run(**kwargs)

class ChartUpdateTask(UpdateTask):
    def __init__(self, simulation_project, analysis_file_name, id, chart_name, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project
        self.analysis_file_name = analysis_file_name
        self.id = id
        self.chart_name = chart_name

    def get_parameters_string(self, **kwargs):
        return self.analysis_file_name + ": " + self.chart_name

    def run_protected(self, keep_charts=True, **kwargs):
        workspace = omnetpp.scave.analysis.Workspace(get_workspace_path("."), [])
        analysis = omnetpp.scave.analysis.load_anf_file(self.simulation_project.get_full_path(self.analysis_file_name))
        for chart in analysis.collect_charts():
            if chart.id == self.id:
                image_export_filename = chart.properties["image_export_filename"]
                if image_export_filename is None or image_export_filename == "":
                    return self.task_result_class(self, result="SKIP", expected_result="SKIP", reason="Chart file name is not specified")
                folder = os.path.dirname(self.simulation_project.get_full_path(self.analysis_file_name))
                file_name = analysis.export_image(chart, folder, workspace, format="png", dpi=150, target_folder=self.simulation_project.media_folder, filename=image_export_filename + "-new")
                new_file_name = os.path.join(folder, file_name)
                old_file_name = os.path.join(folder, re.sub(r"-new\.png$", ".png", file_name))
                diff_file_name = os.path.join(folder, re.sub(r"-new\.png$", "-diff.png", file_name))
                if os.path.exists(diff_file_name):
                    os.remove(diff_file_name)
                if os.path.exists(old_file_name):
                    new_image = matplotlib.image.imread(new_file_name)
                    old_image = matplotlib.image.imread(old_file_name)
                    if old_image.shape != new_image.shape:
                        metric = 1
                    else:
                        metric = sewar_rmse(old_image, new_image)
                    if metric == 0:
                        os.remove(new_file_name)
                    else:
                        if keep_charts:
                            os.rename(old_file_name, re.sub(r"-new\.png$", "-old.png", file_name))
                            image_diff = numpy.abs(new_image - old_image)
                            matplotlib.image.imsave(diff_file_name, image_diff[:, :, :3])
                        else:
                            os.remove(old_file_name)
                        os.rename(new_file_name, old_file_name)
                    return self.task_result_class(self, result="KEEP" if metric == 0 else "UPDATE")
                else:
                    os.rename(new_file_name, old_file_name)
                    return self.task_result_class(self, result="INSERT")
        return self.task_result_class(self, result="ERROR", reason="Chart not found")

class MultipleChartUpdateTasks(MultipleUpdateTasks):
    def __init__(self, multiple_simulation_tasks=None, name="chart update", multiple_task_results_class=MultipleUpdateTaskResults, **kwargs):
        super().__init__(name=name, multiple_task_results_class=multiple_task_results_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.multiple_simulation_tasks = multiple_simulation_tasks

    def run_protected(self, **kwargs):
        multiple_simulation_task_results = self.multiple_simulation_tasks.run_protected(**kwargs)
        # avoid reusing the processes from the process pool because matplotlib can generate different images due to tight layout
        return super().run_protected(**kwargs, maxtasksperchild=1)

def get_update_chart_tasks(simulation_project=None, run_simulations=True, filter=None, working_directory_filter=None, chart_filter=None, exclude_chart_filter=None, **kwargs):
    """
    Returns multiple update chart tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            TODO

    Returns (:py:class:`MultipleUpdateTasks`):
        an object that contains a list of :py:class:`ChartUpdateTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    update_tasks = []
    simulation_tasks = []
    for analysis_file_name in get_analysis_files(simulation_project=simulation_project, filter=filter or working_directory_filter, **kwargs):
        analysis = omnetpp.scave.analysis.load_anf_file(simulation_project.get_full_path(analysis_file_name))
        for chart in analysis.collect_charts():
            if matches_filter(chart.name, chart_filter, exclude_chart_filter, False):
                folder = os.path.dirname(simulation_project.get_full_path(analysis_file_name))
                working_directory = os.path.relpath(folder, simulation_project.get_full_path("."))
                if run_simulations:
                    multiple_simulation_tasks = get_simulation_tasks(simulation_project=simulation_project, working_directory_filter=working_directory, sim_time_limit=get_chart_test_sim_time_limit, **kwargs)
                    for simulation_task in multiple_simulation_tasks.tasks:
                        if not list(builtins.filter(lambda element: element.simulation_config == simulation_task.simulation_config and element.run_number == simulation_task.run_number, simulation_tasks)):
                            simulation_tasks.append(simulation_task)
                update_tasks.append(ChartUpdateTask(simulation_project=simulation_project, analysis_file_name=analysis_file_name, id=chart.id, chart_name=chart.name, task_result_class=UpdateTaskResult))
    return MultipleChartUpdateTasks(tasks=update_tasks, multiple_simulation_tasks=MultipleSimulationTasks(tasks=simulation_tasks, simulation_project=simulation_project, **kwargs), **dict(kwargs, scheduler="process"))

def update_charts(simulation_project=None, **kwargs):
    """
    Updates the stored charts for one or more chart tests that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_update_chart_tasks` function.

    Returns (:py:class:`MultipleUpdateTaskResults`):
        an object that contains a list of :py:class:`UpdateTaskResult` objects. Each object describes the result of running one update task.
    """
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    multiple_update_chart_tasks = get_update_chart_tasks(**kwargs)
    return multiple_update_chart_tasks.run(**kwargs)
