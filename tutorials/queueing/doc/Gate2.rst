Blocking/Unblocking Packet Flow (Active Sink)
=============================================

This step is similar to the previous one, but with an active packet sink and passive packet source.
Packets are collected periodically by an active packet sink
(:ned:`ActivePacketSink`). The packets pass through a packet gate if it is open,
otherwise packets are not generated. The packets are provided by a passive
packet source (:ned:`PassivePacketSource`).

.. figure:: media/Gate2.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network Gate2TutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Gate2
   :end-at: closeTime
   :language: ini
