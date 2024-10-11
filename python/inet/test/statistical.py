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

logger = logging.getLogger(__name__)

class StatisticalTestTask(SimulationTestTask):
    def __init__(self, simulation_config, run, name="statistical test", task_result_class=SimulationTestTaskResult, **kwargs):
        super().__init__(SimulationTask(simulation_config=simulation_config, run=run, name=name, **kwargs), task_result_class=task_result_class, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def check_simulation_task_result(self, simulation_task_result, result_name_filter=None, exclude_result_name_filter=None, result_module_filter=None, exclude_result_module_filter=None, full_match=False, **kwargs):
        simulation_task = simulation_task_result.task
        simulation_config = simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        config = simulation_config.config
        current_results_directory = simulation_project.get_full_path(os.path.join(working_directory, "results"))
        stored_results_directory = simulation_project.get_full_path(os.path.join("statistics", working_directory))
        scalars_match = False
        for current_scalar_result_file_name in glob.glob(os.path.join(current_results_directory, "*.sca")):
            if re.search(r"/" + config + "-#", current_scalar_result_file_name):
                logger.debug(f"Reading result file {current_scalar_result_file_name}")
                current_df = read_result_files(current_scalar_result_file_name)
                current_df = get_scalars(current_df)
                if "runID" in current_df:
                    current_df.drop("runID", axis=1,  inplace=True)
                scalar_file_name = os.path.basename(current_scalar_result_file_name)
                stored_scalar_result_file_name = os.path.join(stored_results_directory, scalar_file_name)
                if os.path.isfile(stored_scalar_result_file_name):
                    logger.debug(f"Reading result file {stored_scalar_result_file_name}")
                    stored_df = read_result_files(stored_scalar_result_file_name)
                    stored_df = get_scalars(stored_df)
                    if "runID" in stored_df:
                        stored_df.drop("runID", axis=1,  inplace=True)
                    scalars_match = current_df.equals(stored_df)
                    if not scalars_match:
                        with open(current_scalar_result_file_name, "r") as file:
                            current_scalar_result_file = file.readlines()
                        with open(stored_scalar_result_file_name, "r") as file:
                            stored_scalar_result_file = file.readlines()
                        scalar_result_diff_file_name = re.sub(r".sca", ".diff", stored_scalar_result_file_name)
                        with open(scalar_result_diff_file_name, "w") as file:
                            scalar_diff = "".join(difflib.ndiff(current_scalar_result_file, stored_scalar_result_file))
                            file.write(scalar_diff)
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
                            reason = re.sub(r" +", " = ", reason)
                            reason = re.sub(r"\n", ", ", reason)
                            return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", reason=reason)
                else:
                    return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="ERROR", reason="Stored statistical results are not found")
        return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="PASS")
    
def get_statistical_result_sim_time_limit(simulation_config, run=0):
    simulation_project = simulation_config.simulation_project
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_project)
    stored_fingerprint_entries = correct_fingerprint_store.filter_entries(ingredients=None, working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=run)
    if stored_fingerprint_entries:
        selected_fingerprint_entry = select_fingerprint_entry_with_largest_sim_time_limit(stored_fingerprint_entries)
        return selected_fingerprint_entry["sim_time_limit"]
    elif simulation_config.sim_time_limit is not None:
        return simulation_config.sim_time_limit
    else:
        # NOTE: 0s means running the simulations indefinitely, 1ps isn't really good either,
        # because depending on the simtime precision it may be silently rounded to zero
        return "1ps"

def get_statistical_test_tasks(sim_time_limit=get_statistical_result_sim_time_limit, **kwargs):
    # remove run=0 parameter and add the same to the github-job-inet_run_statistical-tests.sh
    return get_simulation_tasks(name="statistical test", run=0, sim_time_limit=sim_time_limit, simulation_task_class=StatisticalTestTask, multiple_simulation_tasks_class=MultipleSimulationTestTasks, **kwargs)

def run_statistical_tests(**kwargs):
    multiple_statistical_test_tasks = get_statistical_test_tasks(**kwargs)
    return multiple_statistical_test_tasks.run(extra_args=["--**.param-recording=false"], **kwargs)

class StatisticalResultsUpdateTask(SimulationTask):
    def __init__(self, simulation_config, run, name="statistical results update", **kwargs):
        super().__init__(simulation_config=simulation_config, run=run, name=name, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def run_protected(self, **kwargs):
        update_result = super().run_protected(**kwargs)
        simulation_project = self.simulation_config.simulation_project
        working_directory = self.simulation_config.working_directory
        source_results_directory = simulation_project.get_full_path(os.path.join(working_directory, "results"))
        target_results_directory = simulation_project.get_full_path(os.path.join("statistics", working_directory))
        if not os.path.exists(target_results_directory):
            os.makedirs(target_results_directory)
        for scalar_result_file_name in glob.glob(os.path.join(source_results_directory, "*.sca")):
            shutil.copy(scalar_result_file_name, target_results_directory)
        return update_result

def get_update_statistical_results_tasks(**kwargs):
    return get_simulation_tasks(simulation_task_class=StatisticalResultsUpdateTask, **kwargs)

def update_statistical_results(sim_time_limit=get_statistical_result_sim_time_limit, **kwargs):
    multiple_update_statistical_results_tasks = get_update_statistical_results_tasks(sim_time_limit=sim_time_limit, **kwargs)
    return multiple_update_statistical_results_tasks.run(sim_time_limit=sim_time_limit, extra_args=["--**.param-recording=false"], **kwargs)
