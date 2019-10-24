Priority Queue
==============

In this test, packets are produced periodically (randomly) by an active packet
source (ActivePacketSource). The packets are collected periodically (randomly) by
an active packet sink (ActivePacketSink). The source and the sink is connected
by a priority queue with two inner queues (PriorityQueue) where packets are
stored temporarily.

The network contains ... TODO

.. figure:: media/PriorityQueue.png
   :width: 70%
   :align: center

.. figure:: media/PriorityQueue_Queue.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../PriorityQueue.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-at: collectionInterval
   :language: ini
