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
            self._compare_statistical_results(**self.multiple_tasks.kwargs)
            if self.different_statistical_results is not None:
                self.statistical_comparison_result = "DIFFERENT"
                self.statistical_comparison_color = COLOR_YELLOW
            else:
                self.statistical_comparison_result = "IDENTICAL"
                self.statistical_comparison_color = COLOR_GREEN
            if self.divergence_position:
                self.trajectory_comparison_result = "DIVERGENT"
                self.trajectory_comparison_color = COLOR_YELLOW
            else:
                self.trajectory_comparison_result = "IDENTICAL"
                self.trajectory_comparison_color = COLOR_GREEN
        else:
            self.divergence_position = None
            self.different_statistical_results = pd.DataFrame()

    def __repr__(self):
        if self.divergence_position:
            divergence_description = f"\nFingerprint trajectory comparison result: {self.trajectory_comparison_color}{self.trajectory_comparison_result}{COLOR_RESET}\n{self.divergence_position.get_description()}"
        else:
            divergence_description = ""
        if self.different_statistical_results is not None and not self.different_statistical_results.empty:
            max_num_different_statistics = 3
            different_unique_modules = self.different_statistical_results["module"].unique()
            different_unique_statistics = self.different_statistical_results["name"].unique()
            different_modules = ", ".join(map(lambda s: f"{COLOR_CYAN}{s}{COLOR_RESET}", different_unique_modules[0:max_num_different_statistics])) + (", ..." if len(different_unique_modules) > max_num_different_statistics else "")
            different_statistics = ", ".join(map(lambda s: f"{COLOR_GREEN}{s}{COLOR_RESET}", different_unique_statistics[0:max_num_different_statistics])) + (", ..." if len(different_unique_statistics) > max_num_different_statistics else "")
            statistical_desription = f"\nStatistical comparison result: {self.statistical_comparison_color}{self.statistical_comparison_result}{COLOR_RESET}, summary: {str(len(self.df_1))} and {str(len(self.df_2))} TOTAL, {COLOR_GREEN}{str(len(self.identical_statistical_results))} IDENTICAL{COLOR_RESET}, {COLOR_YELLOW}{str(len(self.different_statistical_results))} DIFFERENT{COLOR_RESET}, some differences: {different_statistics} in {different_modules}"
        else:
            statistical_desription = ""
        return MultipleTaskResults.__repr__(self) + divergence_description + statistical_desription

    def debug_at_divergence_position(self, num_cause_events=0, **kwargs):
        if self.divergence_position:
            event_number_1 = self._get_cause_event_number(self.divergence_position.simulation_event_1, num_cause_events)
            event_number_2 = self._get_cause_event_number(self.divergence_position.simulation_event_2, num_cause_events)
            task_1 = copy.copy(self.multiple_tasks.tasks[0])
            task_1.debug = True
            task_1.mode = "debug"
            task_1.break_at_event_number = event_number_1
            task_2 = copy.copy(self.multiple_tasks.tasks[1])
            task_2.debug = True
            task_2.mode = "debug"
            task_2.break_at_event_number = event_number_2
            multiple_tasks = copy.copy(self.multiple_tasks)
            multiple_tasks.tasks = [task_1, task_2]
            multiple_tasks.run(**kwargs)

    def run_until_divergence_position(self, num_cause_events=0, append_args=[], **kwargs):
        if self.divergence_position:
            # NOTE: add 1 to complete the event that causes the fingerprint trajectory divergence
            event_number_1 = self._get_cause_event_number(self.divergence_position.simulation_event_1, num_cause_events) + 1
            event_number_2 = self._get_cause_event_number(self.divergence_position.simulation_event_2, num_cause_events) + 1
            task_1 = copy.copy(self.multiple_tasks.tasks[0])
            task_1.user_interface = "Qtenv"
            task_1.wait = False
            task_1.run(append_args=append_args + [f"--qtenv-stop-event-number={event_number_1}"], **kwargs)
            task_2 = copy.copy(self.multiple_tasks.tasks[1])
            task_2.user_interface = "Qtenv"
            task_2.wait = False
            task_2.run(append_args=append_args + [f"--qtenv-stop-event-number={event_number_2}"], **kwargs)

    def show_divergence_posisiton_in_sequence_chart(self):
        if self.divergence_position:
            self.divergence_position.show_in_sequence_chart()

    def print_divergence_position_cause_chain(self, **kwargs):
        if self.divergence_position:
            self.divergence_position.print_cause_chain(**kwargs)

    def print_different_statistic_modules(self):
        print("\n".join(self.different_statistical_results["module"].unique()))

    def print_different_statistic_names(self):
        print("\n".join(self.different_statistical_results["name"].unique()))

    def print_different_statistical_results(self, drop_columns_with_equals_values=True, include_absolute_errors=False, include_relative_errors=False, **kwargs):
        df = self.different_statistical_results
        if drop_columns_with_equals_values:
            df = df.loc[:, df.nunique() > 1]
        if not include_absolute_errors:
            df = df.drop("absolute_error", axis=1)
        if not include_relative_errors:
            df = df.drop("relative_error", axis=1)
        print(df.to_string(index=False, **kwargs))

    def _get_cause_event_number(self, simulation_event, num_cause_events):
        event = simulation_event.get_event()
        while num_cause_events > 0:
            event = event.getCauseEvent()
            num_cause_events = num_cause_events - 1
        return event.getEventNumber()

    def _find_divergence_position(self, multiple_task_results):
        fingerprint_trajectory_1 = multiple_task_results.results[0].get_fingerprint_trajectory().get_unique()
        fingerprint_trajectory_2 = multiple_task_results.results[1].get_fingerprint_trajectory().get_unique()
        return find_fingerprint_trajectory_divergence_position(fingerprint_trajectory_1, fingerprint_trajectory_2)

    def _compare_statistical_results(self, result_name_filter=None, exclude_result_name_filter=None, result_module_filter=None, exclude_result_module_filter=None, full_match=False, **kwargs):
        self.df_1 = self._get_result_data_frame(self.results[0])
        self.df_2 = self._get_result_data_frame(self.results[1])
        if not self.df_1.equals(self.df_2):
            merged = self.df_1.merge(self.df_2, on=['experiment', 'measurement', 'replication', 'module', 'name'], how='outer', suffixes=('_1', '_2'))
            df = merged[
                (merged['value_1'].isna() & merged['value_2'].notna()) |
                (merged['value_1'].notna() & merged['value_2'].isna()) |
                (merged['value_1'] != merged['value_2'])].dropna(subset=['value_1', 'value_2'], how='all').copy()
            df["absolute_error"] = df.apply(lambda row: abs(row["value_2"] - row["value_1"]), axis=1)
            df["relative_error"] = df.apply(lambda row: row["absolute_error"] / abs(row["value_1"]) if row["value_1"] != 0 else (float("inf") if row["value_2"] != 0 else 0), axis=1)
            df = df[df.apply(lambda row: matches_filter(row["name"], result_name_filter, exclude_result_name_filter, full_match) and \
                                         matches_filter(row["module"], result_module_filter, exclude_result_module_filter, full_match), axis=1)]
            sorted_df = df.sort_values(by="relative_error", ascending=False)
            self.different_statistical_results = sorted_df
            self.identical_statistical_results = pd.merge(self.df_1, self.df_2, how="inner")
        else:
            self.different_statistical_results = pd.DataFrame()

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
