Easy to Handle Changes
======================

..   1. change or action
     2. phenomenon / effect
     3. solution
     4. example

Some changes in the model don't change fingerprints at all, and are unlikely to cause regressions.
These changes include renaming C++ classes, functions and variables, and extracting methods or classes, refactoring algorithms.

For example, we extract some part of the ``handleUpperCommand()`` function in the :ned:`Udp` module to a new function:

.. literalinclude:: ../sources/Udp.cc.extract
   :diff: ../sources/Udp.cc.orig

The refactoring doesn't change the fingerprint because the code is functionally the same; it doesn't create any new events or data packets, and it doesn't change timing, or anything that the fingerprint calculation takes into account:

.. code-block:: fp

   $ inet_fingerprinttest -m EasyToHandleChanges
   . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
   . -f omnetpp.ini -c Wifi -r 0  ... : PASS

   ----------------------------------------------------------------------
   Ran 2 tests in 6.714s

   OK

   Log has been saved to test.out
   Test results equals to expected results
