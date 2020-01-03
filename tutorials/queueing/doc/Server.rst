Time-Based Server
=================

The :ned:`PacketServer` module connects to passive packet sources and sinks.
The server actively pops packets from the connected packet provider and after
a configurable processing delay, it pushes them onto the connected packet consumer.

In this step, packets are passed through from the source to the sink periodically
(randomly) by the active packet processor (:ned:`PacketServer`). The packets are generated
by a passive packet source (:ned:`PassivePacketSource`) and consumed by a passive packet sink
(:ned:`PassivePacketSink`).

.. figure:: media/Server.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ServerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Server
   :end-at: processingTime
   :language: ini
