Filter 2
========

In this test, packets are collected periodically by an active packet sink
(ActivePacketSink). The packets are provided by a passive packet source
(PassivePacketSource). Packets are passed through from the source to the sink by
a filter (PacketFilter). Every second packet is dropped.

The network contains ... TODO

.. figure:: media/Filter2.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Filter2.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Filter2
   :end-at: filterClass
   :language: ini
