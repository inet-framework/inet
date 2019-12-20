Duplicating Packets Based On Their Ordinal Number
=================================================

The :ned:`OrdinalBasedDuplicator` module duplicates packets based on the ordinal
number of incoming packets. The packets to be duplicated can be configured by
listing their ordinal numbers in a parameter. When the packets to be duplicated
need to be selected using more complex criteria, a duplicator module can be combined
with a classifier and a multiplexer to achieve that.

In this example network, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`).
The packet source pushed packets into a duplicator (:ned:`OrdinalBasedDuplicator`).
The duplicator is configured to duplicate every second packet on its ordinal number.
The packets are then consumed by a passive packet sink (:ned:`PassivePacketSink`).

.. figure:: media/OrdinalBasedDuplicator.png
   :width: 60%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network OrdinalBasedDuplicatorTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config OrdinalBasedDuplicator
   :end-at: duplicatesVector
   :language: ini
