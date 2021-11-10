Clock Drift
===========

Goals
-----

In this showcase, we demonstrate the effects of clock drift and time synchronization
on network behavior. We show how to introduce local clocks in network nodes, how
to configure clock drift for them, and how to use time synchronization mechanisms
to reduce time differences.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/timesynchronization/clockdrift <https://github.com/inet-framework/tree/master/showcases/tsn/timesynchronization/clockdrift>`__

Introduction
~~~~~~~~~~~~

In the real world, there is no globally agreed-upon time, but each node uses its own 
physical clock to keep track of time. Due to finite clock accuracy, clocks in various 
nodes of the network may diverge over time. The operation of applications and protocols 
across the network are often very sensitive to the accuracy of this local time. 

Simulating Clock Drift and Time Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, modules such as network nodes and interfaces in INET don't have local time but use the 
simulation time as a global time. To simulate network nodes with local time, and effects such as 
clock drift and its mitigation (time synchronization), network nodes need to have `clock` submodules. To keep time, most clock modules use `oscillator` submodules. The oscillators produce ticks periodically, and the clock modules count the ticks. Clock drift is caused by inaccurate oscillators.
INET provides various oscillator models, so by combining clock and oscillator models, we can 
simulate `constant` and `random` clock drift rates. To model time synchronization, the time of 
clocks can be set by `synchronizer` modules.

Several clock models are available:

- :ned:`OscillatorBasedClock`: Has an `oscillator` submodule; keeps time by counting the oscillator's ticks. Depending on the oscillator submodule, can model `constant` and `random clock drift rate`
- :ned:`SettableClock`: Same as the oscillator-based clock but the time can be set from C++ or a scenario manager script
- :ned:`IdealClock`: The clock's time is the same as the simulation time; for testing purposes

The following oscillator models are available:

- :ned:`IdealOscillator`: generates ticks periodically with a constant tick length
- :ned:`ConstantDriftOscillator`: generates ticks periodically with a constant drift rate in generation speed; for modeling `constant rate clock drift`
- :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modeling `random clock drift`

Synchronizers are implemented as application-layer modules. For clock synchronization, the synchronizer 
modules need to set the time of clocks, thus only the SettableClock supports synchronization. 
The following synchronizer modules are available:

- :ned:`SimpleClockSynchronizer`: Uses an out-of-band mechanism to synchronize clocks, instead of a real clock synchronization protocol. Useful for simulations where the details of time synchronization are not important.
- :ned:`Gptp`: Uses the General Precision Time Protocol to synchronize clocks.

The :ned:`SimpleClockSynchronizer` module periodically synchronizes a slave clock to a master clock. 
The synchronization interval can be configured with a parameter. Also, in order to model real-life 
synchronization protocols, which are somewhat inaccurate by nature, the accuracy can be specified 
(see the NED documentation of the module for more details).

The General Precision Time Protocol (gPTP, or IEEE 802.1 AS) can synchronize clocks in an Ethernet network 
with a high clock accuracy required by TSN protocols. It is implemented in INET by the :ned:`Gptp` module. 
Several network node types, such as :ned:`EthernetSwitch` have optional gPTP modules, activated by the 
:par:`hasGptp` boolean parameter. It can also be inserted into hosts as one of the applications. 
Gptp is demonstrated in the last example of this showcase. For more details about :ned:`Gptp`, 
check out the `Using gPTP` showcase (release pending) or the module's NED documentation.

The Model and Results
---------------------

The showcase contains four example simulations. All simulations use the following network:

.. figure:: media/Network2.png
   :align: center

The example configurations are the following:

- ``NoClockDrift``: Network nodes don't have clocks, they are synchronized by simulation time
- ``ConstantClockDriftRate``: Network nodes have clocks with constant clock drift rate, and the clocks diverge over time
- ``OutOfBandSynchronization``: Clocks have a constant drift rate, and they are synchronized by an out-of-band synchronization method (without a real protocol)
- ``GptpSynchronization``: Clocks have random constant clock drift, and they are synchronized by gPTP

In the ``General`` configuration, ``source1`` is configured to send UDP packets to ``sink1``, and ``source2`` to ``sink2``.

.. note:: To demonstrate the effects of drifting clocks on the network traffic, we configure the Ethernet MAC layer in switch1 to alternate between forwarding frames from ``source1`` and ``source2`` every 10 us, by using a TSN gating mechamism in switch1. This does not affect the simulation results in the next few sections but becomes important in the `Effects of Clock Drift on End-to-end Delay`_ section. More details about this part of the configuration are provided there.

