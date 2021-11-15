import json
import logging
import os
import subprocess
import time

from inet.common import *

logger = logging.getLogger(__name__)

class FingerprintStore:
    def __init__(self, file_name):
        self.file_name = file_name
        self.entries = None

    def read(self):
        logger.info("Reading fingerprints from " + self.file_name)
        file = open(self.file_name)
        self.entries = json.load(file)
        file.close()

    def write(self):
        logger.info("Writing fingerprints to " + self.file_name)
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
    
    def get_entry(self, **kwargs):
        result = self.filter_entries(**kwargs)
        return result[0] if result and len(result) == 1 else None

    def filter_entries(self, ingredients="tplx", test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None):
        def f(fingerprint):
            return fingerprint["working_directory"] == working_directory and \
                   fingerprint["ini_file"] == ini_file and \
                   fingerprint["config"] == config and \
                   fingerprint["run"] == str(run) and \
                   (sim_time_limit is None or fingerprint["sim_time_limit"] == sim_time_limit) and \
                   (test_result is None or fingerprint["test_result"] == test_result) and \
                   fingerprint["ingredients"] == ingredients
        assert run is not None
        return list(filter(f, self.get_entries()))
    
    def get_fingerprint(self, **kwargs):
        return self.get_entry(**kwargs)["fingerprint"]
    
    def set_fingerprint(self, fingerprint, **kwargs):
        self.get_entry(**kwargs)["fingerprint"] = fingerprint

    def insert_fingerprint(self, fingerprint, ingredients="tplx", test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None):
        git_hash = subprocess.run(["git", "rev-parse", "HEAD"], cwd=get_full_path("."), capture_output=True).stdout.decode("utf-8").strip()
        git_clean = subprocess.run(["git", "diff", "--quiet"], cwd=get_full_path("."), capture_output=True).returncode == 0
        self.get_entries().append({"working_directory": working_directory,
                                   "ini_file": ini_file,
                                   "config": config,
                                   "run": str(run),
                                   "sim_time_limit": sim_time_limit,
                                   "test_result": test_result,
                                   "ingredients": ingredients,
                                   "fingerprint": fingerprint,
                                   "timestamp": time.time(),
                                   "git_hash": git_hash,
                                   "git_clean": git_clean})

    def update_fingerprint(self, fingerprint, **kwargs):
        entry = self.get_entry(**kwargs)
        if entry:
            entry["fingerprint"] = fingerprint
        else:
            self.insert_fingerprint(fingerprint, **kwargs)

    def remove_fingerprint(self, ingredients="tplx", test_result=None, working_directory=os.getcwd(), ini_file="omnetpp.ini", config="General", run=0, sim_time_limit=None):
        pass
    
all_fingerprints = FingerprintStore(get_full_path("python/inet/test/fingerprint/all_fingerprints.json"))
correct_fingerprints = FingerprintStore(get_full_path("python/inet/test/fingerprint/correct_fingerprints.json"))
