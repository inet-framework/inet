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

.. Introduction
   ------------

.. Clocks
   ~~~~~~

   By default, modules in INET don't have their own clocks, and thus have no local time. They all use the simulation time as a global time (so effectively their time is perfectly synchronized/effectively perfect synchronization). For more detailed simulations where the local time and time synchronization need to be modelled, INET has several clock models available, such as the following:

   - :ned:`IdealClock`: The clock's time is the same as the simulation time; for testing purposes
   - :ned:`OscillatorBasedClock`: Has an `oscillator` submodule; keeps time by counting the oscillator's ticks. Depending on the oscillator submodule, can model `constant` and `random clock drift`, for example
   - :ned:`SettableClock`: Same as the oscillator-based clock but the time can be set from C++ or from a scenario manager script

   See all available clocks and oscillators in the ``inet/src/inet/clock`` folder.

   **TODO** more on oscillators; should that be a distinct section?

   Several network node types, such as :ned:`StandardHost` and :ned:`EthernetSwitch`, have clock submodules, disabled by default. To enable them, set the clock submodule's typename, for example:

   ``*.host1.clock.typename = "OscillatorBasedClock"``

   Oscillators
   ~~~~~~~~~~~

   Oscillators produce ticks at certain time intervals, depending on type. The clock types :ned:`OscillatorBasedClock` and :ned:`SettableClock` have oscillator submodules, and keep time by counting the oscillator's ticks. INET contains the following oscillator models:

   - :ned:`IdealOscillator`: generates ticks periodically with a constant tick length
   - :ned:`ConstantDriftOscillator`: generates ticks periodically with a constant drift in generation speed; for modeling `constant clock drift`
   - :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modeling `random clock drift`

   For creating drifting clocks, the various clock and oscillator models can be combined.

      **TODO** another angle? like so:

      By default, network nodes and modules don't have clocks
      To simulate effect of each module having its own local time, the nodes can have their own clock submodules
      Here is how it works: most clock modules have oscillator submodules; the clocks keep time by counting oscillator ticks. The type of the oscillator module used and its configuration results in the modelled clock drift.

      Also, for simulating time synchronization protocols, clocks need to be settable. Settable clocks have synchornizer submodules, which can set the time of the clock on clue.
      The clue may be an out-of-band synchronization mechanism, or the effect of receiving timing protocol messages from other nodes, for example.

   --------------------------------------

.. Simulating clock drift with clocks, oscillators and Synchronizers
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Introduction
~~~~~~~~~~~~

.. real-world problem statement

Protocols often rely on relative/absolute time values. In the real world, there is no globally agreed-uppon time. Each network component keeps track of the (local) time on its own. Because of the differences in software and hardware, the time values of different network components may diverge over time and this may have an effect on time sensitive protocols.

**TODO** should this be in the Goals section ? as the first paragraph

Simulating Clock Drift and Time Synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, modules such as network nodes and interfaces in INET don't have local time but use the simulation time as a global time. To simulate network nodes with local time, and effects such as clock drift and its mitigation (time synchronization), network nodes need to have `clock` submodules. To keep time, most clock module types use `oscillator` submodules. The oscillators produce ticks in certain intervals, and the clock modules count the ticks. Clock drift is produced by oscillators that have a configurable drift in tick generation speed.
Thus, by combining clock and oscillator models, we can simulate `constant` and `random` clock drift. To model time synchronization, the time of clocks can be set by `synchronizer` modules.

Several clock models are available:

- :ned:`OscillatorBasedClock`: Has an `oscillator` submodule; keeps time by counting the oscillator's ticks. Depending on the oscillator submodule, can model `constant` and `random clock drift`
- :ned:`SettableClock`: Same as the oscillator-based clock but the time can be set from C++ or a scenario manager script
- :ned:`IdealClock`: The clock's time is the same as the simulation time; for testing purposes

The following oscillator models are available:

- :ned:`IdealOscillator`: generates ticks periodically with a constant tick length
- :ned:`ConstantDriftOscillator`: generates ticks periodically with a constant drift rate in generation speed; for modeling `constant clock drift rate`
- :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modeling `random clock drift`

Synchronizers are implemented as application-layer modules. For clock synchronization, the synchronizer modules need to set the time of clocks, thus only the SettableClock supports synchronization. The following synchronizer modules are available:

