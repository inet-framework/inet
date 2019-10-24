Classifier
==========

In this test, packets are produced periodically by an active packet source
(ActivePacketSource). The packets are consumed by two passive packet sinks
(PassivePacketSinks). The single source is connected to the two sinks using a
classifier (PacketClassifier). The classifier forwards packets alternately to
one or the other sink.

The network contains ... TODO

.. figure:: media/Classifier.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Classifier.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Classifier
   :end-at: classifierClass
   :language: ini
