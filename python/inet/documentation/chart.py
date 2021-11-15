import functools
import glob
import os
import re

import omnetpp
import omnetpp.scave.analysis

from inet.simulation.run import *

def get_analysis_files(working_directory_filter = ".*", fullMatch = False, **kwargs):
    analysisFiles = glob.glob("examples/**/*.anf", recursive = True) + \
                    glob.glob("showcases/**/*.anf", recursive = True) + \
                    glob.glob("tutorials/**/*.anf", recursive = True)
    return filter(lambda path: re.search(working_directory_filter if fullMatch else ".*" + working_directory_filter + ".*", path), analysisFiles)

def export_charts(**kwargs):
    for analysisFile in get_analysis_files(**kwargs):
        print("Exporting charts, analysisFile = " + analysisFile)
        analysis = omnetpp.scave.analysis.load_anf_file(analysisFile)
        for chart in analysis.collect_charts():
            folder = os.path.dirname(analysisFile)
            analysis.export_image(chart, get_full_path(folder), workspace, format="png", dpi=150, target_folder="doc/media")

def generate_charts(**kwargs):
#    clean_simulations_results(**kwargs)
#    run_simulation_configs(**kwargs)
    export_charts(**kwargs)

try:
    workspace = omnetpp.scave.analysis.Workspace(omnetpp.scave.analysis.Workspace.find_workspace(get_full_path(".")), [])
except:
    pass
