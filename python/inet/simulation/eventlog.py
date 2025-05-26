"""
This module provides abstractions for eventlog.
"""

import logging
import os

from inet.common import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

class SimulationEvent:
    def __init__(self, event_number, eventlog):
        self.event_number = event_number
        self.eventlog = eventlog

    def __repr__(self):
        return repr(self)

    def get_event(self):
        return self.eventlog.getEventForEventNumber(self.event_number)

    def get_module_path(self):
        event = self.get_event()
        module_description_entry = event.getModuleDescriptionEntry()
        path = []
        eventlog_cache = self.eventlog.getEventLogEntryCache()
        while module_description_entry:
            full_name = module_description_entry.getFullName()
            path.append(full_name)
            module_description_entry = eventlog_cache.getModuleDescriptionEntry(module_description_entry.getParentModuleId())
        path.reverse()
        return ".".join(path)

    def get_description(self):
        event = self.get_event()
        simulation_time = event.getSimulationTime()
        module_description_entry = event.getModuleDescriptionEntry()
        full_path = self.get_module_path()
        ned_type_name = module_description_entry.getNedTypeName()
        cause_begin_send_entry = event.getCauseBeginSendEntry()
        message_name = event.getCauseBeginSendEntry().getMessageName() if cause_begin_send_entry else None
        event_description = f"#{COLOR_GREEN}{self.event_number}{COLOR_RESET} at {COLOR_GREEN}{simulation_time}{COLOR_RESET} in {COLOR_CYAN}{full_path}{COLOR_RESET} ({COLOR_GREEN}{ned_type_name}{COLOR_RESET})" + (f" on {COLOR_GREEN}{message_name}{COLOR_RESET}" if message_name else "")
        return event_description

    def print_cause_chain(self, num_cause_events=3):
        simulation_event = self
        while num_cause_events > 0 and simulation_event:
            print(simulation_event.get_description())
            cause_event = simulation_event.get_event().getCauseEvent() if simulation_event else None
            simulation_event = SimulationEvent(cause_event.getEventNumber(), simulation_event.eventlog) if cause_event else None
            num_cause_events = num_cause_events - 1
