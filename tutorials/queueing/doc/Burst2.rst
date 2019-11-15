Burst 2
=======

In this test, packets are periodically (randomly) collected by two active sinks
(ActivePacketSinks). One sink collects packets with a slower rate while the other
sink uses a faster rate. The two packet sinks are combined using a markov
chain with random transition matrix and random wait intervals. The packets are
provided by a single passive source (PassivePacketSource).

The network contains ... TODO

.. figure:: media/Burst2.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Burst2.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Burst2
   :end-at: waitIntervals
   :language: ini
