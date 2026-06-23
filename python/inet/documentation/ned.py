import logging
import subprocess

from inet.common import *
from opp_repl.simulation import *

_logger = logging.getLogger(__name__)

def generate_ned_documentation(excludes = []):
    _logger.info("Generating NED documentation")
    run_command_with_logging(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), get_default_simulation_project().get_full_path(".")])
