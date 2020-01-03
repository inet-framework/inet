RED Dropper
===========

This step demonstrates the :ned:`RedDropper` module, which implements the Random Early Detection
algorithm.

Packets are created by an active packet source (:ned:`ActivePacketSource`), which
pushes packets to the dropper module. The dropper module drops packets with a probability depending
on the number of packets contained in the queue (:ned:`PacketQueue`) connected to its output.
The packets are collected by an active packet sink (:ned:`ActivePacketSink`).

.. figure:: media/RedDropper.png
   :width: 100%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network RedDropperTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config RedDropper
   :end-at: collectionInterval
   :language: ini
