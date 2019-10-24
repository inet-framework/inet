Burst 1
=======

In this test, packets are periodically (randomly) produced by two active sources
(ActivePacketSources). One source produces packets with a slower rate while the other
source uses a faster rate. The two packet sources are combined using a markov
chain with random transition matrix and random wait intervals. The packets are
consumed by a single passive sink (PassivePacketSink).

The network contains ... TODO

.. figure:: media/Burst1.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Burst1.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Burst1
   :end-at: waitIntervals
   :language: ini
