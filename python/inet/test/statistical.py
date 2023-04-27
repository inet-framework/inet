"""
This module provides functionality for statistical testing of multiple simulations.

The main function is :py:func:`run_statistical_tests`. It allows running multiple statistical tests
matching the provided filter criteria. Statistical tests check if scalar results of the simulations
are the same as the saved baseline results. The baseline results can be found in the statistics folder 
of the simulation project.  For the INET Framework the media folder can be found at
https://github.com/inet-framework/statistics in a separate GitHub repository.
"""

import difflib
import glob
import logging
import pandas
import re
import shutil

from omnetpp.scave.results import *

from inet.simulation import *
from inet.test.fingerprint import *
from inet.test.simulation import *

_logger = logging.getLogger(__name__)

def _read_scalar_result_file(file_name):
    df = read_result_files(file_name, include_fields_as_scalars=True)
    df = get_scalars(df)
    if "runID" in df:
        df.drop("runID", axis=1, inplace=True)
    return df

def _write_diff_file(a_file_name, b_file_name, diff_file_name):
    with open(a_file_name, "r") as file:
        a_file = file.readlines()
    with open(b_file_name, "r") as file:
        b_file = file.readlines()
    with open(diff_file_name, "w") as file:
        diff = "".join(difflib.unified_diff(a_file, b_file))
        file.write(diff)

class StatisticalTestTask(SimulationTestTask):
    def __init__(self, simulation_config=None, run_number=0, name="statistical test", task_result_class=SimulationTestTaskResult, **kwargs):
        super().__init__(simulation_task=SimulationTask(simulation_config=simulation_config, run_number=run_number, name=name, **kwargs), task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def get_scalar_file_name(self):
        simulation_config = self.simulation_task.simulation_config
        return f"{simulation_config.ini_file}-{simulation_config.config}-#{self.simulation_task.run_number}.sca"

    def check_simulation_task_result(self, simulation_task_result, result_name_filter=None, exclude_result_name_filter=None, result_module_filter=None, exclude_result_module_filter=None, full_match=False, **kwargs):
        simulation_config = self.simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        current_scalar_result_file_name = simulation_project.get_full_path(os.path.join(working_directory, "results", self.get_scalar_file_name()))
        stored_scalar_result_file_name = simulation_project.get_full_path(os.path.join(simulation_project.statistics_folder, working_directory, self.get_scalar_file_name()))
        _logger.debug(f"Reading result file {current_scalar_result_file_name}")
        current_df = _read_scalar_result_file(current_scalar_result_file_name)
        scalar_result_diff_file_name = re.sub(".sca", ".diff", stored_scalar_result_file_name)
        if os.path.exists(scalar_result_diff_file_name):
            os.remove(scalar_result_diff_file_name)
        if os.path.exists(stored_scalar_result_file_name):
            _logger.debug(f"Reading result file {stored_scalar_result_file_name}")
            stored_df = _read_scalar_result_file(stored_scalar_result_file_name)
            if not current_df.equals(stored_df):
                _write_diff_file(stored_scalar_result_file_name, current_scalar_result_file_name, scalar_result_diff_file_name)
                if current_df.empty:
                    return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", reason="Current statistical results are empty")
                elif stored_df.empty:
                    return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", reason="Stored statistical results are empty")
                else:
                    df = pandas.DataFrame()
                    df["module"] = stored_df["module"]
                    df["name"] = stored_df["name"]
                    df["stored_value"] = stored_df["value"]
                    df["current_value"] = current_df["value"]
                    df["absolute_error"] = df.apply(lambda row: abs(row["current_value"] - row["stored_value"]), axis=1)
                    df["relative_error"] = df.apply(lambda row: row["absolute_error"] / abs(row["stored_value"]) if row["stored_value"] != 0 else (float("inf") if row["current_value"] != 0 else 0), axis=1)
                    df = df[df.apply(lambda row: matches_filter(row["name"], result_name_filter, exclude_result_name_filter, full_match) and \
                                                 matches_filter(row["module"], result_module_filter, exclude_result_module_filter, full_match), axis=1)]
                    reason = df.loc[df["relative_error"].idxmax()].to_string()
                    reason = re.sub(" +", " = ", reason)
                    reason = re.sub("\\n", ", ", reason)
                    return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", reason=reason)
            else:
                return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="PASS")
        else:
            return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="ERROR", reason="Stored statistical results are not found")

def get_statistical_test_sim_time_limit(simulation_config, run_number=0):
    return simulation_config.sim_time_limit

