import os

from omnetpp.common.util import *

def get_inet_relative_path(path):
    return os.path.join(get_workspace_path("inet"), path)
