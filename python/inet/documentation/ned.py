import logging
import subprocess

_logger = logging.getLogger(__name__)

from inet.common import *
from inet.simulation.project import *
from inet.project.inet import *

def generate_ned_documentation(excludes = []):
    _logger.info("Generating NED documentation")
    subprocess.run(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), inet_project.get_full_path(".")])
