Eager Gate Schedule Configuration
=================================

Goals
-----

This showcase demonstrates how the eager gate schedule configurator can
automatically set up transmission gate schedules in a Time-Sensitive Networking
(TSN) environment. The configurator efficiently allocates time slots for
different traffic classes to ensure deterministic latency guarantees while
maximizing network utilization.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/gatescheduling/eager <https://github.com/inet-framework/inet/tree/master/showcases/tsn/gatescheduling/eager>`__

Background
----------

Time-Sensitive Networking (TSN) is a set of IEEE 802.1 standards that provide
deterministic communication over Ethernet networks. One of the key features in
TSN is the Time-Aware Shaper (TAS), defined in IEEE 802.1Qbv.

**Time-Aware Shaper (TAS)**

The Time-Aware Shaper is a traffic shaping mechanism that uses time-based
transmission gates to control when specific traffic classes can transmit. The
key components of TAS include:

1. **Transmission Gates**: Each traffic queue has an associated gate that can be opened or closed according to a schedule
2. **Gate Control Lists (GCLs)**: Define the opening and closing times of gates in a repeating cycle
3. **Time Synchronization**: Ensures that all network devices follow the same schedule

By controlling when different traffic classes can transmit, TAS provides:

- **Deterministic Latency**: Guaranteed maximum end-to-end delay for critical traffic
- **Traffic Isolation**: Prevention of interference between different traffic classes
- **Bandwidth Allocation**: Controlled sharing of network resources

**Gate Scheduling**

Gate scheduling is the process of determining when each transmission gate should
be open or closed. This is a complex optimization problem that needs to
consider:

1. **Traffic Patterns**: The size and frequency of packets in each traffic class
2. **Latency Requirements**: The maximum allowed delay for each traffic class
3. **Network Topology**: The path that packets take through the network
4. **Bandwidth Utilization**: Efficient use of available network capacity

Manual configuration of gate schedules is impractical for all but the simplest
networks, making automatic configurators essential for real-world deployments.

**Eager Gate Scheduling**

The eager gate scheduling algorithm is a greedy approach that:

1. Sorts traffic streams by priority or other criteria
2. Allocates time slots to each stream as early as possible in the schedule
3. Ensures that allocated slots satisfy the latency requirements
4. Maximizes network utilization by minimizing idle time

This approach is "eager" because it attempts to schedule transmissions as soon
as possible, rather than delaying them unnecessarily. While not always optimal,
eager scheduling provides a good balance between simplicity and effectiveness.

The Model
---------

The simulation uses a dumbbell network topology with two clients, two switches,
and two servers. Each client generates two types of traffic (best effort and
video) that are sent to the corresponding servers through the switches.

**Network Topology**

.. figure:: media/Network.png
    :align: center

The network consists of:

1. **Client1 and Client2**: Generate both best effort and video traffic
2. **Switch1 and Switch2**: Forward traffic and implement time-aware shaping
3. **Server1**: Receives best effort traffic from both clients
4. **Server2**: Receives video traffic from both clients

All links operate at 100 Mbps.

**Traffic Configuration**

The clients generate two types of traffic with different characteristics:

1. **Best Effort Traffic**:
   - Packet size: 1000 bytes (plus 58 bytes of headers)
   - Production interval: 500 μs (~16 Mbps per client)
   - Priority Code Point (PCP): 0
   - Maximum latency requirement: 500 μs

2. **Video Traffic**:
   - Packet size: 500 bytes (plus 58 bytes of headers)
   - Production interval: 250 μs (~16 Mbps per client)
   - Priority Code Point (PCP): 4
   - Maximum latency requirement: 500 μs

**Time-Aware Shaping Configuration**

The switches implement time-aware shaping with two traffic classes, each with
its own queue and transmission gate:

1. **Gate 0**: For best effort traffic (PCP 0)
2. **Gate 1**: For video traffic (PCP 4)

The gate cycle duration is set to 1 ms, and the gates are scheduled to open and
close according to the schedule determined by the eager gate schedule
configurator.

**Eager Gate Schedule Configurator**

The :ned:`EagerGateScheduleConfigurator` automatically configures the gate
schedules based on the traffic specifications and latency requirements. It takes
the following parameters:

- :par:`gateCycleDuration`: The duration of one complete gate cycle (1 ms)
- :par:`configuration`: Traffic specifications including PCP, gate index, packet length, packet interval, and maximum latency

Here is the complete configuration:

.. literalinclude:: ../omnetpp.ini
    :language: ini
    :start-at: [Eager]

**Stream Identification and Encoding**

The clients identify outgoing traffic based on UDP destination ports and map them to streams:

- UDP ports 1000 and 1001: Best effort stream
- UDP ports 1002 and 1003: Video stream

These streams are then encoded with the appropriate PCP values:

- Best effort stream: PCP 0
- Video stream: PCP 4

