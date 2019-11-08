Compound Queue
==============

In this test, packets are produced periodically (randomly) by an active packet
source (ActivePacketSource). The packets are collected periodically (randomly) by
an active packet sink (ActivePacketSink). The source and the sink is connected
by a compound priority queue (TestQueue) where packets are stored temporarily.
This queue contains a classifier (PacketClassifier), two queues (PacketQueues),
and a priorty scheduler (PriortyScheduler).

The network contains ... TODO

.. figure:: media/CompoundQueue.png
   :width: 70%
   :align: center

.. figure:: media/CompoundQueue_Queue.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../CompoundQueue.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config CompoundQueue
   :end-at: classifierClass
   :language: ini
