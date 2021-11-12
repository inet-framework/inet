import os
import subprocess
import webbrowser

def generate_ned_documentation(excludes = []):
    print("Generating NED documentation")
    subprocess.run(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), os.environ['INET_ROOT']])

def generate_html_documentation():
    print("Generating HTML documentation")
    subprocess.run(["make", "html"], cwd = os.environ['INET_ROOT'] + "/doc/src/")

def upload_html_documentation(path):
    print("Uploading HTML documentation, path = " + path)
    subprocess.run(["rsync", "-L", "-r", "-e", "ssh -p 2200", ".", "--delete", "--progress", "com@server.omnest.com:" + path], cwd = os.environ['INET_ROOT'] + "/doc/src/_build/html")

def open_html_documentation(path = "index.html"):
    print("Opening HTML documentation, path = " + path)
    webbrowser.open(os.environ['INET_ROOT'] + "/doc/src/_build/html/" + path)
