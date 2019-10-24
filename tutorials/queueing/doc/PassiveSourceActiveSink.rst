Passive Source Active Sink
==========================

In this test, packets are collected periodically by an active packet sink
(ActivePacketSink). The packets are provided by a passive packet source
(PassivePacketSource).

The network contains ... TODO

.. figure:: media/PassiveSourceActiveSink.png
   :width: 60%
   :align: center

**TODO** Config

.. literalinclude:: ../PassiveSourceActiveSink.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PassiveSourceActiveSink
   :end-at: collectionInterval
   :language: ini
