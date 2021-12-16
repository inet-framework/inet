import logging
import re

from inet.simulation.run import *
from inet.test.fingerprint.store import *
from inet.test.run import *

logger = logging.getLogger(__name__)

# TODO
# all_fingerprints.insert_fingerprint(calculated_fingerprint, ingredients=ingredients, test_result=result, sim_time_limit=sim_time_limit,
#                                     working_directory=simulation_run.working_directory, ini_file=simulation_run.ini_file, config=simulation_run.config, run=simulation_run.run)

class Fingerprint:
    def __init__(self, text):
        match = re.match("(.*)/(.*)", text)
        self.fingerprint = match.groups()[0]
        self.ingredients = match.groups()[1]

    def __repr__(self):
        return "Fingerprint(\"" + str(self) + "\")"
    
    def __str__(self):
        return self.fingerprint + "/" + self.ingredients

    def __eq__(self, other):
        return other and self.fingerprint == other.fingerprint and self.ingredients == other.ingredients

class FingerprintTestRun(TestRun):
    def __init__(self, simulation_run, check_test_function, sim_time_limit, ingredients, fingerprint, test_result, **kwargs):
        super().__init__(simulation_run, check_test_function, **kwargs)
        self.sim_time_limit = sim_time_limit
        self.ingredients = ingredients
        self.fingerprint = fingerprint
        self.test_result = test_result

    def __repr__(self):
        return repr(self)

    def run(self, sim_time_limit=None, **kwargs):
        if self.fingerprint:
            return super().run(extra_args=self.get_extra_args(str(self.fingerprint)), sim_time_limit=sim_time_limit or self.sim_time_limit, **kwargs)
        else:
            fingerprint_test_result = FingerprintTestResult(self, None, None, None, result = "SKIP", reason = "Correct fingerprint not found")
            print("Running " + self.simulation_run.get_simulation_parameters_string(**kwargs), end=" ")
            print(fingerprint_test_result.get_description())
            return fingerprint_test_result

    def get_extra_args(self, fingerprint_arg):
        return ["-n", os.environ["INET_ROOT"] + "/tests/networks", "--fingerprintcalculator-class", "inet::FingerprintCalculator", "--fingerprint", fingerprint_arg, "--vector-recording", "false", "--scalar-recording", "false"]

class FingerprintTestGroupRun:
    def __init__(self, sim_time_limit, fingerprint_test_runs, **kwargs):
        for fingerprint_test_run in fingerprint_test_runs:
            assert fingerprint_test_run.sim_time_limit == sim_time_limit
        self.sim_time_limit = sim_time_limit
        self.fingerprint_test_runs = fingerprint_test_runs

    def __repr__(self):
        return repr(self)

    def run(self, sim_time_limit=None, **kwargs):
        fingerprint_test_results = []
        skipped_fingerprint_test_runs = list(filter(lambda fingerprint_test_run: fingerprint_test_run.fingerprint is None, self.fingerprint_test_runs))
        for skipped_fingerprint_test_run in skipped_fingerprint_test_runs:
            fingerprint_test_results.append(skipped_fingerprint_test_run.run())
        checked_fingerprint_test_runs = list(filter(lambda fingerprint_test_run: fingerprint_test_run.fingerprint, self.fingerprint_test_runs))
        if checked_fingerprint_test_runs:
            simulation_run = checked_fingerprint_test_runs[0].simulation_run
            fingerprint_arg = ",".join(map(lambda e: str(e.fingerprint), checked_fingerprint_test_runs))
            extra_args = checked_fingerprint_test_runs[0].get_extra_args(fingerprint_arg)
            simulation_result = simulation_run.run_simulation(print_end=" ", sim_time_limit=sim_time_limit or self.sim_time_limit, extra_args=extra_args, **kwargs)
            fingerprint_test_group_results = check_fingerprint_test_group(simulation_result, self, **kwargs)
            print(fingerprint_test_group_results.get_summary())
            if not fingerprint_test_group_results.is_all_pass():
                print("  " + fingerprint_test_group_results.get_details())
            fingerprint_test_results += fingerprint_test_group_results.test_results
        return MultipleTestResults(self.fingerprint_test_runs, fingerprint_test_results)

class FingerprintTestResult(TestResult):
    def __init__(self, test_run, simulation_result, expected_fingerprint, calculated_fingerprint, **kwargs):
        super().__init__(test_run, simulation_result, **kwargs)
        self.expected_fingerprint = expected_fingerprint
        self.calculated_fingerprint = calculated_fingerprint
        self.fingerprint_mismatch = expected_fingerprint != calculated_fingerprint

    def __repr__(self):
        return "Fingerprint test result: " + self.get_description()
    
    def get_description(self, **kwargs):
        return super().get_description(**kwargs) + (" calculated " + str(self.calculated_fingerprint) + " (correct " + str(self.expected_fingerprint) + ")" if self.fingerprint_mismatch else "")

