Queue
=====

In this test, packets are produced periodically (randomly) by an active packet
source (ActivePacketSource). The packets are collected periodically (randomly) by
an active packet sink (ActivePacketSink). The source and the sink is connected
by a FIFO queue (PacketQueue) where packets are stored temporarily.

TODO

The network contains ... TODO

.. figure:: media/PacketQueue.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Queue.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PacketQueue
   :end-at: collectionInterval
   :language: ini
