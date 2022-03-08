import logging
import re

from inet.common.ide import *
from inet.simulation.project import *
from inet.simulation.run import *
from inet.test.fingerprint.store import *
from inet.test.run import *

logger = logging.getLogger(__name__)
all_fingerprint_ingredients = ["tplx", "~tNl", "~tND", "tyf"]
all_fingerprint_ingredients_extra_args = {
    "~tND": {"--**.crcMode=\"computed\"",
             "--**.fcsMode=\"computed\""},
    "tyf" : {"--cmdenv-fake-gui=true",
             "--cmdenv-fake-gui-before-event-probability=0.1",
             "--cmdenv-fake-gui-after-event-probability=0.1",
             "--cmdenv-fake-gui-on-hold-probability=0.1",
             "--cmdenv-fake-gui-on-hold-numsteps=3",
             "--cmdenv-fake-gui-on-simtime-probability=0.1",
             "--cmdenv-fake-gui-on-simtime-numsteps=3",
             "--**.fadeOutMode=\"animationTime\"",
             "--**.signalAnimationSpeedChangeTimeMode=\"animationTime\""}}

def get_ingredients_extra_args(ingredients):
    global all_fingerprint_ingredients_extra_args
    return list(all_fingerprint_ingredients_extra_args[ingredients]) if ingredients in all_fingerprint_ingredients_extra_args else []

def _run_fingerprint_test(test_run, test_result_filter=None, exclude_test_result_filter="SKIP", output_stream=sys.stdout, **kwargs):
    test_result = test_run.run(test_result_filter=test_result_filter, exclude_test_result_filter=exclude_test_result_filter, output_stream=output_stream, **kwargs)
    test_result.print_result(test_result_filter=test_result_filter, exclude_test_result_filter=exclude_test_result_filter, complete_error_message=False, output_stream=output_stream)
    return test_result

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

    def create_cancel_result(self, simulation_result):
        return FingerprintTestResult(self, simulation_result, None, None, result="CANCEL", reason="Cancel by user")

    def run(self, sim_time_limit=None, test_result_filter=None, exclude_test_result_filter="SKIP", output_stream=sys.stdout, **kwargs):
        if self.fingerprint:
            simulation_project = self.simulation_run.simulation_config.simulation_project
            return super().run(extra_args=self.get_extra_args(simulation_project, str(self.fingerprint)) + get_ingredients_extra_args(self.ingredients), sim_time_limit=sim_time_limit or self.sim_time_limit, output_stream=output_stream, **kwargs)
        else:
            if matches_filter("SKIP", test_result_filter, exclude_test_result_filter, True):
                print("Running " + self.simulation_run.get_simulation_parameters_string(**kwargs), end=" ", file=output_stream)
            return FingerprintTestResult(self, None, None, None, result="SKIP", reason="Correct fingerprint not found")

    def get_extra_args(self, simulation_project, fingerprint_arg):
        return ["-n", simulation_project.get_full_path(".") + "/tests/networks", "--fingerprintcalculator-class", "inet::FingerprintCalculator", "--fingerprint", fingerprint_arg, "--vector-recording", "false", "--scalar-recording", "false"]

