Storing Packets on Behalf of Multiple Queues
============================================

The :ned:`PacketBuffer` module stores packets on
behalf of multiple queues, acting as a shared buffer for them.
Instead of each individual queue having a separate capacity,
the whole group of queues has a global, shared capacity, represented by the buffer.
The buffer keeps a record of which packets belong to which queue, and also maintains
the order of packets.

In this example network, packets are produced at random intervals by two active packet sources
(:ned:`ActivePacketSource`). Each packet source pushes packets into its associated queue (:ned:`PacketQueue`).
The two queues share a packet buffer module (:ned:`PacketBuffer`). The buffer is configured to have
a capacity of 2 packets. Packets are popped from the queues by two active packet sinks (:ned:`ActivePacketSink`).
Since the buffer has a limited packet capacity, and the ``PacketAtCollectionBeginDropper`` function is used
as the dropper function, the buffer drops packets from the beginning of
the buffer when it gets overloaded.

.. figure:: media/Buffer.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network BufferTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Buffer
   :end-at: packetCapacity
   :language: ini
