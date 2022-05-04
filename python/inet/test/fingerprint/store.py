import json
import logging
import os
import subprocess
import time

from inet.common import *
from inet.simulation.project import *

logger = logging.getLogger(__name__)

class FingerprintStore:
    def __init__(self, simulation_project, file_name):
        self.simulation_project = simulation_project
        self.file_name = file_name
        self.entries = None

    def read(self):
        logger.info(f"Reading fingerprints from {self.file_name}")
        if os.path.exists(self.file_name):
            file = open(self.file_name)
            self.entries = json.load(file)
            file.close()
        else:
            self.entries = []
            self.write()

    def write(self):
        self.get_entries().sort(key=lambda element: (element["working_directory"], element["ini_file"], element["config"], element["run"], element["ingredients"], element["sim_time_limit"]))
        logger.info(f"Writing fingerprints to {self.file_name}")
        file = open(self.file_name, "w")
        json.dump(self.entries, file, indent=True)
        file.close()

    def clear(self):
        self.entries = []

    def reset(self):
        self.entries = None

    def get_entries(self):
        if self.entries is None:
            self.read()
        return self.entries
    
    def set_entries(self, entries):
        self.entries = entries

    def get_latest_entries(self):
        latest_entries = []
        for entry in self.entries:
            found = False
            new_latest_entries = []
            for latest_entry in latest_entries:
                if entry["working_directory"] == latest_entry["working_directory"] and \
                   entry["ini_file"] == latest_entry["ini_file"] and \
                   entry["config"] == latest_entry["config"] and \
                   entry["run"] == latest_entry["run"] and \
                   entry["sim_time_limit"] == latest_entry["sim_time_limit"] and \
                   entry["ingredients"] == latest_entry["ingredients"]:
                    if entry["timestamp"] < latest_entry["timestamp"]:
                        found = True
                        new_latest_entries.append(latest_entry)
                else:
                    new_latest_entries.append(latest_entry)
            latest_entries = new_latest_entries
            if not found:
                new_latest_entries.append(entry)
        latest_entries.sort(key=lambda element: (element["working_directory"], element["ini_file"], element["config"], element["run"], element["ingredients"], element["sim_time_limit"]))
        return latest_entries
    
    def find_entry(self, **kwargs):
        result = self.filter_entries(**kwargs)
        return result[0] if len(result) == 1 else None

    def get_entry(self, **kwargs):
        result = self.find_entry(**kwargs)
        if result is not None:
            return result
        else:
            raise Exception("Entry not found")

    def remove_entry(self, entry):
        self.get_entries().remove(entry)

    def filter_entries(self, ingredients="tplx", test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None, itervars=None):
        def f(fingerprint):
            return (working_directory is None or fingerprint["working_directory"] == working_directory) and \
                   (ini_file is None or fingerprint["ini_file"] == ini_file) and \
                   (config is None or fingerprint["config"] == config) and \
                   (run is None or fingerprint["run"] == run) and \
                   (sim_time_limit is None or fingerprint["sim_time_limit"] == sim_time_limit) and \
                   (itervars is None or fingerprint["itervars"] == itervars) and \
                   (test_result is None or fingerprint["test_result"] == test_result) and \
                   (ingredients is None or fingerprint["ingredients"] == ingredients)
        return list(filter(f, self.get_entries()))
    
    def get_fingerprint(self, **kwargs):
        return self.get_entry(**kwargs)["fingerprint"]
    
    def set_fingerprint(self, fingerprint, **kwargs):
        self.get_entry(**kwargs)["fingerprint"] = fingerprint

    def insert_fingerprint(self, fingerprint, ingredients="tplx", test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None, git_hash=None, git_clean=None, itervars="$repetition==0"):
        assert test_result == "ERROR" or sim_time_limit is not None
        if git_hash is None:
            git_hash = subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.simulation_project.get_full_path("."), capture_output=True).stdout.decode("utf-8").strip()
        if git_clean is None:
            git_clean = subprocess.run(["git", "diff", "--quiet"], cwd=self.simulation_project.get_full_path("."), capture_output=True).returncode == 0
        self.get_entries().append({"working_directory": working_directory,
                                   "ini_file": ini_file,
                                   "config": config,
                                   "run": run,
                                   "sim_time_limit": sim_time_limit,
                                   "test_result": test_result,
                                   "ingredients": ingredients,
                                   "fingerprint": fingerprint,
                                   "timestamp": time.time(),
                                   "git_hash": git_hash,
                                   "git_clean": git_clean,
                                   "itervars": itervars})

    def update_fingerprint(self, fingerprint, **kwargs):
        entry = self.find_entry(**kwargs)
        if entry:
            entry["fingerprint"] = fingerprint
        else:
            self.insert_fingerprint(fingerprint, **kwargs)

    def remove_fingerprints(self, **kwargs):
        list(map(lambda element: self.entries.remove(element), self.filter_entries(**kwargs)))

all_fingerprint_stores = dict()
correct_fingerprint_stores = dict()

def get_all_fingerprint_store(simulation_project):
    if not simulation_project in all_fingerprint_stores:
        all_fingerprint_stores[simulation_project] = FingerprintStore(simulation_project, simulation_project.get_full_path("python/inet/test/fingerprint/all_fingerprints.json"))
    return all_fingerprint_stores[simulation_project]

def get_correct_fingerprint_store(simulation_project):
    if not simulation_project in correct_fingerprint_stores:
        correct_fingerprint_stores[simulation_project] = FingerprintStore(simulation_project, simulation_project.get_full_path("python/inet/test/fingerprint/correct_fingerprints.json"))
    return correct_fingerprint_stores[simulation_project]