In the next few sections, we present the above four examples. In the simulations featuring clock drift, 
``switch1`` always has the same clock drift rate. In the ones also featuring clock synchronization, 
the hosts are synchronized to the time of ``switch1``. We plot the time difference of clocks and 
simulation time to see how the clocks diverge from the simulation time and each other. 

Example: No Clock Drift
~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, network nodes don't have clocks. Applications and gate
schedules are synchronized by simulation time. (End-to-end delays in the other 
three cases will be compared to this baseline configuration.)

There are no clocks, so the configuration is empty:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NoClockDrift
   :end-before: ConstantClockDriftRate

Example: Constant Clock Drift Rate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, all network nodes have a clock with a constant drift rate.
Clocks drift away from each other over time.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ConstantClockDriftRate
   :end-before: OutOfBandSynchronization

We configure the network nodes to have OscillatorBasedClock modules, with a ConstantDriftOscillator. 
We also set the drift rate of the oscillators. By setting different drift rates for the different clocks, 
we can control how they diverge over time. Note that the drift rate is defined as compared to simulation time.
Also, we need to explicitly tell the relevant modules (here, the UDP apps and ``switch1``'s queue) to use the 
clock module in the host, otherwise they would use the global simulation time by default.

Here are the drifts (time differences) over time:

.. figure:: media/ConstantClockDrift.png
   :align: center

The three clocks have different drift rates. The magnitude and direction of drift of ``source1`` and ``source2`` 
   compared to ``switch1`` are different as well, i.e. ``source1``'s clock is early and ``source2``'s clock is late 
   compared to ``switch1``'s.

Example: Out-of-Band Synchronization of Clocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, the network node clocks have the same constant drift rate as in the previous one, 
but they are periodically synchronized by an
out-of-band mechanism (C++ function call).

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: OutOfBandSynchronization
   :end-before: GptpSynchronization

To get the constant clock drift rate, this configuration extends ConstantClockDriftRate. Since we want to use 
clock synchronization, we need to be able to set the clocks, so network nodes have SettableClock modules. The 
``defaultOverdueClockEventHandlingMode = "execute"`` setting means that when setting the clock forward, events 
that become overdue are done immediatelly. We use the SimpleClockSynchronizer to have out-of-band synchronization. 
This module is implemented as an application, so we add one to each source host. We set the synchronizer modules 
to sync time to the clock of ``switch1``.

Here are the time differences:

.. figure:: media/OutOfBandSync.png
   :align: center
   :width: 100%

The clock of ``switch1`` has a constant drift rate compared to simulation time. The other clocks diverge from the 
simulation time with some constant rate but are periodically synchronized to the clock of ``switch1``.

Example: Synchronizing Clocks Using gPTP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, the clocks in network nodes have the same drift rates as in the previous two configurations, 
but they are periodically synchronized to a master clock using the Generic Precision Time Protocol (gPTP). The time
synchronization protocol measures the delay of individual links and disseminates
the clock time of the master clock on the network through a spanning tree.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: GptpSynchronization

Here are the time differences:

.. figure:: media/GptpSync.png
   :align: center
   :width: 100%

Similarly to the previous example, ``switch1``'s clock has a constant drift, and the other clocks periodically synchronize to ``switch1``.

Accuracy of Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The precision of the synchronization methods in the ``OutOfBandSynchronization`` and ``GptpSynchronization`` cases 
can be visualized by zooming in on the above clock time charts. We can examine the moment when the times of the 
source hosts change. The distance of the new time from the reference shows the precision:

.. figure:: media/GptpSyncZoomed.png

We configured an arbitrary random accuracy in a range for the SimpleClockSynchronizer. In the case of gPTP, 
the accuracy is not settable but an emergent property of the protocol's operation.

.. note:: - When configuring the SimpleClockSynchronizer with a accuracy of 0, the synchronized time perfectly matches the reference.
          - The accuracy might different for all sawteeth.

Effects of Clock Drift on End-to-end Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section aims to demonstrate that clock drift can have profound effects on the operation of the network.
We take a look at the end-to-end delay in the four examples to show this effect. 

To this end, in all simulations, the Ethernet MAC layer in ``switch1`` is configured to alternate forwarding 
packets from ``source1`` and ``source2`` every 10 us; note that the UDP applications are sending packets every 
20 us, with packets from ``source2`` offset by 10 us compared to ``source1``. Thus packets from both sources 
have a send window in ``switch1``, and the sources generate and send packets to ``switch1`` in sync with that 
send window (they are only in sync if the clocks in the nodes are in sync, as we'll see later).

Here is how we configure this. We configure the EthernetMacLayer in ``switch1`` to contain a GatingPriorityQueue, 
with two inner queues: 

.. literalinclude:: ../omnetpp.ini
   :start-at: GatingPriorityQueue
   :end-at: numQueues
   :language: ini

The inner queues in the GatingPriorityQueue each have their own gate. The gates connect to a PriorityScheduler, 
so the gating piority queue prioritizes packets from the first inner queue. Here is a gating priority queue with 
two inner queues:

.. figure:: media/GatingPriorityQueue.png
   :align: center

In our case, we configure the classifier (set to ContentBasedClassifier) to send packets from ``source1`` to the 
first queue and those from source2 to the second, thus the gating priority queue prioritizes packets from ``source1``. 
The gates are configured to open and close every 10us, with the second gate offset by a 10us period (so they 
alternate being open). Here is the rest of the gating priority queue configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: ContentBasedClassifier
   :end-at: offset
   :language: ini

We measure the end-to-end delay of packets from ``source1`` in ``sink1`` and so on. Here are the results for the four examples:

.. figure:: media/delay.png
   :align: center

The delay is constant when there is no clock drift (NoClockdrift), or when the clocks are synchronized 
(OutOfBandSynchronization and GptpSynchronization). In these cases, the applications send packets in sync 
with the opening of the gates in the queue in ``switch1``. In the no clock drift case, the delay depends 
only on the bitrate and packet length. In the case of ``OutOfBandSynchronization`` and ``GptpSynchronization``, 
the clocks drift but the drift is periodically eliminated by synchronization, so the delay remains constant. 
In the constant clock drift case, however, the delay keeps changing substantially compared to the other cases.

What's the reason behind these graphs? When there is no clock drift (or it is eliminated by synchronization), 
the end-to-end delay is constant, because the packets are generated in the sources in sync with the opening of 
the corresponding gates in ``switch1`` (the send windows). In the constant clock drift case, the delay's 
characteristics depend on the magnitude and direction of the drift between the clocks.

It might be helpful to think of the constant drift rate as time dilation. In ideal conditions (no clock drift 
or eliminated clock drift), the clocks in all three modules keep the same time, so there is no time dilation. 
The packets in both sources are generated in sync with the send windows (when the corresponding gate is open), 
and they are forwarded immediately by ``switch1``. In the constant clock drift case, from the point of view of 
``switch1``, the clock of ``source1`` is slower than its own, and the clock of ``source2`` is faster. Thus, 
the packet stream from ``source1`` is sparser than in the ideal case, and the packet stream from ``source2`` 
is denser, due to time dilation.

If the packet stream is sparser (orange graph), there are on average fewer packets to send than the number of 
send windows in a given amount of time, so packets don't accumulate in the queue. However, due to the drifting 
clocks, the packet generation and the send windows are not in sync anymore but keep shifting. Sometimes a 
packet arrives at the queue in ``switch1`` when the corresponding gate is closed, so it has to wait for the 
next opening. This next opening happens earlier and earlier for subsequent packets (due to the relative 
direction of the drift in the two clocks), so packets wait less and less in the queue, hence the decreasing 
part of the curve. Then the curve becomes horizontal, meaning that the packets arrive when the gate is open 
and they can be sent immediately. After some time, the gate opening shifts again compared to the packet generation, 
so the packets arrive just after the gate is closed, and they have to wait for a full cycle in the queue before 
being sent. 

If the packet stream is denser (blue graph), there are more packets to send on average than there are 
send windows in a given amount of time, so packets eventually accumulate in the queue. This causes the 
delay to keep increasing indefinitely.

.. note:: - The packets are not forwarded by ``switch1`` if the transmission wouldn't finish before the gate closes (a packet takes 6.4us to transmit, the gate is open for 10us).
          - The length of the horizontal part of the orange graph is equal to how much the two clocks drift during the time of ``txWindow - txDuration``. In the case of the orange graph, it is ``(10us - 6.4us) / 700ppm ~= 5ms``

Thus, if constant clock drift is not eliminated, the network can no longer guarantee any bounded delay for the packets. 
The constant clock drift has a predictable repeated pattern, but it still has a huge effect on delay. Unpredictable random 
clock drift might have an even larger impact.


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ClockDriftShowcase.ned <../ClockDriftShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