class FingerprintTestGroupRun:
    def __init__(self, sim_time_limit, fingerprint_test_runs, **kwargs):
        for fingerprint_test_run in fingerprint_test_runs:
            assert fingerprint_test_run.sim_time_limit == sim_time_limit
        self.sim_time_limit = sim_time_limit
        self.fingerprint_test_runs = fingerprint_test_runs

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        for fingerprint_test_run in self.fingerprint_test_runs:
            fingerprint_test_run.set_cancel(cancel)

    def run(self, sim_time_limit=None, output_stream=sys.stdout, **kwargs):
        if len(self.fingerprint_test_runs) == 1:
            return self.fingerprint_test_runs[0].run(sim_time_limit=sim_time_limit, output_stream=output_stream, **kwargs)
        else:
            fingerprint_test_results = []
            skipped_fingerprint_test_runs = list(filter(lambda fingerprint_test_run: fingerprint_test_run.fingerprint is None, self.fingerprint_test_runs))
            for skipped_fingerprint_test_run in skipped_fingerprint_test_runs:
                fingerprint_test_results.append(skipped_fingerprint_test_run.run(**kwargs))
            checked_fingerprint_test_runs = list(filter(lambda fingerprint_test_run: fingerprint_test_run.fingerprint, self.fingerprint_test_runs))
            if checked_fingerprint_test_runs:
                simulation_run = checked_fingerprint_test_runs[0].simulation_run
                simulation_project = simulation_run.simulation_config.simulation_project
                fingerprint_arg = ",".join(map(lambda e: str(e.fingerprint), checked_fingerprint_test_runs))
                extra_args = checked_fingerprint_test_runs[0].get_extra_args(simulation_project, fingerprint_arg)
                ingredients_extra_args = set()
                for checked_fingerprint_test_run in checked_fingerprint_test_runs:
                    ingredients_extra_args = ingredients_extra_args.union(get_ingredients_extra_args(checked_fingerprint_test_run.ingredients))
                simulation_result = simulation_run.run_simulation(print_end=" ", sim_time_limit=sim_time_limit or self.sim_time_limit, output_stream=output_stream, extra_args=extra_args + list(ingredients_extra_args), **kwargs)
                fingerprint_test_group_results = check_fingerprint_test_group(simulation_result, self, **kwargs)
                fingerprint_test_results += fingerprint_test_group_results.test_results
            return MultipleTestResults(self.fingerprint_test_runs, fingerprint_test_results)

class MultipleFingerprintTestRuns(MultipleTestRuns):
    def __init__(self, multiple_simulation_runs, test_runs, **kwargs):
        super().__init__(multiple_simulation_runs, test_runs, **kwargs)

    def store_fingerprint_results(self, multiple_fingerprint_test_results):
        if len(multiple_fingerprint_test_results.test_results) > 0:
            simulation_project = multiple_fingerprint_test_results.test_results[0].test_run.simulation_run.simulation_config.simulation_project
            git_hash = subprocess.run(["git", "rev-parse", "HEAD"], cwd=simulation_project.get_full_path("."), capture_output=True).stdout.decode("utf-8").strip()
            git_clean = subprocess.run(["git", "diff", "--quiet"], cwd=simulation_project.get_full_path("."), capture_output=True).returncode == 0
            all_fingerprint_store = get_all_fingerprint_store(simulation_project)
            for fingerprint_test_result in multiple_fingerprint_test_results.test_results:
                result = fingerprint_test_result.result
                if result != "SKIP" and result != "CANCEL":
                    fingerprint_test_run = fingerprint_test_result.test_run
                    simulation_result = fingerprint_test_result.simulation_result
                    simulation_run = fingerprint_test_run.simulation_run
                    simulation_config = simulation_run.simulation_config
                    simulation_project = simulation_config.simulation_project
                    calculated_fingerprint = fingerprint_test_result.calculated_fingerprint.fingerprint if fingerprint_test_result.calculated_fingerprint else None
                    all_fingerprint_store.insert_fingerprint(calculated_fingerprint, ingredients=fingerprint_test_run.ingredients, test_result=fingerprint_test_result.result, sim_time_limit=fingerprint_test_run.sim_time_limit,
                                                             working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=simulation_run.run,
                                                             git_hash=git_hash, git_clean=git_clean)
            all_fingerprint_store.write()

    def run(self, **kwargs):
        multiple_fingerprint_test_results = super().run(**kwargs)
        self.store_fingerprint_results(multiple_fingerprint_test_results)
        return multiple_fingerprint_test_results

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

    def print_result(self, test_result_filter=None, exclude_test_result_filter="SKIP", complete_error_message=True, output_stream=sys.stdout):
        if matches_filter(self.result, test_result_filter, exclude_test_result_filter, True):
            print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

    def get_fingerprint_trajectory(self):
        simulation_run = self.test_run.simulation_run
        simulation_config = simulation_run.simulation_config
        simulation_project = simulation_config.simulation_project
        eventlog_file_name = simulation_config.config + "-#" + str(simulation_run.run) + ".elog"
        eventlog_file_path = simulation_project.get_full_path(simulation_config.working_directory + "/results/" + eventlog_file_name)
        eventlog_file = open(eventlog_file_path)
        fingerprints = []
        for line in eventlog_file:
            match = re.match("E # .* f (.*)", line)
            if match:
                fingerprints.append(Fingerprint(match.group(1)))
        eventlog_file.close()
        return FingerprintTrajectory(self.simulation_result, self.test_run.ingredients, fingerprints, range(0, len(fingerprints)))

    def run_baseline_fingerprint_test(self, baseline_simulation_project=inet_baseline_project, **kwargs):
        simulation_run = self.simulation_result.simulation_run
        simulation_config = self.simulation_result.simulation_run.simulation_config
        multiple_test_runs = get_fingerprint_tests(simulation_project=baseline_simulation_project, ingredients_list=[self.test_run.ingredients], full_match=True,
                                                   working_directory_filter=simulation_config.working_directory, ini_file_filter=simulation_config.ini_file, config_filter=simulation_config.config, run=simulation_run.run,
                                                   **kwargs)
        assert(len(multiple_test_runs.test_runs) == 1)
        multiple_test_results = multiple_test_runs.run(**kwargs)
        return multiple_test_results.test_results[0]

    def find_fingerprint_trajectory_divergence(self, baseline_fingerprint_test_result=None, baseline_simulation_project=inet_baseline_project):
        if baseline_fingerprint_test_result is None:
            baseline_fingerprint_test_result = self.run_baseline_fingerprint_test(baseline_simulation_project, record_eventlog=True)
        baseline_fingerprint_trajectory = baseline_fingerprint_test_result.get_fingerprint_trajectory()
        current_fingerprint_trajectory = self.get_fingerprint_trajectory()
        return baseline_fingerprint_trajectory.find_divergence_position(current_fingerprint_trajectory)

    def debug_fingerprint_trajectory_divergence(self, baseline_fingerprint_test_result=None, baseline_simulation_project=inet_baseline_project):
        (baseline_simulation_event, current_simulation_event) = self.find_fingerprint_trajectory_divergence(baseline_fingerprint_test_result)
        baseline_simulation_event.debug()
        current_simulation_event.debug()

