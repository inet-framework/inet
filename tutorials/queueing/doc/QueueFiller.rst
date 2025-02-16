Preventing a Queue from Becoming Empty
======================================

The :ned:`QueueFiller` module creates packets whenever a connected queue becomes
empty. One use of this module is to generate continuous traffic on a network
interface.

In this example network, an active packet sink (:ned:`ActivePacketSink`) periodically pops packets from a queue (:ned:`PacketQueue`).
Whenever the queue becomes empty, a queue filler module (:ned:`QueueFiller`) pushes a packet into it.

.. figure:: media/QueueFillerNetwork.png
   :align: center
   :width: 100%

.. figure:: media/QueueFiller.png
   :align: center
   :width: 100%

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network QueueFillerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config QueueFiller
   :end-at: collectionInterval
   :language: ini
