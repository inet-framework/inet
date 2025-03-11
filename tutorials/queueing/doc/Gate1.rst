Blocking/Unblocking Packet Flow (Active Source)
===============================================

The :ned:`PacketGate` module allows packets to pass through when it is open,
and blocks them when it is closed. The gate opens and closes according to the configured schedule.
There are multiple ways to specify the schedule: simple open/close time parameters,
a script parameter, or programmatically (via C++).

In this example network, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The packets pass through a packet gate if it is open,
otherwise packets are not generated. The packets are consumed by a passive
packet sink (:ned:`PassivePacketSink`). The time of opening and closing the gate is
configured via parameters.

.. figure:: media/Gate1.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network Gate1TutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Gate1
   :end-at: closeTime
   :language: ini
