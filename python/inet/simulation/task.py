import datetime
import functools
import logging
import os
import random
import re
import shutil
import signal
import subprocess
import sys
import time

from inet.common import *
from inet.simulation.build import *
from inet.simulation.config import *
from inet.simulation.project import *
from inet.simulation.subprocess import *

logger = logging.getLogger(__name__)

class SimulationTaskResult(TaskResult):
    def __init__(self, subprocess_result=None, cancel=False, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.subprocess_result = subprocess_result
        if subprocess_result:
            stdout = self.subprocess_result.stdout.decode("utf-8") if self.subprocess_result.stdout else ""
            stderr = self.subprocess_result.stderr.decode("utf-8") if self.subprocess_result.stderr else ""
            match = re.search(r"<!> Simulation time limit reached -- at t=(.*), event #(\d+)", stdout)
            self.last_event_number = int(match.group(2)) if match else None
            self.last_simulation_time = match.group(1) if match else None
            self.elapsed_cpu_time = None # TODO
            match = re.search(r"<!> Error: (.*) -- in module (.*)", stderr)
            self.error_message = match.group(1).strip() if match else None
            self.error_module = match.group(2).strip() if match else None
            if self.error_message is None:
                match = re.search(r"<!> Error: (.*)", stderr)
                self.error_message = match.group(1).strip() if match else None
            if self.error_message:
                if re.search(r"The simulation attempted to prompt for user input", self.error_message):
                    self.result = "SKIP"
                    self.color = COLOR_CYAN
                    self.expected_result = "SKIP"
                    self.expected = True
                    self.reason = "Interactive simulation"
        else:
            self.last_event_number = None
            self.last_simulation_time = None
            self.error_message = None
            self.error_module = None

    def get_error_message(self, complete_error_message=True, **kwargs):
        error_message = self.error_message or "<Error message not found>"
        error_module = self.error_module or "<Error module not found>"
        return error_message + " -- in module " + error_module if complete_error_message else error_message

    def get_subprocess_result(self):
        return self.subprocess_result

class SimulationTask(Task):
    def __init__(self, simulation_config=None, run=0, mode="debug", user_interface="Cmdenv", sim_time_limit=None, cpu_time_limit=None, record_eventlog=None, record_pcap=None, name="simulation", task_result_class=SimulationTaskResult, **kwargs):
        super().__init__(name=name, task_result_class=task_result_class, **kwargs)
        assert run is not None
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_config = simulation_config
        self.interactive = None # NOTE delayed to is_interactive()
        self._run = run
        self.mode = mode
        self.user_interface = user_interface
        self.sim_time_limit = sim_time_limit
        self.cpu_time_limit = cpu_time_limit
        self.record_eventlog = record_eventlog
        self.record_pcap = record_pcap

    # TODO replace this with something more efficient?
    def is_interactive(self):
        if self.interactive is None:
            simulation_config = self.simulation_config
            simulation_project = simulation_config.simulation_project
            executable = simulation_project.get_executable()
            default_args = simulation_project.get_default_args()
            args = [executable, *default_args, "-s", "-u", "Cmdenv", "-f", simulation_config.ini_file, "-c", simulation_config.config, "-r", "0", "--sim-time-limit", "0s"]
            subprocess_result = subprocess.run(args, cwd=simulation_project.get_full_path(simulation_config.working_directory), capture_output=True, env=simulation_project.get_env())
            stderr = subprocess_result.stderr.decode("utf-8")
            match = re.search(r"The simulation wanted to ask a question|The simulation attempted to prompt for user input", stderr)
            self.interactive = match is not None
        return self.interactive

    def get_expected_result(self):
        return self.simulation_config.expected_result

    def get_parameters_string(self, **kwargs):
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        return working_directory + \
               (" -f " + ini_file if ini_file != "omnetpp.ini" else "") + \
               (" -c " + config if config != "General" else "") + \
               (" -r " + str(self._run) if self._run != 0 else "") + \
               (" for " + self.sim_time_limit if self.sim_time_limit else "")

    def get_sim_time_limit(self):
        return self.sim_time_limit(self.simulation_config, self._run) if callable(self.sim_time_limit) else self.sim_time_limit

    def get_cpu_time_limit(self):
        return self.cpu_time_limit(self.simulation_config, self._run) if callable(self.cpu_time_limit) else self.cpu_time_limit

    def run_protected(self, extra_args=[], simulation_runner=subprocess_simulation_runner, capture_output=True, **kwargs):
        simulation_project = self.simulation_config.simulation_project
        working_directory = self.simulation_config.working_directory
        ini_file = self.simulation_config.ini_file
        config = self.simulation_config.config
        sim_time_limit_args = ["--sim-time-limit", self.get_sim_time_limit()] if self.sim_time_limit else []
        cpu_time_limit_args = ["--cpu-time-limit", self.get_cpu_time_limit()] if self.cpu_time_limit else []
        record_eventlog_args = ["--record-eventlog", "true"] if self.record_eventlog else []
        record_pcap_args = ["--**.numPcapRecorders=1", "--**.crcMode=\"computed\"", "--**.fcsMode=\"computed\""] if self.record_pcap else []
        executable = simulation_project.get_executable(mode=self.mode)
        default_args = simulation_project.get_default_args()
        args = [executable, *default_args, "-s", "-u", self.user_interface, "-f", ini_file, "-c", config, "-r", str(self._run), *sim_time_limit_args, *cpu_time_limit_args, *record_eventlog_args, *record_pcap_args, *extra_args]
        logger.debug(args)
        expected_result = self.get_expected_result()
        subprocess_result = simulation_runner.run(self, args, capture_output=capture_output)
        if subprocess_result.returncode == signal.SIGINT.value or subprocess_result.returncode == -signal.SIGINT.value:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="CANCEL", expected_result=expected_result, reason="Cancel by user")
        elif subprocess_result.returncode == 0:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="DONE", expected_result=expected_result)
        else:
            return self.task_result_class(task=self, subprocess_result=subprocess_result, result="ERROR", expected_result=expected_result, reason=f"Non-zero exit code: {subprocess_result.returncode}")

