"""
Provides functionality for running simulations.

The main function is :py:func:`omnetpp.simulation.task.run_simulations`. It allows running multiple simulations, even
completely unrelated ones that may have different working directories, INI files, and configurations.

Please note that undocumented features are not supposed to be used by the user.
"""

from omnetpp.simulation.build import *
from omnetpp.simulation.config import *
from omnetpp.simulation.project import *
from omnetpp.simulation.task import *
from omnetpp.simulation.samples import *
from omnetpp.simulation.subprocess import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
