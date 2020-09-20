Step 7. Counting to Infinity (Loop Instability with Higher Number of Nodes)
===========================================================================

Goals
-----

TODO: the same as in the previous step, but with more routers

The model
---------

TODO: description of the problem and the difference from the previous
step

This step uses the following network:

.. literalinclude:: ../RipNetworkC.ned
   :language: ned
   :start-at: RipNetwork

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7
   :end-before: ------

**Split Horizon:**

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7SplitHorizon
   :end-before: ------

**Triggered Update:**

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7TriggeredUpdates
   :end-before: ------

Results
-------

TODO: video

RIP update messages are sent every 30 seconds (not visualized here). The
route metrics increase incrementally (count-to-infinity):

.. video:: media/step7.mp4
   :width: 100%

..   <!--internal video recording, animation speed none, playback speed 2.138, zoom 0.77-->

TODO: what happens

TODO: screenshots of routing tables / RIP packets

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkC.ned <../RipNetworkC.ned>`
