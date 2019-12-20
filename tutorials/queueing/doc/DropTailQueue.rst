Dropping Packets from a Finite Queue
====================================

The :ned:`DropTailQueue` module extends :ned:`PacketQueue` by specifying a packet capacity,
and setting a packet dropper function. By default, the packet capacity is set to 100,
and packets are dropped from the end of the queue.

In this example network, packets are created at random intervals by an active packet source (:ned:`ActivePacketSource`)
The packets are pushed into a drop-tail queue (:ned:`DropTailQueue`) with a capacity of 4 packets.
The packets are popped from the queue at random intervals by an active packet sink (:ned:`ActivePacketSink`).

.. figure:: media/DropTailQueue.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network DropTailQueueTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config DropTailQueue
   :end-at: collectionInterval
   :language: ini
