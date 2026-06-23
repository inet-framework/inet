import os

__sphinx_mock__ = True # ignore this module in documentation

# The generic utility functions live in opp_repl.common.util and are re-exported by
# inet.common. This module only keeps the INET-specific path helper.

def get_inet_relative_path(path):
    return os.path.join(os.environ["INET_ROOT"], path)
