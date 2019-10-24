Delayer
=======

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The produced packets are delayed (PacketDelayer) for a random
amount of time. Finally, the packets are sent into a passive packet sink (PassivePacketSink).

The network contains ... TODO

.. figure:: media/Delayer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Delayer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Delayer
   :end-at: uniform
   :language: ini
