Filter 3
========

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by a passive packet sink (PassivePacketSink).
Packets are passed through from the source to the sink by a packet data based
filter (ContentBasedFilter). Every packet longer than 1B is dropped.

The network contains ... TODO

.. figure:: media/Filter3.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Filter3.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Filter3
   :end-at: packetFilter
   :language: ini
