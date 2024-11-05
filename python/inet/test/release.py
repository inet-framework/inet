import logging

from inet.common import *
from inet.test.all import *

logger = logging.getLogger(__name__)

def run_release_tests(capture_output=True, **kwargs):
    if not os.path.exists(get_omnetpp_relative_path("bin/opp_run_release")) or \
       not os.path.exists(get_omnetpp_relative_path("bin/opp_run_dbg")) or \
       not os.path.exists(get_omnetpp_relative_path("bin/opp_run_sanitize")):
        raise Exception("Cannot run release tests because omnetpp must be first built in release, debug and sanitize mode")
    args = ["opp_featuretool", "enable", "all"]
    subprocess_result = subprocess.run(args, cwd=inet_project.get_full_path("."), capture_output=capture_output)
    if subprocess_result.returncode != 0:
        raise Exception(f"Enabling features failed")
    args = ["opp_featuretool", "disable", "SelfDoc"]
    subprocess_result = subprocess.run(args, cwd=inet_project.get_full_path("."), capture_output=capture_output)
    if subprocess_result.returncode != 0:
        raise Exception(f"Disabling features failed")
    make_makefiles(simulation_project=inet_project)
    build_project(simulation_project=inet_project, mode="release")
    build_project(simulation_project=inet_project, mode="debug")
    build_project(simulation_project=inet_project, mode="sanitize")
    return run_all_tests(**kwargs)
