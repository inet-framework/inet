Connecting a Passive Source to Multiple Active Sinks
====================================================

The :ned:`PacketDemultiplexer` module connects to a passive packet source
on its input and multiple active packet sinks on its outputs. When one of the collectors requests a packet
from the demultiplexer, it pops a packet from the provider and forwards it to the collector.

In this example network, packets are collected at random intervals by several active
packet sinks (:ned:`ActivePacketSink`). The packets are provided by a single passive
packet source (:ned:`PassivePacketSource`). The source is connected to
the sinks using a demultiplexer, which simply forwards packets.

.. figure:: media/Demultiplexer.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network DemultiplexerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Demultiplexer
   :end-at: collectionInterval
   :language: ini
