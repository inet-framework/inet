import logging

from inet.simulation import *
from inet.test.simulation import *
from inet.test.task import *
from inet.test.speed.store import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

_speed_test_append_args = ["--cmdenv-express-mode=true", "--cmdenv-performance-display=false", "--cmdenv-status-frequency=1000000s", 
                           "--record-eventlog=false", "--vector-recording=false", "--scalar-recording=false", "--bin-recording=false", "--param-recording=false"]

_baseline_elapsed_wall_time = None

def get_baseline_elapsed_wall_time():
    global _baseline_elapsed_wall_time
    if _baseline_elapsed_wall_time:
        return _baseline_elapsed_wall_time
    else:
        logging.getLogger("routing").setLevel("WARN")
        with open(os.devnull, 'w') as devnull:
            run_command_with_logging(["make", "MODE=release", "-j16", "samples"], cwd=get_workspace_path("omnetpp"), error_message="Failed to build routing sample")
            simulation_task_result = run_simulations(simulation_project=get_simulation_project("routing"), config_filter="Net60a", sim_time_limit="10000s", append_args=_speed_test_append_args, output_stream=devnull)
            _baseline_elapsed_wall_time = simulation_task_result.elapsed_wall_time
        return _baseline_elapsed_wall_time

class SpeedTestTask(SimulationTestTask):
    def __init__(self, baseline_elapsed_wall_time=None, expected_relative_time=None, max_relative_error = 0.2, **kwargs):
        super().__init__(**kwargs)
        self.baseline_elapsed_wall_time = baseline_elapsed_wall_time
        self.expected_relative_time = expected_relative_time
        self.max_relative_error = max_relative_error

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        elapsed_relative_time = simulation_task_result.elapsed_wall_time / self.baseline_elapsed_wall_time
        if (elapsed_relative_time - self.expected_relative_time) / self.expected_relative_time > self.max_relative_error:
            return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", expected_result="PASS", reason="Elapsed wall time is too large")
        elif (self.expected_relative_time - elapsed_relative_time) / self.expected_relative_time > self.max_relative_error:
            return self.task_result_class(task=self, simulation_task_result=simulation_task_result, result="FAIL", expected_result="PASS", reason="Elapsed wall time is too small")
        else:
            return super().check_simulation_task_result(simulation_task_result, **kwargs)

    def run_protected(self, **kwargs):
        return super().run_protected(nice=-10, append_args=_speed_test_append_args, **kwargs)

def get_speed_test_tasks(mode="release", run_number=0, working_directory_filter="showcases", **kwargs):
    multiple_simulation_tasks = get_simulation_tasks(name="speed test", mode=mode, run_number=run_number, working_directory_filter=working_directory_filter, **kwargs)
    simulation_project = multiple_simulation_tasks.simulation_project
    speed_measurement_store = get_speed_measurement_store(simulation_project)
    baseline_elapsed_wall_time = get_baseline_elapsed_wall_time()
    tasks = []
    for simulation_task in multiple_simulation_tasks.tasks:
        simulation_config = simulation_task.simulation_config
        expected_relative_time = speed_measurement_store.get_elapsed_relative_time(working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run_number=simulation_task.run_number)
        tasks.append(SpeedTestTask(simulation_task=simulation_task, baseline_elapsed_wall_time=baseline_elapsed_wall_time, expected_relative_time=expected_relative_time, **dict(kwargs, simulation_project=simulation_project, mode=mode)))
    return MultipleSimulationTestTasks(tasks=tasks, **dict(kwargs, simulation_project=simulation_project, mode=mode, concurrent=False))

def run_speed_tests(**kwargs):
    multiple_test_tasks = get_speed_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)

