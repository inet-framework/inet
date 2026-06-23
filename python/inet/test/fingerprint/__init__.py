"""
This package supports automated fingerprint testing.

The generic fingerprint store and tasks are provided by the
:py:mod:`opp_repl.test.fingerprint` package. This package adds the INET-specific
fingerprint tasks bound to the INET project (including the old CSV-based store).
"""

from opp_repl.test.fingerprint import *

from inet.test.fingerprint.old import *
from inet.test.fingerprint.task import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
