import datetime
import functools
import logging
import os
import random
import re
import shutil
import subprocess
import sys
import time

from inet.simulation.build import *
from inet.simulation.config import *
from inet.simulation.project import *
from inet.simulation.subprocess import *

logger = logging.getLogger(__name__)

def _run_simulation(simulation_run, output_stream=sys.stdout, **kwargs):
    simulation_result = simulation_run.run_simulation(output_stream=output_stream, **kwargs)
    simulation_result.print_result(complete_error_message=False, output_stream=output_stream)
    return simulation_result

class SimulationRun:
    def __init__(self, simulation_config, run=0, mode="debug", user_interface="Cmdenv", sim_time_limit=None, cpu_time_limit=None, record_eventlog=None, **kwargs):
        assert run is not None
        self.simulation_config = simulation_config
        self.interactive = None # NOTE delayed to is_interactive()
        self.run = run
        self.mode = mode
        self.user_interface = user_interface
        self.sim_time_limit = sim_time_limit
        self.cpu_time_limit = cpu_time_limit
        self.record_eventlog = record_eventlog
        self.cancel = False
        self.kwargs = kwargs

    def __repr__(self):
        return repr(self)

    # TODO replace this with something more efficient?
    def is_interactive(self):
        if self.interactive is None:
            simulation_config = self.simulation_config
            simulation_project = simulation_config.simulation_project
            executable = simulation_project.get_full_path("bin/inet")
            args = [executable, "-s", "-u", "Cmdenv", "-f", simulation_config.ini_file, "-c", simulation_config.config, "-r", "0", "--sim-time-limit", "0s"]
            env = os.environ.copy()
            env["INET_ROOT"] = simulation_project.get_full_path(".")
            subprocess_result = subprocess.run(args, cwd=simulation_project.get_full_path(simulation_config.working_directory), capture_output=True, env=env)
            stderr = subprocess_result.stderr.decode("utf-8")
            match = re.search(r"The simulation wanted to ask a question|The simulation attempted to prompt for user input", stderr)
            self.interactive = match is not None
        return self.interactive

    def set_cancel(self, cancel):
        self.cancel = cancel

    def get_simulation_parameters_string(self, sim_time_limit=None, cpu_time_limit=None, **kwargs):
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        sim_time_limit = sim_time_limit or self.sim_time_limit
        cpu_time_limit = cpu_time_limit or self.cpu_time_limit
        return working_directory + \
               (" -f " + ini_file if ini_file != "omnetpp.ini" else "") + \
               (" -c " + config if config != "General" else "") + \
               (" -r " + str(self.run) if self.run != 0 else "") + \
               (" for " + sim_time_limit if sim_time_limit else "")

    def get_sim_time_limit(self, sim_time_limit=None):
        if sim_time_limit is None:
            return self.sim_time_limit(self.simulation_config, self.run) if callable(self.sim_time_limit) else self.sim_time_limit
        else:
            return sim_time_limit(self.simulation_config, self.run) if callable(sim_time_limit) else sim_time_limit

    def get_cpu_time_limit(self, cpu_time_limit=None):
        if cpu_time_limit is None:
            return self.cpu_time_limit(self.cpuulation_config, self.run) if callable(self.cpu_time_limit) else self.cpu_time_limit
        else:
            return cpu_time_limit(self.cpuulation_config, self.run) if callable(cpu_time_limit) else cpu_time_limit

    def run_simulation(self, mode=None, user_interface=None, sim_time_limit=None, cpu_time_limit=None, record_eventlog=None, record_pcap=None, index=None, count=None, print_end=" ", keyboard_interrupt_handler=None, cancel=False, dry_run=False, output_stream=sys.stdout, extra_args=[], simulation_runner=subprocess_simulation_runner, **kwargs):
        simulation_run_sim_time_limit = self.get_sim_time_limit(sim_time_limit)
        simulation_run_cpu_time_limit = self.get_cpu_time_limit(cpu_time_limit)
        simulation_project = self.simulation_config.simulation_project
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        print(("[" + str(index + 1) + "/" + str(count) + "] " if index is not None and count is not None else "") + "Running " + self.get_simulation_parameters_string(sim_time_limit=simulation_run_sim_time_limit, cpu_time_limit=simulation_run_cpu_time_limit, **kwargs), end=print_end, file=output_stream)
        output_stream.flush()
        sim_time_limit_args = ["--sim-time-limit", simulation_run_sim_time_limit] if simulation_run_sim_time_limit else []
        cpu_time_limit_args = ["--cpu-time-limit", simulation_run_cpu_time_limit] if simulation_run_cpu_time_limit else []
        record_eventlog_args = ["--record-eventlog", "true"] if (record_eventlog or self.record_eventlog) else []
        record_pcap_args = ["--**.numPcapRecorders=1", "--**.crcMode=\"computed\"", "--**.fcsMode=\"computed\""] if record_pcap else []
        env = os.environ.copy()
        env["INET_ROOT"] = simulation_project.get_full_path(".")
        executable = simulation_project.get_full_path("bin/inet")
        args = [executable, "--" + (mode or self.mode), "-s", "-u", (user_interface or self.user_interface), "-f", ini_file, "-c", config, "-r", str(self.run), *sim_time_limit_args, *cpu_time_limit_args, *record_eventlog_args, *record_pcap_args, *extra_args]
        logger.debug(args)
        if cancel or self.cancel:
            return SimulationResult(self, None)
        else:
            try:
                with EnabledKeyboardInterrupts(keyboard_interrupt_handler):
                    start_time = time.time()
                    if not dry_run:
                        subprocess_result = simulation_runner.run(self, args)
                    else:
                        subprocess_result = None
                    end_time = time.time()
                    return SimulationResult(self, subprocess_result, cancel=cancel or self.cancel, elapsed_wall_time=end_time - start_time)
            except KeyboardInterrupt:
                return SimulationResult(self, None)

