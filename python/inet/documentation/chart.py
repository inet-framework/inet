import builtins
import functools
import glob
import logging
import os
import re

import omnetpp.scave.analysis

from inet.simulation.project import *

logger = logging.getLogger(__name__)

def is_anf_v2(filename):
    return 'version="2"' in open(filename, "rt").read()

def get_analysis_files(simulation_project=default_project, filter=".*", exclude_filter=None, full_match=False, **kwargs):
    simulation_project_path = simulation_project.get_full_path(".")
    analysis_file_names = map(lambda path: os.path.relpath(path, simulation_project_path), glob.glob(simulation_project_path + "/**/*.anf", recursive = True))
    return builtins.filter(lambda analysis_file_name: is_anf_v2(simulation_project_path + "/" + analysis_file_name) and matches_filter(analysis_file_name, filter, exclude_filter, full_match), analysis_file_names)

def export_charts(simulation_project=default_project, **kwargs):
    workspace = omnetpp.scave.analysis.Workspace(get_workspace_path("."), [])
    for analysis_file_name in get_analysis_files(**kwargs):
        try:
            logger.info("Exporting charts, analysis file = " + analysis_file_name)
            analysis = omnetpp.scave.analysis.load_anf_file(simulation_project.get_full_path(analysis_file_name))
            for chart in analysis.collect_charts():
                try:
                    folder = os.path.dirname(simulation_project.get_full_path(analysis_file_name))
                    analysis.export_image(chart, folder, workspace, format="png", dpi=150, target_folder="doc/media")
                except Exception as e:
                    logger.error("Failed to export chart: " + str(e))
        except Exception as e:
            logger.error("Failed to load analysis file: " + str(e))

def generate_charts(**kwargs):
    clean_simulations_results(**kwargs)
    create_statistical_results(**kwargs)
    export_charts(**kwargs)
