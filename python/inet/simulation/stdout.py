"""
This module provides abstractions for stdout processing.
"""

import logging
import os

from inet.common import *
from inet.simulation.eventlog import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class StdoutTrajectory:
    def __init__(self, simulation_result, event_numbers, lines):
        self.simulation_result = simulation_result
        self.event_numbers = event_numbers
        self.lines = lines

    def print_trajectory(self):
        for event_number, line in zip(self.event_numbers, self.lines):
            print(f"#{event_number} {line}")

    def get_event_lines(self, event_number):
        return [line for event, line in zip(self.event_numbers, self.lines) if event == event_number]

class StdoutTrajectorySimulationEvent(SimulationEvent):
    def __init__(self, trajectory, line_number, event_number):
        self.simulation_result = trajectory.simulation_result
        simulation_config = self.simulation_result.task.simulation_config
        simulation_project = simulation_config.simulation_project
        eventlog_file_path = simulation_project.get_full_path(os.path.join(simulation_config.working_directory, self.simulation_result.eventlog_file_path))
        eventlog = create_eventlog(eventlog_file_path)
        super().__init__(event_number, eventlog)
        self.trajectory = trajectory
        self.line_number = line_number

    def __repr__(self):
        return repr(self)

    def get_description(self):
        description = SimulationEvent.get_description(self)
        return description + "\n    " + self.trajectory.lines[self.line_number]

class StdoutTrajectoryDivergencePosition:
    def __init__(self, simulation_event_1, simulation_event_2):
        self.simulation_event_1 = simulation_event_1
        self.simulation_event_2 = simulation_event_2

    def __repr__(self):
        return f"STDOUT trajectory divergence point:\n{self.get_description()}"

    def get_description(self):
        event_description_1 = self.simulation_event_1.get_description()
        event_description_2 = self.simulation_event_2.get_description()
        return f"  {event_description_1}\n  {event_description_2}"

def find_stdout_trajectory_divergence_position(stdout_trajectory_1, stdout_trajectory_2):
    min_size = min(len(stdout_trajectory_1.lines), len(stdout_trajectory_2.lines))
    for i in range(0, min_size):
        trajectory_line_1 = stdout_trajectory_1.lines[i]
        trajectory_line_2 = stdout_trajectory_2.lines[i]
        if trajectory_line_1 != trajectory_line_2:
            event_number_1 = stdout_trajectory_1.event_numbers[i]
            event_number_2 = stdout_trajectory_2.event_numbers[i]
            return StdoutTrajectoryDivergencePosition(StdoutTrajectorySimulationEvent(stdout_trajectory_1, i, event_number_1),
                                                      StdoutTrajectorySimulationEvent(stdout_trajectory_2, i, event_number_2))
    return None

