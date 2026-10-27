"""
This is the Python package of the INET Framework. It holds only the Python code that is specific to INET:

- :py:mod:`inet.scave`: the helpers of the chart scripts in the analysis (``.anf``) files
- :py:mod:`inet.test.validation`: the validation tests, which compare simulation results with analytical models and
  with the results of other simulators
- :py:mod:`inet.documentation`: build, upload and open the HTML documentation of INET

To build INET, run simulations and run the tests, use opp_repl (https://github.com/omnetpp/opp_repl), for example
:command:`opp_build_project`, :command:`opp_run_simulations` and the :command:`opp_run_*_tests` commands. The file
``inet.opp`` in the INET root describes the INET project for opp_repl. :doc:`/developers-guide/ch-testing` describes
the tests.

The package imports no sub-package, so that a chart script that imports :py:mod:`inet.scave` needs no opp_repl.
"""
