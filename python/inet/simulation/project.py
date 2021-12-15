import logging

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

inet_project = SimulationProject(get_workspace_path("inet"))
inet_master_project = SimulationProject(get_workspace_path("inet-master"))