def get_calculated_fingerprint(simulation_result, ingredients):
    stdout = simulation_result.subprocess_result.stdout.decode("utf-8")
    match = re.search("Fingerprint successfully verified:.*? ([0-9a-f]{4}-[0-9a-f]{4})/" + ingredients, stdout)
    if match:
        value = match.groups()[0]
    else:
        match = re.search("Fingerprint mismatch! calculated:.*? ([0-9a-f]{4}-[0-9a-f]{4})/" + ingredients + ".*expected", stdout)
        if match:
            value = match.groups()[0]
        else:
            return None
    return Fingerprint(value + "/" + ingredients)

def check_fingerprint_test(fingerprint_test_run, simulation_result, **kwargs):
    expected_fingerprint = fingerprint_test_run.fingerprint
    expected_result = fingerprint_test_run.test_result
    if simulation_result.result == "CANCEL":
        result = "CANCEL"
        reason = "Cancel by user"
        calculated_fingerprint = None
    else:
        calculated_fingerprint = get_calculated_fingerprint(simulation_result, fingerprint_test_run.fingerprint.ingredients)
        if calculated_fingerprint is None:
            result = "ERROR"
            reason = "Calculated fingerprint not found"
        elif expected_fingerprint != calculated_fingerprint:
            result = "FAIL"
            reason = "Fingerprint mismatch"
        else:
            result = "PASS"
            reason = None
    return FingerprintTestResult(fingerprint_test_run, simulation_result, expected_fingerprint, calculated_fingerprint, result=result, expected_result=expected_result, reason=reason)

def check_fingerprint_test_group(simulation_result, fingerprint_test_group, **kwargs):
    fingerprint_test_results = []
    for fingerprint_test_run in fingerprint_test_group.fingerprint_test_runs:
        if fingerprint_test_run.fingerprint:
            fingerprint_test_result = check_fingerprint_test(fingerprint_test_run, simulation_result, **kwargs)
            fingerprint_test_results.append(fingerprint_test_result)
    return MultipleTestResults(fingerprint_test_group.fingerprint_test_runs, fingerprint_test_results)

def select_fingerprint_entry_by_sim_time_limit(fingerprint_entries, largest):
    sim_time_limits = list(map(lambda fingerprint_entry: fingerprint_entry["sim_time_limit"], fingerprint_entries))
    sim_time_limits.sort(reverse=largest, key=convert_to_seconds)
    smallest_sim_time_limit = sim_time_limits[0]
    sorted_fingerprint_entries = list(filter(lambda fingerprint_entry: fingerprint_entry["sim_time_limit"] == smallest_sim_time_limit, fingerprint_entries))
    sorted_fingerprint_entries.sort(reverse=True, key=lambda fingerprint_entry: fingerprint_entry["timestamp"])
    return sorted_fingerprint_entries[0]

def select_fingerprint_entry_with_smallest_sim_time_limit(fingerprint_entries):
    return select_fingerprint_entry_by_sim_time_limit(fingerprint_entries, False)

def select_fingerprint_entry_with_largest_sim_time_limit(fingerprint_entries):
    return select_fingerprint_entry_by_sim_time_limit(fingerprint_entries, True)

def get_fingerprint_test_run(simulation_run, ingredients="tplx", sim_time_limit=None, select_fingerprint_entry=select_fingerprint_entry_with_smallest_sim_time_limit, **kwargs):
    simulation_config = simulation_run.simulation_config
    stored_fingerprint_entries = correct_fingerprints.filter_entries(ingredients=ingredients, sim_time_limit=sim_time_limit,
                                                                     working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=simulation_run.run)
    selected_fingerprint_entry = select_fingerprint_entry(stored_fingerprint_entries) if stored_fingerprint_entries else None
    if selected_fingerprint_entry:
        sim_time_limit = selected_fingerprint_entry["sim_time_limit"]
        fingerprint = selected_fingerprint_entry["fingerprint"]
        test_result = selected_fingerprint_entry["test_result"]
        return FingerprintTestRun(simulation_run, check_fingerprint_test, sim_time_limit, ingredients, Fingerprint(fingerprint + "/" + ingredients), test_result)
    else:
        return FingerprintTestRun(simulation_run, check_fingerprint_test, sim_time_limit, ingredients, None, None)

