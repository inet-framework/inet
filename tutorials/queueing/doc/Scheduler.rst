Scheduler
=========

In this test, packets are collected periodically by an active packet sink
(ActivePacketSink). The packets are produced by two passive packet sources
(PassivePacketSources). The single sink is connected to the two sources using a
scheduler (PacketScheduler). The scheduler forwards packets alternately from
one or the other source.

TODO

The network contains ... TODO

.. figure:: media/Scheduler.png
   :width: 80%
   :align: center

**TODO** Config

.. literalinclude:: ../Scheduler.ned
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Scheduler
   :end-at: schedulerClass
   :language: ini