class MultipleFingerprintTestResults(MultipleTestResults):
    def __init__(self, **kwargs):
        super().__init(**kwargs)

class FingerprintTrajectoryTestRun:
    def __init__(self, simulation_run, check_test_function, sim_time_limit, ingredients, **kwargs):
        super().__init__(simulation_run, check_test_function, **kwargs)
        self.sim_time_limit = sim_time_limit
        self.ingredients = ingredients
        # TODO

class FingerprintTrajectoryTestResult:
    def __init__(self):
        # TODO
        pass

class FingerprintTrajectory:
    def __init__(self, simulation_result, ingredients, fingerprints, event_numbers):
        self.simulation_result = simulation_result
        self.ingredients = ingredients
        self.fingerprints = fingerprints
        self.event_numbers = event_numbers

    def get_unique(self):
        i = 0
        unique_fingerprints = []
        event_numbers = []
        while i < len(self.fingerprints):
            fingerprint = self.fingerprints[i]
            j = i
            while (j < len(self.fingerprints)) and (fingerprint == self.fingerprints[j]):
                j = j + 1
            unique_fingerprints.append(fingerprint)
            event_numbers.append(j - 1)
            i = j
        return FingerprintTrajectory(self.ingredients, unique_fingerprints, event_numbers)

    def find_divergence_position(self, other):
        min_size = min(len(self.fingerprints), len(other.fingerprints))
        for i in range(0, min_size):
            self_fingerprint = self.fingerprints[i]
            other_fingerprint = other.fingerprints[i]
            if self_fingerprint.fingerprint != other_fingerprint.fingerprint:
                return FingerprintTrajectoryDivergence(SimulationEvent(self.simulation_result.simulation_run, self.event_numbers[i]),
                                                       SimulationEvent(other.simulation_result.simulation_run, other.event_numbers[i]))
        return None