class SpeedUpdateTask(SimulationUpdateTask):
    def __init__(self, baseline_elapsed_wall_time=None, action="Updating speed", **kwargs):
        super().__init__(action=action, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.baseline_elapsed_wall_time = baseline_elapsed_wall_time

    def run_protected(self, **kwargs):
        simulation_config = self.simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        start_time = time.time()
        simulation_task_result = self.simulation_task.run_protected(nice=-10, append_args=_speed_test_append_args, **kwargs)
        end_time = time.time()
        simulation_task_result.elapsed_wall_time = end_time - start_time
        speed_measurement_store = get_speed_measurement_store(simulation_project)
        expected_relative_time = speed_measurement_store.find_elapsed_relative_time(working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run_number=self.simulation_task.run_number)
        return SpeedUpdateTaskResult(task=self, simulation_task_result=simulation_task_result, expected_relative_time=expected_relative_time, elapsed_relative_time=simulation_task_result.elapsed_wall_time / self.baseline_elapsed_wall_time)

class MultipleSpeedUpdateTasks(MultipleSimulationUpdateTasks):
    def __init__(self, multiple_simulation_tasks=None, name="update speed", **kwargs):
        super().__init__(name=name, concurrent=False, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.multiple_simulation_tasks = multiple_simulation_tasks

    def run(self, simulation_project=None, concurrent=None, build=True, **kwargs):
        if concurrent is None:
            concurrent = self.multiple_simulation_tasks.concurrent
        simulation_project = simulation_project or self.multiple_simulation_tasks.simulation_project
        if build:
            build_project(simulation_project=simulation_project, **kwargs)
        multiple_speed_update_results = super().run(**kwargs)
        speed_measurement_store = get_speed_measurement_store(simulation_project)
        for speed_update_result in multiple_speed_update_results.results:
            speed_update_task = speed_update_result.task
            simulation_task = speed_update_task.simulation_task
            simulation_config = simulation_task.simulation_config
            elapsed_wall_time = speed_update_result.elapsed_wall_time
            if elapsed_wall_time is not None:
                speed_measurement_store.update_elapsed_wall_time(elapsed_wall_time, speed_update_task.baseline_elapsed_wall_time, test_result="PASS", sim_time_limit=simulation_task.sim_time_limit,
                                                                 working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run_number=simulation_task.run_number)
        speed_measurement_store.write()
        return speed_measurement_store

class SpeedUpdateTaskResult(SimulationUpdateTaskResult):
    def __init__(self, expected_relative_time=None, elapsed_relative_time=None, reason=None, max_relative_error=0.2, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        if expected_relative_time is None:
            self.result = "INSERT"
            self.reason = None
            self.color = COLOR_YELLOW
        elif abs(expected_relative_time - elapsed_relative_time) / expected_relative_time > max_relative_error:
            self.result = "UPDATE"
            self.reason = None
            self.color = COLOR_YELLOW
        else:
            self.result = "KEEP"
            self.reason = None
            self.color = COLOR_GREEN
        self.expected = self.result == self.expected_result

    def get_description(self, complete_error_message=True, include_parameters=False, **kwargs):
        return (self.task.simulation_task.get_parameters_string() + " " if include_parameters else "") + \
               self.color + self.result + COLOR_RESET + \
               ((" " + self.simulation_task_result.get_error_message(complete_error_message=complete_error_message)) if self.simulation_task_result and self.simulation_task_result.result == "ERROR" else "") + \
               (" (" + self.reason + ")" if self.reason else "")

    def __repr__(self):
        return "Speed update result: " + self.get_description(include_parameters=True)

    def print_result(self, complete_error_message=True, output_stream=sys.stdout, **kwargs):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

def get_update_speed_test_results_tasks(mode="release", run_number=0, working_directory_filter="showcases", **kwargs):
    update_tasks = []
    multiple_simulation_tasks = get_simulation_tasks(mode=mode, run_number=run_number, working_directory_filter=working_directory_filter, **kwargs)
    baseline_elapsed_wall_time = get_baseline_elapsed_wall_time()
    for simulation_task in multiple_simulation_tasks.tasks:
        update_task = SpeedUpdateTask(simulation_task=simulation_task, baseline_elapsed_wall_time=baseline_elapsed_wall_time, **kwargs)
        update_tasks.append(update_task)
    return MultipleSpeedUpdateTasks(multiple_simulation_tasks, tasks=update_tasks, **kwargs)

def update_speed_test_results(**kwargs):
    multiple_speed_update_tasks = get_update_speed_test_results_tasks(**kwargs)
    return multiple_speed_update_tasks.run(**kwargs)
