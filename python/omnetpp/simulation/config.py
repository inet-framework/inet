import builtins
import functools
import glob
import itertools
import logging
import multiprocessing
import os
import re
import subprocess

from inet.common import *
from inet.simulation.project import *

logger = logging.getLogger(__name__)

class SimulationConfig:
    def __init__(self, simulation_project, working_directory, ini_file="omnetpp.ini", config="General", num_runs=1, sim_time_limit=None, abstract=False, expected_result="DONE", user_interface="Cmdenv", description=None):
        self.simulation_project = simulation_project
        self.working_directory = working_directory
        self.ini_file = ini_file
        self.config = config
        self.num_runs = num_runs
        self.sim_time_limit = sim_time_limit
        self.abstract = abstract
        self.emulation = working_directory.find("emulation") != -1
        self.expected_result = expected_result
        self.user_interface = user_interface
        self.description = description

    def __repr__(self):
        return repr(self)

    def matches_filter(self, filter=None, exclude_filter=None,
                       working_directory_filter=None, exclude_working_directory_filter=None,
                       ini_file_filter=None, exclude_ini_file_filter=None,
                       config_filter=None, exclude_config_filter=None,
                       simulation_config_filter=lambda simulation_config: not simulation_config.abstract and not simulation_config.emulation,
                       full_match=False, **kwargs):
        return matches_filter(self.__repr__(), filter, exclude_filter, full_match) and \
               matches_filter(self.working_directory, working_directory_filter, exclude_working_directory_filter, full_match) and \
               matches_filter(self.ini_file, ini_file_filter, exclude_ini_file_filter, full_match) and \
               matches_filter(self.config, config_filter, exclude_config_filter, full_match) and \
               simulation_config_filter(self)

num_runs_fast_regex = re.compile(r"(?m).*^\s*(include\s+.*\.ini|repeat\s*=\s*[0-9]+|.*\$\{.*\})")

def get_num_runs_fast(ini_path):
    file = open(ini_path, "r", encoding="utf-8")
    text = file.read()
    file.close()
    return None if num_runs_fast_regex.search(text) else 1

# KLUDGE TODO replace this with a Python binding to the C++ configuration reader
def get_sim_time_limit(config_dicts, config):
    config_dict = config_dicts[config]
    if "sim_time_limit" in config_dict:
        return config_dict["sim_time_limit"]
    if "extends" in config_dict:
        extends = config_dict["extends"]
        for base_config in extends.split(","):
            if base_config in config_dicts:
                sim_time_limit = get_sim_time_limit(config_dicts, base_config)
                if sim_time_limit:
                    return sim_time_limit
    return config_dicts["General"].get("sim_time_limit")

