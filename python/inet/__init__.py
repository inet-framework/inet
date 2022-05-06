"""
This is the main package for the INET Framework.

It provides sub-packages for running simulations, automated testing, and generating
documentation.

Run help(inet.simulation) or using one of the other sub-packages for more details.
"""

from inet.documentation import *
from inet.simulation import *
from inet.test import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
