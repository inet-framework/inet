Tagger
======

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets pass through a packet tagger which attaches
a HopLimitReq tag. The packets are consumed by a passive packet sink (PassivePacketSink).

The network contains ... TODO

.. figure:: media/Tagger.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Tagger.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Tagger
   :end-at: hopLimit
   :language: ini
