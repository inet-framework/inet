Connecting Multiple Active Sources to a Passive Sink
====================================================

The :ned:`PacketMultiplexer` module connects to multiple active packet sources on its
inputs, and pushes all incoming packets onto a passive packet sink on its single output.

In this example network, packets are produced at random intervals by several active
packet sources (:ned:`ActivePacketSource`). The packets are consumed by a single passive
packet sink (:ned:`PassivePacketSink`) upon arrival. The single sink is connected to
multiple sources using an intermediary component (:ned:`PacketMultiplexer`) which simply
forwards packets.

.. figure:: media/Multiplexer.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network MultiplexerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Multiplexer
   :end-at: productionInterval
   :language: ini
