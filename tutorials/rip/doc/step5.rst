Step 5. Triggered Updates: Speeding Up Convergence
==================================================

Introduction
------------

In the previous steps, we've observed how RIP handles network changes through
periodic updates and timers. While this approach ensures eventual convergence,
it can be slow - potentially taking minutes for routing information to propagate
through a large network. This delay can significantly impact network performance
during topology changes.

To address this limitation, RIP includes a feature called *triggered updates*
that can dramatically speed up convergence after network changes. In this step,
we'll explore how triggered updates work and observe their impact on convergence
time.

Goals
-----

In this step, our goals are to:

1. Understand the concept of triggered updates and how they differ from periodic updates
2. Observe how triggered updates accelerate the propagation of routing information after a topology change
3. Compare convergence time with and without triggered updates
4. Understand the benefits and potential drawbacks of triggered updates

Understanding Triggered Updates
-------------------------------

In standard RIP operation, routers exchange routing updates on a fixed schedule

- typically every 30 seconds. This means that when a topology change occurs,
  there can be a significant delay before other routers learn about it:

1. On average, neighbors will wait 15 seconds (half the update interval) before receiving the next scheduled update
2. Each subsequent "hop" of routers will add another update interval to the convergence time
3. In a network with many hops, full convergence could take several minutes

Triggered updates provide a solution to this problem:

* When a router detects a link failure or receives an update that causes it to change its routing table
* It immediately sends an update to its neighbors, without waiting for the next scheduled update
* When neighbors receive this update and change their routing tables, they also send immediate updates
* This creates a cascade of updates that propagates through the network much faster than periodic updates

This mechanism significantly reduces convergence time, especially in larger networks with many hops between routers.

Network Configuration
---------------------

We use the same network topology as in the previous steps, but with one critical
change: we enable triggered updates in the RIP configuration.

The configuration in ``omnetpp.ini`` is shown below:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5
   :end-before: ------

Note the key change from previous steps:

* ``*.router*.rip.triggeredUpdate = true``: Enables triggered updates on all routers

The configuration extends Step 3, which means it inherits the split horizon
configuration and the scenario that breaks the link between ``router2`` and
``switch2`` at t=50s.

Experiment: Observing Accelerated Convergence
---------------------------------------------

In this experiment, we'll observe how triggered updates affect the speed of
convergence after a link failure. We'll break the link between ``router2`` and
``switch2`` at t=50s and observe how quickly routing information propagates
through the network.

.. video:: media/step5.mp4
   :width: 100%

..   <!--internal video recording, zoom 0.77, animation speed none, playback speed 2.138, from 30s to 60s-->

When the link breaks at t=50s, the following sequence of events occurs:

1. ``router2`` immediately detects the link failure and updates its routing table
2. Because triggered updates are enabled, ``router2`` immediately sends updates to its neighbors (around t=52s)
3. When neighbors receive these updates and change their routing tables, they also send immediate updates
4. This cascade of updates propagates through the network much faster than with periodic updates

Without triggered updates (as in Step 3), routers would have to wait until their
next scheduled update (around t=60s) to inform their neighbors about the change.
With triggered updates, the convergence process begins immediately after the
failure is detected.

Benefits and Considerations of Triggered Updates
------------------------------------------------

Triggered updates offer several benefits:

1. **Faster Convergence**: Network adapts to changes much more quickly
2. **Reduced Downtime**: Less time with incorrect or incomplete routing information
3. **Improved Reliability**: Critical routing changes propagate immediately

However, there are also some considerations:

1. **Update Storms**: In unstable networks with frequent changes, triggered updates could lead to a flood of updates
2. **Processing Overhead**: More frequent updates require more router CPU resources
3. **Bandwidth Usage**: Additional updates consume more network bandwidth

To mitigate these concerns, RIP implementations typically include rate-limiting
mechanisms that prevent a router from sending triggered updates too frequently.
This balances the benefits of fast convergence with the need to prevent
excessive update traffic.

Triggered Updates and the Count-to-Infinity Problem
---------------------------------------------------

While triggered updates speed up convergence, they don't solve all of RIP's
limitations. In particular, they don't address the "count-to-infinity" problem
that can occur in certain network topologies with routing loops.

In fact, triggered updates can sometimes make the count-to-infinity problem
worse by accelerating the rate at which incorrect routing information
propagates. We'll explore this issue in more detail in the next steps, where
we'll examine the count-to-infinity problem and solutions like split horizon and
hold-down timers.

Conclusion and Next Steps
-------------------------

In this step, we've seen how triggered updates can significantly speed up
convergence after network changes. By sending immediate updates when routing
tables change, rather than waiting for the next scheduled update, RIP can adapt
to topology changes much more quickly.

While triggered updates improve RIP's responsiveness, the protocol still has
limitations when dealing with certain network topologies. In the next step,
we'll explore one of RIP's most significant challenges: the count-to-infinity
problem that can occur in networks with routing loops.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
