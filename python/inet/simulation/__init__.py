"""
This package supports running INET Framework simulations.

The main function is run_simulations() that allows running multiple simulations,
even completely unrelated ones that have different working directories, INI files,
and configurations. The simulations can be run sequentially or concurrently on a
single computer or on an SSH cluster. Besides, the simulations can be run as
separate processes and also in the same Python process loading INET as a library.
"""

from inet.simulation.build import *
from inet.simulation.config import *
from inet.simulation.optimize import *
from inet.simulation.project import *
from inet.simulation.task import *
from inet.simulation.subprocess import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
