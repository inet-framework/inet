Weighted Round-Robin Scheduler
==============================

The Weighted Round-Robin Scheduler (:ned:`WrrScheduler`) module connects multiple passive packet
sources to a single active packet sink. When the collector requests a packet, the scheduler pops
a packet from one of the connected packet sources.

The :ned:`WrrScheduler` schedules packets from one of its connected packet sources in a round-robin
fashion, based on the configured weights of the inputs. For example, if the configured weights
are [2,3], then the first two packets are popped from input 0, the next three from input 1.

In this example network, packets are created by two passive packet sources (:ned:`PassivePacketSource`).
The sources are connected to a :ned:`WrrScheduler`. The scheduler is connected to an active packet
sink (:ned:`ActivePacketSink`), which pops packets from the scheduler periodically. The inputs of the
scheduler are configured to have the weights [1 3 2], thus the scheduler pops 1, 3 and 2 packet(s)
from the three providers in one round.

.. figure:: media/Scheduler.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network WrrSchedulerTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config WrrScheduler
   :end-at: weights
   :language: ini
