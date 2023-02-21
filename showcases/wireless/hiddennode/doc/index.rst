The Hidden Node Problem
=======================

Goals
-----

The hidden node problem occurs when two nodes in a wireless network can
communicate with a third node, but cannot communicate with each other directly
due to obstacles or being out of range. This can lead to collisions at the third node
when both nodes transmit simultaneously. 
The RTS/CTS mechanism is used to address this problem by allowing nodes
to reserve the channel before transmitting.

In this showcase, we demonstrate the hidden node problem and how the RTS/CTS
mechanism can be used to solve it in an 802.11 wireless network.

| INET version: ``4.0``
| Source files location: `inet/showcases/wireless/hiddennode <https://github.com/inet-framework/inet/tree/master/showcases/wireless/hiddennode>`__

Description of the hidden node problem
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The hidden node problem is the collective name of situations where a
transmitting node does not know about the existence of another node (the
"hidden node") while transmitting to a third node which is within the
range of both nodes. Since the node doesn't know when the hidden node is
transmitting, normal collision avoidance is not effective, and their
transmissions will often collide at the third node. The hidden node
problem reduces channel utilization and damages network performance.
Hidden nodes may be created by distance, by obstacles that block radio
signals, by unequal transmission powers, or by other factors. The
situation may be symmetric or asymmetric (the roles of the originator
and the hidden node may or may not be interchangeable.) You can read
more about the hidden node problem e.g. here.

The 802.11 protocol allows transmissions to be protected against
interference from other stations by using the RTS/CTS mechanism. Using
RTS/CTS, the node wishing to transmit first sends an RTS (Request To
Send) frame to the target node. The target node, if the channel is
clear, responds with a CTS (Clear To Send) frame that not only informs
the originator that it may transmit but also tells other stations to
refrain from using the channel for the specified amount of time. RTS/CTS
obviously adds some overhead to the transmission process, so it is
normally used to protect longer frames which are more expensive to
retransmit. RTS/CTS does not completely solve the hidden node problem,
but it can significantly help under certain conditions. A slightly more
in-depth coverage of RTS/CTS and challenges associated with it can be
found e.g. here.

Demonstrating the hidden node problem
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this showcase, we'll set up a wireless network that contains a hidden
node. To ensure that two selected nodes don't hear one another, we'll
place an obstacle (a section of wall that blocks radio signals) between
them. We'll run the simulation without RTS/CTS, with RTS/CTS turned on,
and for reference, also with the wall removed.

The model
---------

The network for all simulations contains three hosts, arranged in a
triangle. Host A and C are separated by a wall that completely blocks
transmissions, thus the nodes cannot transmit to each other, and cannot
sense when the other is transmitting. The wall is enabled or disabled in
the various simulations. Hosts A and C both send UDP packets to Host B,
which is able to receive the transmissions of both hosts.

.. figure:: media/network.png
   :width: 80%
   :align: center

The RTS/CTS mechanism can be enabled or disabled by setting the
:par:`rtsThresholdBytes` parameter in the ``mac`` submodule of hosts. The
RTS/CTS mechanism is used for transmitting frames whose size exceeds the
threshold.

We will run the simulation in four configurations:

-  ``WallOnRtsOff``: RTS/CTS mechanism disabled
-  ``WallOnRtsOn``: RTS/CTS mechanism enabled
-  ``WallOffRtsOff``: Wall removed, no RTS/CTS
-  ``WallOffRtsOn``: Wall removed, RTS/CTS on

In all configurations, hosts A and C will both send constant size
(1000-byte) UDP packets at a rate that saturates the MAC most of the
time. The transmission power and all other parameters of the two hosts
are identical. We will run each configuration for the same simulation
time interval (5 seconds), and count the number of packets received by
Host B.

Results
-------

RTS/CTS disabled
~~~~~~~~~~~~~~~~

Both Host A and C frequently transmit simultaneously, thus the number of
collisions at Host B is high.

The animation below depicts such a collision. Host C starts
transmitting, and Host A starts transmitting as well before Host C's
transmission is over. As neither packet can be received correctly by
Host B (and thus they are not ACKed), Hosts A and C retry transmitting
the same packet multiple times after the backoff period. The
retransmitted packets also collide, because the packets are long
compared to the backoff period. Finally, Host C manages to send its
packet without interference.

.. video:: media/WallOnRtsOff2.mp4
   :width: 560
   :align: center

   <!-- 8ms-21ms, run, animation speed 1, built-in video recording -->

Here is what a collision looks like in the log:

.. figure:: media/collision.png
   :width: 60%
   :align: center

The number of packets received by Host B (RTS/CTS off): **1470**

RTS/CTS enabled
~~~~~~~~~~~~~~~

With RTS/CTS enabled, there are no more collisions, except for RTS
frames. RTS and CTS frames are much shorter than data frames (about 34us
vs 1.45ms), thus the probability of RTS frames colliding is less than
for data frames. The result is that a low number of RTS frames collide,
and since they are short, the collisions don't take up much time.

The following sequence chart has been recorded from the simulation and
depicts an RTS collision.

.. figure:: media/rtscollision.png
   :width: 60%
   :align: center

The following animation shows the RTS/CTS and data frame exchange.

.. video:: media/WallOnRtsOn.mp4
   :width: 560
   :align: center

The following sequence chart illustrates that the RTS/CTS mechanism
makes the communication more coordinated, as the nodes know when to
transmit to avoid collisions. It also illustrates that RTS and
CTS frames are much shorter than data frames.

.. figure:: media/rts-seq.png
   :width: 100%

The number of received packets at Host B (RTS/CTS on): **1971**

Wall removed
~~~~~~~~~~~~

With the wall removed, hidden nodes are no longer a problem. When the
RTS/CTS mechanism is not used, collision avoidance mechanisms can work,
and the number of collisions is low. The RTS/CTS mechanism stops data
frame collisions, so only the RTS and CTS frames can collide. The RTS
and CTS frames are much shorter than data frames, thus retransmitting
them takes less time. Even though the RTS/CTS frames contribute some
overhead, more packets are received correctly at Host B. When RTS/CTS is
used, the number of packets received correctly at Host B is
approximately the same regardless of the presence of the wall.

The number of received packets at Host B (wall removed, RTS/CTS off):
**1966**\  The number of received packets at Host B (wall removed,
RTS/CTS on): **1987**

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`HiddenNodeShowcase.ned <../HiddenNodeShowcase.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/16>`__ in
the GitHub issue tracker for commenting on this showcase.