class FingerprintTrajectoryDivergence:
    def __init__(self, simulation_event_1, simulation_event_2):
        self.simulation_event_1 = simulation_event_1
        self.simulation_event_2 = simulation_event_2

    def __repr__(self):
        simulation_project_1 = self.simulation_event_1.simulation_run.simulation_config.simulation_project
        simulation_project_2 = self.simulation_event_2.simulation_run.simulation_config.simulation_project
        return "Fingerprint trajectory divergence at " + \
               simulation_project_1.get_name() + " #" + str(self.simulation_event_1.event_number) + ", " + \
               simulation_project_2.get_name() + " #" + str(self.simulation_event_2.event_number)

    def open_sequence_charts(self):
        project_name1 = self.simulation_event_1.simulation_run.simulation_config.simulation_project.get_name()
        project_name2 = self.simulation_event_2.simulation_run.simulation_config.simulation_project.get_name()
        path_name1 = "/" + project_name1 + "/" + self.simulation_event_1.simulation_run.simulation_config.working_directory + "/results/" + self.simulation_event_1.simulation_run.simulation_config.config + "-#" + str(self.simulation_event_1.simulation_run.run) + ".elog"
        path_name2 = "/" + project_name2 + "/" + self.simulation_event_2.simulation_run.simulation_config.working_directory + "/results/" + self.simulation_event_2.simulation_run.simulation_config.config + "-#" + str(self.simulation_event_2.simulation_run.run) + ".elog"
        editor1 = open_editor(path_name1)
        editor2 = open_editor(path_name2)
        goto_event_number(editor1, self.simulation_event_1.event_number)
        goto_event_number(editor2, self.simulation_event_2.event_number)

    def debug_simulation_runs(self):
        pass
        # TODO

class SimulationEvent:
    def __init__(self, simulation_run, event_number):
        self.simulation_run = simulation_run
        self.event_number = event_number

    def __repr__(self):
        return repr(self)

    def debug(self):
        self.simulation_run.run_simulation(debug_event_number=self.event_number)

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
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_config.simulation_project)
    stored_fingerprint_entries = correct_fingerprint_store.filter_entries(ingredients=ingredients, sim_time_limit=sim_time_limit,
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
        return FingerprintTestGroupRun(fingerprint_test_runs[0].sim_time_limit, list(fingerprint_test_runs))
    sorted_fingerprint_test_runs = sorted(fingerprint_test_runs, key=get_sim_time_limit)
    grouped_fingerprint_test_runs = [list(it) for k, it in itertools.groupby(sorted_fingerprint_test_runs, get_sim_time_limit)]
    return list(map(get_fingerprint_test_group_run, grouped_fingerprint_test_runs))

def get_fingerprint_tests(multiple_simulation_runs=None, **kwargs):
    if not multiple_simulation_runs:
        multiple_simulation_runs = get_simulations(run_simulation_function=_run_fingerprint_test, **kwargs)
    fingerprint_test_groups = []
    for simulation_run in multiple_simulation_runs.simulation_runs:
        simulation_config = simulation_run.simulation_config
        fingerprint_test_groups += collect_fingerprint_test_groups(simulation_run, **kwargs)
    return MultipleFingerprintTestRuns(multiple_simulation_runs, fingerprint_test_groups)

def run_fingerprint_tests(multiple_fingerprint_test_results=None, **kwargs):
    multiple_test_runs = None
    try:
        logger.info("Running fingerprint tests")
        multiple_simulation_runs = multiple_fingerprint_test_results.multiple_test_runs.multiple_simulation_runs if multiple_fingerprint_test_results else None
        multiple_test_runs = get_fingerprint_tests(multiple_simulation_runs=multiple_simulation_runs, **kwargs)
        return multiple_test_runs.run(**kwargs)
    except KeyboardInterrupt:
        fingerprint_test_results = list(map(lambda test_run: FingerprintTestResult(test_run, None, None, None, result="CANCEL", reason="Cancel by user"), multiple_test_runs.test_runs)) if multiple_test_runs else []
        return MultipleTestResults(multiple_test_runs, fingerprint_test_results)

def print_correct_fingerprints(**kwargs):
    multiple_test_runs = get_fingerprint_tests(**kwargs)
    for test_run in multiple_test_runs.test_runs:
        print(test_run.simulation_run.get_simulation_parameters_string(**kwargs) + " " + COLOR_GREEN + str(test_run.fingerprint) + COLOR_RESET)

def print_missing_correct_fingerprints(simulation_project=default_project, ingredients="tplx", num_runs=1):
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_project)
    for simulation_config in get_all_simulation_configs(simulation_project):
        if not simulation_config.abstract and not simulation_config.emulation:
            for run in range(0, num_runs or simulation_config.num_runs):
                simulation_run = SimulationRun(simulation_config, run=run)
                if not simulation_run.is_interactive():
                    stored_fingerprint_entries = correct_fingerprint_store.filter_entries(ingredients=ingredients, working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=run)
                    if len(stored_fingerprint_entries) == 0:
                        print(simulation_run.get_simulation_parameters_string())

