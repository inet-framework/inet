Limiting the Data Rate of a Packet Stream
=========================================

Rate meter modules (:ned:`ExponentialRateMeter` and :ned:`SlidingWindowRateMeter`) 
and rate limiter modules ():ned:`StatisticalRateLimiter`)
can be used to limit the data rate of a stream of packets.
The meter measures the data rate of the incoming stream of packets,
and attaches a rate tag to each packet. The limiter module, based on the rate tag,
limits the outgoing data rate to a configurable value.

In this step, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The packets are consumed by a passive packet sink
(:ned:`PassivePacketSink`). The packet rate is measured by a :ned:`ExponentialRateMeter`,
and if the rate of packets is higher than a predefined threshold, then packets are
dropped by the :ned:`StatisticalRateLimiter`.

.. figure:: media/Meter.png
   :width: 90%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network MeterTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Meter
   :end-at: maxPacketrate
   :language: ini
