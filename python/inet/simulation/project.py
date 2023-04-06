"""
Provides abstractions for simulation projects.

The main functions are:

 - :py:func:`get_simulation_project`:
 - :py:func:`get_default_simulation_project`:

In general, it's a good idea to use the default project, because it makes calling the various functions easier and in
most cases there's only one simulation project is worked on. The default simulation project is automatically set to the
one above the current working directory when the omnetpp.repl package is loaded, but it can be overridden by the user
later.

Please note that undocumented features are not supposed to be used by the user.
"""

import glob
import hashlib
import json
import logging
import os
import re
import shutil
import socket
import subprocess

try:
    from omnetpp.runtime.omnetpp import *
except:
    pass

from inet.common.util import *
from inet.simulation.config import *

_logger = logging.getLogger(__name__)

class SimulationProject:
    """
    Represents a simulation project that usually comes with its own modules and their C++ implementation, and also with
    several example simulations.

    Please note that undocumented features are not supposed to be called by the user.
    """

    def __init__(self, name, version, git_hash=None, git_diff_hash=None, folder_environment_variable=None, folder=".",
                 bin_folder=".", library_folder=".", executables=None, dynamic_libraries=None, static_libraries=None, build_types=["dynamic library"],
                 ned_folders=["."], ned_exclusions=[], ini_file_folders=["."], python_folders=["python"], image_folders=["."],
                 include_folders=["."], cpp_folders=["."], cpp_defines=[], msg_folders=["."],
                 media_folder=".", statistics_folder=".", fingerprint_store="fingerprint.json",
                 used_projects=[], external_bin_folders=[], external_library_folders=[], external_libraries=[], external_include_folders=[],
                 simulation_configs=None, **kwargs):
        """
        Initializes a new simulation project.

        Parameters:
            name (string):
                The human readable name of the simulation project.

            version (string or None):
                The version string.

            git_hash (string or None):
                The git hash of the corresponding git repository for the specific version.

            git_diff_hash (string):
                The hash of the local modifications on top of the clean checkout of from the git repository.

            folder_environment_variable (string):
                The operating system environment variable.

            folder (string):
                The directory of the simulation project relative to the value of the folder_environment_variable attribute.

            bin_folder (string):
                The directory of the binary output files relative to the value of the folder_environment_variable attribute.

            library_folder (string):
                The directory of the library output files relative to the value of the folder_environment_variable attribute.

            executables (List of strings):
                The list of executables that are built.

            dynamic_libraries (List of strings):
                The list of dynamic libraries that are built.

            static_libraries (List of strings):
                TODO

            build_types (List of strings):
                The list of build output types. Valid values are "executable", "dynamic library", "static library".

            ned_folders (List of strings):
                The list of folder_environment_variable relative directories for NED files.

            ned_exclusions (List of strings):
                The list of excluded NED packages.

            ini_file_folders (List of strings):
                The list of folder_environment_variable relative directories for INI files.

            python_folders (List of strings):
                The list of folder_environment_variable relative directories for Python source files.

            image_folders (List of strings):
                The list of folder_environment_variable relative directories for image files.

            include_folders (List of strings):
                The list of folder_environment_variable relative directories for C++ include files.

            cpp_folders (List of strings):
                The list of folder_environment_variable relative directories for C++ source files.

            cpp_defines (List of strings):
                The list of C++ macro definitions that are passed to the C++ compiler.

            msg_folders (List of strings):
                The list of folder_environment_variable relative directories for MSG files.

            media_folder (String):
                The relative path of chart image files for chart tests.

            statistics_folder (String):
                The relative path of scalar statistic result files for statistical tests.

            fingerprint_store (String):
                The relative path of the JSON fingerprint store for fingerprint tests.

            used_projects (List of strings):
                The list of used simulation project names.

            external_bin_folders (List of strings):
                The list of absolute directories that contains external binaries.

            external_library_folders (List of strings):
                The list of absolute directories that contains external libraries.

            external_libraries (List of strings):
                The list external library names.

            external_include_folders (List of strings):
                The list of absolute directories that contains external C++ include files.

            simulation_configs (List of :py:class:`omnetpp.simulation.config.SimulationConfig`):
                The list of simulation configs available in this simulation project.

            kwargs (dict):
                Ignored.
        """
        self.name = name
        self.version = version
        self.folder_environment_variable = folder_environment_variable
        self.folder = folder
        # TODO this is commented out because it runs subprocesses, and it even does this from the IDE when some completely unrelated modules are loaded, sigh!
        # self.git_hash = git_hash or subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.get_full_path("."), capture_output=True).stdout.decode("utf-8").strip()
        # if git_diff_hash:
        #     self.git_diff_hash = git_diff_hash
        # else:
        #     git_diff_hasher = hashlib.sha256()
        #     git_diff_hasher.update(subprocess.run(["git", "diff", "--quiet"], cwd=self.get_full_path("."), capture_output=True).stdout)
        #     self.git_diff_hash = git_diff_hasher.digest().hex()
        self.bin_folder = bin_folder
        self.library_folder = library_folder
        self.executables = [name] if executables is None else executables
        self.dynamic_libraries = [name] if dynamic_libraries is None else dynamic_libraries
        self.static_libraries = [name] if static_libraries is None else static_libraries
        self.build_types = build_types
        self.ned_folders = ned_folders
        self.ned_exclusions = ned_exclusions
        self.ini_file_folders = ini_file_folders
        self.python_folders = python_folders
        self.image_folders = image_folders
        self.include_folders = include_folders
        self.cpp_folders = cpp_folders
        self.cpp_defines = cpp_defines
        self.msg_folders = msg_folders
        self.media_folder = media_folder
        self.statistics_folder = statistics_folder
        self.fingerprint_store = fingerprint_store
        self.used_projects = used_projects
        self.external_bin_folders = external_bin_folders
        self.external_library_folders = external_library_folders
        self.external_libraries = external_libraries
        self.external_include_folders = external_include_folders
        self.simulation_configs = simulation_configs
        self.binary_simulation_distribution_file_paths = None

    def __repr__(self):
        return repr(self, ["name", "version", "git_hash", "git_diff_hash"])

    def get_name(self):
        return os.path.basename(self.get_full_path("."))

    def get_hash(self, binary=True, **kwargs):
        hasher = hashlib.sha256()
        if binary:
            for file_path in self.get_binary_simulation_distribution_file_paths():
                hasher.update(get_file_hash(file_path))
        else:
            raise Exception("Not implemented")
        return hasher.digest()

    def get_env(self):
        return os.environ.copy()

    def get_environment_variable_relative_path(self, enviroment_variable, path):
        return os.path.abspath(os.path.join(os.environ[enviroment_variable], path)) if enviroment_variable in os.environ else None

    def get_full_path(self, path):
        return os.path.abspath(os.path.join(self.get_environment_variable_relative_path(self.folder_environment_variable, self.folder), path))

    def get_relative_path(self, path):
        return os.path.relpath(path, self.get_environment_variable_relative_path(self.folder_environment_variable, self.folder))

    def get_executable(self, mode="release", dynamic_loading=True):
        executable_environment_variable = "__omnetpp_root_dir" if dynamic_loading else self.folder_environment_variable
        executable = "bin/opp_run" if dynamic_loading else self.executables[0]
        return self.get_environment_variable_relative_path(executable_environment_variable, executable + self.get_library_suffix(mode=mode))

    def get_library_suffix(self, mode="release"):
        if mode == "release":
            return "_release"
        elif mode == "debug":
            return "_dbg"
        elif mode == "sanitize":
            return "_sanitize"
        else:
            raise Exception(f"Unknown mode: {mode}")

    def get_library_folder_full_path(self):
        return self.get_full_path(self.library_folder)

    def get_dynamic_libraries_for_running(self):
        result = []
        for library in self.dynamic_libraries:
            result.append(os.path.join(self.library_folder, library))
        for used_project in self.used_projects:
            simulation_project = get_simulation_project(used_project, None)
            result = result + list(map(simulation_project.get_full_path, simulation_project.get_dynamic_libraries_for_running()))
        return result

    def get_multiple_args(self, option, elements):
        args = []
        for element in elements:
            args.append(option)
            args.append(element)
        return args

    def get_full_path_args(self, option, paths):
        return self.get_multiple_args(option, map(self.get_full_path, paths))

    def get_default_args(self):
        return [*self.get_full_path_args("-l", self.get_dynamic_libraries_for_running()), *self.get_full_path_args("-n", self.ned_folders), *self.get_multiple_args("-x", self.ned_exclusions), *self.get_full_path_args("--image-path", self.image_folders)]

    def get_direct_include_folders(self):
        return list(map(lambda include_folder: self.get_full_path(include_folder), self.include_folders))

    def get_effective_include_folders(self):
        return self.get_direct_include_folders() + flatten(map(lambda used_project: get_simulation_project(used_project, None).get_direct_include_folders(), self.used_projects))

    def get_cpp_files(self):
        cpp_files = []
        for cpp_folder in self.cpp_folders:
            file_paths = list(filter(lambda file_path: not re.search("_m\\.cc", file_path), glob.glob(self.get_full_path(os.path.join(cpp_folder, "**/*.cc")), recursive=True)))
            cpp_files = cpp_files + list(map(lambda file_path: self.get_relative_path(file_path), file_paths))
        return cpp_files

    def get_header_files(self):
        header_files = []
        for cpp_folder in self.cpp_folders:
            file_paths = list(filter(lambda file_path: not re.search("_m\\.h", file_path), glob.glob(self.get_full_path(os.path.join(cpp_folder, "**/*.h")), recursive=True)))
            header_files = header_files + list(map(lambda file_path: self.get_relative_path(file_path), file_paths))
        return header_files

    def get_msg_files(self):
        msg_files = []
        for msg_folder in self.msg_folders:
            file_paths = glob.glob(self.get_full_path(os.path.join(msg_folder, "**/*.msg")), recursive=True)
            msg_files = msg_files + list(map(lambda file_path: self.get_relative_path(file_path), file_paths))
        return msg_files

    # KLUDGE TODO replace this with a Python binding to the C++ configuration reader
    def collect_ini_file_simulation_configs(self, ini_path):
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
        def create_config_dict(config):
            return {"config": config, "abstract_config": False, "emulation": False, "expected_result": "DONE", "user_interface": None, "description": None, "network": None}
        simulation_configs = []
        working_directory = os.path.dirname(ini_path)
        num_runs_fast = get_num_runs_fast(ini_path)
        ini_file = os.path.basename(ini_path)
        file = open(ini_path, encoding="utf-8")
        config_dicts = {"General": create_config_dict("General")}
        config_dict = {}
        for line in file:
            match = re.match("\\[(Config +)?(.*?)\\]", line)
            if match:
                config = match.group(2) or match.group(3)
                config_dict = create_config_dict(config)
                config_dicts[config] = config_dict
            match = re.match("#? *abstract-config *= *(\w+)", line)
            if match:
                config_dict["abstract_config"] = bool(match.group(1))
            match = re.match("#? *emulation *= *(\w+)", line)
            if match:
                config_dict["emulation"] = bool(match.group(1))
            match = re.match("#? *expected-result *= *\"(\w+)\"", line)
            if match:
                config_dict["expected_result"] = match.group(1)
            line = re.sub("(.*)#.*", "//1", line).strip()
            match = re.match(" *extends *= *(\w+)", line)
            if match:
                config_dict["extends"] = match.group(1)
            match = re.match(" *user-interface *= \"*(\w+)\"", line)
            if match:
                config_dict["user_interface"] = match.group(1)
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
            executable = self.get_executable(mode="release")
            if not os.path.exists(executable):
                executable = self.get_executable(mode="release")
            default_args = self.get_default_args()
            args = [executable, *default_args, "-s", "-f", ini_file, "-c", config, "-q", "numruns"]
            if num_runs_fast:
                num_runs = num_runs_fast
            else:
                try:
                    inifile_contents = InifileContents(ini_path)
                    num_runs = inifile_contents.getNumRunsInConfig(config)
                except Exception as e:
                    _logger.debug(f"Running subprocess: {args}")
                    result = subprocess.run(args, cwd=working_directory, capture_output=True, env=self.get_env())
                    if result.returncode == 0:
                        # KLUDGE: this was added to test source dependency based task result caching
                        result.stdout = re.sub("INI dependency: (.*)", "", result.stdout.decode("utf-8"))
                        num_runs = int(result.stdout)
                    else:
                        _logger.warn("Cannot determine number of runs: " + result.stderr.decode("utf-8"))
                        continue
            sim_time_limit = get_sim_time_limit(config_dicts, config)
            description = config_dict["description"]
            description_abstract = (re.search("\((a|A)bstract\)", description) is not None) if description else False
            abstract = (config_dict["network"] is None and config_dict["config"] == "General") or config_dict["abstract_config"] or description_abstract
            emulation = config_dict["emulation"]
            expected_result = config_dict["expected_result"]
            user_interface = config_dict["user_interface"] or general_config_dict["user_interface"]
            simulation_config = SimulationConfig(self, os.path.relpath(working_directory, self.get_full_path(".")), ini_file=ini_file, config=config, sim_time_limit=sim_time_limit, num_runs=num_runs, abstract=abstract, emulation=emulation, expected_result=expected_result, user_interface=user_interface, description=description)
            simulation_configs.append(simulation_config)
        return simulation_configs

    def collect_all_simulation_configs(self, ini_path_globs, concurrent=True, **kwargs):
        def local_collect_ini_file_simulation_configs(ini_path, **kwargs):
            return self.collect_ini_file_simulation_configs(ini_path, **kwargs)
        _logger.info(f"Collecting {self.name} simulation configs started")
        ini_paths = list(itertools.chain.from_iterable(map(lambda g: glob.glob(g, recursive=True), ini_path_globs)))
        if concurrent:
            pool = multiprocessing.pool.ThreadPool(multiprocessing.cpu_count())
            result = list(itertools.chain.from_iterable(pool.map(local_collect_ini_file_simulation_configs, ini_paths)))
        else:
            result = list(itertools.chain.from_iterable(map(local_collect_ini_file_simulation_configs, ini_paths)))
        result.sort(key=lambda element: (element.working_directory, element.ini_file, element.config))
        _logger.info(f"Collecting {self.name} simulation configs ended")
        return result

    def get_all_simulation_configs(self, **kwargs):
        ini_path_globs = list(map(lambda ini_file_folder: self.get_full_path(os.path.join(ini_file_folder, "**/*.ini")), self.ini_file_folders))
        return self.collect_all_simulation_configs(ini_path_globs, **kwargs)

    def get_simulation_configs(self, **kwargs):
        if self.simulation_configs is None:
            self.simulation_configs = self.get_all_simulation_configs()
        return list(builtins.filter(lambda simulation_config: simulation_config.matches_filter(**kwargs), self.simulation_configs))

    def get_binary_simulation_distribution_file_paths(self):
        if self.binary_simulation_distribution_file_paths is None:
            self.binary_simulation_distribution_file_paths = self.collect_binary_simulation_distribution_file_paths()
        return self.binary_simulation_distribution_file_paths

    def collect_binary_simulation_distribution_file_paths(self):
        file_paths = []
        def append_file_if_exists(file_name):
            if os.path.exists(file_name):
                file_paths.append(file_name)
        file_paths.append(get_omnetpp_relative_path("bin/opp_run"))
        file_paths.append(get_omnetpp_relative_path("bin/opp_run_release"))
        file_paths.append(get_omnetpp_relative_path("bin/opp_run_dbg"))
        file_paths.append(get_omnetpp_relative_path("bin/opp_python_repl"))
        file_paths.append(get_omnetpp_relative_path("bin/opp_ssh_cluster_python"))
        append_file_if_exists(get_omnetpp_relative_path("shell.nix"))
        file_paths.append(self.get_executable(mode="release"))
        file_paths.append(self.get_executable(mode="debug"))
        file_paths += list(glob.glob(get_omnetpp_relative_path("lib/*.so")))
        file_paths += list(filter(lambda path: not re.search("formatter", path), glob.glob(get_omnetpp_relative_path("python/**/*.py"), recursive=True)))
        append_file_if_exists(self.get_full_path(".omnetpp"))
        append_file_if_exists(self.get_full_path(".nedfolders"))
        append_file_if_exists(self.get_full_path(".nedexclusions"))
        file_paths += glob.glob(self.get_full_path(os.path.join("bin", "*")))
        for executable in self.executables:
            append_file_if_exists(self.get_full_path(os.path.join(self.bin_folder, executable)))
            append_file_if_exists(self.get_full_path(os.path.join(self.bin_folder, executable)))
        for dynamic_library in self.dynamic_libraries:
            append_file_if_exists(self.get_full_path(os.path.join(self.library_folder, "lib" + dynamic_library + ".so")))
            append_file_if_exists(self.get_full_path(os.path.join(self.library_folder, "lib" + dynamic_library + "_dbg.so")))
        for ned_folder in self.ned_folders:
            if not re.search("test", ned_folder):
                file_paths += glob.glob(self.get_full_path(os.path.join(ned_folder, "**/*.ini")), recursive=True)
                file_paths += glob.glob(self.get_full_path(os.path.join(ned_folder, "**/*.ned")), recursive=True)
        for python_folder in self.python_folders:
            file_paths += glob.glob(self.get_full_path(os.path.join(python_folder, "**/*.py")), recursive=True)
        return file_paths

    def create_binary_simulation_distribution(self, folder=os.path.join(os.path.expanduser("~"), "omnetpp-distribution")):
        try:
            os.makedirs(folder)
        except FileExistsError:
            pass
        for source_file_name in self.collect_binary_simulation_distribution_file_paths():
            workspace_relative_filename = os.path.relpath(source_file_name, get_omnetpp_relative_path(".."))
            destination_file_name = os.path.join(folder, workspace_relative_filename)
            try:
                os.makedirs(os.path.dirname(destination_file_name))
            except FileExistsError:
                pass
            shutil.copy(source_file_name, destination_file_name)
        with open(os.path.join(folder, "omnetpp/setenv"), "w") as file:
            file.write("""#!/usr/bin/env -S sh -c "echo >&2 \"Error: You are running this script instead of sourcing it. Make sure to use it as 'source setenv' or '. setenv', otherwise its settings won't take effect.\"; exit 1"
export __omnetpp_root_dir=$(pwd)
export PATH=$__omnetpp_root_dir/bin:$PATH
export PYTHONPATH=$__omnetpp_root_dir/python:$PYTHONPATH
export LD_LIBRARY_PATH=$__omnetpp_root_dir/lib:$LD_LIBRARY_PATH
""")
        return folder

    def copy_binary_simulation_distribution_to_cluster(self, worker_hostnames):
        binary_distribution_folder = self.create_binary_simulation_distribution()
        hostname = socket.gethostname()
        for worker_hostname in worker_hostnames:
            if hostname != worker_hostname:
                os.system(f'rsync --exclude "\\*\\*/results" -r "{binary_distribution_folder}" "{worker_hostname}:."')

