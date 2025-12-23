Step 2. Pinging after RIP Convergence
=====================================

Introduction
------------

In Step 1, we used static routing with pre-configured routing tables. Now, we'll
introduce the Routing Information Protocol (RIP) to demonstrate how routers can
dynamically discover routes and build their routing tables through the exchange
of routing information.

RIP is a distance-vector routing protocol that uses hop count as its metric.
Each router maintains a routing table and periodically advertises it to its
neighbors. Through these advertisements, routers learn about available networks
and the "distance" (number of hops) to reach them. Over time, all routers in the
network discover optimal paths to all destinations - a process known as
convergence.

Goals
-----

In this step, our goals are to:

1. Introduce RIP and observe how it dynamically builds routing tables
2. Understand the process of route discovery and convergence in RIP
3. Compare the dynamic routing approach with the static routing from Step 1
4. Observe how routers exchange routing information to learn about the network topology

In order to keep the discussion easy to digest, we do not introduce any changes
in the network while the simulation is running, and we do not use advanced RIP
features like split horizon or triggered updates yet. These will be explored in
subsequent steps.

If you would like to brush up your RIP knowledge, you can find a good discussion
of the distance-vector algorithm and RIP in `this section
<https://book.systemsapproach.org/internetworking/routing.html#distance-vector-rip>`_
of L. Peterson and B. Davie's superb book *Computer Networks: A Systems
Approach*, now freely available `online <https://book.systemsapproach.org>`_.

Understanding Distance-Vector Routing
-------------------------------------

Before diving into the simulation, let's briefly review how distance-vector routing works:

1. **Initial state**: Each router knows only about its directly connected networks
2. **Information sharing**: Routers periodically send their entire routing table to all neighbors
3. **Route learning**: When a router receives routing information from a neighbor, it:
   * Adds the neighbor's hop count to each route
   * Compares these routes with its existing routing table
   * Updates its table if it finds a better route (fewer hops) or a new destination
4. **Convergence**: After several rounds of updates, all routers eventually learn the best routes to all destinations

RIP specifically uses a maximum hop count of 16, with 16 representing "infinity"
(unreachable). Routes with a metric of 16 or more are considered invalid. RIP
also implements various timers to manage route aging and removal, which we'll
explore in later steps.

Network Configuration
---------------------

We use the same network topology as in Step 1, but with a crucial difference:
instead of using the configurator to pre-calculate all routes, we start with
minimal routing tables and let RIP build them dynamically.

The key changes in the configuration are:

1. We enable RIP on all routers
2. We configure the hosts with default routes to their local routers
3. We start with empty routing tables on the routers

The experiment is set up in ``omnetpp.ini``, in the ``Step2`` section:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ------

Let's examine the key RIP configuration parameters:

* ``*.router*.hasRip = true``: Enables RIP on all routers
* ``*.router*.rip.startupTime = uniform(0s,1s)``: Sets a random startup time for RIP on each router (between 0 and 1 second)
* ``*.router*.rip.ripConfig``: XML configuration for RIP, setting the mode to "NoSplitHorizon" (we'll explore split horizon in later steps)
* ``*.router*.rip.triggeredUpdate = false``: Disables triggered updates (we'll explore this feature in Step 5)

The configurator is now only responsible for:

1. Assigning IP addresses to interfaces
2. Setting up default routes for hosts (``autoroute sourceHosts='host*'``)

Experiment: Observing RIP Convergence
-------------------------------------

In this experiment, we observe how RIP progressively builds the routing tables
of all routers through the exchange of routing information. Unlike in Step 1,
where all routes were available immediately, here we'll see routes being
discovered gradually.

When the simulation starts:

1. Each router knows only about its directly connected networks
2. Routers begin exchanging RIP messages containing their routing tables
3. As routers receive these messages, they update their own routing tables
4. Over several iterations, routers discover paths to all networks in the topology
5. Eventually, the network reaches convergence - a stable state where all routers have optimal routes to all destinations
6. After convergence (at t=50s), ``host0`` begins sending ping packets to ``host6`` to test connectivity

The following video demonstrates this process. Watch how the blue and red arrows
(representing routes to ``host6`` and ``host0`` respectively) gradually appear
as routers learn about these destinations:

.. video:: media/step2_3.mp4
   :width: 100%

..   <!--internal video recording, normal run until sendPing, animation speed none, playback speed 2.138, max anim speed kludge in simtimetextfigure-->

Understanding RIP Messages
--------------------------

During the simulation, you can observe RIP messages being exchanged between routers. Each RIP message contains:

1. **RIP header**: Protocol version and other control information
2. **Route entries**: A list of network destinations with their associated metrics (hop counts)

When a router receives a RIP message, it processes each route entry:

* If the route is new, it adds it to its routing table (with the metric increased by 1)
* If the route exists but the new path has a lower metric, it updates the route
* If the route exists but the new path has a higher metric, it ignores the update (unless it came from the same router that originally provided the route)

Analyzing Convergence
---------------------

By the end of the simulation, you'll notice that the network has converged to
the same routing state as in Step 1. This demonstrates an important principle:
given the same network topology and link costs, both static and dynamic routing
should eventually produce the same optimal routes.

However, there are key differences:

1. **Time to convergence**: Static routing is immediate, while RIP takes time to converge
2. **Adaptation to changes**: RIP can adapt to network changes (which we'll explore in subsequent steps)
3. **Overhead**: RIP generates ongoing traffic as routers exchange routing information
4. **Scalability**: RIP has limitations in larger networks due to its maximum hop count of 16

Conclusion and Next Steps
-------------------------

In this step, we've observed how RIP dynamically builds routing tables through
the exchange of routing information. We've seen the process of convergence,
where routers gradually discover optimal paths to all destinations in the
network.

While RIP works well in this stable network, it faces challenges when the
network topology changes. In the next steps, we'll explore:

* How RIP responds to link failures (Step 3)
* The role of various RIP timers in managing route aging and removal (Step 4)
* How triggered updates can speed up convergence (Step 5)
* The "count to infinity" problem and solutions like split horizon (Steps 6-8)

These scenarios will help us understand both the strengths and limitations of
distance-vector routing protocols like RIP.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`
