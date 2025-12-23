Step 8. Hold-Down Timer: Preventing Routing Loops
=================================================

Introduction
------------

In the previous steps, we've explored several mechanisms for addressing the
count-to-infinity problem in RIP networks, including split horizon, poison
reverse, and triggered updates. While these techniques help reduce the
likelihood and impact of routing loops, they don't completely solve the problem,
especially in complex network topologies.

In this step, we'll explore another important mechanism for preventing routing
loops: the hold-down timer. This timer provides an additional layer of protection
against routing loops by preventing routers from accepting potentially incorrect
routing information during network convergence.

.. note:: The hold-down timer is **not defined in RFC 2453** (the RIPv2 standard).
   It is a **vendor extension**, commonly associated with Cisco implementations,
   that has been widely adopted to improve RIP stability. INET also provides this
   feature.

Goals
-----

In this step, our goals are to:

1. Understand the concept of hold-down timers and how they work
2. Observe how hold-down timers prevent routing loops in complex networks
3. Analyze the trade-offs involved in using hold-down timers
4. Understand how hold-down timers complement other mechanisms like split horizon

Understanding Hold-Down Timers
------------------------------

A hold-down timer is a mechanism that prevents a router from accepting updates
for a route that has recently been marked as unreachable. When a router learns
that a route has become unreachable (either through a direct link failure or
from a neighbor's update), it:

1. Marks the route as unreachable (sets the metric to 16)
2. Starts a hold-down timer for that route (typically 180 seconds)
3. During the hold-down period, the router will not accept any updates for that route, even if they advertise a valid metric
4. After the hold-down timer expires, the router will accept updates for the route again

The purpose of the hold-down timer is to give the network time to converge to a
stable state after a topology change. By ignoring potentially outdated or
incorrect routing information during this period, routers can avoid the
formation of routing loops.

Network Configuration
---------------------

This step uses the same complex network topology as the previous step, with four
routers connected in a mesh topology. The key difference is that we now enable
the hold-down timer in the RIP configuration.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step8
   :end-before: ------

Key aspects of this configuration:

* ``*.router*.rip.holdDownTime = 30s``: Enables the hold-down timer with a value
  of 30 seconds (reduced from the default 180 seconds to make the simulation
  faster)
* Split horizon is not enabled, allowing us to focus on the effect of the
  hold-down timer alone

Experiment: Hold-Down Timer in Action
-------------------------------------

In this experiment, we observe how the hold-down timer prevents routing loops
after a link failure. When a link fails, the following sequence of events
occurs:

1. The router directly connected to the failed link marks routes through that link as unreachable (metric 16)
2. It starts the hold-down timer for these routes
3. It propagates this information to its neighbors
4. When neighbors receive this information, they also mark the routes as unreachable and start their own hold-down timers
5. During the hold-down period, routers ignore any updates for these routes, even if they advertise valid metrics
6. This prevents the formation of routing loops that would otherwise occur if routers accepted potentially incorrect routing information

.. video:: media/step8.mp4
   :width: 100%

Let's examine a specific example from the simulation:

1. The 10.0.0.24/29 network becomes unreachable, and the route in ``router1`` is set to metric 16
2. This information is propagated to ``router3``, which also sets the route to 10.0.0.24/29 to metric 16 and starts its hold-down timer
3. Meanwhile, ``router2`` still has a route to 10.0.0.24/29 with a metric of 3 (which is actually incorrect)
4. ``router2`` sends a RIP Response message to ``router3`` containing this route
5. However, ``router3`` does not update its routing table with this information because the route is in hold-down
6. This prevents the formation of a routing loop that would otherwise occur if ``router3`` accepted the incorrect route from ``router2``

The following image shows this scenario in action:

.. figure:: media/step8_1.png
   :width: 100%
   :align: center

In this image, you can see:

* ``router2`` sending a RIP Response message to ``router3`` with a route to 10.0.0.24/29 (metric 3)
* ``router3``'s routing table showing the route to 10.0.0.24/29 with a metric of 16 (unreachable)
* The route is in hold-down, preventing ``router3`` from accepting the update from ``router2``

By ignoring the potentially incorrect routing information during the hold-down period, ``router3`` helps prevent the formation of routing loops.

Benefits and Trade-offs of Hold-Down Timers
-------------------------------------------

Hold-down timers offer several benefits for RIP networks:

1. **Preventing Routing Loops**: By ignoring potentially incorrect routing
   information during convergence, hold-down timers help prevent the formation
   of routing loops
2. **Complementing Other Mechanisms**: Hold-down timers work well in conjunction
   with split horizon and poison reverse, providing an additional layer of
   protection
3. **Effective in Complex Topologies**: Unlike split horizon, hold-down timers
   can help prevent routing loops in complex topologies with multiple potential
   loops

However, there are also trade-offs to consider:

1. **Delayed Convergence**: Hold-down timers can delay network convergence, as routers must wait for the timer to expire before accepting new routes
2. **Temporary Unreachability**: During the hold-down period, some networks may be temporarily unreachable, even if valid alternate paths exist
3. **Configuration Complexity**: The optimal hold-down timer value depends on network size and topology, requiring careful configuration

Despite these trade-offs, hold-down timers are widely used in RIP
implementations as they provide an effective mechanism for preventing routing
loops, especially in complex networks.

Combining Solutions for Optimal RIP Performance
-----------------------------------------------

For optimal RIP performance and stability, network administrators typically combine multiple mechanisms:

1. **Split Horizon**: Prevents routes from being advertised back to the neighbor from which they were learned
2. **Poison Reverse**: Explicitly advertises unreachable routes with a metric of 16
3. **Triggered Updates**: Speeds up convergence by sending immediate updates when routing tables change
4. **Hold-Down Timers**: Prevents the acceptance of potentially incorrect routing information during convergence

By combining these mechanisms, RIP networks can achieve faster convergence while minimizing the risk of routing loops.

Conclusion and Next Steps
-------------------------

In this step, we've explored how hold-down timers help prevent routing loops in
RIP networks by ignoring potentially incorrect routing information during
network convergence. We've seen that while hold-down timers introduce some delay
in convergence, they provide an effective mechanism for preventing routing
loops, especially in complex network topologies.

In the next step, we'll measure RIP recovery time under different configurations
to understand the performance implications of various RIP features and
mechanisms.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkC.ned <../RipNetworkC.ned>`