def insert_missing_correct_fingerprints(source_ingredients, target_ingredients, simulation_project=default_project, **kwargs):
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_project)
    multiple_simulation_runs = get_simulations(**kwargs)
    for simulation_run in multiple_simulation_runs.simulation_runs:
        simulation_config = simulation_run.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        ini_file = simulation_config.ini_file
        config = simulation_config.config
        run = simulation_run.run
        for source_entry in correct_fingerprint_store.filter_entries(ingredients=source_ingredients, working_directory=working_directory, ini_file=ini_file, config=config, run=run):
            sim_time_limit = source_entry["sim_time_limit"]
            target_entries = correct_fingerprint_store.filter_entries(ingredients=target_ingredients, working_directory=working_directory, ini_file=ini_file, config=config, run=run, sim_time_limit=sim_time_limit)
            if len(target_entries) == 0:
                correct_fingerprint_store.insert_fingerprint("0000-0000/" + target_ingredients, ingredients=target_ingredients, working_directory=working_directory, ini_file=ini_file, config=config, run=run, sim_time_limit=sim_time_limit, test_result=source_entry["test_result"])
    correct_fingerprint_store.write()

def _run_fingerprint_update(fingerprint_update_run, output_stream=sys.stdout, **kwargs):
    fingerprint_update_result = fingerprint_update_run.run(output_stream=output_stream, **kwargs)
    fingerprint_update_result.print_result(complete_error_message=False, output_stream=output_stream)
    return fingerprint_update_result

class FingerprintUpdateRun:
    def __init__(self, simulation_run, **kwargs):
        self.simulation_run = simulation_run
        self.cancel = False
        self.kwargs = kwargs

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        self.simulation_run.set_cancel(cancel)

    def run(self, ingredients="tplx", insert_missing_correct_fingerprints=False, sim_time_limit=None, cancel=False, output_stream=sys.stdout, **kwargs):
        if cancel or self.cancel:
            print("Running " + self.simulation_run.get_simulation_parameters_string(sim_time_limit=sim_time_limit, **kwargs), end=" ", file=output_stream)
            return self.create_cancel_result(None)
        else:
            simulation_config = self.simulation_run.simulation_config
            correct_fingerprint_store = get_correct_fingerprint_store(simulation_config.simulation_project)
            stored_fingerprint_entries = correct_fingerprint_store.filter_entries(ingredients=ingredients, sim_time_limit=sim_time_limit,
                                                                                  working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=self.simulation_run.run)
            if len(stored_fingerprint_entries) == 1:
                stored_fingerprint_entry = stored_fingerprint_entries[0]
                if sim_time_limit is None:
                    sim_time_limit = stored_fingerprint_entry["sim_time_limit"]
                correct_fingerprint = Fingerprint(stored_fingerprint_entry["fingerprint"] + "/" + stored_fingerprint_entry["ingredients"])
            else:
                correct_fingerprint = None
            if correct_fingerprint or insert_missing_correct_fingerprints:
                fingerprint_arg = "0000-0000/" + ingredients
                extra_args = ["--fingerprintcalculator-class", "inet::FingerprintCalculator", "--fingerprint", fingerprint_arg, "--vector-recording", "false", "--scalar-recording", "false"] + get_ingredients_extra_args(ingredients)
                self.simulation_run.sim_time_limit = sim_time_limit
                simulation_result = self.simulation_run.run_simulation(sim_time_limit=sim_time_limit, output_stream=output_stream, extra_args=extra_args, **kwargs)
                calculated_fingerprint = get_calculated_fingerprint(simulation_result, ingredients)
                return FingerprintUpdateResult(self, simulation_result, correct_fingerprint, calculated_fingerprint)
            else:
                reason = "No correct fingerprint is found and inserting new fingerprints is disabled" if len(stored_fingerprint_entries) == 0 else "Multiple correct fingerprints are found with different parameters"
                return FingerprintUpdateResult(self, None, None, None, reason=reason)

