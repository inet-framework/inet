"""
This package provides generally useful functionality.

The generic functionality is provided by the :py:mod:`opp_repl.common` package;
this package only adds the INET-specific path helpers.
"""

from opp_repl.common import *

from inet.common.util import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
