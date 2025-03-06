Manual Stream Configuration
===========================

Goals
-----

In this example, we demonstrate manual configuration of stream identification,
stream splitting, stream merging, stream encoding, and stream decoding to
achieve frame replication and elimination for reliability in Time-Sensitive
Networking (TSN). This showcase illustrates how to manually set up redundant
paths for critical traffic streams without relying on automatic configurators.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/framereplication/manualconfiguration <https://github.com/inet-framework/inet/tree/master/showcases/tsn/framereplication/manualconfiguration>`__

Background
----------

Time-Sensitive Networking (TSN) is a set of IEEE 802.1 standards that provide
deterministic communication over Ethernet networks. One of the key features in
TSN is Frame Replication and Elimination for Reliability (FRER), defined in IEEE
802.1CB.

**Frame Replication and Elimination for Reliability (FRER)**

FRER provides seamless redundancy by:

1. **Frame Replication**: Duplicating frames and sending them over multiple disjoint paths
2. **Frame Elimination**: Removing duplicate frames at merge points in the network
3. **Sequence Recovery**: Using sequence numbers to identify and eliminate duplicates

This approach ensures that if one path experiences a failure or excessive delay,
the frame can still reach its destination through an alternative path,
significantly improving reliability.

**Stream Identification and Handling**

To implement FRER, several stream handling functions are required:

1. **Stream Identification**: Classifying frames into streams based on various criteria
2. **Stream Splitting**: Duplicating frames from one stream into multiple streams
3. **Stream Merging**: Combining multiple streams into one, eliminating duplicates
4. **Stream Encoding**: Mapping streams to specific VLAN tags or other identifiers
5. **Stream Decoding**: Extracting stream information from frame headers

**Manual vs. Automatic Configuration**

While INET provides automatic configurators for stream redundancy (as shown in
other showcases), manual configuration offers several advantages:

1. **Fine-grained Control**: Precise control over how streams are identified, split, and merged
2. **Custom Topologies**: Support for complex network topologies with specific redundancy requirements
3. **Educational Value**: Better understanding of the underlying mechanisms
4. **Compatibility**: Works with legacy devices or custom protocols

However, manual configuration is more complex and error-prone, especially in large networks.

The Model
---------

In this configuration, we replicate a network topology that is presented in the
IEEE 802.1CB standard. The network contains one source and one destination node,
where the source sends a redundant data stream through five switches. The stream
is duplicated in three of the switches and merged in two of them.

**Network Topology**

Here is the network:

.. figure:: media/Network.png
   :align: center

The network consists of the following components:

1. **Source**: A TSN-capable device generating traffic
2. **Destination**: A TSN-capable device receiving the traffic
3. **Switches**: Five TSN-capable switches (s1, s2a, s2b, s3a, s3b) forming redundant paths
4. **Links**: Ethernet connections with 100 Mbps capacity

The network is arranged to provide multiple redundant paths from source to destination:

1. **Path 1**: source → s1 → s2a → s3a → destination
2. **Path 2**: source → s1 → s2b → s3b → destination
3. **Path 3**: source → s1 → s2a → s2b → s3b → destination
4. **Path 4**: source → s1 → s2b → s2a → s3a → destination

This topology provides multiple redundant paths, allowing the FRER mechanism to
ensure reliable delivery even when some links fail.

**Traffic Configuration**

The source generates UDP packets with the following characteristics:

- Packet size: 1200 bytes
- Production interval: truncated normal distribution with mean 100μs and standard deviation 50μs
- Destination: The destination device's UDP application

**Link Failures**

To demonstrate the reliability benefits of frame replication, the simulation includes two link failures:

1. At t=0.1s: The link between s1 and s2a fails
2. At t=0.2s: The link between s2b and s3b fails

These failures are configured using the ScenarioManager:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: scenarioManager.script
   :end-at: </scenario>")

**Stream Configuration**

The manual configuration of stream handling is the core of this showcase. Here's how the streams are configured:

1. **Source Node**:
   - Identifies all outgoing traffic as stream "s1"
   - Enables sequence numbering for stream "s1"
   - Encodes stream "s1" to VLAN 1

2. **Switch s1**:
   - Decodes incoming VLAN 1 traffic from source as stream "s1"
   - Splits stream "s1" into streams "s2a" and "s2b"
   - Encodes stream "s2a" to VLAN 1 and stream "s2b" to VLAN 2
   - Forwards VLAN 1 to s2a and VLAN 2 to s2b

3. **Switch s2a**:
   - Decodes incoming VLAN 1 from s1 as stream "s2a" and VLAN 2 from s2b as stream "s2b-s2a"
   - Merges streams "s2a" and "s2b-s2a" into stream "s3a"
   - Splits stream "s3a" into streams "s3a" and "s2b"
   - Encodes stream "s3a" to VLAN 1 and stream "s2b" to VLAN 2
   - Forwards VLAN 1 to s3a and VLAN 2 to s2b

4. **Switch s2b**:
   - Decodes incoming VLAN 2 from s1 as stream "s2b" and VLAN 2 from s2a as stream "s2a-s2b"
   - Merges streams "s2b" and "s2a-s2b" into stream "s3b"
   - Splits stream "s3b" into streams "s3b" and "s2a"
   - Encodes stream "s3b" to VLAN 1 and stream "s2a" to VLAN 2
   - Forwards VLAN 1 to s3b and VLAN 2 to s2a

