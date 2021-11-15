import subprocess
import webbrowser

from inet.common import *

def generate_html_documentation():
    print("Generating HTML documentation")
    subprocess.run(["make", "html"], cwd = get_full_path("/doc/src/"))

def upload_html_documentation(path):
    print("Uploading HTML documentation, path = " + path)
    subprocess.run(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = get_full_path("/doc/src/_build/html"))

def open_html_documentation(path = "index.html"):
    print("Opening HTML documentation, path = " + path)
    webbrowser.open(get_full_path("/doc/src/_build/html/" + path))
