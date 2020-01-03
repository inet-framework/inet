Priority Buffer
===============

The :ned:`PriorityBuffer` module extends :ned:`PacketBuffer` by configuring
a packet drop strategy. By default, when the buffer is full, packets belonging to the last
queue are dropped first. This is the behavior that can be changed by the packet drop strategy.

In this example network, packets are produced at random intervals by two active packet
sources (:ned:`ActivePacketSource`). The packet packet sources push packets into two
queues (:ned:`PacketQueue`). The queues share a :ned:`PriorityBuffer` with a packet capacity of
2. Packets are popped from the queues at random intervals by two active packet sinks (:ned:`ActivePacketSink`).
When the buffer becomes full, it drops packets from the second queue first.

.. figure:: media/PriorityBuffer.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network PriorityBufferTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityBuffer
   :end-at: packetCapacity
   :language: ini
