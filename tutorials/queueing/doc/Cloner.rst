Cloning Packets from One Input To Multiple Outputs
==================================================

.. The :ned:`PacketCloner` module creates copies of packets pushed into its input,
   and

The :ned:`PacketCloner` module has one input gate and multiple output gates.
Packets pushed to the input are copied and pushed out on all outputs.

In this example network, packets are created by an active packet source (:ned:`ActivePacketSource`).
The packet source pushes packets into the cloner (:ned:`PacketCloner`), which pushes a copy
of the packet into the two passive packet sources (:ned:`PassivePacketSource`) it is connected to.

.. figure:: media/Cloner.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network ClonerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Cloner
   :end-at: productionInterval
   :language: ini