class MultipleSimulationTasks(MultipleTasks):
    def __init__(self, simulation_project=default_project, build=True, name="simulation", **kwargs):
        super().__init__(build=build, name=name, **kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.build = build
        self.simulation_project = simulation_project

    def run(self, **kwargs):
        if self.build:
            build_project(simulation_project=self.simulation_project, **kwargs)
        return super().run(**kwargs)

def clean_simulation_results(simulation_config):
    logger.info("Cleaning simulation results, folder = " + simulation_config.working_directory)
    simulation_project = simulation_config.simulation_project
    path = os.path.join(simulation_project.get_full_path(simulation_config.working_directory), "results")
    if not re.search(r".*/home/.*", path):
        raise Exception("Path is not in home")
    if os.path.exists(path):
        shutil.rmtree(path)

def clean_simulations_results(simulation_configs=None, **kwargs):
    if not simulation_configs:
        simulation_configs = get_simulation_configs(**kwargs)
    for simulation_config in simulation_configs:
        clean_simulation_results(simulation_config)

def get_simulation_tasks(simulation_project=None, simulation_configs=None, run=None, sim_time_limit=None, cpu_time_limit=None, concurrent=True, simulation_task_class=SimulationTask, multiple_simulation_tasks_class=MultipleSimulationTasks, **kwargs):
    if simulation_project is None:
        simulation_project = default_project
    if simulation_configs is None:
        simulation_configs = get_simulation_configs(simulation_project, concurrent=concurrent, **kwargs)
    simulation_tasks = []
    for simulation_config in simulation_configs:
        if run is not None:
            simulation_run_sim_time_limit = sim_time_limit(simulation_config, run) if callable(sim_time_limit) else sim_time_limit
            simulation_task = simulation_task_class(simulation_config=simulation_config, run=run, sim_time_limit=simulation_run_sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs)
            simulation_tasks.append(simulation_task)
        else:
            for generated_run in range(0, simulation_config.num_runs):
                simulation_run_sim_time_limit = sim_time_limit(simulation_config, generated_run) if callable(sim_time_limit) else sim_time_limit
                simulation_task = simulation_task_class(simulation_config=simulation_config, run=generated_run, sim_time_limit=simulation_run_sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs)
                simulation_tasks.append(simulation_task)
    return multiple_simulation_tasks_class(tasks=simulation_tasks, simulation_project=simulation_project, concurrent=concurrent, **kwargs)

def get_simulation_task(**kwargs):
    simulation_tasks = get_simulation_tasks(**kwargs)
    if len(simulation_tasks.tasks) == 0:
        raise Exception("Simulation task not found")
    elif len(simulation_tasks.tasks) == 1:
        return simulation_tasks.tasks[0]
    else:
        raise Exception("Found more than 1 simulation tasks")

def run_simulations(**kwargs):
    multiple_simulation_tasks = get_simulation_tasks(**kwargs)
    return multiple_simulation_tasks.run(**kwargs)

def run_simulation(simulation_project, working_directory, ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None, cpu_time_limit=None, **kwargs):
    simulation_config = SimulationConfig(simulation_project, working_directory, ini_file, config, 1, False, None)
    simulation_task = SimulationTask(simulation_config, run, sim_time_limit=sim_time_limit, cpu_time_limit=cpu_time_limit, **kwargs)
    return simulation_task.run(**kwargs)
