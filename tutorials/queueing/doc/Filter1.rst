Filter 1
========

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by a passive packet sink (PassivePacketSink).
Packets are passed through from the source to the sink by a filter (PacketFilter).
Every second packet is dropped.

The network contains ... TODO

.. figure:: media/Filter1.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Filter1.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Filter1
   :end-at: filterClass
   :language: ini