The switches decode incoming traffic based on PCP values, process it according
to the gate schedule, and encode it again with the same PCP values for
forwarding.

Results
-------

The simulation results demonstrate how the eager gate schedule configurator
effectively allocates time slots to meet the latency requirements of both
traffic classes.

**Gate Schedule Visualization**

A gate cycle duration of 1ms is displayed on the following sequence chart. The
colored bars represent when each gate is open, allowing the corresponding
traffic class to transmit:

.. figure:: media/seqchart.png
    :align: center

The sequence chart shows how the eager gate schedule configurator has allocated
time slots for the different traffic classes. Note the efficiency of the
schedule, with minimal idle time between transmissions. The gates open just in
time to transmit packets from their respective queues, ensuring that packets
flow from the sources to the sinks with minimal delay.

**Packet Delay Analysis**

The following figure shows the delay for the second packet of ``client2`` in the
best effort traffic class, from the packet source to the packet sink:

.. figure:: media/timediff.png
    :align: center

This packet experiences a delay of approximately 480 μs, which is within the 500
μs requirement but close to the limit. This is typical of the eager scheduling
approach, which tends to push delays close to their maximum allowed values in
order to maximize network utilization.

**End-to-End Delay Distribution**

The following chart displays the end-to-end delay for individual packets of the
different traffic classes:

.. figure:: media/delay.png
    :align: center
    :width: 90%

The chart shows that:

1. All delays are within the specified 500 μs constraint
2. Video streams (red and green) have lower delays than best effort streams (blue and yellow)
3. Some traffic classes show two distinct delay clusters

The presence of two cluster points for video streams and the ``client2 best
effort`` stream is due to these traffic classes having multiple packets per gate
cycle. As the different flows interact in the network, some packets experience
increased delay due to queuing behind other packets.

**Analysis**

The results demonstrate several key aspects of eager gate scheduling:

1. **Deterministic Latency**: All packets meet their latency requirements
2. **Prioritization**: Higher priority traffic (video) generally experiences lower delays
3. **Efficiency**: The schedule maximizes network utilization with minimal idle time
4. **Automatic Configuration**: The configurator successfully generates a valid schedule without manual intervention

Practical Applications
---------------------

Eager gate scheduling is particularly valuable in:

1. **Industrial Automation**: Where deterministic communication is required for control systems
2. **Automotive Networks**: In-vehicle networks with mixed-criticality traffic
3. **Audio/Video Bridging**: Professional audio and video systems requiring synchronized delivery
4. **Building Automation**: Control systems with real-time requirements

These applications benefit from:

- **Determinism**: Guaranteed maximum latency for critical traffic
- **Coexistence**: Allowing different traffic classes to share the same network
- **Simplicity**: Automatic configuration without complex optimization
- **Scalability**: Applicable to networks of various sizes and complexities

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

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/gatescheduling/eager && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Further Experiments
------------------

To deepen your understanding of gate scheduling, consider experimenting with the following modifications:

1. **Different Traffic Patterns**: Modify the packet sizes and production intervals to see how they affect the schedule and delays.

2. **Different Latency Requirements**: Change the maximum latency constraints to see how they impact the schedule.

3. **More Traffic Classes**: Add additional traffic classes with different priorities and requirements.

4. **Different Gate Cycle Duration**: Modify the gate cycle duration to see how it affects the schedule's efficiency.

5. **Comparison with Other Scheduling Algorithms**: Compare the eager scheduling approach with other algorithms like SAT-based scheduling (available in the `sat` showcase).

References
----------

1. `IEEE 802.1Qbv-2015 <https://standards.ieee.org/ieee/802.1Qbv/6243/>`__ - Enhancements for Scheduled Traffic

2. `IEEE 802.1Q-2018 <https://standards.ieee.org/ieee/802.1Q/7098/>`__ - IEEE Standard for Local and Metropolitan Area Networks--Bridges and Bridged Networks

3. Craciunas, S. S., Oliver, R. S., Chmelík, M., & Steiner, W. (2016). `Scheduling real-time communication in IEEE 802.1Qbv time sensitive networks <https://ieeexplore.ieee.org/document/7557870>`__. In Proceedings of the 24th International Conference on Real-Time Networks and Systems (pp. 183-192).

4. Nasrallah, A., Thyagaturu, A. S., Alharbi, Z., Wang, C., Shao, X., Reisslein, M., & ElBakoury, H. (2019). `Ultra-Low Latency (ULL) Networks: The IEEE TSN and IETF DetNet Standards and Related 5G ULL Research <https://ieeexplore.ieee.org/document/8458130>`__. IEEE Communications Surveys & Tutorials, 21(1), 88-145.

5. Finn, N. (2018). `Introduction to Time-Sensitive Networking <https://ieeexplore.ieee.org/document/8412457>`__. IEEE Communications Standards Magazine, 2(2), 22-28.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/791>`__ page in the GitHub issue tracker for commenting on this showcase.

