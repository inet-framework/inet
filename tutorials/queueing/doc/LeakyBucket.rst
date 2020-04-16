Leaky Bucket
============

The :ned:`LeakyBucket` compound module implements the leaky bucket algorithm.
By default, the module contains a :ned:`DropTailQueue`, and a :ned:`PacketServer`.
The queue capacity and the processing time of the server can be used to
parameterize the leaky bucket algorithm.

In this example network, packets are produced by an active packet source (:ned:`ActivePacketSource`).
The packet source pushes packets into a :ned:`LeakyBucket` module, which pushes them into
a passive packet sink (:ned:`PassivePacketSink`). The leaky bucket is configured with a
processing time of 1s, and a packet capacity of 1.

.. figure:: media/LeakyBucket.png
   :width: 60%
   :align: center

.. figure:: media/LeakyBucket_Bucket.png
   :width: 65%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network LeakyBucketTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config LeakyBucket
   :end-at: processingTime
   :language: ini
