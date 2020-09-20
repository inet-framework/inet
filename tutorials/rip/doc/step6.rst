Step 6. Count to Infinity Problem (Two-node Loop Instability)
=============================================================

Goals
-----

We will demonstrate the count to infinity problem of the distance-vector routing
algorithm used by RIP.

The distance-vector algorithm has many advantages. For example, it is very easy
to implement and responds to topology changes rapidly. However, if there are routing
loops in the topology, the algorithm may take a very long time to stabilize,
and during this transient period (which can be quite long) some parts of the
network will remain inaccessible. There are only partial solutions to this
problem (e.g. split horizon).

Network Configuration
---------------------

Our network topology for this exercise is defined by the ``RipNetworkB.ned``
file:

TODO screenshot

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6
   :end-before: ------

At t = 50s, the link connecting ``router1`` to ``switch1`` is cut. Then, ``router0``
advertises a route hosts 3, 4 and 5 to ``router1``. Even though this route is a
"phantom" one, ``router1`` does not know this, increments the hop count, adds it to
its routing table, and advertises it to ``router0``. This cycle keeps going until
the hop count reaches a big number (default is 16).

TODO move above para to different place?

Experiments
-----------

Demonstration of the problem
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``[Config Step6]`` demonstrates the problem: RIP convergence takes 220 seconds
(from the link break at t = 50 s to no routes to host3 at 270 s). Note the
increasing hop count (indicated on the route in parentheses).

TODO screenshot?

Observe how *ping* packets go back and forth between the two routers, indicating
the presence of a routing loop. The ping packets time out after 8 hops, and are
dropped.

Count to Infinity is a classical race condition issue
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In [Config Step6DifferentTimings] we demonstrate that depending on the time in
which router informs the other one, the problem may or may not appear.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6DifferentTimings
   :end-before: ------

.. video:: media/step6_1.mp4
   :width: 100%

..   <!--internal video recording, zoom 0.77, playback speed 1, no animation speed-->

TODO Rip start, link break, counting to infinity, ping packets:

.. video:: media/step6_2.mp4
   :width: 100%

Triggered updates
~~~~~~~~~~~~~~~~~

In [Config Step6Solution3] we demonstrate that triggered updates do not solve
the problem.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6TriggeredUpdates
   :end-before: ------

Solution 1: Split horizon
~~~~~~~~~~~~~~~~~~~~~~~~~

``[Config Step6Solution1]`` shows how the addition of split horizon solves the
count to infinity problem. Here, when a router sends a routing update to its
neighbors, it does not send those routes it learned from each neighbor back to
that neighbor.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6Solution1
   :end-before: ------

Solution 2: Split horizon with poison reverse
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In ``[Config Step6Solution2]`` we demonstrate a stronger version of split horizon,
called split horizon with poison reverse. In this variation of split horizon,
when a router sends a routing update to its neighbors, it sends those routes it
learned from each neighbor back to that neighbor with infinite cost information
to make sure that the neighbour does not use that route.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6Solution2
   :end-before: ------


Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkB.ned <../RipNetworkB.ned>`