def collect_fingerprint_test_groups(simulation_run, ingredients_list=["tplx"], sim_time_limit=None, **kwargs):
    fingerprint_test_runs = []
    for ingredients in ingredients_list:
        fingerprint_test_run = get_fingerprint_test_run(simulation_run, ingredients=ingredients, sim_time_limit=sim_time_limit, **kwargs)
        if fingerprint_test_run:
            fingerprint_test_runs.append(fingerprint_test_run)
    def get_sim_time_limit(element):
        return element.sim_time_limit
    def get_fingerprint_test_group_run(fingerprint_test_runs):
        if len(fingerprint_test_runs) == 1:
            return fingerprint_test_runs[0]
        else:
            return FingerprintTestGroupRun(fingerprint_test_runs[0].sim_time_limit, list(fingerprint_test_runs))
    sorted_fingerprint_test_runs = sorted(fingerprint_test_runs, key=get_sim_time_limit)
    grouped_fingerprint_test_runs = [list(it) for k, it in itertools.groupby(sorted_fingerprint_test_runs, get_sim_time_limit)]
    return list(map(get_fingerprint_test_group_run, grouped_fingerprint_test_runs))

def _run_fingerprint_test(test_run, **kwargs):
    return test_run.run(**kwargs)

def get_fingerprint_tests(**kwargs):
    multiple_simulation_runs = get_simulations(run_simulation_function=_run_fingerprint_test, **kwargs)
    test_runs = list(map(lambda simulation_run: TestRun(simulation_run, check_fingerprint_test, **kwargs), multiple_simulation_runs.simulation_runs))
    fingerprint_test_groups = []
    for test_run in test_runs:
        simulation_run = test_run.simulation_run
        simulation_config = simulation_run.simulation_config
        fingerprint_test_groups += collect_fingerprint_test_groups(simulation_run, **kwargs)
    return MultipleTestRuns(multiple_simulation_runs, fingerprint_test_groups)

def run_fingerprint_tests(**kwargs):
    logger.info("Running fingerprint tests")
    multiple_test_runs = get_fingerprint_tests(**kwargs)
    return multiple_test_runs.run()

def print_correct_fingerprints(**kwargs):
    multiple_test_runs = get_fingerprint_tests(**kwargs)
    for test_run in multiple_test_runs.test_runs:
        print(test_run.simulation_run.get_simulation_parameters_string(**kwargs) + " " + COLOR_GREEN + str(test_run.fingerprint) + COLOR_RESET)

def update_correct_fingerprint(simulation_config, run=0, sim_time_limit=None, **kwargs):
    fingerprint_test_groups = collect_fingerprint_test_groups(simulation_config, sim_time_limit=sim_time_limit, **kwargs)
    fingerprint_test_results = []
    for fingerprint_test_group in fingerprint_test_groups:
        sim_time_limit = fingerprint_test_group.sim_time_limit
        fingerprint_arg = ",".join(map(lambda fingerprint_test_run: "0000-0000/" + fingerprint_test_run.ingredients, fingerprint_test_group.fingerprint_test_runs))
        extra_args = ["--fingerprintcalculator-class", "inet::FingerprintCalculator", "--fingerprint", fingerprint_arg, "--vector-recording", "false", "--scalar-recording", "false"]
        simulation_result = run_simulation_config(simulation_config, sim_time_limit=sim_time_limit, print_end=" ", extra_args=extra_args, **kwargs)
        simulation_run = simulation_result.simulation_run
        if simulation_result.subprocess_result.returncode != 0:
            print("\033[1;31m" + "ERROR" + "\033[0;0m" + " (" + simulation_result.error_message + ")")
        else:
            for fingerprint_test_run in fingerprint_test_group.fingerprint_test_runs:
                calculated_fingerprint = get_calculated_fingerprint(simulation_result, fingerprint_test_run.ingredients)
                print("\033[0;32m" + str(calculated_fingerprint) + "\033[0;0m", end=" ")
                correct_fingerprints.update_fingerprint(calculated_fingerprint.fingerprint, ingredients=calculated_fingerprint.ingredients, test_result="PASS", sim_time_limit=fingerprint_test_run.sim_time_limit,
                                                        working_directory=simulation_run.working_directory, ini_file=simulation_run.ini_file, config=simulation_run.config, run=simulation_run.run)
            print("")

def remove_correct_fingerprint(simulation_config, run=0, ingredients="tplx", sim_time_limit=None, **kwargs):
    correct_fingerprints.remove_fingerprint(ingredients=ingredients, sim_time_limit=sim_time_limit,
                                            working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=run)

def update_correct_fingerprints(**kwargs):
    print("Updating correct fingerprints")
    map_simulation_configs(update_correct_fingerprint, **kwargs)
    correct_fingerprints.write()

def remove_fingerprints(**kwargs):
    print("Removing correct fingerprints")
    map_simulation_configs(remove_correct_fingerprint, **kwargs)
