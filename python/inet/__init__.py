"""
INET Framework Python package — INET-specific extensions on top of ``opp_repl``.

The generic OMNeT++ simulation, testing, documentation and analysis tooling lives in
the ``opp_repl`` package. This package adds only the INET-specific pieces: the INET
test types (validation, packet/protocol/queueing/module/unit tests), result analysis
(``scave``), HTML/NED documentation, the cffi inprocess runner, and the INET path helpers.

Use the ``opp_repl`` command-line tools and REPL to run and test INET simulations (the
INET project is defined by the bundled ``inet.opp`` descriptor). Inside the REPL,
``from inet import *`` makes the INET-specific functions available alongside the generic
``opp_repl`` API.
"""

from inet.documentation import *
from inet.scave import *
from opp_repl.simulation import *
from inet.test import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]
