import logging
import os

from inet.common import *

logger = logging.getLogger(__name__)

class SimulationProject:
    def __init__(self, project_directory, executable=None, libraries=[], ned_folders=["."], ned_exclusions=[], ini_file_folders=["."], image_folders=["."]):
        self.project_directory = project_directory
        self.executable = executable
        self.libraries = libraries
        self.ned_folders = ned_folders
        self.ned_exclusions = ned_exclusions
        self.ini_file_folders = ini_file_folders
        self.image_folders = image_folders
        self.simulation_configs = None

    def get_name(self):
        return os.path.basename(self.get_full_path("."))

    def get_env(self):
        env = os.environ.copy()
        env["INET_ROOT"] = self.get_full_path(".")
        env["TSNSCHED_ROOT"] = self.get_full_path("../TSNsched")
        return env

    def get_full_path(self, path):
        return os.path.abspath(os.path.join(self.project_directory, path))

    def get_executable(self, mode="debug"):
        if mode == "release":
            return self.get_full_path(self.executable + ("_release" if self.executable == "opp_run" else ""))
        elif mode == "debug":
            return self.get_full_path(self.executable + "_dbg")
        elif mode == "sanitize":
            return self.get_full_path(self.executable + "_sanitize")
        else:
            raise Exception(f"Unknown mode: {mode}")

    def get_multiple_args(self, option, elements):
        args = []
        for element in elements:
            args.append(option)
            args.append(element)
        return args

    def get_full_path_args(self, option, paths):
        return self.get_multiple_args(option, map(self.get_full_path, paths))

    def get_default_args(self):
        return [*self.get_full_path_args("-l", self.libraries), *self.get_full_path_args("-n", self.ned_folders), *self.get_multiple_args("-x", self.ned_exclusions), *self.get_full_path_args("--image-path", self.image_folders)]

simulation_projects = dict()

def get_simulation_project(name, **kwargs):
    if not name in simulation_projects:
        workspace_path = get_workspace_path(name)
        simulation_projects[name] = SimulationProject(workspace_path, **kwargs)
    return simulation_projects[name]

aloha_project = get_simulation_project("omnetpp/samples/aloha", executable="aloha")

tictoc_project = get_simulation_project("omnetpp/samples/tictoc", executable="tictoc")

inet_project = get_simulation_project("inet",
                                      executable=get_workspace_path("omnetpp/bin/opp_run"),
                                      libraries=["src/INET"],
                                      ned_folders=["src", "examples", "showcases", "tutorials", "tests/networks", "tests/validation"],
                                      ned_exclusions=[s.strip() for s in open(get_workspace_path("inet/.nedexclusions")).readlines()],
                                      ini_file_folders=["examples", "showcases", "tutorials", "tests/fingerprint", "tests/validation"],
                                      image_folders=["images"])

inet_baseline_project = get_simulation_project("inet-baseline",
                                               executable=inet_project.executable,
                                               libraries=inet_project.libraries,
                                               ned_folders=inet_project.ned_folders,
                                               ini_file_folders=inet_project.ini_file_folders,
                                               image_folders=inet_project.image_folders)

default_project=inet_project
