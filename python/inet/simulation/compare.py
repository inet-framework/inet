import copy

from inet.util import *

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

    def find_divergence_position(self, multiple_task_results):
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

    def run_protected(self, ingredients="tplx", **kwargs):
        append_args = ["--fingerprint=0000-0000/" + ingredients] + get_ingredients_append_args(ingredients)
        multiple_task_results = super().run_protected(append_args=append_args, **kwargs)
        multiple_task_results.divergence_position = self.find_divergence_position(multiple_task_results)
        return multiple_task_results

class CompareSimulationsTaskResult(MultipleTaskResults):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def __repr__(self):
        return MultipleTaskResults.__repr__(self) + "\n" + self.divergence_position.__repr__()

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
            self.divergence_position.open_sequence_charts()

    def compare_statistical_results(self):
        pass

def compare_simulations(task1, task2, **kwargs):
    simulation_comparison_task = CompareSimulationsTask(tasks=[task1, task2], multiple_task_results_class=CompareSimulationsTaskResult, **kwargs)
    return simulation_comparison_task.run(**kwargs)
