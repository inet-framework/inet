import logging
import subprocess

from inet.common import *
from inet.simulation.project import *
from inet.project.inet import *

_logger = logging.getLogger(__name__)

def generate_ned_documentation(excludes = []):
    _logger.info("Generating NED documentation")
    run_command_with_logging(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), inet_project.get_full_path(".")])