simulation_projects = {}

def get_simulation_project(name, version=None):
    """
    Returns a defined simulation project for the provided name and version.

    Parameters:
        name (string):
            The name of the simulation project.

        version (string or None):
            The version of the simulation project. If unspecified, then the latest version is returned.

    Returns (py:class:`SimulationProject` or None):
        a simulation project.
    """
    return simulation_projects[(name, version)]

def set_simulation_project(name, version, simulation_project):
    simulation_projects[(name, version)] = simulation_project

def define_simulation_project(name, version=None, **kwargs):
    """
    Defines a simulation project for the provided name, version and additional parameters.

    Parameters:
        name (string):
            The name of the simulation project.

        version (string or None):
            The version of the simulation project. If unspecified, then no version is assumed.

        kwargs (dict):
            Additional parameters are inherited from the constructor of :py:class:`SimulationProject`.

    Returns (:py:class:`SimulationProject`):
        the new simulation project.
    """
    simulation_project = SimulationProject(name, version, **kwargs)
    set_simulation_project(name, version, simulation_project)
    return simulation_project

def find_simulation_project_from_current_working_directory():
    current_working_directory = os.getcwd()
    path = current_working_directory
    while True:
        project_file_name = os.path.join(path, ".omnetpp")
        if os.path.exists(project_file_name):
            with open(project_file_name) as project_file:
                kwargs = json.load(project_file)
                return define_simulation_project(**kwargs)
        parent_path = os.path.abspath(os.path.join(path, os.pardir))
        if path == parent_path:
            break
        else:
            path = parent_path
    for k, simulation_project in simulation_projects.items():
        if current_working_directory.startswith(simulation_project.get_full_path(".")):
            return simulation_project

def determine_default_simulation_project(name=None, version=None, required=True, **kwargs):
    if name:
        simulation_project = get_simulation_project(name, version, **kwargs)
    else:
        simulation_project = find_simulation_project_from_current_working_directory(**kwargs)
    if simulation_project is None:
        message = "No enclosing simulation project is found from current working directory, and no simulation project is specified explicitly"
        if required:
            raise Exception(message)
        else:
            _logger.warn(message)
    else:
        _logger.info(f"Default project is set to {simulation_project.name}")
    set_default_simulation_project(simulation_project)
    return simulation_project

default_project = None

def get_default_simulation_project():
    """
    Returns the currently selected default simulation project from the set of defined simulation projects. The default
    simulation project is usually the one that is above the current working directory.

    Returns (:py:class:`SimulationProject`):
        a simulation project.
    """
    global default_project
    if default_project is None:
        raise Exception("No default simulation project is set")
    return default_project

def set_default_simulation_project(project):
    """
    Changes the currently selected default simulation project from the set of defined simulation projects.

    Parameters:
        project (:py:class:`SimulationProject`):
            The simulation project that is set as the default.

    Returns (None):
        nothing.
    """
    global default_project
    default_project = project
