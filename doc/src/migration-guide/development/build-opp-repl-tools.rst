Python Test Tools
~~~~~~~~~~~~~~~~~

INET no longer has its own copy of opp_repl's simulation and test framework in
:file:`python/inet`, and :file:`bin/` no longer has the tools that called it.
Install opp_repl with
``pip install "opp_repl @ git+https://github.com/omnetpp/opp_repl.git"``, set up
the OMNeT++ and INET environment, and run the opp_repl commands in the INET tree,
where they find INET's :file:`inet.opp` by themselves. Each removed tool has a
command that takes its place; ``-h`` shows its options:

==============================  ==============================
Removed INET tool               opp_repl command
==============================  ==============================
``inet_run_<kind>_tests``       ``opp_run_<kind>_tests``
``inet_run_simulations``        ``opp_run_simulations``
``inet_build_project``          ``opp_build_project``
``inet_python_repl``            ``opp_repl``
``inet_run_self_tests``         ``opp_repl_self_test``
``inet_diffcharts``             ``opp_diff_charts``
==============================  ==============================

``<kind>`` is one of ``all``, ``chart``, ``feature``, ``module``, ``packet``,
``protocol``, ``queueing``, ``release``, ``sanitizer``, ``smoke``, ``speed``,
``statistical``, ``unit`` and ``validation``. ``inet_run_serializer_tests``
stays, and runs ``opp_run_opp_tests`` in :file:`tests/serializer`.

The INET package keeps :py:mod:`inet.scave`, which the chart scripts of the
analysis files import, :py:mod:`inet.test.validation`, which
``opp_run_validation_tests`` calls, and :py:mod:`inet.documentation`. A Python
program of your own that imported ``inet.common``, ``inet.simulation``,
``inet.project`` or another ``inet.test`` module imports the same names from
opp_repl, for example ``from opp_repl import *``, and loads the INET project
description with ``load_opp_file()`` from the file :file:`inet.opp` in the INET root.
