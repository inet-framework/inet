Requesting Protocol-Specific Behavior for Packets
=================================================

This step demonstrates the :ned:`PacketTagger` module, which attaches various protocol-specific request tags to packets.
Several of the request tags available in INET can be attached, such as the following:

- Request to send the packet via a particular interface
- Request to use a specific transmission power when transmitting the packet
- etc.

For the list of tags that can be attached, see the parameters of :ned:`PacketTagger`.

The packet tagger module has a filter class to filter which packets to attach a tag onto.
By default, packets are not filtered.

In this step, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The packets pass through a packet tagger which attaches
a HopLimitReq tag. The packets are consumed by a passive packet sink (:ned:`PassivePacketSink`).

.. figure:: media/Tagger.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network TaggerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Tagger
   :end-at: hopLimit
   :language: ini
