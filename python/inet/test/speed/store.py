import json
import logging
import os
import subprocess
import time

from inet.common import *
from inet.simulation.project import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

_speed_measurement_stores = dict()

class SpeedMeasurementStore:
    def __init__(self, simulation_project, file_name):
        self.simulation_project = simulation_project
        self.file_name = file_name
        self.entries = None

    def read(self):
        _logger.info(f"Reading speed measurements from {self.file_name}")
        file = open(self.file_name, "r")
        self.entries = json.load(file)
        file.close()

    def write(self):
        self.get_entries().sort(key=lambda element: (element["working_directory"], element["ini_file"], element["config"], element["run_number"], element["sim_time_limit"]))
        _logger.info(f"Writing speed measurements to {self.file_name}")
        file = open(self.file_name, "w")
        json.dump(self.entries, file, indent=True)
        file.close()

    def ensure(self):
        if os.path.exists(self.file_name):
            self.read()
        else:
            self.entries = []
            self.write()

    def clear(self):
        self.entries = []

    def reset(self):
        self.entries = None

    def get_entries(self):
        if self.entries is None:
            self.ensure()
        return self.entries
    
    def set_entries(self, entries):
        self.entries = entries

    def find_entry(self, **kwargs):
        result = self.filter_entries(**kwargs)
        return result[0] if len(result) == 1 else None

    def get_entry(self, **kwargs):
        result = self.find_entry(**kwargs)
        if result is not None:
            return result
        else:
            print(kwargs)
            raise Exception("Entry not found")

    def remove_entry(self, entry):
        self.get_entries().remove(entry)

    def filter_entries(self, test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run_number=0, sim_time_limit=None, itervars=None):
        def f(entry):
            return (working_directory is None or entry["working_directory"] == working_directory) and \
                   (ini_file is None or entry["ini_file"] == ini_file) and \
                   (config is None or entry["config"] == config) and \
                   (run_number is None or entry["run_number"] == run_number) and \
                   (sim_time_limit is None or entry["sim_time_limit"] == sim_time_limit) and \
                   (itervars is None or entry["itervars"] == itervars) and \
                   (test_result is None or entry["test_result"] == test_result)
        return list(filter(f, self.get_entries()))

    def find_elapsed_wall_time(self, **kwargs):
        entry = self.find_entry(**kwargs)
        if entry is not None:
            return entry["elapsed_wall_time"]
        else:
            return None

    def get_elapsed_wall_time(self, **kwargs):
        return self.get_entry(**kwargs)["elapsed_wall_time"]
    
    def set_elapsed_wall_time(self, elapsed_wall_time, **kwargs):
        self.get_entry(**kwargs)["elapsed_wall_time"] = elapsed_wall_time

    def insert_elapsed_wall_time(self, elapsed_wall_time, test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run_number=0, sim_time_limit=None, itervars="$repetition==0"):
        # assert test_result == "ERROR" or sim_time_limit is not None
        self.get_entries().append({"working_directory": working_directory,
                                   "ini_file": ini_file,
                                   "config": config,
                                   "run_number": run_number,
                                   "sim_time_limit": sim_time_limit,
                                   "test_result": test_result,
                                   "elapsed_wall_time": elapsed_wall_time,
                                   "timestamp": time.time(),
                                   "itervars": itervars})

    def update_elapsed_wall_time(self, elapsed_wall_time, **kwargs):
        entry = self.find_entry(**kwargs)
        if entry:
            entry["elapsed_wall_time"] = elapsed_wall_time
        else:
            self.insert_elapsed_wall_time(elapsed_wall_time, **kwargs)

    def remove_elapsed_wall_times(self, **kwargs):
        list(map(lambda element: self.entries.remove(element), self.filter_entries(**kwargs)))

def get_speed_measurement_store(simulation_project):
    if not simulation_project in _speed_measurement_stores:
        _speed_measurement_stores[simulation_project] = SpeedMeasurementStore(simulation_project, simulation_project.get_full_path(simulation_project.speed_store))
    return _speed_measurement_stores[simulation_project]