- :ned:`SimpleClockSynchronizer`: Uses an out-of-band mechanism to synchronize clocks, instead of a real clock synchronization protocol. Useful for simulations where the details of time synchronization are not important.
- :ned:`Gptp`: Uses the General Precision Time Protocol to synchronize clocks.

The SimpleClockSynchronizer module periodically synchronizes a slave clock to a master clock. The master and slave clock, the synchronization interval, and the accuracy can be configured with parameters (see the NED documentation of the module for more details).

The General Precision Time Protocol (gPTP, or IEEE 802.1 AS) can synchronize clocks in an Ethernet network with a high clock accuracy required by TSN protocols. It is implemented in INET by the :ned:`Gptp` module. Several network node types, such as :ned:`EthernetSwitch` have optional gPTP modules, activated by the :par:`hasGptp` boolean parameter. It can also be inserted into hosts as one of the applications. Gptp is used in the last example to synchronize clocks. For more details about :ned:`Gptp`, check out the `Using gPTP` showcase (release pending) or the module's NED documentation).

.. --------------------------------------

   Clocks, oscillators and synchronizers in INET
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   Here are the available models:

   **Clock**

   - :ned:`OscillatorBasedClock`: Has an `oscillator` submodule; keeps time by counting the oscillator's ticks. Depending on the oscillator submodule, can model `constant` and `random clock drift`
   - :ned:`SettableClock`: Same as the oscillator-based clock but the time can be set from C++ or from a scenario manager script
   - :ned:`IdealClock`: The clock's time is the same as the simulation time; for testing purposes

   **Oscillators**

   - :ned:`IdealOscillator`: generates ticks periodically with a constant tick length
   - :ned:`ConstantDriftOscillator`: generates ticks periodically with a constant drift in generation speed; for modeling `constant clock drift`
   - :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modeling `random clock drift`

   **Synchronizers**

   - :ned:`IdealTimeSynchronizer`
   - :ned:`Gptp`

   --------------------------------------

   **TODO** just as in reality? there are oscillators for timekeeping


   Clock Synchronizers
   ~~~~~~~~~~~~~~~~~~~

   **TODO** or synchronizing clocks ?

   .. The General Precision Time Protocol (IEEE 802.1 AS) is used to synchronize clock in an Ethernet network with a clock accuracy in the sub-microsecond range. The protocol measures residence time and link delay, and synchronize time to a selected Grand Master (GM) clock. Time sync messages travel in a hierarchical structure around the GM. The GM can be pre-selected, or selected by the Best Master Clock Algorithm.

   .. The protocol is implemented by the :ned:`Gptp` application in INET.

   **TODO** simple clock sync

   The General Precision Time Protocol (gPTP, or IEEE 802.1 AS) can synchronize clocks in an Ethernet network with a high clock accuracy required by TSN protocols. It is implemented in INET as an application-layer module, :ned:`Gptp`. Several network node types, such as :ned:`EthernetSwitch` has optional gPTP modules, activated by the :par:`hasGptp` boolean parameter. It can also be inserted into hosts as one of the applications. It is used in the last example to synchronize clocks. For more details about :ned:`Gptp`, check out the TODO showcase.

   .. Check the TODO showcase for more information on the :ned:`Gptp` module.

The Model and Results
---------------------

.. **TODO** About the structure (what simulations)

The showcase contains four example simulations. All simulations use the following network:

.. figure:: media/Network2.png
   :align: center

The example configurations are the following:

- ``NoClockDrift``: Network nodes don't have clocks, they are synchronized by simulation time
- ``ConstantClockDriftRate``: Network nodes have clocks with constant clock drift rate, and the clocks diverge over time
- ``OutOfBandSynchronization``: Clocks have random constant clock drift, and they are synchronized by an out-of-band synchronization method (without a real protocol)
- ``GptpSynchronization``: Clocks have random constant clock drift, and they are synchronized by gPTP

.. The simulations use the following traffic pattern: 

  - ``source1`` sends UDP packets to ``sink1``
  - ``source2`` sends UDP packets to ``sink2``

In the ``General`` configuration, ``source1`` is configured to send UDP packets to ``sink1``, and ``source2`` to ``sink2``. All Ethernet interfaces are configured to be :ned:`LayeredEthernetInterface`.

