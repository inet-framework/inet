Buffer
======

In this test, packets are produced periodically (randomly) by an active packet
source (ActivePacketSource). The packets are collected periodically (randomly) by
an active packet sink (ActivePacketSink). The source and the sink is connected
by a compound priority queue (TestQueue) where packets are stored temporarily.
This queue contains a classifier (PacketClassifier), two queues (PacketQueues)
that use a limited shared buffer (PacketBuffer) to store packets, and a priorty
scheduler (PriortyScheduler).

The network contains ... TODO

.. figure:: media/Buffer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Buffer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Buffer
   :end-at: classifierClass
   :language: ini