class MultipleFingerprintUpdateRuns:
    def __init__(self, multiple_simulation_runs, fingerprint_update_runs, **kwargs):
        self.multiple_simulation_runs = multiple_simulation_runs
        self.fingerprint_update_runs = fingerprint_update_runs

    def __repr__(self):
        return repr(self)

    def run(self, simulation_project=None, concurrent=None, build=True, **kwargs):
        if concurrent is None:
            concurrent = self.multiple_simulation_runs.concurrent
        simulation_project = simulation_project or self.multiple_simulation_runs.simulation_project
        if build:
            build_project(simulation_project, **kwargs)
        print("Updating correct fingerprints " + str(kwargs))
        start_time = time.time()
        fingerprint_update_results = map_sequentially_or_concurrently(self.fingerprint_update_runs, self.multiple_simulation_runs.run_simulation_function, concurrent=concurrent, **kwargs)
        end_time = time.time()
        correct_fingerprint_store = get_correct_fingerprint_store(simulation_project)
        for fingerprint_update_result in fingerprint_update_results:
            fingerprint_update_run = fingerprint_update_result.fingerprint_update_run
            simulation_run = fingerprint_update_run.simulation_run
            simulation_config = simulation_run.simulation_config
            calculated_fingerprint = fingerprint_update_result.calculated_fingerprint
            if calculated_fingerprint is not None:
                correct_fingerprint_store.update_fingerprint(calculated_fingerprint.fingerprint, ingredients=calculated_fingerprint.ingredients, test_result="PASS", sim_time_limit=simulation_run.sim_time_limit,
                                                             working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=simulation_run.run)
        correct_fingerprint_store.write()
        return MultipleFingerprintUpdateResults(self.multiple_simulation_runs, fingerprint_update_results, elapsed_wall_time=end_time - start_time)

class FingerprintUpdateResult:
    def __init__(self, fingerprint_update_run, simulation_result, correct_fingerprint, calculated_fingerprint, reason=None, **kwargs):
        self.fingerprint_update_run = fingerprint_update_run
        self.simulation_result = simulation_result
        self.calculated_fingerprint = calculated_fingerprint
        if calculated_fingerprint is None:
            self.result = "ERROR"
            self.reason = reason or "Calculated fingerprint not found"
            self.color = COLOR_RED
        elif correct_fingerprint is None:
            self.result = "INSERT"
            self.reason = None
            self.color = COLOR_YELLOW
        elif calculated_fingerprint != correct_fingerprint:
            self.result = "UPDATE" if calculated_fingerprint else "ERROR"
            self.reason = None
            self.color = COLOR_YELLOW
        else:
            self.result = "KEEP"
            self.reason = None
            self.color = COLOR_GREEN

    def get_description(self, complete_error_message=True, include_simulation_parameters=False):
        return (self.fingerprint_update_run.simulation_run.get_simulation_parameters_string() + " " if include_simulation_parameters else "") + \
               self.color + self.result + COLOR_RESET + " " + str(self.calculated_fingerprint) + \
               ((" " + self.simulation_result.get_error_message(complete_error_message=complete_error_message)) if self.simulation_result and self.simulation_result.result == "ERROR" else "") + \
               (" (" + self.reason + ")" if self.reason else "")

    def __repr__(self):
        return "Fingerprint update result: " + self.get_description(include_simulation_parameters=True)

    def print_result(self, complete_error_message=True, output_stream=sys.stdout):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

