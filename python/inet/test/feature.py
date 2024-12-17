import itertools
import pathlib
import logging
import subprocess
import xml

from inet.common.summary import *
from inet.project.inet import *
from inet.simulation.build import *
from inet.simulation.task import *
from inet.test.simulation import *

# TODO force at least setting up interactive/emulation simulations
# TODO add ifdefs around includes in C++ where the dependency is meant to be optional

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

def disable_features(feature):
    run_command_with_logging(["opp_featuretool", "disable", "-f", feature])

def enable_features(feature):
    run_command_with_logging(["opp_featuretool", "enable", "-f", feature])

class FeatureTestTask(TestTask):
    def __init__(self, simulation_project, feature, packages, type="enable", **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.simulation_project = simulation_project
        self.feature = feature
        self.type = type
        self.packages = packages
        self.multiple_simulation_tasks = self.get_multiple_simulation_tasks()

    def get_multiple_simulation_tasks(self):
        if len(self.packages) != 0:
            folders = []
            for package in self.packages:
                 folder = get_package_folder(package)
                 folders.append(folder)
                 folders.append(folder + "/.*")
            working_directory_filter = "|".join(folders)
            multiple_simulation_tasks = get_simulation_tasks(working_directory_filter=working_directory_filter, full_match=True, run_number=0)
        else:
            multiple_simulation_tasks = MultipleSimulationTasks()
        if len(multiple_simulation_tasks.tasks) == 0:
            multiple_simulation_tasks = get_simulation_tasks(working_directory_filter="examples/empty", full_match=True, run_number=0)
        return multiple_simulation_tasks

    def get_parameters_string(self, **kwargs):
        return self.type + " " + (self.feature or "")

    def run_protected(self, **kwargs):
        if self.type == "default":
            run_command_with_logging(["opp_featuretool", "reset"])
        elif self.type == "enable all":
            enable_features("all")
        elif self.type == "disable all":
            disable_features("all")
        elif self.type == "enable":
            disable_features("all")
            enable_features(self.feature)
        elif self.type == "disable":
            enable_features("all")
            disable_features(self.feature)
        else:
            raise Exception("Unknown test type")
        make_makefiles(simulation_project=self.simulation_project)
        clean_project(simulation_project=self.simulation_project)
        build_project(simulation_project=self.simulation_project)
        multiple_simulation_tasks_result = self.multiple_simulation_tasks.run(**kwargs)
        result="PASS" if multiple_simulation_tasks_result.result == "DONE" else "FAIL" if multiple_simulation_tasks_result.result == "FAIL" else "ERROR"
        return self.task_result_class(self, result=result)

class MultipleFeatureTestTasks(MultipleTestTasks):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs

    def get_total_simulation_task_count(self):
        count = 0
        for task in self.tasks:
            count += len(list(filter(lambda task: task.simulation_config.working_directory != "examples/empty", task.multiple_simulation_tasks.tasks)))
        return count

    def run_protected(self, **kwargs):
        _logger.info("Collected " + str(self.get_total_simulation_task_count()) + " simulations in total")
        multiple_test_tasks_result = super().run_protected(**kwargs)
        _logger.info("Run " + str(self.get_total_simulation_task_count()) + " simulations in total")
        enable_features("all")
        return multiple_test_tasks_result


def get_feature_test_tasks(simulation_project=None, filter=".*", full_match=False, **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    oppfeatures = read_xml_file(simulation_project.get_full_path(".oppfeatures"))
    features = get_features(oppfeatures)
    feature_to_packages = get_feature_to_packages(oppfeatures)
    def create_test_tasks(feature):
        return [FeatureTestTask(simulation_project, feature, feature_to_packages[feature], type="enable", task_result_class=TestTaskResult, **kwargs),
                FeatureTestTask(simulation_project, feature, [], type="disable", task_result_class=TestTaskResult, **kwargs)]
    test_features = list(builtins.filter(lambda feature: matches_filter(feature, filter, None, full_match), features))
    test_tasks = list(itertools.chain.from_iterable(map(create_test_tasks, test_features)))
    if filter == ".*":
        test_tasks = test_tasks + [FeatureTestTask(simulation_project, None, [], type="default", task_result_class=TestTaskResult, **kwargs),
                                   FeatureTestTask(simulation_project, None, [], type="enable all", task_result_class=TestTaskResult, **kwargs),
                                   FeatureTestTask(simulation_project, None, [], type="disable all", task_result_class=TestTaskResult, **kwargs)]
    return MultipleFeatureTestTasks(tasks=test_tasks, multiple_task_results_class=MultipleTestTaskResults, **dict(kwargs, concurrent=False))

def run_feature_tests(**kwargs):
    multiple_test_tasks = get_feature_test_tasks(**kwargs)
    return multiple_test_tasks.run(**kwargs)

def read_xml_file(filename, repair_hint=None):
    try:
        f = open(filename, "r")
        dom_tree = xml.dom.minidom.parse(f)
        f.close()
        return dom_tree
    except (IOError, OSError) as e:
        fail("Cannot read '{}': {}".format(filename, e))
    except Exception as e:
        fail("Cannot parse XML file '{}': {}".format(filename, e), repair_hint)

def get_package_folder(package):
    if re.search(r"inet.examples", package):
        return re.sub(r"inet/", "", re.sub(r"\.", "/", package))
    elif re.search(r"inet.showcases", package):
        return re.sub(r"inet/", "", re.sub(r"\.", "/", package))
    elif re.search(r"inet.tutorials", package):
        return re.sub(r"inet/", "", re.sub(r"\.", "/", package))
    elif re.search(r"inet.tests", package):
        return re.sub(r"inet/", "", re.sub(r"\.", "/", package))
    elif re.search(r"inet.validation", package):
        return re.sub(r"inet/", "tests/", re.sub(r"\.", "/", package))
    else:
        return "src/" + re.sub(r"\.", "/", package)

def get_features(oppfeatures):
    result = []
    for feature_dom in oppfeatures.documentElement.getElementsByTagName("feature"):
        id = str(feature_dom.getAttribute("id"))
        result.append(id)
    result.sort()
    return result

def get_feature_to_packages(oppfeatures):
    result = dict()
    for feature_dom in oppfeatures.documentElement.getElementsByTagName("feature"):
        id = str(feature_dom.getAttribute("id"))
        packages = feature_dom.getAttribute("nedPackages").split()
        result[id] = sorted(packages)
    return result

def get_packages(oppfeatures):
    result = []
    for feature_dom in oppfeatures.documentElement.getElementsByTagName("feature"):
        packages = feature_dom.getAttribute("nedPackages").split()
        result += packages
    result = list(set(result))
    result.sort()
    return result

def get_package_to_defined_types(simulation_project, packages):
    result = dict()
    for package in packages:
        folder = get_package_folder(package)
        modules = list(map(lambda module: "inet." + module, collect_modules(simulation_project, folder)))
        chunks = list(map(lambda chunk: package + "." + chunk, collect_chunks(simulation_project, folder)))
        tags = list(map(lambda tag: package + "." + tag, collect_tags(simulation_project, folder)))
        result[package] = sorted(modules + chunks + tags)
    return result

def get_defined_types(package_to_defined_types):
    result = []
    for key, value in package_to_defined_types.items():
        result += value
    result = list(set(result))
    result.sort()
    return result

def get_defined_type_to_package(package_to_defined_types):
    result = dict()
    for package, defined_types in package_to_defined_types.items():
        for defined_type in defined_types:
            result[defined_type] = package
    return result

def get_package_to_feature(feature_to_packages):
    result = dict()
    for feature, packages in feature_to_packages.items():
        for package in packages:
            result[package] = feature
    return result

def get_defined_types_to_feature(package_to_defined_types, feature_to_packages):
    result = dict()
    defined_types = get_defined_types(package_to_defined_types)
    defined_type_to_package = get_defined_type_to_package(package_to_defined_types)
    package_to_feature = get_package_to_feature(feature_to_packages)
    for defined_type in defined_types:
        package = defined_type_to_package[defined_type]
        feature = package_to_feature[package]
        result[defined_type] = feature
    return result

def get_folder_to_simulations(simulation_results):
    result = {}
    for simulation_result in simulation_results.results:
        simulation_task = simulation_result.task
        path = pathlib.Path(simulation_task.simulation_config.working_directory)
        while str(path) != ".":
            folder = str(path)
            if folder in result:
                result[folder].append(simulation_task)
            else:
                result[folder] = [simulation_task]
            path = path.parent
    return result

def get_simulation_to_used_types(simulation_results):
    result = {}
    for simulation_result in simulation_results.results:
        if len(simulation_result.used_types) == 0:
            raise Exception("The number of used NED types in the simulation result is 0. Did you forget to recompile omnetpp with defining PRINT_MODULE_TYPES_USED macro?")
        result[simulation_result.task] = simulation_result.used_types
    return result

def get_package_to_used_types(packages):
    result = dict()
    for package in packages:
        used_types = []
        folder = get_package_folder(package)
        for file_name in glob.glob(folder + "/**/*.ned", recursive=True):
            with open(file_name, "r") as file:
                for line in file:
                    match = re.match(r"^import ([\w\.]+)", line)
                    if match:
                        used_types.append(match.group(1))
        for file_name in glob.glob(folder + "/**/*.msg", recursive=True):
            with open(file_name, "r") as file:
                for line in file:
                    match = re.match(r"^import ([\w\.]+)", line)
                    if match:
                        used_types.append(match.group(1))
        result[package] = sorted(list(set(used_types)))
    return result

def get_package_to_used_headers(packages):
    result = dict()
    for package in packages:
        used_headers = []
        folder = get_package_folder(package)
        for file_name in glob.glob(folder + "/**/*.cc", recursive=True) + glob.glob(folder + "/**/*.h", recursive=True):
            with open(file_name, "r") as file:
                if_counter = 0
                for line in file:
                    if re.search(r"^#if[ d]", line):
                        if_counter += 1
                    elif re.search(r"^#endif", line):
                        if_counter -= 1
                    if if_counter == 0:
                        match = re.match(r"^#include \"([\w\.\/]+)\"", line)
                        if match:
                            used_headers.append("src/" + match.group(1))
        result[package] = sorted(list(set(used_headers)))
    return result

def get_header_to_feature(features, feature_to_packages):
    result = dict()
    for feature in features:
        for package in feature_to_packages[feature]:
            folder = get_package_folder(package)
            for file_name in glob.glob(folder + "/**/*.h", recursive=True):
                result[file_name] = feature
    return result

def get_feature_to_required_features(simulation_project, features, feature_to_packages, package_to_used_types, package_to_used_headers, defined_types_to_feature, folder_to_simulations, simulation_to_used_types):
    result = {}
    header_to_feature = get_header_to_feature(features, feature_to_packages)
    for feature in features:
        required_features = []
        packages = feature_to_packages[feature]
        for package in packages:
            used_headers = package_to_used_headers[package]
            for used_header in used_headers:
                if used_header in header_to_feature:
                    used_feature = header_to_feature[used_header]
                    if feature != used_feature:
                        required_features.append(used_feature)
        folders = list(map(get_package_folder, packages))
        folders.sort()
        used_types = []
        for package in packages:
            used_types += package_to_used_types[package]
        for folder in folders:
            if folder in folder_to_simulations:
                simulations = folder_to_simulations[folder]
                for simulation in simulations:
                    used_types += simulation_to_used_types[simulation]
        used_types = list(set(used_types))
        for used_type in used_types:
            if used_type in defined_types_to_feature:
                used_feature = defined_types_to_feature[used_type]
                if feature != used_feature:
                    required_features.append(used_feature)
        required_features = list(set(required_features))
        result[feature] = sorted(required_features)
    return result

def get_feature_to_required_features_for_simulation_project(simulation_project, simulation_results):
    oppfeatures = read_xml_file(simulation_project.get_full_path(".oppfeatures"))
    features = get_features(oppfeatures)
    feature_to_packages = get_feature_to_packages(oppfeatures)
    packages = get_packages(oppfeatures)
    package_to_used_types = get_package_to_used_types(packages)
    package_to_used_headers = get_package_to_used_headers(packages)
    package_to_defined_types = get_package_to_defined_types(simulation_project, packages)
    defined_types_to_feature = get_defined_types_to_feature(package_to_defined_types, feature_to_packages)
    folder_to_simulations = get_folder_to_simulations(simulation_results)
    simulation_to_used_types = get_simulation_to_used_types(simulation_results)
    return get_feature_to_required_features(simulation_project, features, feature_to_packages, package_to_used_types, package_to_used_headers, defined_types_to_feature, folder_to_simulations, simulation_to_used_types)

def update_oppfeatures_file(simulation_project, feature_to_required_features, feature_id_filter=None):
    file_name = simulation_project.get_full_path(".oppfeatures")
    with open(file_name, "r") as file:
        lines = []
        feature_id = None
        for line in file:
            match = re.search(r"id *= *\"(.*?)\"", line)
            if match:
                feature_id = match.group(1)
            match = re.search(r"( *requires *= *\").*?(\")", line)
            if match and (feature_id_filter is None or re.search(feature_id_filter, feature_id)):
                lines.append(match.group(1) + " ".join(feature_to_required_features[feature_id]) + match.group(2) + "\n")
            else:
                lines.append(line)
    with open(file_name, "w") as file:
        file.write("".join(lines))

def update_oppfeatures(simulation_project=None, simulation_results=None, feature_id_filter=None, mode="release", **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    enable_features("all")
    make_makefiles(simulation_project=simulation_project)
    clean_project(simulation_project=simulation_project, mode=mode)
    build_project(simulation_project=simulation_project, mode=mode)
    if simulation_results is None:
        simulation_results = run_simulations(simulation_project=simulation_project, mode=mode, run_number=0, append_args=["--print-instantiated-ned-types=true"], **kwargs)
    feature_to_required_features = get_feature_to_required_features_for_simulation_project(simulation_project, simulation_results)
    update_oppfeatures_file(simulation_project, feature_to_required_features, feature_id_filter=feature_id_filter)
