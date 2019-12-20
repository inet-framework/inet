Labeling Packets with Textual Tags
==================================

The :ned:`PacketLabeler` module attaches a textual label to incoming packets
based on a configured packet classifier function. Other modules such as
:ned:`LabelScheduler` and :ned:`LabelClassifier`, can use these labels as input
for the treatment of the packet. For example, :ned:`LabelClassifier` classifies
packets to one of its outputs, based on the presence of certain labels. Labels
allow the place of identifying the packet as belonging to some category and the
place of acting upon that classification information to be physically separate.

In this example network, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The single source is connected to a
classifier (:ned:`LabelClassifier`). The packets are consumed by two passive packet sinks
(:ned:`PassivePacketSink`). The classifier forwards packets alternately to
one or the other sink based on the packet's label. The label is attached by
a :ned:`PacketLabeler` based on the packet length.

In this example network, an active packet source (:ned:`ActivePacketSource`)
generates randomly either 1-byte or 2-byte packets. The packets are pushed into
a :ned:`PacketLabeler`, which attaches the ``small`` text label to 1-byte packets,
and the ``large`` text label to 2-byte packets. The labeler pushes packets into a
:ned:`LabelClassifier`, which classifies packets labeled ``small`` to
``consumer1``, and packets labeled ``large`` to ``consumer2``.

.. figure:: media/Labeler.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network LabelerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Labeler
   :end-at: labelsToGateIndices
   :language: ini