class MultipleFingerprintUpdateResults:
    def __init__(self, multiple_fingerprint_update_runs, fingerprint_update_results, elapsed_wall_time=None, **kwargs):
        self.multiple_fingerprint_update_runs = multiple_fingerprint_update_runs
        self.fingerprint_update_results = fingerprint_update_results
        self.elapsed_wall_time = elapsed_wall_time
        self.num_different_results = 0
        self.num_keep = self.count_results("KEEP")
        self.num_insert = self.count_results("INSERT")
        self.num_update = self.count_results("UPDATE")
        self.num_error = self.count_results("ERROR")

    def __repr__(self):
        if len(self.fingerprint_update_results) == 1:
            return self.fingerprint_update_results[0].__repr__()
        else:
            details = self.get_details(exclude_result_filter="KEEP", include_simulation_parameters=True)
            return ("" if details.strip() == "" else "\nDetails:\n" + details + "\n\n") + \
                    "Fingerprint update summary: " + self.get_summary()

    def count_results(self, result):
        num = sum(e.result == result for e in self.fingerprint_update_results)
        if num != 0:
            self.num_different_results += 1
        return num

    def get_result_class_texts(self, result, color, num):
        texts = []
        if num != 0:
            texts += [(color + str(num) + " " + result + COLOR_RESET if num != 0 else "")]
        return texts

    def get_summary(self):
        if len(self.fingerprint_update_results) == 1:
            return self.fingerprint_update_results[0].get_description()
        else:
            texts = []
            if self.num_different_results != 1:
                texts.append(str(len(self.fingerprint_update_results)) + " TOTAL")
            texts += self.get_result_class_texts("KEEP", COLOR_GREEN, self.num_keep)
            texts += self.get_result_class_texts("INSERT", COLOR_YELLOW, self.num_insert)
            texts += self.get_result_class_texts("UPDATE", COLOR_YELLOW, self.num_update)
            texts += self.get_result_class_texts("ERROR", COLOR_RED, self.num_error)
            return ", ".join(texts) + (" in " + str(datetime.timedelta(seconds=self.elapsed_wall_time)) if self.elapsed_wall_time else "")

    def get_details(self, separator="\n  ", result_filter=None, exclude_result_filter=None, **kwargs):
        texts = []
        def matches_simulation_result(simulation_result, result):
            return simulation_result and simulation_result.result == result and \
                   matches_filter(result, result_filter, exclude_result_filter, True)
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "KEEP"), self.fingerprint_update_results):
            texts.append(simulation_result.get_description(**kwargs))
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "INSERT"), self.fingerprint_update_results):
            texts.append(simulation_result.get_description(**kwargs))
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "UPDATE"), self.fingerprint_update_results):
            texts.append(simulation_result.get_description(**kwargs))
        for simulation_result in filter(lambda simulation_result: matches_simulation_result(simulation_result, "ERROR"), self.fingerprint_update_results):
            texts.append(simulation_result.get_description(**kwargs))
        return "  " + separator.join(texts)

def update_correct_fingerprints(**kwargs):
    multiple_fingerprint_update_runs = None
    try:
        fingerprint_update_runs = []
        multiple_simulation_runs = get_simulations(run_simulation_function=_run_fingerprint_update, **kwargs)
        for simulation_run in multiple_simulation_runs.simulation_runs:
            fingerprint_update_run = FingerprintUpdateRun(simulation_run, **kwargs)
            fingerprint_update_runs.append(fingerprint_update_run)
        multiple_fingerprint_update_runs = MultipleFingerprintUpdateRuns(multiple_simulation_runs, fingerprint_update_runs)
        return multiple_fingerprint_update_runs.run(**kwargs)
    except KeyboardInterrupt:
        fingerprint_update_results = list(map(lambda fingerprint_update_run: FingerprintUpdateResult(fingerprint_update_run, None, None, None, reason="Cancel by user"), multiple_fingerprint_update_runs.fingerprint_update_runs)) if multiple_fingerprint_update_runs else []
        return MultipleFingerprintUpdateResults(multiple_simulation_runs, fingerprint_update_results)

def remove_correct_fingerprint(simulation_run, run=0, ingredients="tplx", sim_time_limit=None, **kwargs):
    simulation_config = simulation_run.simulation_config
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_config.simulation_project)
    correct_fingerprint_store.remove_fingerprint(ingredients=ingredients, sim_time_limit=sim_time_limit,
                                                 working_directory=simulation_config.working_directory, ini_file=simulation_config.ini_file, config=simulation_config.config, run=run)

def remove_correct_fingerprints(**kwargs):
    print("Removing correct fingerprints")
    multiple_simulation_runs = get_simulations(run_simulation_function=remove_correct_fingerprint, **kwargs)
    multiple_simulation_runs.run(**kwargs)

def remove_extra_correct_fingerprints(simulation_project=default_project, **kwargs):
    correct_fingerprint_store = get_correct_fingerprint_store(simulation_project)
    for entry in correct_fingerprint_store.get_entries():
        found = False
        for simulation_config in get_all_simulation_configs(simulation_project):
            if simulation_config.working_directory == entry["working_directory"] and \
               simulation_config.ini_file == entry["ini_file"] and \
               simulation_config.config == entry["config"]:
                found = True
        if not found:
            correct_fingerprint_store.remove_entry(entry)
    correct_fingerprint_store.write()
