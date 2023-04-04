"""
Provides generic functionality for other modules.

Please note that undocumented features are not supposed to be used by the user.
"""

from omnetpp.common.cluster import *
from omnetpp.common.compile import *
from omnetpp.common.task import *
from omnetpp.common.util import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
