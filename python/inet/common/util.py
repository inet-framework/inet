import glob
import os
import re

__sphinx_mock__ = True # ignore this module in documentation

# The generic utility functions live in opp_repl.common.util and are re-exported
# by inet.common. This module only keeps the INET-specific helpers: the INET/OMNeT++
# path helpers and the collectors that scan the INET source tree for NED/MSG/C++ types.

def get_omnetpp_relative_path(path):
    return os.path.abspath(os.path.join(os.environ["__omnetpp_root_dir"], path)) if "__omnetpp_root_dir" in os.environ else None

def get_inet_relative_path(path):
    return os.path.join(os.environ["INET_ROOT"], path)

def get_workspace_path(path):
    return os.path.join(os.path.realpath(get_omnetpp_relative_path("..")), path)

def collect_existing_ned_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ned"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^simple (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^module (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^network (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^moduleinterface (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^channel (\\w+)", text, re.M):
                    types.add(type)
    return types

def collect_referenced_ned_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ini"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                for type in re.findall("typename = \"(\\w+?)\"", f.read()):
                    types.add(type)
    for ned_file_path in glob.glob(get_inet_relative_path("**/*.ned"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ned_file_path, "r") as f:
                for type in re.findall("~(\\w+)", f.read()):
                    types.add(type)
    for rst_file_path in glob.glob(get_inet_relative_path("**/*.rst"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(rst_file_path, "r") as f:
                for type in re.findall(":ned:`(\\w+?)`", f.read()):
                    types.add(type)
    return types

def collect_ned_type_reference_file_paths(type):
    references = []
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.ini"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                if re.search(f"typename = \"{type}\"", f.read()):
                    references.append(ini_file_path)
    for rst_file_path in glob.glob(get_inet_relative_path("**/*.rst"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(rst_file_path, "r") as f:
                if re.search(f":ned:`{type}`", f.read()):
                    references.append(rst_file_path)
    return references

def collect_existing_msg_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.msg"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^class (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^packet (\\w+)", text, re.M):
                    types.add(type)
    return types

def collect_existing_cpp_types():
    types = set()
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.h"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("^class INET_API (\\w+)", text, re.M):
                    types.add(type)
                for type in re.findall("^enum (\\w+)", text, re.M):
                    types.add(type)
    for ini_file_path in glob.glob(get_inet_relative_path("**/*.cc"), recursive=True):
        if not re.search(r"doc/src/_deploy", ini_file_path):
            with open(ini_file_path, "r") as f:
                text = f.read()
                for type in re.findall("Register_Packet_Dropper_Function\\((\\w+),", text, re.M):
                    types.add(type)
                for type in re.findall("Register_Packet_Comparator_Function\\((\\w+),", text, re.M):
                    types.add(type)
    return types

def collect_referenced_non_existing_ned_types():
    referenced_ned_types = collect_referenced_ned_types()
    existing_ned_types = collect_existing_ned_types()
    existing_msg_types = collect_existing_msg_types()
    existing_cpp_types = collect_existing_cpp_types()
    return referenced_ned_types.difference(existing_ned_types).difference(existing_msg_types).difference(existing_cpp_types)
