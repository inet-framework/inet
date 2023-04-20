import glob
import json
import os

from inet.simulation.project import *

def define_omnetpp_sample_projects():
    for folder in os.listdir(get_omnetpp_relative_path("samples")):
        define_simulation_project(name=folder, folder_environment_variable="__omnetpp_root_dir", folder=f"samples/{folder}", build_types=["executable"])
    # TODO revive when .omnetpp project files are checked in the omnetpp repository
    # for project_file_name in glob.glob(get_omnetpp_relative_path("samples/**/.omnetpp")):
    #     with open(project_file_name) as project_file:
    #         kwargs = json.load(project_file)
    #         define_simulation_project(**kwargs)
