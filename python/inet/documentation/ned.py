import subprocess

from inet.common import *
from inet.simulation.project import *

def generate_ned_documentation(excludes = []):
    print("Generating NED documentation")
    subprocess.run(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), inet_project.get_full_path(".")])
