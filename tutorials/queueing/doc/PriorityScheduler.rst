Priority Scheduler
==================

This step demonstrates the :ned:`PriorityScheduler` module. The module pops packets from the first
connected non-empty queue or packet provider.

In this example network, two active packet sources (:ned:`ActivePacketSource`) generate packets
in random periods. The packets are pushed into queues (:ned:`PacketQueue`), where they are
stored temporarily. The queues are connected to a priority scheduler (:ned:`PriorityScheduler`).
An active packet sink (:ned:`ActivePacketSink`) pops packets from the scheduler, which in turn
pops packets from one of the queues in a prioritized way, favoring the first queue.

.. figure:: media/PriorityScheduler.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network PrioritySchedulerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityScheduler
   :end-at: collectionInterval
   :language: ini