.. note:: To demonstrate the effects of drifting clocks on the network traffic, we configure the Ethernet MAC layer in switch1 to alternate between forwarding frames from ``source1`` and ``source2`` every 10 us, by using gates in switch1. This does not affect the simulation results in the next few sections but becomes important in the `Effects of Clock Drift on End-to-end Delay`_ section. More details about this part of the configuration are provided there.

In the next few sections, we present the four above examples. In the simulations featuring clock drift, ``switch1`` always has the same clock drift rate. In the ones also featuring clock synchronization, the hosts are synchronized to the time of ``switch1``. We plot the time difference of clocks and simulation time to see how the clocks diverge from the simulation time and each other.

.. **TODO** somewhere: switch1 always have some clock drift and the other clocks sync to that

.. **TODO** WHY are we doing this?

   We configure the EthernetMacLayer in ``switch1`` to contain a GatingPriorityQueue, with two inner queues: 

   .. literalinclude:: ../omnetpp.ini
      :start-at: GatingPriorityQueue
      :end-at: numQueues
      :language: ini

   Inner queues in the GatingPriorityQueue each have their own gate. The gates connect to a PriorityScheduler, so the gating piority queue prioritizes packets from the first inner queue. Here is a gating priority queue with two inner queues:

   .. figure:: media/GatingPriorityQueue.png
      :align: center

   Here is the rest of the gating priority queue configuration:

   .. literalinclude:: ../omnetpp.ini
      :start-at: ContentBasedClassifier
      :end-at: offset
      :language: ini

   In our case, we configure the classifier (set to ContentBasedClassifier) to send packets from source1 to the first queue and those from source2 to the second, thus the gating priority queue prioritizes packets from source1. The gates are configured to open and close every 10us, with the second gate offset by a 10us period (so they alternate being open).

   .. Inner queues in the GatingPriorityQueue each have their own gate. The scheduler is a PriorityScheduler, so it prioritizes packets from the first inner queue. In our case, we configure the gating priority queue to have two inner queues, and the classifier (set to ContentBasedClassifier) sends packets from source1 to the first queue and those from source2 to the second, thus the gating priority queue prioritizes packets from source1.

   .. A gating priority queue looks like this/here is a gating priority queue:

.. Here is the parts common for all the example simulations below, in the ``General`` configuration:

.. .. literalinclude:: ../omnetpp.ini
      :language: ini
      :end-before: NoClockDrift

.. In this case, the time of all clocks is the same as the simulation time.

Example: No Clock Drift
~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, network nodes don't have clocks. Applications and gate
schedules are synchronized by simulation time. (End-to-end delay in the other three case can be compared to the delay in this baseline configuration, in the TODO section.)

.. End-to-end delay in later sections can be compared to the delay in this baseline configuration.

.. This configuration serves as a baseline to compare end-to-end delay to in a later section.

.. Here is the configuration:

.. **V1** This simulation doesn't need any configuration (other than the ``General`` configuration section):

There are no clocks, so the configuration is empty:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NoClockDrift
   :end-before: ConstantClockDriftRate

Example: Constant Clock Drift
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, all network nodes have a clock with a constant drift rate.
Clocks drift away from each other over time.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ConstantClockDriftRate
   :end-before: OutOfBandSynchronization