class MultipleSimulationRuns:
    def __init__(self, simulation_project, simulation_runs, concurrent=True, run_simulation_function=_run_simulation, **kwargs):
        self.simulation_project = simulation_project
        self.simulation_runs = simulation_runs
        self.concurrent = concurrent
        self.run_simulation_function = run_simulation_function

    def __repr__(self):
        return repr(self)

    def run(self, simulation_project=None, concurrent=None, build=True, **kwargs):
        if concurrent is None:
            concurrent = self.concurrent
        if build:
            build_project(simulation_project = self.simulation_project, **kwargs)
        logger.info("Running simulations " + str(kwargs))
        start_time = time.time()
        simulation_results = map_sequentially_or_concurrently(self.simulation_runs, self.run_simulation_function, concurrent=concurrent, **kwargs)
        end_time = time.time()
        return MultipleSimulationResults(self, simulation_results, elapsed_wall_time=end_time - start_time, **kwargs)

class SimulationResult:
    def __init__(self, simulation_run, subprocess_result, cancel=False, elapsed_wall_time=None, **kwargs):
        self.simulation_run = simulation_run
        self.subprocess_result = subprocess_result
        self.result = ("DONE" if self.subprocess_result.returncode == 0 else "ERROR") if subprocess_result and not cancel else "CANCEL" 
        self.color = (COLOR_GREEN if self.subprocess_result.returncode == 0 else COLOR_RED) if subprocess_result and not cancel else COLOR_CYAN
        if subprocess_result:
            stdout = self.subprocess_result.stdout.decode("utf-8")
            stderr = self.subprocess_result.stderr.decode("utf-8")
            match = re.search(r"<!> Simulation time limit reached -- at t=(.*), event #(\d+)", stdout)
            self.last_event_number = int(match.group(2)) if match else None
            self.last_simulation_time = match.group(1) if match else None
            self.elapsed_cpu_time = None # TODO
            self.elapsed_wall_time = elapsed_wall_time
            match = re.search("<!> Error: (.*) -- in module (.*)", stderr)
            self.error_message = match.group(1).strip() if match else None
            self.error_module = match.group(2).strip() if match else None
            if self.error_message is None:
                match = re.search("<!> Error: (.*)", stderr)
                self.error_message = match.group(1).strip() if match else None
        else:
            self.last_event_number = None
            self.last_simulation_time = None
            self.elapsed_wall_time = None
            self.error_message = None
            self.error_module = None

    def __repr__(self):
        return "Simulation result: " + self.get_description()

    def get_error_message(self, complete_error_message=True):
        error_message = self.error_message or "Error message not found"
        error_module = self.error_module or "Error module not found"
        return (error_message + " -- in module " + error_module if complete_error_message else error_message) if self.result == "ERROR" else None

    def get_description(self, complete_error_message=True, include_simulation_parameters=False):
        return (self.simulation_run.get_simulation_parameters_string() + " " if include_simulation_parameters else "") + \
                self.color + self.result + COLOR_RESET + \
               (" " + self.get_error_message(complete_error_message=complete_error_message) if self.result == "ERROR" else "")

    def get_subprocess_result(self):
        return self.subprocess_result

    def print_result(self, complete_error_message=False, output_stream=sys.stdout):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

    def rerun(self, **kwargs):
        return self.simulation_run.run_simulation(**kwargs)