# KLUDGE TODO replace this with a Python binding to the C++ configuration reader
def collect_ini_file_simulation_configs(simulation_project, ini_path):
    def create_config_dict(config):
        return {"config": config, "abstract_config": False, "expected_result": "DONE", "user_interface": None, "description": None, "network": None}
    simulation_configs = []
    working_directory = os.path.dirname(ini_path)
    num_runs_fast = get_num_runs_fast(ini_path)
    ini_file = os.path.basename(ini_path)
    file = open(ini_path, encoding="utf-8")
    config_dicts = {"General": create_config_dict("General")}
    config_dict = {}
    for line in file:
        match = re.match("\\[(Config +)?(.*?)\\]|\\[(General)\\]", line)
        if match:
            config = match.group(2) or match.group(3)
            config_dict = create_config_dict(config)
            config_dicts[config] = config_dict
        match = re.match(" *extends *= *(\w+)", line)
        if match:
            config_dict["extends"] = match.group(1)
        match = re.match(" *user-interface *= \"*(\w+)\"", line)
        if match:
            config_dict["user_interface"] = match.group(1)
        match = re.match("#? *abstract-config *= *(\w+)", line)
        if match:
            config_dict["abstract_config"] = bool(match.group(1))
        match = re.match("#? *expected-result *= *\"(\w+)\"", line)
        if match:
            config_dict["expected_result"] = match.group(1)
        match = re.match("description *= *\"(.*)\"", line)
        if match:
            config_dict["description"] = match.group(1)
        match = re.match("network *= *(.*)", line)
        if match:
            config_dict["network"] = match.group(1)
        match = re.match("sim-time-limit *= *(.*)", line)
        if match:
            config_dict["sim_time_limit"] = match.group(1)
    general_config_dict = config_dicts["General"]
    for config, config_dict in config_dicts.items():
        config = config_dict["config"]
        executable = simulation_project.get_executable(mode="release")
        default_args = simulation_project.get_default_args()
        args = [executable, *default_args, "-s", "-f", ini_file, "-c", config, "-q", "numruns"]
        logger.debug(args)
        if num_runs_fast:
            num_runs = num_runs_fast
        else:
            result = subprocess.run(args, cwd=working_directory, capture_output=True, env=simulation_project.get_env())
            if result.returncode == 0:
                num_runs = int(result.stdout)
            else:
                raise Exception("Cannot determine number of runs")
        sim_time_limit = get_sim_time_limit(config_dicts, config)
        description = config_dict["description"]
        description_abstract = (re.search("\((a|A)bstract\)", description) is not None) if description else False
        abstract = (config_dict["network"] is None and config_dict["config"] == "General") or config_dict["abstract_config"] or description_abstract
        expected_result = config_dict["expected_result"]
        user_interface = config_dict["user_interface"] or general_config_dict["user_interface"]
        simulation_config = SimulationConfig(simulation_project, os.path.relpath(working_directory, simulation_project.get_full_path(".")), ini_file=ini_file, config=config, sim_time_limit=sim_time_limit, num_runs=num_runs, abstract=abstract, expected_result=expected_result, user_interface=user_interface, description=description)
        simulation_configs.append(simulation_config)
    return simulation_configs

def collect_all_simulation_configs(simulation_project, ini_path_globs, concurrent=True, **kwargs):
    logger.info("Collecting all simulation configs started")
    ini_paths = list(itertools.chain.from_iterable(map(lambda g: glob.glob(g, recursive=True), ini_path_globs)))
    if concurrent:
        pool = multiprocessing.pool.ThreadPool(multiprocessing.cpu_count())
        result = list(itertools.chain.from_iterable(pool.map(functools.partial(collect_ini_file_simulation_configs, simulation_project), ini_paths)))
    else:
        result = list(itertools.chain.from_iterable(map(functools.partial(collect_ini_file_simulation_configs, simulation_project), ini_paths)))
    result.sort(key=lambda element: (element.working_directory, element.ini_file, element.config))
    logger.info("Collecting all simulation configs ended")
    return result

def get_all_simulation_configs(simulation_project, **kwargs):
    # TODO when scripts are run from a folder, only look for simulation configs under that folder
    if simulation_project.simulation_configs is None:
        relative_path = os.path.relpath(os.getcwd(), simulation_project.get_full_path("."))
        if relative_path == "." or relative_path[0:2] == "..":
            ini_path_globs = list(map(lambda ini_file_folder: simulation_project.get_full_path(os.path.join(ini_file_folder, "**/*.ini")), simulation_project.ini_file_folders))
        else:
            ini_path_globs = [os.getcwd() + "/**/*.ini"]
        simulation_project.simulation_configs = collect_all_simulation_configs(simulation_project, ini_path_globs, **kwargs)
    return simulation_project.simulation_configs

def get_simulation_configs(simulation_project=default_project, simulation_configs=None, **kwargs):
    if simulation_configs is None:
        simulation_configs = get_all_simulation_configs(simulation_project, **kwargs)
    return list(builtins.filter(lambda simulation_config: simulation_config.matches_filter(**kwargs), simulation_configs))
