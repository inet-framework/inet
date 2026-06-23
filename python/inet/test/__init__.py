"""
This package supports automated testing.

The generic test infrastructure (smoke, fingerprint, statistical, chart, speed,
sanitizer, coverage, profile, ...) is provided by the :py:mod:`opp_repl.test`
package. This package adds the INET-specific test types and overrides:
 - :py:func:`run_validation_tests <inet.test.validation.run_validation_tests()>`: compare simulation results to analytical models
 - :py:func:`run_all_tests <inet.test.all.run_all_tests()>`: also runs the INET packet/unit/module/protocol/queueing tests
 - INET-specific feature, release and fingerprint tests bound to the INET project
"""

from opp_repl.test import *

# INET-specific test types and overrides (imported after opp_repl so they win on name clashes)
from inet.test.feature import *
from inet.test.fingerprint import *
from inet.test.release import *
from inet.test.validation import *
from inet.test.all import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
