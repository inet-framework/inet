import functools
import multiprocessing
import os
import re
import shutil
import subprocess
import sys

from inet.simulation.config import *
from pickle import NONE

logger = logging.getLogger(__name__)

def _run_simulation(simulation_run, **kwargs):
    return simulation_run.run_simulation(**kwargs)

class SimulationRun:
    def __init__(self, simulation_config, run=0, sim_time_limit=None, cpu_time_limit=None, mode="debug", ui="Cmdenv", **kwargs):
        assert run is not None
        self.simulation_config = simulation_config
        self.run = run
        self.sim_time_limit = sim_time_limit
        self.cpu_time_limit = cpu_time_limit
        self.mode = mode
        self.ui = ui
        self.kwargs = kwargs
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def get_simulation_parameters_string(self, sim_time_limit=None, cpu_time_limit=None, **kwargs):
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        return working_directory + \
               (" -f " + ini_file if ini_file != "omnetpp.ini" else "") + \
               (" -c " + config if config != "General" else "") + \
               (" -r " + str(self.run) if self.run != 0 else "") + \
               (" for " + sim_time_limit if sim_time_limit else "")

    def run_simulation(self, mode=None, ui=None, sim_time_limit=None, cpu_time_limit=None, print_end="\n", flush=False, extra_args=[], cancel=False, **kwargs):
        if sim_time_limit is None:
            sim_time_limit = self.sim_time_limit
        if cpu_time_limit is None:
            cpu_time_limit = self.cpu_time_limit
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        print("Running " + self.get_simulation_parameters_string(sim_time_limit=sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs), end=print_end)
        if flush:
            sys.stdout.flush()
        sim_time_limit_args = ["--sim-time-limit", sim_time_limit] if sim_time_limit else []
        cpu_time_limit_args = ["--cpu-time-limit", cpu_time_limit] if cpu_time_limit else []
        args = ["inet", "--" + (mode or self.mode), "-s", "-u", (ui or self.ui), "-f", ini_file, "-c", config, "-r", str(self.run), *sim_time_limit_args, *cpu_time_limit_args, *extra_args]
        logger.debug(args)
        try:
            if cancel or self.cancel:
                return SimulationResult(self, None)
            else:
                subprocess_result = subprocess.run(args, cwd=get_full_path(working_directory), capture_output=True)
                return SimulationResult(self, subprocess_result)
        except KeyboardInterrupt:
            return SimulationResult(self, None)

class MultipleSimulationRuns:
    def __init__(self, simulation_runs, concurrent=True, run_simulation_function=_run_simulation, **kwargs):
        self.simulation_runs = simulation_runs
        self.concurrent = concurrent
        self.run_simulation_function = run_simulation_function

    def __repr__(self):
        return repr(self)

    def run(self, **kwargs):
        simulation_results = self.run_simulation_runs(self.simulation_runs, **kwargs)
        return MultipleSimulationResults(self, simulation_results, **kwargs)

    def run_simulation_runs(self, simulation_runs, concurrent=None, **kwargs):
        for simulation_run in simulation_runs:
            simulation_run.cancel = False
        if concurrent is None:
            concurrent = self.concurrent
        if concurrent:
            pool = multiprocessing.Pool(multiprocessing.cpu_count())
            partial = functools.partial(self.run_simulation_function, flush=False, **kwargs)
            results = pool.map_async(partial, simulation_runs)
            try:
                return results.get(0xFFFF)
            except KeyboardInterrupt:
                for simulation_run in simulation_runs:
                    simulation_run.cancel = True
                return results.get(0xFFFF)
        else:
            results = []
            cancel = False
            for simulation_run in simulation_runs:
                result = self.run_simulation_function(simulation_run, flush=True, cancel=cancel, **kwargs)
                if result.get_subprocess_result() is None:
                    cancel = True
                results.append(result)
            return results

class SimulationResult:
    def __init__(self, simulation_run, subprocess_result, **kwargs):
        self.simulation_run = simulation_run
        self.subprocess_result = subprocess_result
        self.result = ("DONE" if self.subprocess_result.returncode == 0 else "ERROR") if subprocess_result else "CANCEL" 
        self.color = (COLOR_GREEN if self.subprocess_result.returncode == 0 else COLOR_RED) if subprocess_result else COLOR_CYAN
        if subprocess_result:
            stdout = self.subprocess_result.stdout.decode("utf-8")
            stderr = self.subprocess_result.stderr.decode("utf-8")
            self.last_event_number = -1 # TODO parse stdout
            self.last_simulation_time = -1 # TODO parse stdout
            self.elapsed_cpu_time = -1 # TODO parse stdout
            self.error_message = re.sub("<!> Error: (.*)", "\\1", stderr).strip()
        else:
            self.last_event_number = None
            self.last_simulation_time = None
            self.elapsed_cpu_time = None
            self.error_message = None

    def __repr__(self):
        return "Simulation result: " + self.color + self.result + COLOR_RESET + (" " + self.error_message if self.result == "ERROR" else "")

    def get_subprocess_result(self):
        return self.subprocess_result

    def rerun(self, **kwargs):
        return self.simulation_run.run_simulation(**kwargs)

