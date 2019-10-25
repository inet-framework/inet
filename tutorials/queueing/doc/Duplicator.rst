Duplicator
==========

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The produced packets are randomly either duplicated or not
(PacketDuplicator). Finally, the packets are all sent into a passive packet
sink (PassivePacketSink).

The network contains ... TODO

.. figure:: media/Duplicator.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Duplicator.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Duplicator
   :end-at: numDuplicates
   :language: ini
