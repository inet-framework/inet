import logging
import multiprocessing
import subprocess

from inet.common import *

logger = logging.getLogger(__name__)

def build_project(simulation_project, mode="debug", capture_output=True, **kwargs):
    logger.info(f"Building {simulation_project.get_name()} started")
    args = ["make", "MODE=" + mode, "-j", str(multiprocessing.cpu_count())]
    subprocess_result = subprocess.run(args, cwd=simulation_project.get_full_path("."), capture_output=capture_output)
    if subprocess_result.returncode != 0:
        raise Exception(f"Build {simulation_project.get_name()} failed")
    logger.info(f"Building {simulation_project.get_name()} ended")
