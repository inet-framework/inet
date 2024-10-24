import glob
import logging
import os
import re
import shutil

from inet.common import *
from inet.common.util import get_workspace_path

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
            return self.get_full_path(self.executable + ("_release" if re.search(r"opp_run", self.executable) else ""))
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

    def collect_binary_simulation_distribution_file_names(self):
        file_names = []
        file_names.append(get_workspace_path("omnetpp/bin/opp_run_release"))
        file_names.append(get_workspace_path("omnetpp/bin/opp_python_repl"))
        file_names += list(filter(lambda path: not re.search(r"dbg", path) and not re.search(r"sanitize", path), glob.glob(get_workspace_path("omnetpp/lib/*.so"))))
        file_names += list(filter(lambda path: not re.search(r"formatter", path), glob.glob(get_workspace_path("omnetpp/python/**/*.py"), recursive=True)))
        file_names.append(self.get_full_path(".nedfolders"))
        file_names.append(self.get_full_path(".nedexclusions"))
        file_names.append(self.get_full_path("bin/inet"))
        file_names.append(self.get_full_path("bin/inet_python_repl"))
        file_names.append(self.get_full_path("bin/inet_run_simulations"))
        file_names.append(self.get_full_path("src/libINET.so"))
        file_names += glob.glob(self.get_full_path("python/**/*.py"), recursive=True)
        for ned_folder in self.ned_folders:
            if not re.search(r"test", ned_folder):
                file_names += glob.glob(os.path.join(self.get_full_path(ned_folder), "**/*.ini"), recursive=True)
                file_names += glob.glob(os.path.join(self.get_full_path(ned_folder), "**/*.ned"), recursive=True)
        return file_names

    def create_binary_simulation_distribution(self, folder=os.path.join(os.path.expanduser("~"), "inet-distribution")):
        for source_file_name in self.collect_binary_simulation_distribution_file_names():
            workspace_relative_filename = os.path.relpath(source_file_name, get_workspace_path("."))
            destination_file_name = os.path.join(folder, workspace_relative_filename)
            try:
                os.makedirs(os.path.dirname(destination_file_name))
            except FileExistsError:
                pass
            shutil.copy(source_file_name, destination_file_name)
        with open(os.path.join(folder, "omnetpp/setenv"), "w") as file:
            file.write("""#!/usr/bin/env -S sh -c "echo >&2 \"Error: You are running this script instead of sourcing it. Make sure to use it as 'source setenv' or '. setenv', otherwise its settings won't take effect.\"; exit 1"
export OMNETPP_ROOT=$(pwd)
export PATH=$OMNETPP_ROOT/bin:$PATH
export PYTHONPATH=$OMNETPP_ROOT/python:$PYTHONPATH
""")
        with open(os.path.join(folder, "inet/setenv"), "w") as file:
            file.write("""#!/usr/bin/env -S sh -c "echo >&2 \"Error: You are running this script instead of sourcing it. Make sure to use it as 'source setenv' or '. setenv', otherwise its settings won't take effect.\"; exit 1"
export INET_ROOT=$(pwd)
export INET_OMNETPP_OPTIONS="--image-path=$INET_ROOT/images"
export PATH=$INET_ROOT/bin:$PATH
export PYTHONPATH=$INET_ROOT/python:$PYTHONPATH
""")
        return folder

    def copy_binary_simulation_distribution_to_cluster(self, cluster):
        folder = self.create_binary_simulation_distribution()
        hostname = socket.gethostname()
        for k, worker in cluster.workers.items():
            worker_hostname = worker.address
            if hostname != worker_hostname:
                os.system(f'rsync -r "{folder}" "{worker_hostname}:."')
            os.system(f'rsync "{self.get_full_path("bin/inet_ssh_cluster_python")}" "{worker_hostname}:{folder}/inet/bin"')

simulation_projects = dict()

def get_simulation_project(name, workspace_path, **kwargs):
    if not name in simulation_projects:
        simulation_projects[name] = SimulationProject(workspace_path, **kwargs)
    return simulation_projects[name]

inet_project = get_simulation_project("inet", os.getenv("INET_ROOT"),
                                      executable=os.path.join(os.getenv("__omnetpp_root_dir"), "bin/opp_run"),
                                      libraries=["src/INET"],
                                      ned_folders=["src", "examples", "showcases", "tutorials", "tests/networks", "tests/validation"],
                                      ned_exclusions=[s.strip() for s in open(get_workspace_path("inet/.nedexclusions")).readlines()],
                                      ini_file_folders=["examples", "showcases", "tutorials", "tests/fingerprint", "tests/validation"],
                                      image_folders=["images"])

inet_baseline_project = get_simulation_project("inet-baseline", get_workspace_path("inet-baseline"),
                                               executable=inet_project.executable,
                                               libraries=inet_project.libraries,
                                               ned_folders=inet_project.ned_folders,
                                               ini_file_folders=inet_project.ini_file_folders,
                                               image_folders=inet_project.image_folders)

default_project=inet_project
