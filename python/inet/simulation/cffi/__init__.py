import os

from inet.common.util import *

omnetpp_project_path = get_workspace_path("omnetpp")
inet_project_path = get_workspace_path("inet")

if omnetpp_project_path is not None and inet_project_path is not None and \
   os.path.exists(omnetpp_project_path) and os.path.exists(inet_project_path) and \
   os.path.exists(os.path.join(omnetpp_project_path, "include")) and os.path.exists(os.path.join(inet_project_path, "src")):
    from inet.simulation.cffi.event import *
    from inet.simulation.cffi.inetlib import *
    from inet.simulation.cffi.inprocess import *
    from inet.simulation.cffi.omnetpplib import *
