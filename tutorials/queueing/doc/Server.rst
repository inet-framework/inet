Server
======

In this test, packets are passed through from the source to the sink periodically
(randomly) by an active packet processor (PacketServer). The packets are generated
by a passive packet source (PassivePacketSource) and consumed by a passive packet sink
(PassivePacketSink).

TODO

The network contains ... TODO

.. figure:: media/Server.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Server.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Server
   :end-at: processingTime
   :language: ini
