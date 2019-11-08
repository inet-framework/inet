Priority Classifier
===================

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by two active packet sinks
(ActivePacketSinks). The sinks are connected to FIFO queues (PacketQueue) with
limited capacity where packets are stored temporarily. The single source is
connected to the two queues using a classifier (PriorityClassifier). The
classifier forwards packets from the producer to the queues in a prioritized
way.

TODO

The network contains ... TODO

.. figure:: media/PriorityClassifier.png
   :width: 90%
   :align: center

**TODO** Config

.. literalinclude:: ../PriorityClassifier.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityClassifier
   :end-at: collectionInterval
   :language: ini
