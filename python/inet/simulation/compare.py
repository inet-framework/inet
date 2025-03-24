import copy

from omnetpp.scave.results import *

from inet.common.util import *
from inet.simulation.task import *
from inet.test.fingerprint.task import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class CompareSimulationsTask(MultipleSimulationTasks):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        num_tasks = len(self.tasks)
        if num_tasks != 2:
            raise Exception(f"Found {num_tasks} simulation tasks instead of two")
        index = 0
        for task in self.tasks:
            index += 1
            task.record_eventlog = True
            task.eventlog_file_path = f"results/{task.simulation_config.config}-#{str(task.run_number)}-{index}.elog"
            task.scalar_file_path = f"results/{task.simulation_config.config}-#{str(task.run_number)}-{index}.sca"
            task.vector_file_path = f"results/{task.simulation_config.config}-#{str(task.run_number)}-{index}.vec"

    def run_protected(self, ingredients="tplx", **kwargs):
        append_args = ["--fingerprint=0000-0000/" + ingredients] + get_ingredients_append_args(ingredients)
        multiple_task_results = super().run_protected(append_args=append_args, **kwargs)
        return multiple_task_results

class CompareSimulationsTaskResult(MultipleTaskResults):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        if self.result == "DONE":
            self.divergence_position = self._find_divergence_position(self)
            self.different_statistical_results = self._get_different_statistical_results(exclude_result_name_filter="numTicks")
            if self.divergence_position:
                self.result = "DIVERGENT"
                self.color = COLOR_YELLOW
            else:
                self.result = "IDENTICAL"
        else:
            self.divergence_position = None
            self.different_statistical_results = pd.DataFrame()

    def __repr__(self):
        divergence_description = "\n" + self.divergence_position.__repr__() if self.divergence_position else ""
        statistical_desription = "\nFound " + COLOR_YELLOW + str(len(self.different_statistical_results)) + COLOR_RESET + " different statistical results" if not self.different_statistical_results.empty else ""
        return MultipleTaskResults.__repr__(self) + divergence_description + statistical_desription

    def debug_at_divergence_position(self, **kwargs):
        if self.divergence_position:
            task_1 = copy.copy(self.multiple_tasks.tasks[0])
            task_1.debug = True
            task_1.mode = "debug"
            task_1.break_at_event_number = self.divergence_position.simulation_event_1.event_number
            task_2 = copy.copy(self.multiple_tasks.tasks[1])
            task_2.debug = True
            task_2.mode = "debug"
            task_2.break_at_event_number = self.divergence_position.simulation_event_2.event_number
            multiple_tasks = copy.copy(self.multiple_tasks)
            multiple_tasks.tasks = [task_1, task_2]
            multiple_tasks.run(**kwargs)

    def run_until_divergence_position(self, **kwargs):
        if self.divergence_position:
            for task in self.multiple_tasks.tasks:
                task = copy.copy(task)
                task.user_interface = "Qtenv"
                task.wait = False
                task.run(**kwargs)

    def show_divergence_posisiton_in_sequence_chart(self):
        if self.divergence_position:
            simulation_event_1 = self.divergence_position.simulation_event_1
            simulation_event_2 = self.divergence_position.simulation_event_2
            project_name1 = simulation_event_1.simulation_result.task.simulation_config.simulation_project.get_name()
            project_name2 = simulation_event_2.simulation_result.task.simulation_config.simulation_project.get_name()
            path_name1 = "/" + project_name1 + "/" + simulation_event_1.simulation_result.task.simulation_config.working_directory + "/" + simulation_event_1.simulation_result.eventlog_file_path
            path_name2 = "/" + project_name2 + "/" + simulation_event_2.simulation_result.task.simulation_config.working_directory + "/" + simulation_event_2.simulation_result.eventlog_file_path
            editor1 = open_editor(path_name1)
            editor2 = open_editor(path_name2)
            goto_event_number(editor1, simulation_event_1.event_number)
            goto_event_number(editor2, simulation_event_2.event_number)

    def print_different_statistical_results(self):
        print(self.different_statistical_results)

    def _find_divergence_position(self, multiple_task_results):
        fingerprint_trajectory_1 = multiple_task_results.results[0].get_fingerprint_trajectory().get_unique()
        fingerprint_trajectory_2 = multiple_task_results.results[1].get_fingerprint_trajectory().get_unique()
        min_size = min(len(fingerprint_trajectory_1.fingerprints), len(fingerprint_trajectory_2.fingerprints))
        for i in range(0, min_size):
            trajectory_fingerprint_1 = fingerprint_trajectory_1.fingerprints[i]
            trajectory_fingerprint_2 = fingerprint_trajectory_2.fingerprints[i]
            if trajectory_fingerprint_1.fingerprint != trajectory_fingerprint_2.fingerprint:
                return FingerprintTrajectoryDivergencePosition(SimulationEvent(fingerprint_trajectory_1.simulation_result, fingerprint_trajectory_1.event_numbers[i]),
                                                               SimulationEvent(fingerprint_trajectory_2.simulation_result, fingerprint_trajectory_2.event_numbers[i]))
        return None

    def _get_different_statistical_results(self, result_name_filter=None, exclude_result_name_filter=None, result_module_filter=None, exclude_result_module_filter=None, full_match=False):
        df_1 = self._get_result_data_frame(self.results[0])
        df_2 = self._get_result_data_frame(self.results[1])
        if not df_1.equals(df_2):
            merged = df_1.merge(df_2, on=['experiment', 'measurement', 'replication', 'module', 'name'], how='outer', suffixes=('_1', '_2'))
            df = merged[
                (merged['value_1'].isna() & merged['value_2'].notna()) |
                (merged['value_1'].notna() & merged['value_2'].isna()) |
                (merged['value_1'] != merged['value_2'])].dropna(subset=['value_1', 'value_2'], how='all').copy()
            df["absolute_error"] = df.apply(lambda row: abs(row["value_2"] - row["value_1"]), axis=1)
            df["relative_error"] = df.apply(lambda row: row["absolute_error"] / abs(row["value_1"]) if row["value_1"] != 0 else (float("inf") if row["value_2"] != 0 else 0), axis=1)
            df = df[df.apply(lambda row: matches_filter(row["name"], result_name_filter, exclude_result_name_filter, full_match) and \
                                         matches_filter(row["module"], result_module_filter, exclude_result_module_filter, full_match), axis=1)]
            sorted_df = df.sort_values(by="relative_error", ascending=False)
            return sorted_df
        else:
            return pd.DataFrame()

    def _get_result_file_name(self, simulation_task, extension):
        simulation_config = simulation_task.simulation_config
        return f"{simulation_config.ini_file}-{simulation_config.config}-#{simulation_task.run_number}.{extension}"

    def _read_scalar_result_file(self, file_name):
        df = read_result_files(file_name, include_fields_as_scalars=True)
        df = get_scalars(df, include_runattrs=True)
        df = df if df.empty else df[["experiment", "measurement", "replication", "module", "name", "value"]]
        return df

    def _get_result_data_frame(self, simulation_task_result):
        simulation_task = simulation_task_result.task
        simulation_config = simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        scalar_file_path = simulation_project.get_full_path(os.path.join(working_directory, simulation_task_result.scalar_file_path))
        vector_file_path = simulation_project.get_full_path(os.path.join(working_directory, simulation_task_result.vector_file_path))
        if os.path.exists(vector_file_path):
            run_command_with_logging(["opp_scavetool", "x", "--type", "sth", "-w", vector_file_path, "-o", scalar_file_path])
            os.remove(vector_file_path)
        stored_scalar_result_file_name = simulation_project.get_full_path(os.path.join(simulation_project.statistics_folder, working_directory, simulation_task_result.scalar_file_path))
        _logger.debug(f"Reading result file {scalar_file_path}")
        return self._read_scalar_result_file(scalar_file_path)

def compare_simulations(task1, task2, **kwargs):
    simulation_comparison_task = CompareSimulationsTask(tasks=[task1, task2], multiple_task_results_class=CompareSimulationsTaskResult, **kwargs)
    return simulation_comparison_task.run(**kwargs)
