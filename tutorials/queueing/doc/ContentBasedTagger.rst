Requesting Protocol-Specific Behavior Based on Packet Data
==========================================================

The :ned:`ContentBasedTagger` module, similarly to :ned:`PacketTagger`, attaches protocol-specific
request tags to packets based on their content, according to the configured packet filter and packet data filter.

In this example network, an active packet source (:ned:`ActivePacketSource`) generates
1-byte and 2-byte packets randomly.
The packet source pushes packets into the tagger (:ned:`ContentBasedTagger`), which is configured
to attach a hop limit tag to 1-byte packets. Packets are consumed by a passive packet sink
(:ned:`PassivePacketSink`).

.. figure:: media/ContentBasedTagger.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ContentBasedTaggerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config ContentBasedTagger
   :end-at: hopLimit
   :language: ini
