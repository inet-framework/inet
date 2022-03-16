import functools
import glob
import logging
import os
import re

import omnetpp
import omnetpp.scave.analysis

from inet.simulation.project import *
from inet.simulation.run import *

logger = logging.getLogger(__name__)

def get_analysis_files(simulation_project=default_project, working_directory_filter = ".*", fullMatch = False, **kwargs):
    analysisFiles = glob.glob(simulation_project.get_full_path(".") + "/**/*.anf", recursive = True)
    return filter(lambda path: re.search(working_directory_filter if fullMatch else ".*" + working_directory_filter + ".*", path), analysisFiles)

def export_charts(simulation_project=default_project, **kwargs):
    workspace = omnetpp.scave.analysis.Workspace(omnetpp.scave.analysis.Workspace.find_workspace(get_workspace_path(".")), [])
    for analysisFile in get_analysis_files(**kwargs):
        try:
            logger.info("Exporting charts, analysisFile = " + analysisFile)
            analysis = omnetpp.scave.analysis.load_anf_file(analysisFile)
            for chart in analysis.collect_charts():
                try:
                    folder = os.path.dirname(analysisFile)
                    analysis.export_image(chart, folder, workspace, format="png", dpi=150, target_folder="doc/media")
                except Exception as e:
                    logger.error("Failed to export chart: " + str(e))
        except Exception as e:
            logger.error("Failed to load analysis file: " + str(e))

def generate_charts(**kwargs):
    clean_simulations_results(**kwargs)
    create_statistical_results(**kwargs)
    export_charts(**kwargs)