def get_statistical_test_tasks(sim_time_limit=get_statistical_test_sim_time_limit, run_number=0, **kwargs):
    """
    Returns multiple statistical test tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:meth:`get_simulation_tasks <inet.simulation.task.get_simulation_tasks>` method.

    Returns (:py:class:`MultipleTestTasks`):
        an object that contains a list of :py:class:`StatisticalTestTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    return get_simulation_tasks(name="statistical test", run_number=run_number, sim_time_limit=sim_time_limit, simulation_task_class=StatisticalTestTask, multiple_simulation_tasks_class=MultipleSimulationTestTasks, **kwargs)

def run_statistical_tests(**kwargs):
    """
    Runs one or more statistical tests that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_statistical_test_tasks` function.

    Returns (:py:class:`MultipleSimulationTestTaskResults`):
        an object that contains a list of :py:class:`SimulationTestTaskResult` objects. Each object describes the result of running one test task.
    """
    multiple_statistical_test_tasks = get_statistical_test_tasks(**kwargs)
    return multiple_statistical_test_tasks.run(extra_args=["--**.param-recording=false", "--**.vector-recording=false", "--output-scalar-file=${resultdir}/${inifile}-${configname}-#${repetition}.sca"], **kwargs)

class StatisticalResultsUpdateTask(SimulationUpdateTask):
    def __init__(self, simulation_config=None, run_number=0, name="statistical results update", **kwargs):
        super().__init__(simulation_task=SimulationTask(simulation_config=simulation_config, run_number=run_number, name=name, **kwargs), simulation_config=simulation_config, run_number=run_number, name=name, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def get_scalar_file_name(self):
        simulation_config = self.simulation_task.simulation_config
        return f"{simulation_config.ini_file}-{simulation_config.config}-#{self.simulation_task.run_number}.sca"

    def run_protected(self, **kwargs):
        simulation_config = self.simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        target_results_directory = simulation_project.get_full_path(os.path.join(simulation_project.statistics_folder, working_directory))
        os.makedirs(target_results_directory, exist_ok=True)
        update_result = super().run_protected(**kwargs)
        if update_result.result == "INSERT" or update_result.result == "UPDATE":
            stored_scalar_result_file_name = simulation_project.get_full_path(os.path.join(working_directory, "results", self.get_scalar_file_name()))
            shutil.copy(stored_scalar_result_file_name, target_results_directory)
        return update_result

    def check_simulation_task_result(self, simulation_task_result, result_name_filter=None, exclude_result_name_filter=None, result_module_filter=None, exclude_result_module_filter=None, full_match=False, **kwargs):
        simulation_config = self.simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        current_scalar_result_file_name = simulation_project.get_full_path(os.path.join(working_directory, "results", self.get_scalar_file_name()))
        stored_scalar_result_file_name = simulation_project.get_full_path(os.path.join(simulation_project.statistics_folder, working_directory, self.get_scalar_file_name()))
        _logger.debug(f"Reading result file {current_scalar_result_file_name}")
        current_df = _read_scalar_result_file(current_scalar_result_file_name)
        scalar_result_diff_file_name = re.sub(".sca", ".diff", stored_scalar_result_file_name)
        if os.path.exists(scalar_result_diff_file_name):
            os.remove(scalar_result_diff_file_name)
        if not os.path.exists(stored_scalar_result_file_name):
            return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="INSERT")
        else:
            _logger.debug(f"Reading result file {stored_scalar_result_file_name}")
            stored_df = _read_scalar_result_file(stored_scalar_result_file_name)
            if not current_df.equals(stored_df):
                _write_diff_file(stored_scalar_result_file_name, current_scalar_result_file_name, scalar_result_diff_file_name)
                return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="UPDATE")
            else:
                return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="KEEP")

def get_update_statistical_result_tasks(run_number=0, **kwargs):
    """
    Returns multiple update statistical results tasks matching the provided filter criteria. The returned tasks can be run by
    calling the :py:meth:`run <inet.common.task.MultipleTasks.run>` method.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:meth:`get_simulation_tasks <inet.simulation.task.get_simulation_tasks>` method.

    Returns (:py:class:`MultipleUpdateTasks`):
        an object that contains a list of :py:class:`StatisticalResultsUpdateTask` objects matching the provided filter criteria.
        The result can be run (and re-run) without providing additional parameters.
    """
    return get_simulation_tasks(run_number=run_number, multiple_simulation_tasks_class=MultipleSimulationUpdateTasks, simulation_task_class=StatisticalResultsUpdateTask, **kwargs)

def update_statistical_results(sim_time_limit=get_statistical_test_sim_time_limit, **kwargs):
    """
    Updates the stored statistical results for one or more chart tests that match the provided filter criteria.

    Parameters:
        kwargs (dict):
            The filter criteria parameters are inherited from the :py:func:`get_update_statistical_result_tasks` function.

    Returns (:py:class:`MultipleUpdateTaskResults`):
        an object that contains a list of :py:class:`UpdateTaskResult` objects. Each object describes the result of running one update task.
    """
    multiple_update_statistical_result_tasks = get_update_statistical_result_tasks(sim_time_limit=sim_time_limit, **kwargs)
    return multiple_update_statistical_result_tasks.run(sim_time_limit=sim_time_limit, extra_args=["--**.param-recording=false", "--**.vector-recording=false", "--output-scalar-file=${resultdir}/${inifile}-${configname}-#${repetition}.sca"], **kwargs)
