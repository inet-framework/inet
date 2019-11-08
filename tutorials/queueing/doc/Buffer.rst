Buffer
======

In this test, packets are produced periodically (randomly) by two active packet
sources (ActivePacketSource). The packets are collected periodically (randomly)
by two active packet sinks (ActivePacketSink). The sources and the sinkes are
connected by packet queues (TestQueue) and packets are stored in shared packet
buffer (PacketBuffer). The packet buffer drops packets from the beginning of
the buffer when it gets overloaded.

The network contains ... TODO

.. figure:: media/Buffer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Buffer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Buffer
   :end-at: packetCapacity
   :language: ini
