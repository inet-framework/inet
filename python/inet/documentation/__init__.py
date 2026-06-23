"""
This package supports generating documentation artifacts.

The generic NED/chart documentation support is provided by the
:py:mod:`opp_repl.documentation` package; this package adds the INET-specific
HTML documentation generation and NED documentation helpers.
"""

from opp_repl.documentation import *

from inet.documentation.html import *
from inet.documentation.ned import *

__sphinx_mock__ = True # ignore this module in documentation

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
