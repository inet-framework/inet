import logging
import subprocess

logger = logging.getLogger(__name__)

from inet.common import *
from inet.simulation.project import *

def generate_ned_documentation(excludes = []):
    logger.info("Generating NED documentation")
    subprocess.run(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), inet_project.get_full_path(".")])
