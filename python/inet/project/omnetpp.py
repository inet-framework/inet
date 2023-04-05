import glob
import json

from inet.simulation.project import *

__sphinx_mock__ = True # ignore this module in documentation

def define_sample_projects():
    for project_file_name in glob.glob(get_omnetpp_relative_path("samples/**/.omnetpp")):
        with open(project_file_name) as project_file:
            kwargs = json.load(project_file)
            define_simulation_project(**kwargs)
