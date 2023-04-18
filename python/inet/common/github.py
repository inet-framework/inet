import logging
import os

_logger = logging.getLogger(__name__)

import requests

def dispatch_workflow(workflow_name, owner = "inet-framework", repository = "inet"):
    github_token = open(os.path.expanduser("~/.ssh/github_repo_token"), "r").read()
    url = f"https://api.github.com/repos/{owner}/{repository}/actions/workflows/{workflow_name}/dispatches"
    headers = {
        "Accept": "application/vnd.github+json",
        "Authorization": f"token {github_token}"
    }
    response = requests.post(url, json={"ref": "master"}, headers=headers)
    if response.status_code != 204:
        raise Exception(f"Error: {response.status_code} - {response.text}")

def start_chart_tests_github_workflow():
    dispatch_workflow("chart-tests.yml")

def start_feature_tests_github_workflow():
    dispatch_workflow("feature-tests.yml")

def start_fingerprint_tests_github_workflow():
    dispatch_workflow("fingerprint-tests.yml")

def start_module_tests_github_workflow():
    dispatch_workflow("module-tests.yml")

def start_other_tests_github_workflow():
    dispatch_workflow("other-tests.yml")

def start_statistical_tests_github_workflow():
    dispatch_workflow("statistical-tests.yml")

def start_unit_tests_github_workflow():
    dispatch_workflow("unit-tests.yml")

def start_validation_tests_github_workflow():
    dispatch_workflow("validation-tests.yml")

def start_all_tests_github_workflows():
    start_chart_tests_github_workflow()
    start_feature_tests_github_workflow()
    start_fingerprint_tests_github_workflow()
    start_module_tests_github_workflow()
    start_other_tests_github_workflow()
    start_statistical_tests_github_workflow()
    start_unit_tests_github_workflow()
    start_validation_tests_github_workflow()
