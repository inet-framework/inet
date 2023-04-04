"""
Provides functionality for the automated testing of simulations.

The main functions are:

 - :py:func:`omnetpp.test.smoke.run_smoke_tests`: checks if simulations run without crashing and terminate properly
 - :py:func:`omnetpp.test.fingerprint.run_fingerprint_tests`: checks for regressions in the execution trajectory of simulations

Please note that undocumented features are not supposed to be used by the user.
"""

from omnetpp.test.fingerprint import *
from omnetpp.test.task import *
from omnetpp.test.simulation import *
from omnetpp.test.smoke import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
