import logging
import subprocess
import webbrowser

from inet.common import *
from inet.project.inet import *
from inet.simulation.project import *

_logger = logging.getLogger(__name__)

_BUILD_PARTS = {"ned": "api", "python": "python-api"}

def generate_html_documentation(docker=False, clean_build=False, targets=None):
    _logger.info("Generating HTML documentation (docker=" + str(docker) + ", " + "clean_build=" + str(clean_build) + ", targets=" + str(targets) + ")")
    if clean_build:
        run_command_with_logging(["rm", "-rf", "doc/src/_build", "doc/site"], cwd = inet_project.get_full_path("."))
    import os
    env = os.environ.copy()
    if not docker:
        env["ENGINE"] = "none"
    cmd = ["./build"]
    if targets is not None:
        cmd += ["--only", ",".join(_BUILD_PARTS.get(target, target) for target in targets)]
    # run_command_with_logging doesn't work in docker, it adds lots of extra spaces
    subprocess.run(cmd, cwd = inet_project.get_full_path("doc/src/"), env=env)

def upload_html_documentation(path):
    _logger.info("Uploading HTML documentation, path = " + path)
    run_command_with_logging(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = inet_project.get_full_path("doc/site"))

def open_html_documentation(path = "docs/dev/index.html"):
    _logger.info("Opening HTML documentation, path = " + path)
    webbrowser.open(inet_project.get_full_path("doc/site/" + path))
