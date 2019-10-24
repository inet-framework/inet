Labeler
=======

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by two passive packet sinks
(PassivePacketSinks). The single source is connected to the two sinks using a
classifier (MarkerClassifier). The classifier forwards packets alternately to
one or the other sink based on the packet's label. The label is attached by
a PacketLabelere based on the packet length.

The network contains ... TODO

.. figure:: media/Labeler.png
   :width: 90%
   :align: center

**TODO** Config

.. literalinclude:: ../Labeler.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Labeler
   :end-at: labelsToGateIndices
   :language: ini
