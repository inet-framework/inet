"""
This package supports automated testing.

It provides several functions to run various tests:
 - :py:func:`run_chart_tests <inet.test.chart.run_chart_tests()>`: find graphical regressions in plotted charts
 - :py:func:`run_fingerprint_tests <inet.test.fingerprint.task.run_fingerprint_tests()>`: protect against regressions in the simulation trajectory
 - :py:func:`run_smoke_tests <inet.test.smoke.run_smoke_tests()>`: quickly check if simulations run without crashing and terminate properly
 - :py:func:`run_statistical_tests <inet.test.statistical.run_statistical_tests()>`: finds regressions in scalar statistical results
 - :py:func:`run_validation_tests <inet.test.validation.run_validation_tests()>`: compare simulation results to analytical models
"""

from inet.test.all import *
from inet.test.chart import *
from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.opp import *
from inet.test.release import *
from inet.test.sanitizer import *
from inet.test.simulation import *
from inet.test.smoke import *
from inet.test.speed import *
from inet.test.statistical import *
from inet.test.task import *
from inet.test.validation import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]

