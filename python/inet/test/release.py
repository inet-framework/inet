import logging

from inet.test.all import *

logger = logging.getLogger(__name__)

def run_release_tests(capture_output=True, **kwargs):
    args = ["opp_featuretool", "enable", "all"]
    subprocess_result = subprocess.run(args, cwd=inet_project.get_full_path("."), capture_output=capture_output)
    if subprocess_result.returncode != 0:
        raise Exception(f"Enabling features failed")
    args = ["opp_featuretool", "disable", "SelfDoc"]
    subprocess_result = subprocess.run(args, cwd=inet_project.get_full_path("."), capture_output=capture_output)
    if subprocess_result.returncode != 0:
        raise Exception(f"Disabling features failed")
    return run_all_tests(**kwargs)