We configure the network nodes to have OscillatorBasedClock modules, with a ConstantDriftOscillator. We also set the drift rate of the oscillators. By setting different drift rates for the different clocks, we can control how they diverge over time. Note that the drift rate is defined as compared to simulation time.
Also, we need to explicitly tell the relevant modules (here, the UDP apps and ``switch1``'s queue) to use the clock module in the host, otherwise they would use the global simulation time by default.

Here are the drfits (time differences) over  time:

.. figure:: media/ConstantClockDrift.png
   :align: center

The three clocks have different drift rates. The magnitude and direction of drift of ``source1`` and ``source2`` compared to ``switch1`` are different as well, i.e. ``source1``'s clock is early and ``source2``'s clock is late compared to ``switch1``'s.

Example: Out-of-Band Synchronization of Clocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. **TODO** this configuration extends the ConstantDriftRate, so the clocks have the same drift rate (they are just synchronized periodically)

   the clocks have the same drift rate as introduced in the ConstantClockDrift

In this configuration, the network node clocks have the same constant drift rate as in the previous one, but they are periodically synchronized by an
out-of-band mechanism that doesn't use the underlying network.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: OutOfBandSynchronization
   :end-before: GptpSynchronization

To get the constant clock drift rate, this configuration extends ConstantClockDriftRate. Since we want to use clock synchronization, we need to be able to set the clocks, so network nodes have SettableClock modules. **TODO** eventhandlingmode.
We use the SimpleClockSynchronizer to have out of band synchronization. This module is implemented as an application, so we add one to each source host. We set the synchronizer modules to sync time to switch1's clock.

Here are the time differences:

.. figure:: media/OutOfBandSync2.svg
   :align: center
   :width: 100%

The clock of ``switch1`` has a constant drift rate compared to simulation time. The other clocks diverge from the simulation time with some constant rate but are periodically synchronized to the clock of ``switch1``.

Example: Synchronizing Clocks Using gPTP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. **TODO** this configuration extends the ConstantDriftRate, so the clocks have the same drift rate (they are just synchronized periodically)

In this configuration, the clocks in network nodes have the same drift rate as in the previous two configurations, but they are periodically synchronized
to a master clock using the Generic Precision Time Protocol (gPTP). The time
synchronization protocol measures the delay of individual links and disseminates
the clock time of the master clock on the network through a spanning tree.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: GptpSynchronization

Here are the time differences:

.. figure:: media/Gptp.svg
   :align: center
   :width: 100%

Similarly to the previous example, ``switch1``'s clock has a constant drift, and the other clocks periodically synchronize to ``switch1``.

.. Results
   -------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

.. Here are the simulation results:

.. .. figure:: media/results.png
   :align: center
   :width: 100%

Effects of Clock Drift on End-to-end Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. **TODO** WHY are we doing this?

This section aims to demonstrate that clock drift can have profound effects on the operation of the network. We take a look at the end-to-end delay in the four examples to show this effect. 

To this end, in all simulations, the Ethernet MAC layer in ``switch1`` is configured to alternate forwarding packets from ``source1`` and ``source2`` every 10 us; note that the UDP applications are sending packets every 20 us, with packets from ``source2`` offset by 10 us compared to ``source1``. Thus packets from both sources have a send window in ``switch1``, and the sources generate and send packets to ``switch1`` in sync with that send window (they are only in sync if the clocks in the nodes are in sync, as we'll see later).

Here is how we configure this. We configure the EthernetMacLayer in ``switch1`` to contain a GatingPriorityQueue, with two inner queues: 

.. literalinclude:: ../omnetpp.ini
   :start-at: GatingPriorityQueue
   :end-at: numQueues
   :language: ini

Inner queues in the GatingPriorityQueue each have their own gate. The gates connect to a PriorityScheduler, so the gating piority queue prioritizes packets from the first inner queue. Here is a gating priority queue with two inner queues:

.. figure:: media/GatingPriorityQueue.png
   :align: center

Here is the rest of the gating priority queue configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: ContentBasedClassifier
   :end-at: offset
   :language: ini

In our case, we configure the classifier (set to ContentBasedClassifier) to send packets from ``source1`` to the first queue and those from source2 to the second, thus the gating priority queue prioritizes packets from ``source1``. The gates are configured to open and close every 10us, with the second gate offset by a 10us period (so they alternate being open).

.. Inner queues in the GatingPriorityQueue each have their own gate. The scheduler is a PriorityScheduler, so it prioritizes packets from the first inner queue. In our case, we configure the gating priority queue to have two inner queues, and the classifier (set to ContentBasedClassifier) sends packets from source1 to the first queue and those from source2 to the second, thus the gating priority queue prioritizes packets from source1.

.. A gating priority queue looks like this/here is a gating priority queue:

We measure the end-to-end delay of packets from ``source1`` in ``sink1`` and so on. Here are the results for the four examples:

.. figure:: media/delay.png
   :align: center

.. **V1** In the case of ``NoClockDrift``, the end-to-end delay is fairly constant, as the applications send packets in sync with the opening of the gates in the queue in ``switch1``. The delay depends only on the bitrate and packet length. In the case of ``OutOfBandSynchronization`` and ``GptpSynchronization``, the clocks drift but the drift is periodically eliminated by synchronization, so the delay is fairly constant and bounded.

The delay is constant when there is no clock drift (NoClockdrift), or when the clocks are synchronized (OutOfBandSynchronization and GptpSynchronization).
In these cases, the applications send packets in sync with the opening of the gates in the queue in ``switch1``. In the no clock drift case, the delay depends only on the bitrate and packet length. In the case of ``OutOfBandSynchronization`` and ``GptpSynchronization``, the clocks drift but the drift is periodically eliminated by synchronization (see the TODO chart), so the delay remains constant. In the constant clock drift case, however, the delay keeps changing substantially compared to the other cases.

.. In the case of ``ConstandClockDrift``, it's more complicated. The delay's characteristic depends on the drift between the clocks, and the drift's direction.

.. --------

   **V1**

   In the case of ``ConstandClockDrift``, it's more complicated. The delay's characteristics depend on the magnitude and direction of the drift between the clocks.

   In this case, the clocks in ``switch1`` and the the sources have a different drift rate, and the their time keeps diverging as the simulation progresses. It might be helpful to think about the constant drift rate as time dilation. From the point of view of ``switch1``, the clock of ``source1`` is slower, so the density of the packet stream coming from ``source1`` is thinner. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``. If the frequency of arriving frames is lower than the ideal case, the frames fit in the send window.

   It might be helpful to think of the constant drift rate as time dilation. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``. 

   In the constant clock drift case, the clocks in ``switch1`` and the other network nodes have a different rate, and the their time keeps diverging as the simulation progresses. From the point of view of ``switch1``, the clock of ``source1`` is `slower`, so the density of the packet stream coming from source1 is `thinner`. If the frequency of arriving frames is lower than the ideal case, the frames fit in the send window. Similarly, from the point of view of ``switch1``, the clock of ``source2`` is `faster`, so the frame stream from ``source2`` is more dense than in the ideal case, and the frames (**TODO** isnt it one frame?) don't fit in the send window, and they have to wait for the next window. However, it also needs to wait for the previous packet to be sent as well. This means that the end-to-end delay keep increasing.

   **TODO** because of clock drift, there are two phenomena: time dilation, and diverging periods of transmissions and gates - and there is a direction ?

   In the case of ConstantClockDrift, the end-to-end delay behaves unexpectedly/weirdly. In one case, it changes constantly and resets, in the other case, it is not even bounded but keeps increasing. This is due to the magnitude and direction of the clock drift, i.e. how fast the difference between the times of a pair of clocks increases, and which of the clocks is faster than the other.

   The clock of ``source1`` is faster than that of ``switch1``, but the clock of ``source2`` is slower (so they diverge in the opposite direction?).

   So what is the reason behind these graphs? A helpful way to think about the effect of clock drift is time dilation. From the point of view of ``switch1``, ``source1``'s time if "faster", soure2's time is "slower". If there is no clock drift, the clocks keep the same time, and the sources generate packets in sync with the opening of the gates in ``switch1``'s MAC layer. Each packet is sent in it's "own" send window, i.e. the period when the corresponding gate is open.
   The period of the gates is 20us, the send window corresponding to one gate is 10us. The traffic generation period of the switches is 20us, with switch2 offsetted to correspond to the opening of the corresponding gate. The transmission time of the 800B packet is 6.4us, so only one packet can "fit" in a send window (the TODO calculates if the packet can fit in the window, and doesn't send it if it can't).

   However, when there is clock drift, the traffic generation periods and gate openings no longer match. From the point of view of ``switch1``, traffic is generated faster than 20us in ``source1``. Similarly, it is generated slower than 20us in ``source2`` (more thin or more dense packet streams).

   The clock drift causes the send windows in ``switch1`` to drift as well. For example, from the point of view of ``source1``, the gate in ``switch1`` opens a bit earlier as time passes, so packets need to wait increasingly less. At a point, the window actually opens before the packet arrives, so it doesn't have to wait in the queue at all. Then the packets start arriving just after the window closes, so they can only be sent in the next window. However, the "next window" keeps opening earlier and earlier, and so on.

   In the case of ``source2``, the window drifts in the other direction. The window opens later and later in each cycle, so packets have to wait longer and longer (hence the increasing curve). However, packets are generated faster than the windows. So in any amount of time, there are more packets than send windows. Thus, eventually, packets accumate in the queue, and the end-to-end delay increases indefinitely.

   Packets arrive late, and this lateness keeps increasing. So much that at one point, a packet is "late" with a complete period, 20us. So effectively when it arrives, the previous packet is still in the queue.

   Note that the drift rate (and time dilation) can increase so much that the send window falls below 6.4us from the point of view of ``source1``. In this case,

   **TODO** nope. because the window is in ``switch1``. this is the same as the blue line but steeper.

   --------

   **V2**

   In the case of ``ConstandClockDrift``, the delay keeps changing due to clock drift. The delay's characteristics depend on the magnitude and direction of the drift between the clocks.

   In the case of ConstantClockDrift, the end-to-end delay behaves unexpectedly/weirdly. In the case of packets going to sink2 (orange curve), it changes decreases linearly, keeps at a minimum value, then resets to a high value and starts decreasing again. In the case of sink1, it keeps increasing indefinitely. This is due to the magnitude and direction of the clock drift, i.e. how fast the difference between the times of a pair of clocks increases, and which of the clocks is faster than the other. The clock of ``source1`` is faster than that of ``switch1``, but the clock of ``source2`` is slower (so they diverge in the opposite direction? that is, one of them is late the other is early and this lateness/earliness keeps increasing).

   So what is the reason behind these graphs? A helpful way to think about the effect of clock drift is time dilation. When there is clock drift, from the point of view of ``switch1``, ``source1``'s time if "faster", ``source2``'s time is "slower". If there is no clock drift, the clocks keep the same time, and the sources generate packets in sync with the opening of the gates in ``switch1``'s MAC layer. Each packet is sent in it's "own" send window, i.e. the period when the corresponding gate is open, so the delay is constant.

   .. note:: The period of the gates is 20us, the send window corresponding to one gate is 10us. The traffic generation period of the switches is 20us, with switch2 offsetted to correspond to the opening of the corresponding gate. The transmission time of the 800B packet is 6.4us, so only one packet can "fit" in a send window (the TODO calculates if the packet can fit in the window, and doesn't send it if it can't).

   However, when there is clock drift, the traffic generation periods and gate openings no longer match. From the point of view of ``switch1``, traffic is generated faster than 20us in ``source1``. Similarly, it is generated slower than 20us in ``source2`` (more thin or more dense packet streams).

   The clock drift causes the send windows in ``switch1`` to drift as well. For example, from the point of view of ``source1``, the gate in ``switch1`` opens a bit earlier as time passes, so packets need to wait increasingly less. At a point, the window actually opens before the packet arrives, so it doesn't have to wait in the queue at all. Then the packets start arriving just after the window closes, so they can only be sent in the next window. However, the "next window" keeps opening earlier and earlier, and so on.

   In the case of ``source2``, the window drifts in the other direction. The window opens later and later in each cycle, so packets have to wait longer and longer (hence the increasing curve). However, packets are generated faster than the windows. So in any amount of time, there are more packets than send windows. Thus, eventually, packets accumate in the queue, and the end-to-end delay increases indefinitely.

   Packets arrive late, and this lateness keeps increasing. So much that at one point, a packet is "late" with a complete period, 20us. So effectively when it arrives, the previous packet is still in the queue.

   **TODO** which one? of the last two paragraphs

   .. Note that the drift rate (and time dilation) can increase so much that the send window falls below 6.4us from the point of view of source1. In this case,

      **TODO** nope. because the window is in switch1. this is the same as the blue line but steeper.

   --------

   **V3**

   In the case of ``ConstandClockDrift``, it's more complicated. The delay's characteristics depend on the magnitude and direction of the drift between the clocks.

   In this case, the clocks in ``switch1`` and the the sources have a different drift rate, and the their time keeps diverging as the simulation progresses. It might be helpful to think about the constant drift rate as time dilation. From the point of view of ``switch1``, the clock of ``source1`` is slower, so the density of the packet stream coming from ``source1`` is thinner. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``. If the frequency of arriving frames is lower than the ideal case, the frames fit in the send window.

   It might be helpful to think of the constant drift rate as time dilation. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``. 

   In the constant clock drift case, the clocks in ``switch1`` and the other network nodes have a different drift rate, and the their time keeps diverging as the simulation progresses. From the point of view of ``switch1``, the clock of ``source1`` is `slower`, so the density of the packet stream coming from source1 is `thinner`. If the frequency of arriving frames is lower than the ideal case, the frames fit in the send window. Similarly, from the point of view of ``switch1``, the clock of ``source2`` is `faster`, so the frame stream from ``source2`` is more dense than in the ideal case, and the frames (**TODO** isnt it one frame?) don't fit in the send window, and they have to wait for the next window. However, it also needs to wait for the previous packet to be sent as well. This means that the end-to-end delay keep increasing.

   **TODO** because of clock drift, there are two phenomena: time dilation, and diverging periods of transmissions and gates - and there is a direction ?

   In the case of ConstantClockDrift, the end-to-end delay behaves unexpectedly/weirdly. In one case, it changes constantly and resets, in the other case, it is not even bounded but keeps increasing. This is due to the magnitude and direction of the clock drift, i.e. how fast the difference between the times of a pair of clocks increases, and which of the clocks is faster than the other.

   The clock of ``source1`` is faster than that of ``switch1``, but the clock of ``source2`` is slower (so they diverge in the opposite direction?).

   So what is the reason behind these graphs? A helpful way to think about the effect of clock drift is time dilation. From the point of view of ``switch1``, ``source1``'s time if "faster", soure2's time is "slower". If there is no clock drift, the clocks keep the same time, and the sources generate packets in sync with the opening of the gates in ``switch1``'s MAC layer. Each packet is sent in it's "own" send window, i.e. the period when the corresponding gate is open.
   The period of the gates is 20us, the send window corresponding to one gate is 10us. The traffic generation period of the switches is 20us, with switch2 offsetted to correspond to the opening of the corresponding gate. The transmission time of the 800B packet is 6.4us, so only one packet can "fit" in a send window (the TODO calculates if the packet can fit in the window, and doesn't send it if it can't).

   However, when there is clock drift, the traffic generation periods and gate openings no longer match. From the point of view of ``switch1``, traffic is generated faster than 20us in ``source1``. Similarly, it is generated slower than 20us in ``source2`` (more thin or more dense packet streams).

   The clock drift causes the send windows in ``switch1`` to drift as well. For example, from the point of view of ``source1``, the gate in ``switch1`` opens a bit earlier as time passes, so packets need to wait increasingly less. At a point, the window actually opens before the packet arrives, so it doesn't have to wait in the queue at all. Then the packets start arriving just after the window closes, so they can only be sent in the next window. However, the "next window" keeps opening earlier and earlier, and so on.

   In the case of ``source2``, the window drifts in the other direction. The window opens later and later in each cycle, so packets have to wait longer and longer (hence the increasing curve). However, packets are generated faster than the windows. So in any amount of time, there are more packets than send windows. Thus, eventually, packets accumate in the queue, and the end-to-end delay increases indefinitely.

   Packets arrive late, and this lateness keeps increasing. So much that at one point, a packet is "late" with a complete period, 20us. So effectively when it arrives, the previous packet is still in the queue.

   Note that the drift rate (and time dilation) can increase so much that the send window falls below 6.4us from the point of view of ``source1``. In this case,

   **TODO** nope. because the window is in ``switch1``. this is the same as the blue line but steeper.

   --------

   **V4**

   So what's the reason behind these graphs? When there is no clock drift (or it is eliminated by synchronization), the end-to-end delay is constant, because the packets are generated in the sources in sync with the opening of the corresponding gates in ``switch1`` (the send windows). In the ConstantClockDrift case, however, the delay keeps changing substantially compared to the other cases. In this case, the delay's characteristics depend on the magnitude and direction of the drift between the clocks.

   It might be helpful to think of the constant drift rate as time dilation. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``, so there is no time dilation.
   In the ConstantClockDrift case, from the point of view of ``switch1``, the clock of source1 is slower than its own, and the clock of source2 is faster.
   Thus, the packet stream from source1 is thinner than in the ideal case, and the packet stream from source2 is more dense, due to time dilation.

   .. In the ConstantClockDrift case, from the point of view of switch1, source1’s time if “faster”, and source2's time is “slower” than its own.
      Thus, the packet stream from source1 is thinner than in the ideal case, and the packet stream from source2 is more dense, due to time dilation.

   If the packet stream if thinner, there are on average less packets to send than the number of send windows  in a given amount of time, so packets don't accumulate in the queue. However, due to the drifting clocks, the packet generation and the send windows are not in sync anymore, but keep shifting. Sometimes it happens that a packet arrives at the queue in switch1 when the corresponding gate is closed, so it has to wait for the next opening. This next opening happens earlier and earlier for subsequent packets (due to the relative direction of the drift in the two clocks), so packets wait less and less in the queue, hence the decreasing part of the curve. Then the curve becomes horizontal, meaning that the packets arrive when the gate is open and they can be sent immediately. After some time, the gate opening shifts again compared to the packet generation, so the packets arrive just after the gate is closed, and they have to wait a full cycle in the queue before being sent.

   If the packet stream if denser, there are more packets to send on average than there are send windows in a given amount of time, so packets eventually accumulate in the queue. This causes the delay to keep increasing indefinitely.

   .. note:: The packets are not forwarded by switch1 if they wouldn't fit in the gate opening (a packet takes 6.4us to transmit, the window is 10us long). SOMEWHERE

   Thus, if constant clock drift is not eliminated, the network can no longer guarantee any bounded delay for the packets.

   --------

   **V5**

   - in the case of ConstantClockDrift, the delay changes substantially
   - this is due to the magnitude and direction of relative drift rate between switch1, source1 and source2
   - it might be helpful to think about drift rate as time dilation from the point of view of switch1
   - when there is no drift, the send windows are in sync, and packets fit into the send window and are sent immediately (hence constant delay)
   - when there is drift, from the point of view of switch1, the packet stream from source1 is thinner than in the ideal case, the packet stream from source2 is denser
   - the thinner stream means on average, there are more send windows than packets, compared to the ideal case (in which there is an equal amount)
   - the denser stream means on average, there are more packets than send windows, so packets accumulate in the queue over time (hence the delay increases indefinatell)

   --------

What's the reason behind these graphs? When there is no clock drift (or it is eliminated by synchronization), the end-to-end delay is constant, because the packets are generated in the sources in sync with the opening of the corresponding gates in ``switch1`` (the send windows). In the constant clock drift case, the delay's characteristics depend on the magnitude and direction of the drift between the clocks.

It might be helpful to think of the constant drift rate as time dilation. In ideal conditions (no clock drift or eliminated clock drift), the clocks in all three modules keep the same time, so there is no time dilation. The packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediately by ``switch1``, so there is no time dilation.
In the constant clock drift case, from the point of view of ``switch1``, the clock of ``source1`` is slower than its own, and the clock of ``source2`` is faster. Thus, the packet stream from ``source1`` is thinner than in the ideal case, and the packet stream from ``source2`` is more dense, due to time dilation.

.. In the ConstantClockDrift case, from the point of view of switch1, source1’s time if “faster”, and source2's time is “slower” than its own.
   Thus, the packet stream from source1 is thinner than in the ideal case, and the packet stream from source2 is more dense, due to time dilation.

If the packet stream is thinner (orange graph), there are on average fewer packets to send than the number of send windows in a given amount of time, so packets don't accumulate in the queue. However, due to the drifting clocks, the packet generation and the send windows are not in sync anymore but keep shifting. Sometimes a packet arrives at the queue in ``switch1`` when the corresponding gate is closed, so it has to wait for the next opening. This next opening happens earlier and earlier for subsequent packets (due to the relative direction of the drift in the two clocks), so packets wait less and less in the queue, hence the decreasing part of the curve. Then the curve becomes horizontal, meaning that the packets arrive when the gate is open and they can be sent immediately. After some time, the gate opening shifts again compared to the packet generation, so the packets arrive just after the gate is closed, and they have to wait for a full cycle in the queue before being sent.

If the packet stream is denser (blue graph), there are more packets to send on average than there are send windows in a given amount of time, so packets eventually accumulate in the queue. This causes the delay to keep increasing indefinitely.

.. note:: The packets are not forwarded by ``switch1`` if the transmission wouldn't finish before the gate closes (a packet takes 6.4us to transmit, the gate is open for 10us).

Thus, if constant clock drift is not eliminated, the network can no longer guarantee any bounded delay for the packets.


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ClockDriftShowcase.ned <../ClockDriftShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

