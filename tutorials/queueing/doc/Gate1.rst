Gate1
=====

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets pass through a packet gate if it's open,
otherwise packets are not generated. The packets are consumed by a passive
packet sink (PassivePacketSink).

The network contains ... TODO

.. figure:: media/Gate1.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Gate1.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Gate1
   :end-at: closeTime
   :language: ini
