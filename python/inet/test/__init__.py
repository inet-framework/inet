"""
This package supports automated testing for the INET Framework.

It provides functions to run various tests:
 - run_smoke_tests(): quickly check if simulations run without crashing and terminate properly
 - run_fingerprint_tests(): protect against regressions in the simulation trajectory
 - run_validation_tests(): compare simulation results to analytical models
 - run_sanitizer_tests(): find various C++ programming errors such as memory leaks
 - run_statistical_tests(): finds regressions in scalar statistical results
 - run_chart_tests(): find graphical regressions in plotted charts
 - run_module_tests(): check behavior of specific modules
 - run_unit_tests(): check behavior of specific C++ classes and APIs
 - run_speed_tests(): check simulation execution time
 - run_packet_tests(): check packet and chunk API behavior
 - run_queueing_tests(): check behavior of queueing model element modules
 - run_protocol_tests(): check behavior of protocol element modules
"""

from inet.test.all import *
from inet.test.chart import *
from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.opp import *
from inet.test.release import *
from inet.test.task import *
from inet.test.simulation import *
from inet.test.sanitizer import *
from inet.test.smoke import *
from inet.test.speed import *
from inet.test.statistical import *
from inet.test.validation import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
