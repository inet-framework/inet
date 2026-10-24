Easy to Handle Changes
======================

..   1. change or action
     2. phenomenon / effect
     3. solution
     4. example

Some changes in the model don't change fingerprints at all, and are unlikely to cause regressions.
These changes include renaming C++ classes, functions and variables, and extracting methods or classes, refactoring algorithms.

For example, we extracted a part of the ``handleUpperCommand()`` function in the :ned:`Udp` module to a new function:

.. literalinclude:: ../sources/Udp.cc.extract
   :diff: ../sources/Udp.cc.orig

The refactoring doesn't change the fingerprint because the code is functionally the same. It doesn't create any new events or data packets, and it doesn't change timing or anything that the fingerprint calculation takes into account:

.. code-block:: fp

   $ opp_run_fingerprint_tests --load @opp -p inet -w tutorials/fingerprint --ingredients tplx -c '^(Ethernet|Wifi)$'
   tutorials/fingerprint -c Ethernet for 0.2s PASS
   tutorials/fingerprint -c Wifi for 5s PASS