class MultipleSimulationResults:
    def __init__(self, multiple_simulation_runs, simulation_results, **kwargs):
        self.multiple_simulation_runs = multiple_simulation_runs
        self.simulation_results = simulation_results
        self.num_done = self.count_results("DONE")
        self.num_cancel = self.count_results("CANCEL")
        self.num_error = self.count_results("ERROR")

    def __repr__(self):
        if len(self.simulation_results) == 1:
            return str(self)
        else:
            return "Simulation results: " + str(self)

    def __str__(self):
        if len(self.simulation_results) == 1:
            return str(self.simulation_results[0]())
        else:
            return str(len(self.simulation_results)) + " TOTAL" + \
                   self.get_result_class_text("DONE", COLOR_GREEN, self.num_done) + \
                   self.get_result_class_text("CANCEL", COLOR_CYAN, self.num_cancel) + \
                   self.get_result_class_text("ERROR", COLOR_RED, self.num_error)

    def is_all_done(self):
        return self.num_done == len(self.simulation_results)

    def count_results(self, result):
        return sum(e.result == result for e in self.simulation_results)

    def get_result_class_text(self, result, color, num):
        return (", " + color + str(num) + " " + result + COLOR_RESET if num != 0 else "")

    def rerun(self, **kwargs):
        return self.multiple_simulation_runs.run(**kwargs)

    def filter(self, result_filter=None, full_match=True):
        simulation_results = list(filter(lambda simulation_result: re.search(result_filter if full_match else ".*" + result_filter + ".*", simulation_result.result), self.simulation_results))
        simulation_runs = list(map(lambda simulation_result: simulation_result.simulation_run, simulation_results))
        orignial_multiple_simulation_runs = self.multiple_simulation_runs
        multiple_simulation_runs = MultipleSimulationRuns(simulation_runs, concurrent=orignial_multiple_simulation_runs.concurrent, run_simulation_function=orignial_multiple_simulation_runs.run_simulation_function)
        return MultipleSimulationResults(multiple_simulation_runs, simulation_results)

def clean_simulation_results(simulation_config):
    logger.info("Cleaning simulation results, folder = " + simulation_config.working_directory)
    path = get_full_path(simulation_config.working_directory) + "results"
    if not re.search(".*/home/.*", path):
        raise Exception("Path is not in home")
    if os.path.exists(path):
        shutil.rmtree(path)

def clean_simulations_results(simulation_configs=None, **kwargs):
    if not simulation_configs:
        simulation_configs = get_simulation_configs(**kwargs)
    for simulation_config in simulation_configs:
        clean_simulation_results(simulation_config)

def get_simulations(simulation_configs=None, run=None, sim_time_limit=None, cpu_time_limit=None, concurrent=True, run_simulation_function=_run_simulation, **kwargs):
    if simulation_configs is None:
        simulation_configs = get_simulation_configs(**kwargs)
    simulation_runs = []
    for simulation_config in simulation_configs:
        if run:
            simulation_runs.append(SimulationRun(simulation_config, run, sim_time_limit, cpu_time_limit, **kwargs))
        else:
            for generated_run in range(0, simulation_config.num_runs):
                simulation_runs.append(SimulationRun(simulation_config, generated_run, sim_time_limit, cpu_time_limit, **kwargs))
    return MultipleSimulationRuns(simulation_runs, concurrent=concurrent, run_simulation_function=run_simulation_function)

def run_simulations(**kwargs):
    logger.info("Running simulations")
    multiple_simulation_runs = get_simulations(**kwargs)
    return multiple_simulation_runs.run(**kwargs)

def run_simulation(working_directory, ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None, cpu_time_limit=None, **kwargs):
    simulation_config = SimulationConfig(working_directory, ini_file, config, 1)
    simulation_run = SimulationRun(simulation_config, run, sim_time_limit, cpu_time_limit)
    return simulation_run.run_simulation(**kwargs)
