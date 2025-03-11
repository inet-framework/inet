Content-Based Filtering (Active Sink)
=====================================

This step is very similar to the previous one, but packets are popped from the filter by an
active packet sink (:ned:`ActivePacketSink`), and provided by a passive packet source
(:ned:`PassivePacketSource`).

.. In this step, packets are collected periodically by an active packet sink
   (:ned:`ActivePacketSink`). The packets are provided by a passive packet source
   (:ned:`PassivePacketSource`). Packets are passed through from the source to the sink by
   a filter (:ned:`ContentBasedFilter`). Every second packet is dropped.

.. figure:: media/Filter2.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network Filter2TutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Filter2
   :end-at: packetFilter
   :language: ini
