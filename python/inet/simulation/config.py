import functools
import glob
import itertools
import logging
import multiprocessing
import os
import re
import subprocess

from inet.common import *

logger = logging.getLogger(__name__)

class SimulationConfig:
    def __init__(self, simulation_project, working_directory, ini_file, config, num_runs, abstract, description):
        self.simulation_project = simulation_project
        self.working_directory = working_directory
        self.ini_file = ini_file
        self.config = config
        self.num_runs = num_runs
        self.abstract = abstract
        self.emulation = working_directory.find("emulation") != -1
        self.description = description

    def __repr__(self):
        return repr(self)

num_runs_fast_regex = re.compile(r"(?m).*^\s*(include\s+.*\.ini|repeat\s*=\s*[0-9]+|.*\$\{.*\})")

def get_num_runs_fast(ini_path):
    file = open(ini_path, "r", encoding="utf-8")
    text = file.read()
    file.close()
    return None if num_runs_fast_regex.search(text) else 1

def collect_ini_file_simulation_configs(simulation_project, ini_path):
    simulation_configs = []
    working_directory = os.path.dirname(ini_path)
    num_runs_fast = get_num_runs_fast(ini_path)
    ini_file = os.path.basename(ini_path)
    file = open(ini_path, encoding="utf-8")
    config_dicts = []
    config_dict = {}
    for line in file:
        match = re.match("\\[Config (.*)\\]|\\[(General)\\]", line)
        if match:
            config = match.group(1) or match.group(2)
            config_dict = {"config": config, "abstract_config": False, "description": None, "network": None}
            config_dicts.append(config_dict)
        match = re.match("#? *abstract-config *= *(\w+)", line)
        if match:
            config_dict["abstract_config"] = bool(match.group(1))
        match = re.match("description *= *\"(.*)\"", line)
        if match:
            config_dict["description"] = match.group(1)
        match = re.match("network *= *(.*)", line)
        if match:
            config_dict["network"] = match.group(1)
    for config_dict in config_dicts:
        config = config_dict["config"]
        executable = simulation_project.get_full_path("bin/inet")
        args = [executable, "-s", "-f", ini_file, "-c", config, "-q", "numruns"]
        logger.debug(args)
        if num_runs_fast:
            num_runs = num_runs_fast
        else:
            env = os.environ.copy()
            env["INET_ROOT"] = simulation_project.get_full_path(".")
            result = subprocess.run(args, cwd=working_directory, capture_output=True, env=env)
            num_runs = int(result.stdout)
        description = config_dict["description"]
        description_abstract = (re.search("\((a|A)bstract\)", description) is not None) if description else False
        abstract = (config_dict["network"] is None and config_dict["config"] == "General") or config_dict["abstract_config"] or description_abstract 
        simulation_config = SimulationConfig(simulation_project, os.path.relpath(working_directory, simulation_project.get_full_path(".")), ini_file, config, num_runs, abstract, description)
        simulation_configs.append(simulation_config)
    return simulation_configs

def collect_all_simulation_configs(simulation_project, ini_path_globs, concurrent=True, **kwargs):
    logger.info("Collecting all simulation configs")
    ini_paths = list(itertools.chain.from_iterable(map(lambda g: glob.glob(g, recursive=True), ini_path_globs)))
    if concurrent:
        pool = multiprocessing.pool.ThreadPool(multiprocessing.cpu_count())
        result = list(itertools.chain.from_iterable(pool.map(functools.partial(collect_ini_file_simulation_configs, simulation_project), ini_paths)))
    else:
        result = list(itertools.chain.from_iterable(map(functools.partial(collect_ini_file_simulation_configs, simulation_project), ini_paths)))
    result.sort(key=lambda element: (element.working_directory, element.ini_file, element.config))
    return result

def get_all_simulation_configs(simulation_project, **kwargs):
    # TODO when scripts are run from a folder, only look for simulation configs under that folder
    if simulation_project.simulation_configs is None:
        relative_path = os.path.relpath(os.getcwd(), simulation_project.get_full_path("."))
        if relative_path == "." or relative_path[0:2] == "..":
            ini_path_globs = [simulation_project.get_full_path(".") + "/examples/**/*.ini",
                              simulation_project.get_full_path(".") + "/showcases/**/*.ini",
                              simulation_project.get_full_path(".") + "/tutorials/**/*.ini",
                              simulation_project.get_full_path(".") + "/tests/fingerprint/*.ini",
                              simulation_project.get_full_path(".") + "/tests/validation/**/*.ini"]
        else:
            ini_path_globs = [os.getcwd() + "/**/*.ini"]
        simulation_project.simulation_configs = collect_all_simulation_configs(simulation_project, ini_path_globs, **kwargs)
    return simulation_project.simulation_configs

def get_simulation_configs(simulation_project, simulation_configs=None,
                           working_directory_filter=None, exclude_working_directory_filter=None,
                           ini_file_filter=None, exclude_ini_file_filter=None,
                           config_filter=None, exclude_config_filter=None,
                           simulation_config_filter=lambda simulation_config: not simulation_config.abstract and not simulation_config.emulation,
                           full_match=False, **kwargs):
    if simulation_configs is None:
        simulation_configs = get_all_simulation_configs(simulation_project, **kwargs)
    return list(filter(lambda simulation_config: matches_filter(simulation_config.working_directory, working_directory_filter, exclude_working_directory_filter, full_match) and
                                                 matches_filter(simulation_config.ini_file, ini_file_filter, exclude_ini_file_filter, full_match) and
                                                 matches_filter(simulation_config.config, config_filter, exclude_config_filter, full_match) and
                                                 simulation_config_filter(simulation_config),
                       simulation_configs))
