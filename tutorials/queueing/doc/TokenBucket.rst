Token Bucket
============

The :ned:`TokenBucket` compound module implements the token bucket algorithm.
By default, the module contains a :ned:`DropTailQueue` and a :ned:`TokenBasedServer`.
The queue capacity and the token consumption of the token-based server can be used to
parameterize the token bucket algorithm. The module uses an external token generator
for the token-based server.

In this example network, packets are produced periodically by an active packet source (:ned:`ActivePacketSource`).
The packets are pushed into a token bucket (:ned:`TokenBucket`), which pushes them
into a passive packet sink (:ned:`PassivePacketSink`). A token generator (:ned:`TimeBasedTokenGenerator`)
generates tokens periodically into the token bucket module.

.. figure:: media/TokenBucket.png
   :width: 60%
   :align: center

.. figure:: media/TokenBucket_Bucket.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network TokenBucketTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config TokenBucket
   :end-at: generationInterval
   :language: ini
