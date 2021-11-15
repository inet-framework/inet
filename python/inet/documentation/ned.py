import subprocess

from inet.common import *

def generate_ned_documentation(excludes = []):
    print("Generating NED documentation")
    subprocess.run(["opp_neddoc", "--no-automatic-hyperlinks", "-x", ','.join(excludes), get_full_path(".")])
