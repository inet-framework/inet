Step 9. Configuring node movements
==================================

Goals
-----

In this step, we make the model more interesting by adding node
mobility. Namely, we make the intermediate nodes travel north during the
simulation. After a while, they will move out of the range of host A
(and B), breaking the communication path.

The model
---------

Mobility
~~~~~~~~

In the INET Framework, node mobility is handled by the ``mobility``
submodule of hosts. Several mobility module types exist that can be
plugged into a host. The movement trail may be deterministic (such as
line, rectangle or circle), probabilistic (e.g. random waypoint),
scripted (e.g. a "turtle" script) or trace-driven. There are also
individual and group mobility models.

Here we install :ned:`LinearMobility` into the intermediate nodes.
:ned:`LinearMobility` implements movement along a line, where the heading
and speed are parameters. We configure the nodes to move north at the
speed of 12 m/s.

Other changes
~~~~~~~~~~~~~

So far, L2 queues (the queues in the wireless interfaces) have been
unlimited, that is, no packet would be dropped due to congestion.
Meanwhile, there was indeed congestion in host R1 and host A, because
the application in host A was generating packets at a higher rate
than what the network could carry.

From this step on, we limit the queue lengths at 10 packets. This allows
the network to react faster to topology changes caused by node
movements because queues will not be clogged up by old packets.
However, as a consequence of packet drops, we expect that the sequence
numbers of packets received by host B will no longer be continuous.

We also update the visualization settings and turn on an option that
will cause mobile nodes to leave a trail as they move. We also enable a
visualizer option that will display the velocity vector of the moving
nodes.

The configuration:



.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Wireless09]
   :end-before: #---

Results
-------

It is advisable to run the simulation in Fast mode, because the nodes
move very slowly if viewed in Normal mode.

It can be seen in the animation below as host R1 leaves host A's
communication range at around 11 seconds. After that, the communication
path is broken. Traffic could be routed through R2 and R3, but that does
not happen because the routing tables are static and have been
configured according to the initial positions of the nodes. When the
communication path breaks, the blue arrow that represents successful
network layer communication paths fades away, because there are no more
packets to reinforce it.



.. video:: media/wireless-step9-1.mp4
   :width: 655
   :height: 575



   <!--internal video recording, playback speed 2, animation speed 1, fadeOutMode animationTime, fadeOutTime 1s-->

As mentioned before, a communication path could be established between
host A and B by routing traffic through hosts R2 and R3. To reconfigure
routes according to the changing topology of the network, an ad-hoc
routing protocol is required.

**Number of packets received by host B: 547**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WirelessB.ned <../WirelessB.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.
