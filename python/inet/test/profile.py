import logging
import os

from inet.common import *
from inet.simulation import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

def generate_profile_report(simulation_project=None, output_file="perf.data", **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    simulation_task_results = run_simulations(simulation_project=simulation_project, mode="profile", prepend_args=["perf", "record", "-g", "-o", output_file], **kwargs)
    working_directory = simulation_task_results.results[0].task.simulation_config.working_directory
    return simulation_project.get_relative_path(os.path.join(working_directory, output_file))

def open_profile_report(simulation_project=None, output_file="perf.data", **kwargs):
    if simulation_project is None:
        simulation_project = get_default_simulation_project()
    report_file = generate_profile_report(simulation_project=simulation_project, output_file=output_file, **kwargs)
    args = ["hotspot", simulation_project.get_full_path(report_file)]
    run_command_with_logging(args)
