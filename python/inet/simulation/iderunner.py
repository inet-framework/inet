import logging
import os
import subprocess

from inet.common import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class IdeSimulationRunner:
    def run(self, simulation_task, args):
        simulation_config = simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        full_working_directory = simulation_project.get_full_path(working_directory)
        name = simulation_task.get_parameters_string()
        if simulation_task.debug:
            if simulation_task.break_at_matching_event:
                debugger_init_commands = [f"breakpoint set -G true -n cSimulation::setupNetwork -C \"expression (void) omnetpp::cmdenv::Cmdenv::setMatchEventCondition([] (omnetpp::cEvent *event) -> bool {{ return {simulation_task.break_at_matching_event.replace('\"', '\\\"')}; }})\"",
                                          f"breakpoint set -G true -n omnetpp::cmdenv::Cmdenv::handleMatchingEvent -C \"break set -o true -r '.*::handleMessage'\""]
            elif simulation_task.break_at_event_number:
                debugger_init_commands = [f"breakpoint set -G true -n cSimulation::setupNetwork -C \"expression (void) omnetpp::cmdenv::Cmdenv::setMatchEventCondition([] (omnetpp::cEvent *event) -> bool {{ return omnetpp::cSimulation::getActiveSimulation()->getEventNumber() == {simulation_task.break_at_event_number - 1}; }})\"",
                                          f"breakpoint set -G true -n omnetpp::cmdenv::Cmdenv::handleMatchingEvent -C \"break set -o true -r '.*::handleMessage' -c 'omnetpp::cSimulation::getActiveSimulation()->getEventNumber() == {simulation_task.break_at_event_number}'\""]
            else:
                debugger_init_commands = []
            return debug_program(name, args[0], args[1:], full_working_directory, debugger_init_commands=debugger_init_commands, remove_launch=simulation_task.remove_launch)
        else:
            return launch_program(name, args[0], args[1:], full_working_directory, remove_launch=simulation_task.remove_launch)
