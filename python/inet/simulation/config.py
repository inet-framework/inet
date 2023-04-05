"""
Provides abstractions for simulation configs.

Please note that undocumented features are not supposed to be used by the user.
"""

import builtins
import functools
import glob
import hashlib
import itertools
import logging
import multiprocessing
import os
import re
import shutil
import subprocess

from inet.common.util import *

_logger = logging.getLogger(__name__)

class SimulationConfig:
    """
    Represents a simulation config from an INI file under a working directory in a specific simulation project.
    """

    def __init__(self, simulation_project, working_directory, ini_file="omnetpp.ini", config="General", num_runs=1, sim_time_limit=None, abstract=False, emulation=False, expected_result="DONE", user_interface="Cmdenv", description=None):
        """
        Initializes a new simulation config.

        Parameters:
            simulation_project (:py:class:`omnetpp.simulation.project.SimulationProject`):
                The simulation project that this simulation config belongs to.

            working_directory (string):
                The working directory relative to the root directory of the simulation project.

            ini_file (string):
                The name of the INI file within the working directory.

            config (string):
                The name of the configuration section within the INI file.

            num_runs (integer):
                The total number of runs specified by the configuration section of the INI file.

            sim_time_limit (string):
                The simulation time limit specified by the configuration section of the INI file.

            abstract (bool):
                Specifies if the simulation config is not final (i.e. it's meant to be derived from) and thus it cannot be run.

            emulation (bool):
                Specifies if the simulation config is used for emulation, so it cannot be run without setting up the
                external software and hardware resources.

            expected_result (string):
                The expected result of running the simulation.

            user_interface (string):
                The user interface extracted from the INI file.

            description (string):
                The human readable description extracted from the INI file.
        """
        self.simulation_project = simulation_project
        self.working_directory = working_directory
        self.ini_file = ini_file
        self.config = config
        self.num_runs = num_runs
        self.sim_time_limit = sim_time_limit
        self.abstract = abstract
        self.emulation = emulation or working_directory.find("emulation") != -1
        self.expected_result = expected_result
        self.user_interface = user_interface
        self.description = description

    def __repr__(self):
        return repr(self, ["working_directory", "ini_file", "config"])

    def get_ini_file_full_path(self):
        return self.simulation_project.get_full_path(os.path.join(self.working_directory, self.ini_file))

    def get_hash(self, **kwargs):
        hasher = hashlib.sha256()
        hasher.update(open(self.get_ini_file_full_path(), "rb").read())
        hasher.update(self.config.encode("utf-8"))
        return hasher.digest()

    def matches_filter(self, filter=None, exclude_filter=None,
                       working_directory_filter=None, exclude_working_directory_filter=None,
                       ini_file_filter=None, exclude_ini_file_filter=None,
                       config_filter=None, exclude_config_filter=None,
                       simulation_config_filter=lambda simulation_config: not simulation_config.abstract and not simulation_config.emulation,
                       full_match=False, **kwargs):
        """
        Determines if this simulation config matches the provided filter parameters or not. If a filter parameter is
        not specified, then it is ignored.

        Parameters:
            filter (string or None):
                A regular expression that must match the string representation of the simulation config.

            exclude_filter (string or None):
                A regular expression that must not match the string representation of the simulation config.

            working_directory_filter (string or None):
                A regular expression that must match the working directory of the simulation config.

            exclude_working_directory_filter (string or None):
                A regular expression that must not match the working directory of the simulation config.

            ini_file_filter (string or None):
                A regular expression that must match the INI file of the simulation config.

            exclude_ini_file_filter (string or None):
                A regular expression that must not match the INI file of the simulation config.

            config_filter (string or None):
                A regular expression that must match the config of the simulation config.

            exclude_config_filter (string or None):
                A regular expression that must not match the config of the simulation config.

            simulation_config_filter (function or None):
                A predicate that is additionally called with the simulation config. By default, abstract and emulation
                simulation configs do not match.

            full_match (bool):
                Specifies if the regular expression parameters must match completely or a substring match is sufficient.

            kwargs (dict):
                Ignored.

        Returns (bool):
            Flag indicating that the simulation config matches the provided filter criteria.
        """
        return matches_filter(self.__repr__(), filter, exclude_filter, full_match) and \
               matches_filter(self.working_directory, working_directory_filter, exclude_working_directory_filter, full_match) and \
               matches_filter(self.ini_file, ini_file_filter, exclude_ini_file_filter, full_match) and \
               matches_filter(self.config, config_filter, exclude_config_filter, full_match) and \
               simulation_config_filter(self)
    
    def clean_simulation_results(self):
        _logger.info("Cleaning simulation results, folder = " + self.working_directory)
        simulation_project = self.simulation_project
        path = os.path.join(simulation_project.get_full_path(self.working_directory), "results")
        if not re.search(".*/home/.*", path):
            raise Exception("Path is not in home")
        if os.path.exists(path):
            shutil.rmtree(path)
