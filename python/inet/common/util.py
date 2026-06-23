import os

__sphinx_mock__ = True # ignore this module in documentation

# The generic utility functions live in opp_repl.common.util and are re-exported by
# inet.common. This module only keeps the INET-specific path helpers.

def get_omnetpp_relative_path(path):
    return os.path.abspath(os.path.join(os.environ["__omnetpp_root_dir"], path)) if "__omnetpp_root_dir" in os.environ else None

def get_inet_relative_path(path):
    return os.path.join(os.environ["INET_ROOT"], path)

def get_workspace_path(path):
    return os.path.join(os.path.realpath(get_omnetpp_relative_path("..")), path)
