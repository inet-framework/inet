import logging
import os

from inet.common import *

logger = logging.getLogger(__name__)

class SimulationProject:
    def __init__(self, project_directory):
        self.project_directory = project_directory
        self.simulation_configs = None

    def get_name(self):
        return os.path.basename(self.get_full_path("."))

    def get_full_path(self, path):
        return os.path.abspath(self.project_directory + "/" + path)

simulation_projects = dict()

def get_simulation_project(name):
    if not name in simulation_projects:
        workspace_path = get_workspace_path(name)
        simulation_projects[name] = SimulationProject(workspace_path)
    return simulation_projects[name]

inet_project = get_simulation_project("inet")
inet_baseline_project = get_simulation_project("inet-baseline")
default_project=inet_project