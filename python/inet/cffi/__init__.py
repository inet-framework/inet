import os

from omnetpp.runtime import *

from inet.common.util import *

inet_project_path = get_inet_relative_path(".")

if inet_project_path is not None and \
   os.path.exists(os.path.join(inet_project_path, "src/libINET.so")):
    from inet.cffi.event import *
    from inet.cffi.inprocess import *
    from inet.cffi.libinet import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
