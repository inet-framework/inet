Building Complex Queues via Composition
=======================================

This step demonstrates a compound priority queue (ExampleCompoundPriorityQueue) built from queueing components.
The compound queue contains three packet queues. A classifier pushes packets to the first two queues and
a passive packet sink in a round-robin fashion.
An active packet source produces packets into the third queue. The three queues are
connected to a priority scheduler, which pops packets
from the first non-empty queue; the earlier queues have priority.

.. In this step, packets are produced at random intervals by an active packet
   source (:ned:`ActivePacketSource`). The packets are collected at random intervals by
   an active packet sink (ActivePacketSink). The source and the sink is connected
   by a compound priority queue (ExampleCompoundPriorityQueue) where packets are stored temporarily.
   This queue contains a classifier (:ned:`PacketClassifier`), two queues (:ned:`PacketQueue`),
   and a priorty scheduler (:ned:`PriorityScheduler`).

In this step, packets are produced at random intervals by an active packet
source (:ned:`ActivePacketSource`). The source is connected
to a compound priority queue (ExampleCompoundPriorityQueue) where packets are stored temporarily.
Packets are collected at random intervals by
an active packet sink (:ned:`ActivePacketSink`).

.. figure:: media/CompoundQueue.png
   :width: 70%
   :align: center

.. figure:: media/CompoundQueue_Queue.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network CompoundPacketQueueTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: module ExampleCompoundPriorityQueue
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config CompoundQueue
   :end-at: weights
   :language: ini
