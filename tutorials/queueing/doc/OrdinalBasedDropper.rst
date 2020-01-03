Ordinal-Based Dropper
=====================

This step demonstrates the :ned:`OrdinalBasedDropper` module. The module drops packets based
on the ordinal number of incoming packets.

In this step, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`).
The packet source pushes packets into a dropper (:ned:`OrdinalBasedDropper`).
Every second packet is dropped based on its ordinal number.
Packets that are not dropped are consumed by a passive packet sink (:ned:`PassivePacketSink`).

.. figure:: media/OrdinalBasedDropper.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network OrdinalBasedDropperTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config OrdinalBasedDropper
   :end-at: dropsVector
   :language: ini
