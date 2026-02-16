Step 3. Link Breakage and Routing Table Updates
===============================================

Introduction
------------

In the previous steps, we observed how RIP builds routing tables in a stable
network. However, one of the key advantages of dynamic routing protocols like
RIP is their ability to adapt to network changes. In this step, we'll explore
how RIP responds when a link in the network fails.

When a link fails, routers need to update their routing tables to reflect the
new network topology. This process involves detecting the failure, propagating
this information to other routers, and recalculating routes to affected
destinations. RIP handles this through its regular update mechanism, with some
additional features to improve convergence time.

Goals
-----

In this step, our goals are to:

1. Demonstrate how RIP updates routing tables when a link failure occurs
2. Observe the process of route invalidation and removal
3. Understand how routing information propagates through the network after a topology change
4. Introduce the concept of split horizon and its importance in preventing routing loops

Network Configuration
---------------------

We use the same network topology as in the previous step, but with two important changes:

1. We schedule a link break using the :ned:`ScenarioManager` module
2. We enable split horizon in the RIP configuration

The :ned:`ScenarioManager` allows us to simulate network events, such as link
failures, at specific times during the simulation. In this case, we'll break the
link connecting ``router2`` and ``switch2`` at t=50 seconds.

Split horizon is a technique used to prevent routing loops in distance-vector
protocols like RIP. With split horizon enabled, a router will not advertise
routes back to the neighbor from which it learned them. This helps prevent the
"count to infinity" problem, which we'll explore in more detail in Step 6.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

Note the key changes from Step 2:

* ``*.router*.rip.ripConfig``: Changed from "NoSplitHorizon" to "SplitHorizon" mode
* ``*.scenarioManager.script``: XML script that disconnects the link between ``router2`` and ``switch2`` at t=50s
* ``*.host0.numApps = 0``: Disabled the ping application to focus on routing updates

Understanding Link Failure Detection in RIP
-------------------------------------------

When a link fails, there are two ways RIP can detect and respond to the failure:

1. **Physical layer detection**: If the router can detect the physical link
   failure (as in our simulation), it can immediately invalidate routes that use
   that link.

2. **Timeout-based detection**: If a router stops receiving updates for a route
   from a neighbor, the timeout timer for that route will eventually expire
   (default 180 seconds), and the route will be marked as unreachable.

In our simulation, we're using physical layer detection, which allows for faster convergence after the link failure.

Experiment: Observing Route Updates After Link Failure
------------------------------------------------------

In the video below, observe what happens when the link connecting ``router2``
and ``switch2`` breaks at t=50 seconds. The video starts at approximately t=30
seconds, after the routing tables have stabilized from the initial RIP
convergence.

.. video:: media/step3_linkbreak.mp4
   :width: 100%

..   <!--internal video recording, normal run from 31s to 71+s, animation speed none, playback speed 2.138-->

When the link fails, the following sequence of events occurs:

1. ``router2`` detects the link failure and immediately invalidates routes that use that link
2. ``router2`` sends RIP updates to its neighbors, informing them of the route changes
3. The neighbors update their routing tables and propagate the information to their neighbors
4. Eventually, all routers update their routing tables to reflect the new network topology

Notice that some blue arrows (routes to ``host6``) disappear after the link
failure, as these routes are no longer valid. However, the network still
maintains connectivity to ``host6`` through alternative paths.

Understanding Route Persistence
-------------------------------

You may notice that the blue arrow from ``host6`` remains even after the link
failure. This is because it represents a static route, not a RIP-managed route.
RIP only controls routes between routers, while host routes are statically
configured by the :ned:`Ipv4NetworkConfigurator`.

The configurator sets up default routes for hosts to reach their local routers,
and these routes remain unchanged regardless of RIP updates. This is a common
practice in real networks as well, where end hosts typically have a single
default route to their gateway router, and it's the routers' responsibility to
find the best path to destinations.

The Role of Split Horizon
-------------------------

In this step, we enabled split horizon to ensure reliable convergence after the
link failure. Split horizon prevents a router from advertising routes back to
the neighbor from which it learned them. This is crucial when dealing with
network changes, as it helps prevent routing loops that could otherwise occur
during convergence.

For example, after the link between ``router2`` and ``switch2`` breaks:

1. Without split horizon, ``router1`` might advertise a route to ``host6`` back to ``router2``
2. ``router2`` might accept this route and start using it, creating a potential loop
3. With split horizon, ``router1`` will not advertise routes learned from ``router2`` back to ``router2``

We'll explore the importance of split horizon in more detail in Step 6 when we
discuss the "count to infinity" problem.

Conclusion and Next Steps
-------------------------

In this step, we've observed how RIP responds to a link failure by updating
routing tables and propagating this information through the network. We've seen
that RIP can adapt to topology changes, though the convergence process takes
some time as routing updates propagate through the network.

In the next step, we'll explore the role of RIP timers in managing route aging
and removal, which is another important aspect of how RIP handles network
changes.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1084>`__ in
the GitHub issue tracker for commenting on this tutorial.
