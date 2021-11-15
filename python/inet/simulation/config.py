import glob
import itertools
import logging
import multiprocessing
import os
import re
import subprocess

from inet.common import *

logger = logging.getLogger(__name__)
all_simulation_configs = None

class SimulationConfig:
    def __init__(self, working_directory, ini_file, config, num_runs, abstract, interactive, description):
        self.working_directory = working_directory
        self.ini_file = ini_file
        self.config = config
        self.num_runs = num_runs
        self.abstract = abstract
        self.interactive = interactive
        self.description = description

    def __repr__(self):
        return repr(self)

num_runs_fast_regex = re.compile("(?m).*^\\s*(include\\s+.*\.ini|repeat\\s*=\s*[0-9]+|${.*}).*")

def get_num_runs_fast(ini_path):
    file = open(ini_path, "r")
    text = file.read()
    file.close()
    return None if num_runs_fast_regex.findall(text) else 1

def collect_ini_file_simulation_configs(ini_path):
    simulation_configs = []
    working_directory = os.path.dirname(ini_path)
    num_runs_fast = get_num_runs_fast(ini_path)
    ini_file = os.path.basename(ini_path)
    file = open(ini_path)
    config_objects = []
    config_object = {}
    for line in file:
        match = re.match("\\[Config (.*)\\]|\\[(General)\\]", line)
        if match:
            config = match.group(1) or match.group(2)
            config_object = {"config": config, "description": None}
            config_objects.append(config_object)
        match = re.match("description *= *\"(.*)\"", line)
        if match:
            config_object["description"] = match.group(1)
    for config_object in config_objects:
        config = config_object["config"]
        args = ["inet", "-s", "-f", ini_file, "-c", config, "-q", "numruns"]
        logger.debug(args)
        if num_runs_fast:
            num_runs = num_runs_fast
        else:
            result = subprocess.run(args, cwd=working_directory, capture_output=True)
            num_runs = int(result.stdout)
        description = config_object["description"]
        abstract = (re.search("(a|A)bstract.*", description) is not None) if description else False
        interactive = False # TODO
        simulation_config = SimulationConfig(os.path.relpath(working_directory, get_full_path(".")), ini_file, config, num_runs, abstract, interactive, description)
        simulation_configs.append(simulation_config)
    return simulation_configs

def collect_all_simulation_configs(ini_path_globs):
    logger.info("Collecting all simulation configs")
    ini_paths = list(itertools.chain.from_iterable(map(lambda g: glob.glob(g, recursive=True), ini_path_globs)))
    pool = multiprocessing.Pool(multiprocessing.cpu_count())
    result = list(itertools.chain.from_iterable(pool.map(collect_ini_file_simulation_configs, ini_paths)))
    result.sort(key=lambda element: (element.working_directory, element.ini_file, element.config))
    return result

def get_all_simulation_configs():
    global all_simulation_configs
    if all_simulation_configs is None:
        if get_full_path(".") == os.getcwd():
            ini_path_globs = [get_full_path(".") + "/examples/**/*.ini",
                              get_full_path(".") + "/showcases/**/*.ini",
                              get_full_path(".") + "/tutorials/**/*.ini",
                              get_full_path(".") + "/tests/validation/**/*.ini"]
        else:
            ini_path_globs = [os.getcwd() + "/**/*.ini"]
        all_simulation_configs = collect_all_simulation_configs(ini_path_globs)
    return all_simulation_configs

def matches_filter(value, positive_filter, negative_filter, full_match):
    return (re.search(positive_filter if full_match else ".*" + positive_filter + ".*", value) if positive_filter else True) and \
           not (re.search(negative_filter if full_match else ".*" + negative_filter + ".*", value) if negative_filter else False)

def get_simulation_configs(simulation_configs=None,
                           working_directory_filter=None, exclude_working_directory_filter=None,
                           ini_file_filter=None, exclude_ini_file_filter=None,
                           config_filter=None, exclude_config_filter=None,
                           simulation_config_filter=lambda simulation_config: not simulation_config.abstract and not simulation_config.interactive,
                           full_match=False, **kwargs):
    if simulation_configs is None:
        simulation_configs = get_all_simulation_configs()
    return list(filter(lambda simulation_config: matches_filter(simulation_config.working_directory, working_directory_filter, exclude_working_directory_filter, full_match) and
                                                 matches_filter(simulation_config.ini_file, ini_file_filter, exclude_ini_file_filter, full_match) and
                                                 matches_filter(simulation_config.config, config_filter, exclude_config_filter, full_match) and
                                                 simulation_config_filter(simulation_config),
                       simulation_configs))
