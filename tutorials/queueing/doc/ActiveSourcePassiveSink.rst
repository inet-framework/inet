Active Source - Passive Sink
============================

This step demonstrates the :ned:`ActivePacketSource` and :ned:`PassivePacketSink` modules.
The active packet source has a configurable packet production interval; the passive packet
sink has a configurable consumption interval, which limits when packets can be pushed into
the module.

In this example network, packets are produced periodically by the active packet source (:ned:`ActivePacketSource`)
and pushed into the passive packet sink (:ned:`PassivePacketSink`).

.. figure:: media/ActiveSourcePassiveSink.png
   :width: 50%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ProducerConsumerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config ActiveSourcePassiveSink
   :end-at: productionInterval
   :language: ini
