import logging

from inet.common import *
from inet.test.all import *

_logger = logging.getLogger(__name__)

def run_release_tests(clean=True, build=True, **kwargs):
    if not os.path.exists(get_omnetpp_relative_path("bin/opp_run_release")) or \
       not os.path.exists(get_omnetpp_relative_path("bin/opp_run_dbg")) or \
       not os.path.exists(get_omnetpp_relative_path("bin/opp_run_sanitize")):
        raise Exception("Cannot run release tests because omnetpp must be first built in release, debug and sanitize mode")
    logging.getLogger("inet.common.task").setLevel("INFO")
    logging.getLogger("inet.simulation.build").setLevel("INFO")
    run_command_with_logging(["opp_featuretool", "enable", "all"], cwd=inet_project.get_full_path("."), error_message="Enabling all features failed")
    run_command_with_logging(["opp_featuretool", "disable", "SelfDoc"], cwd=inet_project.get_full_path("."), error_message="Disabling SelfDoc feature failed")
    make_makefiles(simulation_project=inet_project)
    if clean:
        clean_project(simulation_project=inet_project, mode="release")
        clean_project(simulation_project=inet_project, mode="debug")
        clean_project(simulation_project=inet_project, mode="sanitize")
    if build:
        build_project(simulation_project=inet_project, mode="release")
        build_project(simulation_project=inet_project, mode="debug")
        build_project(simulation_project=inet_project, mode="sanitize")
    return run_all_tests(**kwargs)
