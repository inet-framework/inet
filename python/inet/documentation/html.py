import subprocess
import webbrowser

from inet.common import *
from inet.simulation.project import *

def generate_html_documentation(docker=False, clean_build=False):
    print("Generating HTML documentation (docker=" + str(docker) + ", " + "clean_before_build=" + str(clean_before_build) + ")")
    
    if clean_build:
        subprocess.run(["rm", "-r", "_build"], cwd = inet_project.get_full_path("/doc/src/"))
    if docker:
        make_cmd = "./docker-make"
    else:
        make_cmd = "make"
    subprocess.run([make_cmd, "html"], cwd = inet_project.get_full_path("/doc/src/"))

def upload_html_documentation(path):
    print("Uploading HTML documentation, path = " + path)
    subprocess.run(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = inet_project.get_full_path("/doc/src/_build/html"))

def open_html_documentation(path = "index.html"):
    print("Opening HTML documentation, path = " + path)
    webbrowser.open(inet_project.get_full_path("/doc/src/_build/html/" + path))
