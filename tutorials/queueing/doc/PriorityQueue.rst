Priority Queue
==============

The :ned:`PriorityQueue` module is a compound module that implements priority queueing
with the help of a classifier submodule, multiple queues and a scheduler.

It contains a configurable number of :ned:`PacketQueue`'s. A :ned:`PacketClassifier` module
classifies packets into the queues according to the configured packet classifier function.
A :ned:`PriorityScheduler` pops packets from the first non-empty queue, thus earlier queues
have a priority over the later ones.

In this example network, packets are produced at random intervals by an active packet
source (:ned:`ActivePacketSource`). The source is connected
to a priority queue (:ned:`PriorityQueue`) with two inner queues (:ned:`PacketQueue`).
The packets are collected at random intervals by an active packet sink (:ned:`ActivePacketSink`).

.. figure:: media/PriorityQueue.png
   :width: 70%
   :align: center

.. figure:: media/PriorityQueue_Queue.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network PriorityQueueTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-at: collectionInterval
   :language: ini
