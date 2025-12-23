Step 7. Count to Infinity in Complex Networks
=============================================

Introduction
------------

In the previous step, we explored the count-to-infinity problem in a simple
two-router network and saw how solutions like split horizon can effectively
prevent routing loops in such topologies. However, real networks often have more
complex topologies with multiple routers forming potential loops.

In this step, we'll examine how the count-to-infinity problem manifests in
networks with more than two routers, and why solutions like split horizon and
poison reverse, while helpful, don't completely solve the problem in these more
complex topologies.

Goals
-----

In this step, our goals are to:

1. Understand how the count-to-infinity problem occurs in networks with more than two routers
2. Observe why split horizon alone cannot completely solve the problem in complex topologies
3. Evaluate the effectiveness of different solutions, including split horizon and triggered updates
4. Understand the implications for real-world RIP deployments

Understanding Count-to-Infinity in Complex Networks
---------------------------------------------------

In networks with more than two routers, the count-to-infinity problem becomes
more challenging to solve. This is because split horizon only prevents a router
from advertising routes back to the neighbor from which it learned them. It
doesn't prevent routing loops that involve three or more routers.

Consider a simple scenario with three routers (A, B, and C) in a triangle
topology:

1. Router A has a direct connection to Network X
2. Routers B and C learn about Network X through Router A
3. If Router A's connection to Network X fails:

   * Router A will mark the route as unreachable
   * However, Router B still has a valid route to Network X (through Router A)
   * Router B will advertise this route to Router C
   * Router C will then advertise the route to Router A
   * Router A will accept this route, not knowing it ultimately leads nowhere
   * A routing loop is formed: A → C → B → A

Split horizon doesn't prevent this because Router C isn't advertising the route
back to Router B (from which it learned it), but to Router A. This is why more
complex topologies require additional mechanisms beyond split horizon.

Network Model
-------------

For this step, we use a more complex network topology defined in
``RipNetworkC.ned``. This network includes four routers (router0, router1,
router2, and router3) connected in a mesh topology, creating multiple potential
routing loops:

.. figure:: media/ripC_Network.png
   :align: center

This step uses the following network:

.. literalinclude:: ../RipNetworkC.ned
   :language: ned
   :start-at: RipNetwork

The key difference from the previous step is that we now have four routers
instead of two, with multiple paths between them. This creates the potential for
more complex routing loops that can't be prevented by split horizon alone.

Experiment Configurations
-------------------------

We'll explore several configurations to understand the count-to-infinity problem
in this more complex topology and evaluate different solutions.

Basic Configuration
~~~~~~~~~~~~~~~~~~~

The basic configuration in ``omnetpp.ini`` is:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7
   :end-before: ------

Key aspects of this configuration:

* Split horizon is disabled (``mode='NoSplitHorizon'``)
* Triggered updates are disabled
* A scenario manager script disconnects a link at t=50s

Split Horizon Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~

We also test with split horizon enabled:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7SplitHorizon
   :end-before: ------

Triggered Updates Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, we test with triggered updates enabled:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7TriggeredUpdates
   :end-before: ------

Experiment 1: Basic Count-to-Infinity in Complex Networks
---------------------------------------------------------

In our first experiment, we observe the basic count-to-infinity problem in the
complex network. When a link fails at t=50s, routing loops form among the
routers, and metrics gradually increase as routing information circulates
through the network.

RIP update messages are sent every 30 seconds (not visualized here). The route
metrics increase incrementally (count-to-infinity):

.. video:: media/step7.mp4
   :width: 100%

..   <!--internal video recording, animation speed none, playback speed 2.138, zoom 0.77-->

In this scenario, we observe:

1. After the link failure, routers continue to advertise routes they learned from other routers
2. These advertisements create routing loops where packets circulate among multiple routers
3. With each update, the metrics for these routes increase
4. Eventually, after many updates, the metrics reach 16 (infinity) and the routes are removed

.. figure:: media/step7_metrics.png
   :align: center

The key insight is that the count-to-infinity problem in complex networks can
take much longer to resolve than in simple two-router networks, potentially
causing extended periods of network instability.

Experiment 2: Split Horizon in Complex Networks
-----------------------------------------------

In this experiment, we enable split horizon to see if it resolves the
count-to-infinity problem in our complex network.

With split horizon enabled:

1. Routers don't advertise routes back to the neighbors from which they learned them
2. This prevents some routing loops, particularly direct back-and-forth loops between two routers
3. However, loops involving three or more routers can still form

For example, if router0 learns a route from router1, split horizon prevents
router0 from advertising that route back to router1. However, router0 can still
advertise it to router3, which can advertise it to router1, creating a loop.

The result is that while split horizon helps reduce the severity of the
count-to-infinity problem, it doesn't completely eliminate it in complex
networks. Convergence is faster than without split horizon, but routing loops
can still occur.

.. TODO: A comparison of convergence times with and without split horizon would be helpful here

Experiment 3: Triggered Updates in Complex Networks
---------------------------------------------------

In our final experiment, we enable triggered updates to see how they affect convergence in the complex network.

With triggered updates:

1. Routing information propagates more quickly after the link failure
2. This can help the network converge faster to a stable state
3. However, it can also accelerate the rate at which incorrect routing information propagates

The result is a trade-off: triggered updates can speed up convergence in some
cases, but they can also make the count-to-infinity problem more intense by
accelerating the rate at which metrics increase.

When combined with split horizon, triggered updates provide a more effective
solution, though still not perfect for complex networks.

Implications for Real-World RIP Deployments
-------------------------------------------

The count-to-infinity problem in complex networks has several implications for real-world RIP deployments:

1. **Network Design**: Avoid complex topologies with multiple potential loops when using RIP
2. **Combined Solutions**: Use a combination of split horizon, poison reverse, and triggered updates
3. **Route Summarization**: Summarize routes where possible to limit the scope of routing loops
4. **Alternative Protocols**: Consider link-state protocols like OSPF for complex networks, which don't suffer from the count-to-infinity problem

In the next step, we'll explore another important mechanism for addressing the count-to-infinity problem: the hold-down timer.

Conclusion and Next Steps
-------------------------

In this step, we've seen that the count-to-infinity problem becomes more
challenging in networks with more than two routers. Solutions like split horizon
and triggered updates help but don't completely solve the problem in complex
topologies.

In the next step, we'll explore the hold-down timer, which provides another
mechanism for preventing routing loops and improving convergence in RIP
networks.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkC.ned <../RipNetworkC.ned>`
