Passive Source - Active Sink
============================

This step demonstrates the :ned:`ActivePacketSink` and :ned:`PassivePacketSource` modules.
The active packet sink (:ned:`ActivePacketSink`) periodically pops packets from the
passive packet source (:ned:`PassivePacketSource`).

The active packet sink module has a configurable collection interval. The passive packet
source also has a configurable providing interval, which limits the times at which packets
can be popped from the module.

.. figure:: media/PassiveSourceActiveSink.png
   :width: 60%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ProviderCollectorTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PassiveSourceActiveSink
   :end-at: collectionInterval
   :language: ini