class MultipleSimulationResults:
    def __init__(self, multiple_simulation_runs, simulation_results, elapsed_wall_time=None, **kwargs):
        self.multiple_simulation_runs = multiple_simulation_runs
        self.simulation_results = simulation_results
        self.elapsed_wall_time = elapsed_wall_time
        self.num_different_results = 0
        self.num_done = self.count_results("DONE")
        self.num_cancel = self.count_results("CANCEL")
        self.num_error = self.count_results("ERROR")

    def __repr__(self):
        if len(self.simulation_results) == 1:
            return self.simulation_results[0].__repr__()
        else:
            details = self.get_details(exclude_result_filter="DONE|SKIP|CANCEL", include_simulation_parameters=True)
            return ("" if details.strip() == "" else "\nDetails:\n" + details + "\n\n") + \
                    "Simulation summary: " + self.get_summary()

    def is_all_done(self):
        return self.num_done == len(self.simulation_results)

    def count_results(self, result):
        num = sum(e.result == result for e in self.simulation_results)
        if num != 0:
            self.num_different_results += 1
        return num

    def get_result_class_texts(self, result, color, num):
        texts = []
        if num != 0:
            texts += [(color + str(num) + " " + result + COLOR_RESET if num != 0 else "")]
        return texts

    def get_summary(self):
        if len(self.simulation_results) == 1:
            return self.simulation_results[0].get_description()
        else:
            texts = []
            if self.num_different_results != 1:
                texts.append(str(len(self.simulation_results)) + " TOTAL")
            texts += self.get_result_class_texts("DONE", COLOR_GREEN, self.num_done)
            texts += self.get_result_class_texts("CANCEL", COLOR_CYAN, self.num_cancel)
            texts += self.get_result_class_texts("ERROR", COLOR_RED, self.num_error)
            return ", ".join(texts) + (" in " + str(datetime.timedelta(seconds=self.elapsed_wall_time)) if self.elapsed_wall_time else "")

    def get_details(self, separator="\n  ", result_filter=None, exclude_result_filter=None, **kwargs):
        texts = []
        def matches_simulation_result(simulation_result, result):
            return simulation_result.result == result and \
                   matches_filter(result, result_filter, exclude_result_filter, True)
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "DONE"), self.simulation_results):
            texts.append(simulation_result.get_description(**kwargs))
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "CANCEL"), self.simulation_results):
            texts.append(simulation_result.get_description(**kwargs))
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "ERROR"), self.simulation_results):
            texts.append(simulation_result.get_description(**kwargs))
        return "  " + separator.join(texts)

    def rerun(self, **kwargs):
        return self.multiple_simulation_runs.run(**kwargs)

    def filter(self, result_filter=None, full_match=True):
        simulation_results = list(filter(lambda simulation_result: re.search(result_filter if full_match else ".*" + result_filter + ".*", simulation_result.result), self.simulation_results))
        simulation_runs = list(map(lambda simulation_result: simulation_result.simulation_run, simulation_results))
        orignial_multiple_simulation_runs = self.multiple_simulation_runs
        multiple_simulation_runs = MultipleSimulationRuns(self.simulation_project, simulation_runs, concurrent=orignial_multiple_simulation_runs.concurrent, run_simulation_function=orignial_multiple_simulation_runs.run_simulation_function)
        return MultipleSimulationResults(multiple_simulation_runs, simulation_results)

def clean_simulation_results(simulation_config):
    logger.info("Cleaning simulation results, folder = " + simulation_config.working_directory)
    simulation_project = simulation_config.simulation_project
    path = os.path.join(simulation_project.get_full_path(simulation_config.working_directory), "results")
    if not re.search(".*/home/.*", path):
        raise Exception("Path is not in home")
    if os.path.exists(path):
        shutil.rmtree(path)

def clean_simulations_results(simulation_configs=None, **kwargs):
    if not simulation_configs:
        simulation_configs = get_simulation_configs(**kwargs)
    for simulation_config in simulation_configs:
        clean_simulation_results(simulation_config)

def get_simulations(simulation_project=None, simulation_configs=None, run=None, sim_time_limit=None, cpu_time_limit=None, concurrent=True, run_simulation_function=_run_simulation, **kwargs):
    if simulation_project is None:
        simulation_project = default_project
    if simulation_configs is None:
        simulation_configs = get_simulation_configs(simulation_project, concurrent=concurrent, **kwargs)
    simulation_runs = []
    for simulation_config in simulation_configs:
        if run is not None:
            simulation_run_sim_time_limit = sim_time_limit(simulation_config, run) if callable(sim_time_limit) else sim_time_limit
            simulation_runs.append(SimulationRun(simulation_config, run, sim_time_limit=simulation_run_sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs))
        else:
            for generated_run in range(0, simulation_config.num_runs):
                simulation_run_sim_time_limit = sim_time_limit(simulation_config, run) if callable(sim_time_limit) else sim_time_limit
                simulation_runs.append(SimulationRun(simulation_config, generated_run, sim_time_limit=simulation_run_sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs))
    return MultipleSimulationRuns(simulation_project, simulation_runs, concurrent=concurrent, run_simulation_function=run_simulation_function)

def run_simulations(**kwargs):
    multiple_simulation_runs = None
    try:
        logger.info("Running simulations")
        multiple_simulation_runs = get_simulations(**kwargs)
        return multiple_simulation_runs.run(**kwargs)
    except KeyboardInterrupt:
        simulation_results = list(map(lambda simulation_run: SimulationResult(simulation_run, None), multiple_simulation_runs.simulation_runs)) if multiple_simulation_runs else []
        return MultipleSimulationResults(multiple_simulation_runs, simulation_results)

def run_simulation(simulation_project, working_directory, ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None, cpu_time_limit=None, **kwargs):
    simulation_config = SimulationConfig(simulation_project, working_directory, ini_file, config, 1, False, None)
    simulation_run = SimulationRun(simulation_config, run, sim_time_limit=sim_time_limit, cpu_time_limit=cpu_time_limit)
    return simulation_run.run_simulation(**kwargs)
