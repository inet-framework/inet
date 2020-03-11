Duplicating Packets from One Input to One Output
================================================

.. This step demonstrates the :ned:`PacketDuplicator` module, which creates a configured number of duplicates
   of incoming packets.

The :ned:`PacketDuplicator` module creates a configured number of duplicates
of incoming packets.

.. In this step, packets are produced periodically by an active packet source
   (:ned:`ActivePacketSource`). The produced packets are randomly either duplicated or not
   (:ned:`PacketDuplicator`). Finally, the packets are all sent into a passive packet
   sink (:ned:`PassivePacketSink`).

In this example network, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The packets are pushed into a
(:ned:`PacketDuplicator`), which creates 0 or 1 copy of them randomly.
Finally, the packets are all sent into a passive packet
sink (:ned:`PassivePacketSink`).

.. figure:: media/Duplicator.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network DuplicatorTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Duplicator
   :end-at: numDuplicates
   :language: ini
