Gate2
=====

In this test, packets are collected periodically by an active packet sink
(ActivePacketSink). The packets pass through a packet gate if it's open,
otherwise packets are not generated. The packets are provided by a passive
packet source (PassivePacketSource).

The network contains ... TODO

.. figure:: media/Gate2.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Gate2.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Gate2
   :end-at: closeTime
   :language: ini
