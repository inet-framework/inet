Demultiplexer
=============

In this test, packets are collected periodically (randomly) by several active
packet sinks (ActivePacketSinks). The packets are provided by a single passive
packet source upon request (PassivePacketSource). The single source is connected to
the multiple sinks using an intermediary component (PacketDemultiplexer) which
simply forwards packets.

The network contains ... TODO

.. figure:: media/Demultiplexer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Demultiplexer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Demultiplexer
   :end-at: collectionInterval
   :language: ini
