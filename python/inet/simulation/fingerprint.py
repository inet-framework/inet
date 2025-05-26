"""
This module provides abstractions for fingerprints.
"""

import logging
import os

from inet.common import *
from inet.simulation.eventlog import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class Fingerprint:
    def __init__(self, fingerprint, ingredients):
        self.fingerprint = fingerprint
        self.ingredients = ingredients

    def __repr__(self):
        return repr(self)

    def __str__(self):
        return self.fingerprint + "/" + self.ingredients

    def __eq__(self, other):
        return other and self.fingerprint == other.fingerprint and self.ingredients == other.ingredients

    def __ne__(self, other):
        return not self.__eq__(other)

    def __composite_values__(self):
        return self.fingerprint, self.ingredients

    @classmethod
    def parse(self, text):
        match = re.match(r"(.*)/(.*)", text)
        fingerprint = match.groups()[0]
        ingredients = match.groups()[1]
        return Fingerprint(fingerprint, ingredients)

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
            event_number = self.event_numbers[i]
            j = i
            while (j < len(self.fingerprints)) and (fingerprint == self.fingerprints[j]):
                j = j + 1
            unique_fingerprints.append(fingerprint)
            event_numbers.append(event_number)
            i = j
        return FingerprintTrajectory(self.simulation_result, self.ingredients, unique_fingerprints, event_numbers)

    def print_trajectory(self):
        for fingerprint, event_number in zip(self.fingerprints, self.event_numbers):
            print(f"#{event_number} {fingerprint}")

class FingerprintSimulationTrajectoryEvent(SimulationEvent):
    def __init__(self, simulation_result, event_number):
        self.simulation_result = simulation_result
        simulation_config = self.simulation_result.task.simulation_config
        simulation_project = simulation_config.simulation_project
        eventlog_file_path = simulation_project.get_full_path(os.path.join(simulation_config.working_directory, self.simulation_result.eventlog_file_path))
        eventlog = create_eventlog(eventlog_file_path)
        super().__init__(event_number, eventlog)

class FingerprintTrajectoryDivergencePosition:
    def __init__(self, simulation_event_1, simulation_event_2):
        self.simulation_event_1 = simulation_event_1
        self.simulation_event_2 = simulation_event_2

    def __repr__(self):
        return f"Fingerprint trajectory divergence point:\n{self.get_description()}"

    def get_description(self):
        event_description_1 = self.simulation_event_1.get_description()
        event_description_2 = self.simulation_event_2.get_description()
        return f"  {event_description_1}\n  {event_description_2}"

    def print_cause_chain(self, num_cause_events=3):
        print("Simulation event cause chain 1:")
        self.simulation_event_1.print_cause_chain(num_cause_events=num_cause_events)
        print("Simulation event cause chain 2:")
        self.simulation_event_2.print_cause_chain(num_cause_events=num_cause_events)

    def show_in_sequence_chart(self):
        simulation_event_1 = self.simulation_event_1
        simulation_event_2 = self.simulation_event_2
        project_name1 = simulation_event_1.simulation_result.task.simulation_config.simulation_project.get_name()
        project_name2 = simulation_event_2.simulation_result.task.simulation_config.simulation_project.get_name()
        path_name1 = "/" + project_name1 + "/" + simulation_event_1.simulation_result.task.simulation_config.working_directory + "/" + simulation_event_1.simulation_result.eventlog_file_path
        path_name2 = "/" + project_name2 + "/" + simulation_event_2.simulation_result.task.simulation_config.working_directory + "/" + simulation_event_2.simulation_result.eventlog_file_path
        editor1 = open_editor(path_name1)
        editor2 = open_editor(path_name2)
        goto_event_number(editor1, simulation_event_1.event_number)
        goto_event_number(editor2, simulation_event_2.event_number)

def find_fingerprint_trajectory_divergence_position(fingerprint_trajectory_1, fingerprint_trajectory_2):
    min_size = min(len(fingerprint_trajectory_1.fingerprints), len(fingerprint_trajectory_2.fingerprints))
    for i in range(0, min_size):
        trajectory_fingerprint_1 = fingerprint_trajectory_1.fingerprints[i]
        trajectory_fingerprint_2 = fingerprint_trajectory_2.fingerprints[i]
        if trajectory_fingerprint_1.fingerprint != trajectory_fingerprint_2.fingerprint:
            # NOTE: if the fingerprints are first different at event_number_1 and event_number_2
            # then something works differently at one event earlier because the event fingerprint
            # contains the hash of the simulation trajectory up to the given event
            event_number_1 = fingerprint_trajectory_1.event_numbers[i] - 1
            event_number_2 = fingerprint_trajectory_2.event_numbers[i] - 1
            return FingerprintTrajectoryDivergencePosition(FingerprintSimulationTrajectoryEvent(fingerprint_trajectory_1.simulation_result, event_number_1),
                                                           FingerprintSimulationTrajectoryEvent(fingerprint_trajectory_2.simulation_result, event_number_2))
    return None
