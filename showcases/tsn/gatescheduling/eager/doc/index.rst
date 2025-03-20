Eager Gate Schedule Configuration
=================================

Goals
-----

.. This showcase focuses on the EagerGateScheduleConfigurator, demonstrating how it
.. can automatically configure gate schedules in a simple network to meet specified
.. latency requirements.

This showcase demonstrates the EagerGateScheduleConfigurator, one of several gate scheduling 
approaches available in INET for Time-Sensitive Networking (TSN). The "eager" approach uses a greedy algorithm that allocates time slots as early 
as possible while respecting constraints. We show how this configurator
can automatically generate gate schedules in a simple network to meet specified latency 
requirements.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/eager <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/eager>`__

.. Background
.. ----------

.. Time-Aware Shapers and Gate Scheduling
.. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. Time-Aware Shapers (TAS) are a key component of Time-Sensitive Networking (TSN)
.. that enable deterministic communication by controlling when different traffic
.. classes can transmit. For a detailed explanation of Time-Aware Shapers, please
.. refer to the `Time-Aware Shaper showcase
.. <../../../trafficshaping/timeawareshaper/doc/index.html>`_.

.. Gate scheduling is the process of determining when each transmission gate in a
.. Time-Aware Shaper should open and close. In TSN networks, these schedules need
.. to be carefully coordinated across multiple network devices to ensure end-to-end
.. deterministic behavior. A well-designed gate schedule ensures that:

.. - Each traffic class has its own time window
.. - End-to-end latency requirements are met
.. - Network resources are efficiently utilized

.. While simple networks can have manually configured gate schedules, this becomes
.. impractical in larger networks with multiple switches and complex traffic
.. patterns. This is where gate schedule configurators come in.

.. .. _gate-schedule-configurators:

Gate Schedule Configurators
---------------------------

In time-aware shaping, gate schedules (i.e. when the gates corresponding to
different traffic categories are open or closed) can be specified manually. This
might be sufficient in some simple cases. However, in complex cases, manual
calculation of gate schedules may be impossible, thus automation may be
required. Gate schedule configurators can be used for this purpose.

One needs to specify constraints for the different traffic categories, such as maximum delay,
and the configurator automatically calculates and configures the gate schedules that satisfy
these constraints.

All gate schedule configurators in INET 
inherit from GateScheduleConfiguratorBase and share a common configuration approach:

1. **INI File Parameters**:

   - ``gateCycleDuration``: Defines the length of the repeating gate cycle (e.g., 1ms)
   - ``configuration``: An array of flow specifications that define the traffic requirements

.. _flow-spec:

2. **Flow Specification**:

   Each flow in the :par:`configuration` array is specified by these parameters:

   - ``name``: Optional name for the flow (for identification purposes). If not provided, a default name like "flow0" will be generated.
   - ``source`` and ``destination``: Pattern strings matching the source and destination modules
   - ``application``: Path to the application module generating the traffic. This is optional in the configuration but required if you want the configurator to set application start times.
   - ``pcp``: Priority Code Point value for the traffic class
   - ``gateIndex``: Index of the transmission gate to use
   - ``packetLength``: Size of packets including protocol overhead
   - ``packetInterval``: Time between packet generation
   - ``maxLatency``: Maximum allowed end-to-end delay (optional)
   - ``maxJitter``: Maximum allowed jitter (optional)
   - ``pathFragments``: Custom path specification (optional, otherwise shortest path is used)

3. **Configuration Process**:

   The configurator automatically:

   - Extracts the network topology
   - Identifies devices, switches, and ports
   - Creates flow objects based on the configuration
   - Computes gate schedules according to the specific algorithm
   - Configures the ``durations`` parameters of PeriodicGate modules
   - Sets application start times to coordinate with the gate schedule

This configuration approach allows you to specify your traffic requirements 
declaratively, and the configurator handles the complex task of computing a valid 
schedule that meets those requirements.

The Model
---------

The Eager Gate Schedule Configurator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The EagerGateScheduleConfigurator implements a greedy algorithm for automatic gate 
scheduling in Time-Sensitive Networks. As the name suggests, it takes an "eager" 
approach, attempting to allocate time slots as early as possible while respecting 
constraints.

The algorithm works through the following steps:

1. It computes a start offset for each stream to avoid initial conflicts
2. It allocates gate openings for each packet in the cycle
3. When it encounters conflicts (two packets needing the same resource at the 
   same time), it shifts the later packet in time
4. It verifies that all latency requirements can be met with the generated schedule

This approach is computationally efficient and straightforward, but it has 
important limitations. Since it makes local decisions without backtracking, the 
eager configurator may fail to find a valid schedule even when one exists. If 
early scheduling decisions lead to conflicts later that cannot be resolved within 
the specified constraints, the algorithm will fail.

Additionally, even when the algorithm does find a valid solution, it may not be
optimal. The greedy nature of the algorithm means it prioritizes earlier flows,
potentially leading to suboptimal use of network resources or unnecessarily high
latency for some flows. For example, if the algorithm eagerly schedules an early
flow in a way that blocks a critical time slot needed by a later flow with tight
latency constraints, it won’t go back and reconsider the earlier decision. A
more sophisticated algorithm might find solutions with better overall latency
characteristics or more efficient resource utilization.

.. Despite these limitations, the eager approach works well for many network 
.. configurations, especially those with sufficient bandwidth and reasonable latency 
.. requirements. It provides a good balance between simplicity and effectiveness for 
.. many practical TSN applications. TODO is this even needed?

Network Configuration
~~~~~~~~~~~~~~~~~~~~~

The simulation uses a dumbbell network topology with two clients (:ned:`TsnDevice`), two switches (:ned:`TsnSwitch`), and 
two servers (:ned:`TsnDevice`). All links are 100Mbps Ethernet connections:

.. figure:: media/Network.png
    :align: center

In this network:

- Two clients (client1 and client2) are connected to switch1
- Two servers (server1 and server2) are connected to switch2
- The switches are connected to each other
- All traffic flows from clients to servers through both switches

The configurator will calculate the gate schedules of the time-aware shapers in
the outgoing interfaces of switch1 and switch2

Traffic Configuration
~~~~~~~~~~~~~~~~~~~~

Each client generates two types of traffic:

1. **Best effort traffic** (PCP 0):

   - 1000-byte packets sent every 500μs (~16Mbps)
   - Sent from client1 to server1 and from client2 to server1

2. **Video traffic** (PCP 4):

   - 500-byte packets sent every 250μs (~16Mbps)
   - Sent from client1 to server2 and from client2 to server2

The traffic is classified based on destination port and assigned appropriate PCP 
values using stream identification and stream encoding in the clients.

Gate Scheduling Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The EagerGateScheduleConfigurator is configured with:

- A gate cycle duration of 1ms
- Maximum latency requirement of 500μs for all flows
- Two traffic classes with corresponding gates (gate index 0 for best effort, 
  gate index 1 for video)

Here is the automatic gate scheduling configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini
    :start-at: # automatic gate scheduling
    :end-before: # gate scheduling visualization

The configuration includes a detailed specification of each flow with its parameters:

- PCP values and gate indices
- Source and destination
- Packet length (including protocol overhead)
- Packet interval
- Maximum allowed latency

Note the comment explaining the protocol overhead calculation (58B = UDP + IP + 802.1Q-TAG + 
ETH MAC + ETH FCS + ETH PHY), which is added to the base packet length for accurate timing 
calculations.

Results
-------

The configurator calculates the gate schedules and set the :par:`durations` parameters of the
time-aware shaper's gates. For example, here is the calculated gate schedule of ``gate[0]``:

.. figure:: media/gate0.png
   :align: center

Sequence Chart Analysis
~~~~~~~~~~~~~~~~~~~~~~

The following sequence chart shows the packet flow through the network over a gate 
cycle duration of 1ms:

.. figure:: media/seqchart.png
    :align: center

While the schedule meets all latency requirements, it's important to note that
it may not be optimal. The eager algorithm prioritizes earlier flows and makes
local decisions without considering the global optimization of the schedule.

Delay Analysis
~~~~~~~~~~~~~

The following figure shows the delay for the second packet of ``client2`` in the 
best effort traffic class, from the packet source to the packet sink:

.. figure:: media/timediff.png
    :align: center

This stream is the outlier in the sequence chart. While its delay is within the 
500μs requirement, it comes quite close to this limit. This demonstrates a key 
limitation of the eager approach: it prioritizes earlier flows, potentially pushing 
later flows closer to their latency constraints.

The next chart displays the delay for individual packets of all traffic categories:

.. figure:: media/delay.png
    :align: center
    :width: 90%

Key observations from this chart:

1. All delays are within the specified 500μs constraint, demonstrating that the 
   EagerGateScheduleConfigurator successfully met all requirements.

2. Both video streams and the ``client2 best effort`` stream show two distinct 
   cluster points. This is a direct result of the suboptimal scheduling produced by 
   the eager algorithm. Since the algorithm processes packets sequentially without 
   global optimization, earlier packets in a cycle receive preferential treatment 
   while later packets encounter more resource conflicts and experience significantly 
   higher delays.

This result validates that the eager approach can produce working schedules for 
this network configuration. However, it also illustrates the trade-off between 
simplicity and optimality. While the schedule meets all requirements, a more 
sophisticated algorithm might produce a schedule with more evenly distributed 
delays or better overall network utilization.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/gatescheduling/eager`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/gatescheduling/eager && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/791>`__ page in the GitHub issue tracker for commenting on this showcase.
