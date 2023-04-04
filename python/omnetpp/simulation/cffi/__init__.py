import os

from omnetpp.common.util import *

__sphinx_mock__ = True # ignore this module in documentation

omnetpp_project_path = get_omnetpp_relative_path(".")

if omnetpp_project_path is not None and \
   os.path.exists(omnetpp_project_path) and \
   os.path.exists(os.path.join(omnetpp_project_path, "include")):
    from omnetpp.simulation.cffi.event import *
    from omnetpp.simulation.cffi.inprocess import *
    from omnetpp.simulation.cffi.lib import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
