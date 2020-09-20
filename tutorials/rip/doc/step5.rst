Step 5. Triggered Updates
=========================

Goals
-----

We will demonstrate the benefits of triggered updates on speeding up the routing
table convergence after a router detects a topology change.

In distance-vector routing algorithm, routers periodically (by default every 30
seconds) send update messages to their neighbours (i.e. other routers directly
attached to them). This means that if a topology change occurs, the other router
will not become aware of this on average for 15 seconds. The propagation of
topology changes from router to router can be significantly delayed and may
impact on the network performance.

To alleviate the effects of this problem, a
second mechanism called *triggered updates* is used. A triggered update happens
whenever a node notices a link failure or receives an update from one of its
neighbors that causes it to change one of the routes in its routing table.
Whenever a node's routing table changes, it immediately sends an update to its
neighbors, which may lead to a change in their tables, causing them to send an
update to their neighbors quickly.

Network Configuration
---------------------

Our network configuration is the same as the one we used in the earlier steps.

The configuration in ``omnetpp.ini`` is shown below:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5
   :end-before: ------

Experiments
-----------

We break the link between ``router2`` and ``switch2`` at t = 50s. (The scenario
step doing that is inherited from Step 3, see the ``extends = Step3`` line under
``[Config Step5]``.) Observe that, since triggered updates are enabled,
``router2`` notifies its directly attached neighbours about the link change
immediately after it breaks (around t = 52s), not after the next scheduled
update at t = 60s.

.. video:: media/step5.mp4
   :width: 100%

..   <!--internal video recording, zoom 0.77, animation speed none, playback speed 2.138, from 30s to 60s-->

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
