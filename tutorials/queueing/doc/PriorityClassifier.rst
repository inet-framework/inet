Priority Classifier
===================

The :ned:`PriorityClassifier` module pushes packets to the first non-full connected queue.

In this step, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The packet source is connected to a priority classifier (:ned:`PriorityClassifier`),
which connects to two queues (:ned:`PacketQueue`). The queues are configured to have capacity
for storing one packet. The queues are connected to two active packet sinks (:ned:`ActivePacketSink`).
The classifier sends packets to the queues, favoring ``queue1``. It will only consider
``queue2`` when ``queue1`` is full.
The packets are popped from the queues by two active packet sinks (:ned:`ActivePacketSink`).

.. figure:: media/PriorityClassifier.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network PriorityClassifierTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityClassifier
   :end-at: collectionInterval
   :language: ini
