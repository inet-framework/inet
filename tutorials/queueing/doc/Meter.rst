Meter
=====

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by a passive packet sink
(PassivePacketSink). The packet rate is measured and if the rate of packets
is higher than a predefined threshold, then packets are dropped.

The network contains ... TODO

.. figure:: media/Meter.png
   :width: 90%
   :align: center

**TODO** Config

.. literalinclude:: ../Meter.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Meter
   :end-at: maxPacketrate
   :language: ini
