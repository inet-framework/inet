Content-Based Filtering (Active Source)
=======================================

The :ned:`ContentBasedFilter` module filters packets according to the configured packet filter
and packet data filter. Packets that match the filter expressions are pushed/popped on the output;
non-maching packets are dropped.

In this example network, an active packet source (:ned:`ActivePacketSource`) generates
1-byte and 2-byte packets randomly.
The source pushes packets into a filter (:ned:`ContentBasedFilter`), which
pushes 1-byte packets into a passive packet sink (:ned:`PassivePacketSink`) and drops 2-byte ones.

.. figure:: media/Filter1.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network Filter1TutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Filter1
   :end-at: packetFilter
   :language: ini
