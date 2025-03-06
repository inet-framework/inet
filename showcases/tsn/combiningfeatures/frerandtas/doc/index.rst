Frame Replication with Time-Aware Shaping
=========================================

Goals
-----

In this example, we demonstrate how to automatically configure time-aware
shaping in the presence of frame replication in Time-Sensitive Networking (TSN).
This showcase illustrates how these two critical TSN features can work together
to provide both reliability and deterministic latency guarantees.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/frerandtas <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/frerandtas>`__

Background
----------

Time-Sensitive Networking (TSN) is a set of IEEE 802.1 standards that provide
deterministic communication over Ethernet networks. Two key features in TSN are
Frame Replication and Elimination for Reliability (FRER) and Time-Aware Shaper
(TAS).

**Frame Replication and Elimination for Reliability (FRER)**

FRER is defined in IEEE 802.1CB and provides seamless redundancy by:

1. **Frame Replication**: Duplicating frames and sending them over multiple disjoint paths
2. **Frame Elimination**: Removing duplicate frames at merge points in the network
3. **Sequence Recovery**: Using sequence numbers to identify and eliminate duplicates

This approach ensures that if one path experiences a failure or excessive delay,
the frame can still reach its destination through an alternative path,
significantly improving reliability.

**Time-Aware Shaper (TAS)**

TAS is defined in IEEE 802.1Qbv and provides deterministic latency by:

1. **Gate Control Lists**: Defining when specific traffic queues can transmit
2. **Time-Based Scheduling**: Synchronizing transmission windows across the network
3. **Traffic Isolation**: Preventing interference between different traffic classes

TAS enables precise scheduling of frame transmissions, ensuring that
high-priority frames have guaranteed transmission opportunities without
interference from lower-priority traffic.

**Combining FRER and TAS**

When combining these features, several challenges arise:

1. **Scheduling Complexity**: Replicated frames need coordinated transmission windows
2. **Path Diversity**: Different paths may have different latency characteristics
3. **Resource Allocation**: Redundant paths consume additional bandwidth
4. **Configuration Complexity**: Manual configuration becomes impractical

Automatic configuration tools are essential to address these challenges, ensuring that:

1. Gate schedules accommodate all replicated frames
2. Timing constraints are met despite path diversity
3. Resources are efficiently allocated
4. Configuration remains consistent across the network

The Model
---------

This showcase demonstrates a network with redundant paths and automatic configuration of both FRER and TAS features.

Here is the network topology:

.. figure:: media/Network.png
   :align: center

The network consists of the following components:

1. **Source**: A TSN-capable device generating periodic traffic
2. **Destination**: A TSN-capable device receiving the traffic
3. **Switches**: Five TSN-capable switches (s1, s2a, s2b, s3a, s3b) forming redundant paths
4. **Links**: Ethernet connections with a 20% packet error rate to simulate unreliable conditions

**Network Topology**

The network is arranged to provide two main paths from source to destination:

1. **Path 1**: source → s1 → s2a → s3a → destination
2. **Path 2**: source → s1 → s2b → s3b → destination

Additionally, there's a cross-connection between s2a and s2b, allowing for:

3. **Path 3**: source → s1 → s2a → s2b → s3b → destination
4. **Path 4**: source → s1 → s2b → s2a → s3a → destination

This topology provides multiple redundant paths, allowing the FRER mechanism to
ensure reliable delivery even when some links or nodes fail.

**Traffic Configuration**

The source generates UDP packets with the following characteristics:

- Packet size: 100 bytes (plus 64 bytes of headers)
- Production interval: 5 milliseconds (200 packets per second)
- Destination: The destination device's UDP application

**TSN Features Configuration**

The showcase uses several automatic configurators to set up the TSN features:

1. **StreamRedundancyConfigurator**: Configures frame replication and elimination
2. **FailureProtectionConfigurator**: Specifies protection requirements against node and link failures
3. **EagerGateScheduleConfigurator**: Automatically generates gate schedules for time-aware shaping

The configuration specifies:

- A gate cycle duration of 20ms
- Maximum latency requirement of 100μs
- Protection against any single node failure among s2a, s2b, s3a, or s3b
- Protection against link failures with specific redundancy requirements

Here is the complete configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [General]

**Network Definition**

The network is defined in the NED file:

.. literalinclude:: ../FrerAndTasShowcase.ned
   :language: ned
   :start-at: package inet

Results
-------

The simulation results demonstrate how the combination of frame replication and
time-aware shaping provides reliable and deterministic communication despite the
unreliable links (20% packet error rate).

**Packets Received vs. Sent**

.. figure:: media/packetsreceivedsent.png
   :align: center
   :width: 100%

This figure shows the cumulative number of packets sent by the source (blue
line) and received by the destination (red line) over time. Despite the high
packet error rate, the frame replication mechanism ensures that most packets
reach the destination. The slight gap between sent and received packets
represents the small percentage of cases where all replicated frames were lost.

**Packet Delivery Ratio**

.. figure:: media/packetratio.png
   :align: center

This figure shows the ratio of received to sent packets over time. After an
initial stabilization period, the ratio remains consistently high (above 95%),
demonstrating the effectiveness of the frame replication mechanism in providing
reliability. Without frame replication, we would expect a delivery ratio of only
about 80% given the 20% packet error rate on each link.

**Analysis**

The results demonstrate several key aspects of combining FRER and TAS:

1. **Enhanced Reliability**: Frame replication significantly improves packet delivery despite the high error rate
2. **Deterministic Latency**: Time-aware shaping ensures that packets meet their latency requirements
3. **Automatic Configuration**: The configurators successfully coordinate both features
4. **Failure Protection**: The system maintains performance despite simulated link and node failures

The slight variation in the packet ratio over time is due to the probabilistic nature of the errors and the specific paths taken by the replicated frames.

Practical Applications
---------------------

The combination of frame replication and time-aware shaping is particularly valuable in:

1. **Industrial Automation**: Where both reliability and deterministic timing are critical
2. **Automotive Networks**: In-vehicle networks requiring high reliability for safety-critical functions
3. **Power Substations**: Where protection and control messages must be delivered reliably with bounded latency
4. **Aerospace Systems**: Where redundancy and timing guarantees are essential for safety

These applications benefit from:

- **Reliability**: Ensuring critical messages reach their destination
- **Determinism**: Guaranteeing bounded latency for real-time control
- **Resilience**: Maintaining operation despite link or node failures
- **Automatic Configuration**: Reducing the complexity of network setup and maintenance

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`FrerAndTasShowcase.ned <../FrerAndTasShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/combiningfeatures/frerandtas`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/combiningfeatures/frerandtas && inet'

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

To deepen your understanding of frame replication and time-aware shaping, consider experimenting with the following modifications:

1. **Vary the Packet Error Rate**: Change the ``per`` parameter to see how different error rates affect the delivery ratio.

2. **Modify the Network Topology**: Add or remove links to create different redundant paths and observe the impact on reliability.

3. **Change the Traffic Pattern**: Modify the packet size and production interval to see how they affect the system's performance.

4. **Adjust Protection Requirements**: Modify the failure protection configuration to require different levels of redundancy.

5. **Disable Features**: Run simulations with either FRER or TAS disabled to understand their individual contributions.

References
----------

1. `IEEE 802.1CB-2017 <https://standards.ieee.org/ieee/802.1CB/6844/>`__ - Frame Replication and Elimination for Reliability

2. `IEEE 802.1Qbv-2015 <https://standards.ieee.org/ieee/802.1Qbv/6243/>`__ - Enhancements for Scheduled Traffic

3. `IEEE 802.1Qcc-2018 <https://standards.ieee.org/ieee/802.1Qcc/6825/>`__ - Stream Reservation Protocol (SRP) Enhancements and Performance Improvements

4. Nasrallah, A., Thyagaturu, A. S., Alharbi, Z., Wang, C., Shao, X., Reisslein, M., & ElBakoury, H. (2019). `Ultra-Low Latency (ULL) Networks: The IEEE TSN and IETF DetNet Standards and Related 5G ULL Research <https://ieeexplore.ieee.org/document/8458130>`__. IEEE Communications Surveys & Tutorials, 21(1), 88-145.

5. Kehrer, S., Kleineberg, O., & Heffernan, D. (2014). `A comparison of fault-tolerance concepts for IEEE 802.1 Time Sensitive Networks (TSN) <https://ieeexplore.ieee.org/document/6899165>`__. IEEE Emerging Technology and Factory Automation (ETFA), 1-8.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/783>`__ page in the GitHub issue tracker for commenting on this showcase.
