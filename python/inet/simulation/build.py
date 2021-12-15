import logging
import multiprocessing
import subprocess

from inet.common import *

logger = logging.getLogger(__name__)

def build_project(simulation_project, mode="debug", **kwargs):
    logger.info("Building INET")
    args = ["make", "MODE=" + mode, "-j", str(multiprocessing.cpu_count())]
    subprocess_result = subprocess.run(args, cwd=simulation_project.get_full_path("."), capture_output=True)
    if subprocess_result.returncode != 0:
        raise Exception("Build failed")
