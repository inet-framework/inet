Weighted Round-Robin Classifier
===============================

The Weighted Round-Robin Classifier (:ned:`WrrClassifier`) module classifies packets
to its outputs in a round-robin fashion, based on the configured weights of the outputs.
For example, if the configured weights are [2,3], then first two packets are sent to output 0,
the next three to output 1. If a component connected to an output is busy (the output is
not *pushable*), the scheduler waits until it becomes available.

In this step, packets are produced periodically by an active packet source
(:ned:`ActivePacketSource`). The single source is connected to a
classifier (:ned:`WrrClassifier`), which is connected to three passive packet sinks
(:ned:`PassivePacketSink`). The outputs of the classifier are configured to have
the weights [1,3,2], thus the classifier forwards 1, 3 and 2 packet(s)
to the three sinks in one round.

.. figure:: media/WrrClassifier.png
   :width: 80%
   :align: center

.. literalinclude:: ../QueueingTutorial.ned
   :start-at: network WrrClassifierTutorialStep
   :end-before: //----
   :language: ned

.. literalinclude:: ../omnetpp.ini
   :start-at: Config WrrClassifier
   :end-at: weights
   :language: ini
