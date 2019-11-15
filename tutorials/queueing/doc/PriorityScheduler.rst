Priority Scheduler
==================

In this test, packets are collected periodically by an active packet sink
(ActivePacketSink). The packets are produced by two active packet sources
(ActivePacketSources). The sources are connected to FIFO queues (PacketQueue)
where packets are stored temporarily. The single sink is connected to the
queues using a scheduler (PriorityScheduler). The scheduler forwards packets
from the queues to the sink in a prioritized way.

TODO

The network contains ... TODO

.. figure:: media/PriorityScheduler.png
   :width: 90%
   :align: center

**TODO** Config

.. literalinclude:: ../PriorityScheduler.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityScheduler
   :end-at: collectionInterval
   :language: ini
