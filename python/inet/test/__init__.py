"""
This package supports automated testing.

The generic test infrastructure (smoke, fingerprint, statistical, chart, speed,
sanitizer, coverage, profile, ...) is provided by the :py:mod:`opp_repl.test`
package. This package adds the INET-specific test types:
 - :py:func:`run_validation_tests <inet.test.validation.run_validation_tests()>`: compare simulation results to analytical models
"""

from opp_repl.test import *

# INET-specific test types (imported after opp_repl so they win on name clashes)
from inet.test.validation import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
