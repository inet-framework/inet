Fingerprint Tests
~~~~~~~~~~~~~~~~~

INET no longer has its own fingerprint test tool. The baseline is the JSON
store ``tests/fingerprint/store.json``, and the commands of opp_repl run and
update it. Install opp_repl as the GitHub workflow does, with
``pip install "opp_repl @ git+https://github.com/omnetpp/opp_repl.git@opp_ci"``, set up the OMNeT++
and INET environment, and run the commands from anywhere in the INET tree:

.. code-block:: bash

   opp_run_fingerprint_tests --load @opp -p inet -m release -f wireless
   opp_update_fingerprint_test_results --load @opp -p inet -m release -f wireless

The second command replaces the ``.UPDATED`` files of ``fingerprinttest``: it
writes the new values into the store, and ``git diff`` shows them. The command
line options of ``fingerprinttest`` have these replacements:

============================================  ==========================================================
``fingerprinttest``                           ``opp_run_fingerprint_tests``
============================================  ==========================================================
``-m <regex>``, ``-x <regex>``                ``-f <regex>``, ``--exclude-filter <regex>``
``-f <ingredients>``                          ``--ingredients <ingredients>``
``-a <opp_run arguments>``                    ``--append-args <opp_run arguments>`` (the last option)
``-n <count> -i <index>``                     ``--split-count <count> --split-index <index>``
``-t <threads>``                              runs on all cores; ``--no-concurrent`` runs one at a time
``-d``, ``-s``                                ``-m debug``, ``-m release``
============================================  ==========================================================

A project of your own that keeps fingerprint CSV files in the format of INET
can convert them with opp_repl's
``opp_repl.test.fingerprint.old.update_correct_fingerprints_from_csvs()``. The
converted store holds the same values, and an empty CSV limit takes the limit
of the ini file. A project of your own that called ``bin/inet_fingerprinttest``
or ``bin/inet_smoketest`` uses ``opp_run_fingerprint_tests`` and
``opp_run_smoke_tests`` with its own ``.opp`` file.
