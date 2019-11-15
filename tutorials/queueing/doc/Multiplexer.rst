Multiplexer
===========

In this test, packets are produced periodically (randomly) by several active
packet sources (ActivePacketSources). The packets are consumed by a single passive
packet sink upon arrival (PassivePacketSink). The single sink is connected to the
multiple sources using an intermediary component (PacketMultiplexer) which simply
forwards packets.

The network contains ... TODO

.. figure:: media/Multiplexer.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Multiplexer.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Multiplexer
   :end-at: productionInterval
   :language: ini
