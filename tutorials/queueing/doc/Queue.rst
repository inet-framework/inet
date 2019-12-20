Enqueueing Packets
==================

This step demonstrates the :ned:`PacketQueue` module, which can store a configurable
number of packets. By default, it works as a FIFO queue and has unlimited capacity.
Optionally, the queue can be limited (both by number of packets and byte count),
and ordering can be specified.

In the following example network, packets are produced at random intervals by an active packet
source (:ned:`ActivePacketSource`). The packets are pushed into an unlimited FIFO queue (:ned:`PacketQueue`),
where they are stored temporarily. An active packet sink (:ned:`ActivePacketSink`) module
pops packets from the queue at random intervals.

.. figure:: media/PacketQueue.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network PacketQueueTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PacketQueue
   :end-at: collectionInterval
   :language: ini
