import logging
import subprocess
import webbrowser

from inet.common import *
from inet.project.inet import *
from inet.simulation.project import *

_logger = logging.getLogger(__name__)

def generate_html_documentation(docker=False, clean_build=False):
    _logger.info("Generating HTML documentation (docker=" + str(docker) + ", " + "clean_build=" + str(clean_build) + ")")
    if clean_build:
        run_command_with_logging(["rm", "-r", "_build"], cwd = inet_project.get_full_path("doc/src/"))
    if docker:
        make_cmd = "./docker-make"
    else:
        make_cmd = "make"
    run_command_with_logging([make_cmd, "html"], cwd = inet_project.get_full_path("doc/src/"))

def upload_html_documentation(path):
    _logger.info("Uploading HTML documentation, path = " + path)
    run_command_with_logging(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = inet_project.get_full_path("doc/src/_build/html"))

def open_html_documentation(path = "index.html"):
    _logger.info("Opening HTML documentation, path = " + path)
    webbrowser.open(inet_project.get_full_path("doc/src/_build/html/" + path))
