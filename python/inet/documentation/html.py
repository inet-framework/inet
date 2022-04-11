import logging
import subprocess
import webbrowser

logger = logging.getLogger(__name__)

from inet.common import *
from inet.simulation.project import *

def generate_html_documentation(docker=False, clean_build=False):
    logger.info("Generating HTML documentation (docker=" + str(docker) + ", " + "clean_build=" + str(clean_build) + ")")
    
    if clean_build:
        subprocess.run(["rm", "-r", "_build"], cwd = inet_project.get_full_path("doc/src/"))
    if docker:
        make_cmd = "./docker-make"
    else:
        make_cmd = "make"
    subprocess.run([make_cmd, "html"], cwd = inet_project.get_full_path("doc/src/"))

def upload_html_documentation(path):
    logger.info("Uploading HTML documentation, path = " + path)
    subprocess.run(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = inet_project.get_full_path("doc/src/_build/html"))

def open_html_documentation(path = "index.html"):
    logger.info("Opening HTML documentation, path = " + path)
    webbrowser.open(inet_project.get_full_path("doc/src/_build/html/" + path))