5. **Switches s3a and s3b**:
   - Decode incoming VLAN 1 as streams "s3a" and "s3b" respectively
   - Encode these streams to VLAN 1
   - Forward to destination

6. **Destination Node**:
   - Decodes incoming VLAN 1 from s3a as stream "s3a" and from s3b as stream "s3b"
   - Merges streams "s3a" and "s3b", eliminating duplicates

Here is the complete configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # configure all egress traffic

**Implementation Details**

The manual configuration uses several key components:

1. **StreamIdentifierLayer**: Identifies and classifies frames into streams
2. **StreamCoderLayer**: Encodes and decodes stream information using VLAN tags
3. **StreamRelayLayer**: Contains the splitter and merger components
4. **StreamSplitterLayer**: Duplicates frames from one stream into multiple streams
5. **StreamMergerLayer**: Combines multiple streams, eliminating duplicates based on sequence numbers

These components are configured in the bridging layer of each node:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # enable stream policing in layer 2
   :end-at: *.s1.bridging.streamCoder.typename

Results
-------

The simulation results demonstrate how frame replication and elimination provide reliability despite link failures.

**Packets Received vs. Sent**

.. figure:: media/packetsreceivedsent.png
   :align: center
   :width: 100%

This figure shows the cumulative number of packets sent by the source (blue
line) and received by the destination (red line) over time. Despite the link
failures at t=0.1s and t=0.2s, packets continue to be delivered to the
destination, demonstrating the reliability benefits of frame replication.

The slight gap between sent and received packets represents packets that were
lost due to both redundant paths being affected by the failures. This can happen
when a packet is in transit exactly when a failure occurs, or when multiple
failures affect all possible paths for a specific packet.

**Packet Delivery Ratio**

.. figure:: media/packetratio.png
   :align: center

This figure shows the ratio of received to sent packets over time. The ratio
remains high (above 0.65) despite the link failures, demonstrating the
effectiveness of frame replication in providing reliability. Without frame
replication, we would expect a much lower delivery ratio after the failures.

The expected packet delivery ratio of approximately 0.657 is verified by
analytical calculation, taking into account the network topology, link failure
timing, and packet production rate.

**Analysis**

The results demonstrate several key aspects of frame replication and elimination:

1. **Reliability**: Frame replication significantly improves packet delivery despite link failures
2. **Seamless Redundancy**: The transition during failures is seamless, with minimal packet loss
3. **Efficiency**: The elimination of duplicates ensures that the destination receives each packet only once
4. **Manual Configuration**: The manual configuration successfully implements the desired redundancy behavior

Practical Applications
---------------------

Manual configuration of frame replication and elimination is particularly valuable in:

1. **Industrial Automation**: Where reliability is critical for control systems
2. **Power Substations**: Where protection and control messages must be delivered reliably
3. **Automotive Networks**: In-vehicle networks requiring high reliability for safety-critical functions
4. **Aerospace Systems**: Where redundancy is essential for safety

These applications benefit from:

- **Reliability**: Ensuring critical messages reach their destination
- **Determinism**: Maintaining predictable behavior even during failures
- **Customization**: Tailoring the redundancy configuration to specific requirements
- **Integration**: Working with existing network infrastructure

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ManualConfigurationShowcase.ned <../ManualConfigurationShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/framereplication/manualconfiguration`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/framereplication/manualconfiguration && inet'

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

To deepen your understanding of frame replication and elimination, consider experimenting with the following modifications:

1. **Different Network Topologies**: Modify the network to create different redundant paths and observe the impact on reliability.

2. **Different Failure Scenarios**: Change the timing and location of link failures to see how they affect packet delivery.

3. **Traffic Patterns**: Modify the packet size and production interval to see how they affect the system's performance.

4. **Comparison with Automatic Configuration**: Compare this manual configuration with the automatic configuration showcases to understand the trade-offs.

5. **Performance Metrics**: Add additional statistics collection to measure metrics like end-to-end delay and jitter.

References
----------

1. `IEEE 802.1CB-2017 <https://standards.ieee.org/ieee/802.1CB/6844/>`__ - Frame Replication and Elimination for Reliability

2. `IEEE 802.1Q-2018 <https://standards.ieee.org/ieee/802.1Q/7098/>`__ - IEEE Standard for Local and Metropolitan Area Networks--Bridges and Bridged Networks

3. Kehrer, S., Kleineberg, O., & Heffernan, D. (2014). `A comparison of fault-tolerance concepts for IEEE 802.1 Time Sensitive Networks (TSN) <https://ieeexplore.ieee.org/document/6899165>`__. IEEE Emerging Technology and Factory Automation (ETFA), 1-8.

4. Nasrallah, A., Thyagaturu, A. S., Alharbi, Z., Wang, C., Shao, X., Reisslein, M., & ElBakoury, H. (2019). `Ultra-Low Latency (ULL) Networks: The IEEE TSN and IETF DetNet Standards and Related 5G ULL Research <https://ieeexplore.ieee.org/document/8458130>`__. IEEE Communications Surveys & Tutorials, 21(1), 88-145.

5. Finn, N. (2018). `Introduction to Time-Sensitive Networking <https://ieeexplore.ieee.org/document/8412457>`__. IEEE Communications Standards Magazine, 2(2), 22-28.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/789>`__ page in the GitHub issue tracker for commenting on this showcase.
