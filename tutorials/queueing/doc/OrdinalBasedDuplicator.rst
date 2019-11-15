Ordinal Based Duplicator
========================

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by a passive packet sink (PassivePacketSink).
Packets are passed through from the source to the sink by a duplicator (OrdinalBasedDuplicator).
Every second packet is duplicated based on its ordinal number.

The network contains ... TODO

.. figure:: media/OrdinalBasedDuplicator.png
   :width: 60%
   :align: center

**TODO** Config

.. literalinclude:: ../OrdinalBasedDuplicator.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config OrdinalBasedDuplicator
   :end-at: duplicatesVector
   :language: ini
