Step 3. Root Bridge Election by Priority
=========================================

Goals
-----

This step shows how to control which switch becomes the root bridge by
configuring bridge priorities, instead of relying on MAC address ordering.

Configuration
~~~~~~~~~~~~~

The configuration extends ``Step2`` with a single extra parameter:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step3]
   :end-before: ------

Setting ``bridgePriority = 1`` on ``switch3`` gives it a bridge ID of
``1:AAAAAA000003``, which is lower than all other switches (whose bridge IDs
are ``32768:AAAAAA0000xx``). Therefore ``switch3`` wins the root election
regardless of its MAC address.

The bridge priority can be set per-switch using the ``**.switchN.**.bridgePriority``
parameter, or configured across multiple switches using :ned:`L2NetworkConfigurator`.
The default value is 32768; valid values are multiples of 4096 from 0 to 61440.

Results
~~~~~~~

With ``switch3`` as root, the spanning tree is rooted at a different location
compared to Step 2. The blocked ports change accordingly — different links are
placed in blocking state to create the new loop-free tree.

.. figure:: media/step3result.png
   :width: 80%
   :align: center

Comparing the two figures (Step 2 vs Step 3) clearly shows how the root bridge
location determines the shape of the spanning tree. Traffic from ``host2`` to
``host1`` still succeeds after convergence, but may follow a different path.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
