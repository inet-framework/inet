Priority Buffer
===============

In this test, packets are produced periodically (randomly) by two active packet
sources (ActivePacketSource). The packets are collected periodically (randomly)
by two active packet sinks (ActivePacketSink). The sources and the sinkes are
connected by packet queues (TestQueue) and packets are stored in shared packet
buffer (PacketBuffer). The packet buffer drops packets when it gets overloaded
prioritizing over the packet queues.

The network contains ... TODO

.. figure:: media/PriorityBuffer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../PriorityBuffer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityBuffer
   :end-at: packetCapacity
   :language: ini
