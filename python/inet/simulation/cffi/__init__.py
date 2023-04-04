import os

from omnetpp.simulation.cffi import *

from inet.common.util import *

inet_project_path = get_inet_relative_path(".")

if inet_project_path is not None and \
   os.path.exists(os.path.join(inet_project_path, "src/libINET.so")):
    from inet.simulation.cffi.lib import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
