Clock Drift
===========

Goals
-----

In this example we demonstrate the effects of clock drift and time synchronization
on network behavior. We show how to introduce local clocks in network nodes, how
to configure clock drift for them, and how to use time synchronization mechanisms
to reduce time differences.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/timesynchronization/clockdrift <https://github.com/inet-framework/inet-showcases/tree/master/tsn/timesynchronization/clockdrift>`__

Introduction
------------

Clocks
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
- :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modelling `random clock drift`

For creating drifting clocks, the various clock and oscillator models can be combined.

   **TODO** another angle? like so:

   By default, network nodes and modules don't have clocks
   To simulate effect of each module having its own local time, the nodes can have their own clock submodules
   Here is how it works: most clock modules have oscillator submodules; the clocks keep time by counting oscillator ticks. The type of the oscillator module used and its configuration results in the modelled clock drift.

   Also, for simulating time synchronization protocols, clocks need to be settable. Settable clocks have synchornizer submodules, which can set the time of the clock on clue.
   The clue may be an out-of-band synchronization mechanism, or the effect of receiving timing protocol messages from other nodes, for example.

--------------------------------------

Simulating clock drift with clocks, oscillators and Synchronizers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Simulating clock drift and time synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, modules such as network nodes and interfaces in INET don't have local time, but use the simulation time as a global time. To simulate network nodes with local time, and effects such as clock drift and its mitigation (time synchronization), network nodes need to have `clock` submodules. To keep time, most clock module types use oscillator submodules. The oscillators produce ticks in certain intervals, and the clock modules count the ticks. The clock drift is produced by oscillators that have a configurable drift in tick generation speed.
Thus by combining clock and oscillator models, we can simulate `constant` and `random` clock drift. To model time synchronization, the time of clocks can be set by `synchronizer` submodules.

Several clock models are available:

- :ned:`OscillatorBasedClock`: Has an `oscillator` submodule; keeps time by counting the oscillator's ticks. Depending on the oscillator submodule, can model `constant` and `random clock drift`
- :ned:`SettableClock`: Same as the oscillator-based clock but the time can be set from C++ or from a scenario manager script
- :ned:`IdealClock`: The clock's time is the same as the simulation time; for testing purposes

The following oscillator models are available:

- :ned:`IdealOscillator`: generates ticks periodically with a constant tick length
- :ned:`ConstantDriftOscillator`: generates ticks periodically with a constant drift in generation speed; for modeling `constant clock drift`
- :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modelling `random clock drift`

--------------------------------------

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
- :ned:`RandomDriftOscillator`: the oscillator changes drift rate over time; for modelling `random clock drift`

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

The example configurations are the following:

- ``NoClockDrift``: Network nodes don't have clocks, they are synchronized by simulation time
- ``ConstantClockDrift``: Network nodes have clocks with random constant clock drift, and the clocks diverge over time
- ``OutOfBandSynchronization``: Clocks have random constant clock drift, and they are synchronized by an out-of-band synchronization method (without a real protocol)
- ``GptpSynchronization``: Clocks have random constant clock drift, and they are synchronized by gPTP

The simulations use the following network:

.. figure:: media/Network2.png
   :align: center

In the ``General`` configuration, ``source1`` is configured to send UDP packets to ``sink1``, and ``source2`` to ``sink2``. All Ethernet interfaces are configured to be layered (:ned:`LayeredEthernetInterface`).

.. note:: To demonstrate the effects of drifting clocks, we configure the Ethernet MAC layer in switch1 to alternate between forwarding frames from ``switch1`` and ``switch2`` every 10 us. More details about this are provided in the **TODO** section.

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

In this configuration network nodes don't have clocks. Applications and gate
schedules are synchronized by simulation time.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: NoClockDrift
   :end-before: ConstantClockDrift

Example: Constant Clock Drift
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration all network nodes have a clock with a random constant drift.
Clocks drift away from each other over time.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: ConstantClockDrift
   :end-before: OutOfBandSynchronization

Here are the results:

.. figure:: media/ConstantClockDrift.png
   :align: center

Example: Out-of-Band Synchronization of Clocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration the network node clocks are periodically synchronized by an
out-of-band mechanism that doesn't use the underlying network.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: OutOfBandSynchronization
   :end-before: GptpSynchronization

Here are the results:

.. figure:: media/OutOfBandSync2.svg
   :align: center
   :width: 100%

Example: Synchronizing Clocks using gPTP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration the clocks in network nodes are periodically synchronized
to a master clock using the Generic Precision Time Protocol (gPTP). The time
synchronization protocol measures the delay of individual links and disseminates
the clock time of the master clock on the network through a spanning tree.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: GptpSynchronization

Here are the results:

.. figure:: media/Gptp.svg
   :align: center
   :width: 100%

.. Results
   -------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

.. Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%

Effects of Clock Drift on End-to-end Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. **TODO** WHY are we doing this?

This section aims to demonstrate that clock drift can have profound effects on the operation of the network. To this end, in all simulations, the Ethernet MAC layer in ``switch1`` is configured to alternate forwarding packets from ``source1`` and ``source2`` every 10 us; note that the UDP applications are sending packets every 20 us, with packets from source2 offset by 10 us compared to source1.

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

Here are the results:

.. figure:: media/delay.png
   :align: center

In the case of ``NoClockDrift``, the end-to-end delay is fairly constant, as the applications send packets in sync with the opening of the gates in the queue in ``switch1``. The delay depends only on the bitrate and packet length. In the case of ``OutOfBandSynchronization`` and ``GptpSynchronization``, the clock drift but the drift is periodically eliminated by synchronization, so the delay is fairly constant and bounded.

In the case of ``ConstandClockDrift``, it's more complicated. The delay's characteristic depends on the drift between the clocks, and the drift's direction.

In this case, the clocks in switch1 and the other network nodes have a different rate, and the their time keeps diverging as the simulation progresses. It might be easy to think of the constant drift rate as time dilation. From the point of view of switch1, the clock of source1 is slower, so the density of the packet stream is thinner. In ideal conditions (no clock drift), the packets in both sources are generated in sync with the send windows (when the corresponding gate is open), and they are forwarded immediatelly by switch1. If the frequency of arriving frames is lower than the ideal case, the frames fit in the send window.

**TODO** because of clock drift, there are two phenomena: time dilation, and diverging periods of transmissions and gates - and there is a direction ?


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ClockDriftShowcase.ned <../ClockDriftShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

