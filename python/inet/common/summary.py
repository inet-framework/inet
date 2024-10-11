import glob
import logging
import os
import re
import subprocess

logger = logging.getLogger(__name__)

def collect_features(simulation_project):
    file_name = simulation_project.get_full_path(".oppfeatures")
    file = open(file_name, encoding="utf-8")
    features = []
    for line in file:
        match = re.match(r"\s*name\s*=\s*\"(.+)\"", line)
        if match:
            features.append(match.group(1))
    return features

def collect_folders(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    folders = []
    for root, subdirs, files in os.walk(project_path):
        for subdir in subdirs:
            if not subdir.startswith("_"):
                folders.append(os.path.relpath(root, project_path) + "/" + subdir)
    return folders

def collect_modules(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    modules = []
    for file_name in glob.glob(project_path + "/**/*.ned", recursive=True):
        file = open(file_name, encoding="utf-8")
        package = None
        for line in file:
            match = re.match(r"^package ([\w\.]+)", line)
            if match:
                package = match.group(1)
                package = re.sub(r"^\w+?\.", "", package)
            match = re.match(r"^(simple|module) (\w+)\b", line)
            if match:
                module = match.group(2)
                modules.append(package + "." + module if package else module)
        file.close()
    return modules

def collect_parameters(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    parameters = []
    for file_name in glob.glob(project_path + "/**/*.ned", recursive=True):
        args = ["opp_nedtool", "c", file_name]
        result = subprocess.run(args, capture_output=True)
        file = open(file_name + ".xml", encoding="utf-8")
        module = None
        for line in file:
            match = re.match(r"\s*<(simple-module|compound-module|module-interface|channel) name=\"(.*?)\"", line)
            if match:
                module = match.group(2)
            match = re.match(r"\s*<param type=\"(.*?)\" name=\"(.*?)\"", line)
            if match:
                parameters.append(module + ":" + match.group(2))
        file.close()
        os.remove(file_name + ".xml")
    return list(set(parameters))

def collect_signals(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    signals = []
    for file_name in glob.glob(project_path + "/**/*.ned", recursive=True):
        file = open(file_name, encoding="utf-8")
        for line in file:
            match = re.match(r"\s*@signal\[(.*?)\]", line)
            if match:
                signals.append(match.group(1))
    return signals

def collect_statistics(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    statistics = []
    for file_name in glob.glob(project_path + "/**/*.ned", recursive=True):
        file = open(file_name, encoding="utf-8")
        for line in file:
            match = re.match(r"\s*@statistic\[(.*?)\]", line)
            if match:
                statistics.append(match.group(1))
    return statistics

def collect_chunks(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    chunks = []
    for file_name in glob.glob(project_path + "/**/*.msg", recursive=True):
        file = open(file_name, encoding="utf-8")
        for line in file:
            match = re.match(r"^class (\w+) extends FieldsChunk", line)
            if match:
                chunks.append(match.group(1))
            else:
                match = re.match(r"^class ((\w+)(Header|Trailer|Packet|Frame))", line)
                if match:
                    chunks.append(match.group(1))
    return chunks

def collect_tags(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    tags = []
    for file_name in glob.glob(project_path + "/**/*.msg", recursive=True):
        file = open(file_name, encoding="utf-8")
        for line in file:
            match = re.match(r"^class ((\w+)(Tag|Ind|Req))", line)
            if match:
                tags.append(match.group(1))
    return tags

def collect_classes(simulation_project, path="src"):
    project_path = simulation_project.get_full_path(path)
    classes = []
    for file_name in glob.glob(project_path + "/**/*.h", recursive=True):
        file = open(file_name, encoding="utf-8")
        for line in file:
            match = re.match(r"^class INET_API (\w+)\b", line)
            if match:
                class_name = match.group(1)
                relative_path = os.path.relpath(os.path.dirname(file_name), project_path)
                relative_path = re.sub(r"^(\w+)/", "", relative_path)
                classes.append(relative_path + "/" + class_name)
        file.close()
    return classes

def find_collection_changes(from_collection, to_collection):
    added = list(set(to_collection).difference(set(from_collection)))
    removed = list(set(from_collection).difference(set(to_collection)))
    return (sorted(added), sorted(removed))

def print_topic_changes(topic, from_simulation_project, to_simulation_project, collector_function):
    (added, removed) = find_collection_changes(collector_function(from_simulation_project), collector_function(to_simulation_project))
    if len(added) > 0:
        print(f"\nAdded {topic}\n" + ("=" * (len(topic) + 6)))
        list(map(print, added))
    if len(removed) > 0:
        print(f"\nRemoved {topic}\n" + ("=" * (len(topic) + 8)))
        list(map(print, removed))

def print_feature_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("features", from_simulation_project, to_simulation_project, collect_features)

def print_folder_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("folders", from_simulation_project, to_simulation_project, collect_folders)

def print_module_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("modules", from_simulation_project, to_simulation_project, collect_modules)

def print_parameter_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("parameters", from_simulation_project, to_simulation_project, collect_parameters)

def print_signal_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("signals", from_simulation_project, to_simulation_project, collect_signals)

def print_statistic_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("statistics", from_simulation_project, to_simulation_project, collect_statistics)

def print_chunk_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("chunks", from_simulation_project, to_simulation_project, collect_chunks)

def print_tag_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("tags", from_simulation_project, to_simulation_project, collect_tags)

def print_class_changes(from_simulation_project, to_simulation_project):
    print_topic_changes("C++ classes", from_simulation_project, to_simulation_project, collect_classes)

def print_changes(from_simulation_project, to_simulation_project):
    print("Simulation project change summary (should be edited manually):")
    print_feature_changes(from_simulation_project, to_simulation_project)
    print_folder_changes(from_simulation_project, to_simulation_project)
    print_module_changes(from_simulation_project, to_simulation_project)
    print_parameter_changes(from_simulation_project, to_simulation_project)
    print_signal_changes(from_simulation_project, to_simulation_project)
    print_statistic_changes(from_simulation_project, to_simulation_project)
    print_chunk_changes(from_simulation_project, to_simulation_project)
    print_tag_changes(from_simulation_project, to_simulation_project)
    print_class_changes(from_simulation_project, to_simulation_project)
