In-vehicle Network
==================

Goals
-----

In this example, we demonstrate the combined features of Time-Sensitive Networking
(TSN) in a complex in-vehicle network. The showcase illustrates how multiple TSN
features work together to provide deterministic communication for critical
automotive applications while efficiently handling various traffic classes with
different requirements.

The network utilizes:

- Time-aware shaping
- Automatic gate scheduling
- Clock drift and time synchronization
- Credit-based shaping
- Per-stream filtering and policing
- Stream redundancy
- Unicast and multicast streams
- Link failure protection
- Frame preemption
- Cut-through switching

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/invehicle <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/invehicle>`__

Background
----------

Modern vehicles contain dozens of electronic control units (ECUs) that need to
communicate with each other reliably and with deterministic timing guarantees.
Traditional automotive networks like CAN, LIN, and FlexRay are being supplemented
or replaced by Ethernet-based networks to handle the increasing bandwidth
requirements of advanced driver assistance systems (ADAS), infotainment, and
autonomous driving functions.

**Automotive Ethernet and TSN**

Automotive Ethernet provides high bandwidth (100Mbps to 10Gbps) and the ability
to use standard Ethernet components, but standard Ethernet lacks the deterministic
guarantees required for safety-critical automotive applications. Time-Sensitive
Networking (TSN) extends standard Ethernet with features that provide determinism,
reliability, and quality of service guarantees.

**Key TSN Features for Automotive Applications**

1. **Time Synchronization (IEEE 802.1AS)**: Provides a common time reference
   across the network, essential for coordinated actions and time-aware traffic
   shaping.

2. **Time-Aware Shaper (IEEE 802.1Qbv)**: Enables deterministic transmission by
   scheduling traffic according to a predefined timetable, ensuring critical
   frames are transmitted at specific times.

3. **Credit-Based Shaper (IEEE 802.1Qav)**: Allocates bandwidth to different
   traffic classes while preventing traffic bursts from consuming excessive
   bandwidth.

4. **Frame Preemption (IEEE 802.1Qbu)**: Allows high-priority frames to interrupt
   the transmission of lower-priority frames, reducing latency for critical
   traffic.

5. **Per-Stream Filtering and Policing (IEEE 802.1Qci)**: Provides protection
   against bandwidth-consuming failures and malicious attacks by filtering and
   policing individual streams.

6. **Frame Replication and Elimination (IEEE 802.1CB)**: Improves reliability by
   sending duplicate frames over different paths and eliminating duplicates at
   the receiver.

**Traffic Classes in Automotive Networks**

Automotive networks typically handle several traffic classes with different requirements:

1. **Control Data Traffic (CDT)**: Safety-critical control messages with strict
   latency and reliability requirements (e.g., steering, braking).

2. **Audio/Video Bridging Class A (AVB-A)**: Time-sensitive traffic with moderate
   latency requirements (e.g., camera feeds for ADAS).

3. **Audio/Video Bridging Class B (AVB-B)**: Less time-sensitive traffic (e.g.,
   infotainment, navigation).

4. **Best Effort**: Non-critical traffic with no specific timing requirements
   (e.g., diagnostic data, software updates).

The Model
---------

In this showcase, we model the communication network inside a vehicle. The network
consists of several Ethernet switches connected in a redundant way and multiple
end devices representing various automotive systems. There are several data flows
between the end device applications with different traffic classes and requirements.

**Network Topology**

The network topology represents a modern in-vehicle network with a distributed architecture:

.. figure:: media/Network.png
   :align: center
   :width: 100%

The network consists of:

1. **Switches**:
   - Front Switch: Central switch for front components
   - Front Left/Right Switches: Switches for front left/right components
   - Rear Switch: Central switch for rear components
   - Rear Left/Right Switches: Switches for rear left/right components

2. **End Devices**:
   - Sensors: Cameras (front/rear, left/right), LIDAR
   - Actuators: Steering, Engine Actuator, Wheels (front/rear, left/right)
   - Control/Display Units: HUD (Head-Up Display), OBU (On-Board Unit), Rear Display
   - Master Clock: Time synchronization source (in TSN configurations)

3. **Links**:
   - 1Gbps links between switches
   - 100Mbps links to end devices
   - Redundant paths between critical components

**Traffic Flows**

The showcase includes multiple traffic flows with different characteristics:

1. **Control Data Traffic (CDT, PCP 6)**:
   - Telemetry from steering to engine actuator (2.8Mbps)
   - Telemetry from wheels to engine actuator (2.8Mbps each)
   - Unicast with link failure protection

2. **Audio/Video Bridging Class A (AVB-A, PCP 5)**:
   - LIDAR data to HUD (10.4Mbps)
   - Video streams from cameras to OBU and HUD (20.8Mbps each)
   - Unicast (LIDAR) and multicast (cameras)

3. **Audio/Video Bridging Class B (AVB-B, PCP 4)**:
   - Navigation from HUD to rear display (20.8Mbps)
   - Entertainment from OBU to rear display (10.4Mbps)
   - Unicast

4. **Best Effort (PCP 0)**:
   - Navigation from OBU to HUD (20.8Mbps)
   - Unicast

**Failure Scenarios**

The showcase includes several failure scenarios to demonstrate the resilience of the network:

1. **Link Failure**: A link between the front switch and front left switch fails
2. **Wheel Failure**: The front left wheel sends excessive traffic (10x normal rate)
3. **Camera Failure**: The front left camera sends excessive traffic (10x normal rate)

**Configurations**

The showcase includes three different configurations:

1. **Standard Ethernet**: Uses standard Ethernet features without TSN
2. **Manual TSN**: Uses TSN features with manual configuration
3. **Automatic TSN**: Uses TSN features with automatic configuration through configurators

Here is the complete configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [General]
   :end-before: [Config StandardEthernet]

Standard Ethernet
-----------------

In the Standard Ethernet configuration, we use only standard Ethernet features to
establish a baseline for comparison. This configuration uses:

- Standard Ethernet switches and hosts
- No time synchronization
- No traffic shaping or scheduling
- No stream redundancy or failure protection

This represents a traditional Ethernet network without TSN enhancements.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config StandardEthernet]
   :end-before: [Config TimeSensitiveNetworkingBase]

Time-Sensitive Networking
-------------------------

The TSN configurations enhance the network with deterministic communication
capabilities. The base TSN configuration includes:

- TSN-capable switches and devices
- Clock synchronization with random drift
- Time synchronization using gPTP
- Stream identification and coding
- Traffic shaping and scheduling
- Per-stream filtering and policing

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config TimeSensitiveNetworkingBase]
   :end-before: [Config ManualTsn]

**Manual TSN Configuration**

The Manual TSN configuration explicitly defines all stream identification, coding,
filtering, and shaping parameters:

- Stream identification based on packet filters
- Stream coding with appropriate PCP values
- Per-stream filtering with token bucket meters
- Credit-based shaping for AVB traffic classes
- Stream redundancy for reliability

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ManualTsn]
   :end-before: [Config AutomaticTsn]

**Automatic TSN Configuration**

The Automatic TSN configuration uses configurators to automatically set up the TSN features:

- Gate scheduling with AlwaysOpenGateScheduleConfigurator
- Stream redundancy with StreamRedundancyConfigurator
- Failure protection with FailureProtectionConfigurator
- Detailed stream specifications for all traffic flows

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config AutomaticTsn]

Results
-------

The simulation results demonstrate how the different configurations handle the
traffic flows and failure scenarios.

**Time Synchronization**

The following figure shows the time synchronization between network nodes in the TSN configurations:

.. figure:: media/TimeSynchronization.png
   :align: center
   :width: 100%

The time synchronization ensures that all nodes have a common time reference,
which is essential for time-aware shaping and coordinated actions. Despite the
random clock drift in each node, the gPTP protocol maintains synchronization
within tight bounds.

**Traffic Source Applications - Normal Operation**

The following figure shows the traffic generated by the source applications under normal operation with standard Ethernet:

.. figure:: media/TrafficSourceApplications_standard_ethernet_normal_op.png
   :align: center
   :width: 100%

Each application generates traffic according to its specified rate and pattern.
The different colors represent different traffic classes: red for CDT, blue for
AVB-A, green for AVB-B, and gray for Best Effort.

**Traffic Sink Applications - Normal Operation**

The following figure shows the traffic received by the sink applications under normal operation with standard Ethernet:

.. figure:: media/TrafficSinkApplications_standard_ethernet_normal_op.png
   :align: center
   :width: 100%

In normal operation with standard Ethernet, all traffic is delivered successfully,
but without deterministic guarantees on latency or jitter.

**Traffic Source Applications - Link Failure**

The following figure shows the traffic generated by the source applications during a link failure with standard Ethernet:

.. figure:: media/TrafficSourceApplications_standard_ethernet_broken_link.png
   :align: center
   :width: 100%

The applications continue to generate traffic at their specified rates, unaware
of the link failure in the network.

**Traffic Sink Applications - Link Failure**

The following figure shows the traffic received by the sink applications during a link failure with standard Ethernet:

.. figure:: media/TrafficSinkApplications_standard_ethernet_broken_link.png
   :align: center
   :width: 100%

With standard Ethernet, some traffic is lost due to the link failure. In contrast,
the TSN configurations with stream redundancy and failure protection maintain
delivery of critical traffic despite the link failure.

**Traffic Source Applications - Wheel Failure**

The following figure shows the traffic generated by the source applications during a wheel failure with standard Ethernet:

.. figure:: media/TrafficSourceApplications_standard_ethernet_broken_wheel.png
   :align: center
   :width: 100%

The front left wheel generates excessive traffic (10x normal rate), potentially
overwhelming the network.

**Traffic Sink Applications - Wheel Failure**

The following figure shows the traffic received by the sink applications during a wheel failure with standard Ethernet:

.. figure:: media/TrafficSinkApplications_standard_ethernet_broken_wheel.png
   :align: center
   :width: 100%

With standard Ethernet, the excessive traffic from the wheel affects other traffic
flows. In contrast, the TSN configurations with per-stream filtering and policing
contain the impact of the wheel failure.

**Traffic Source Applications - Camera Failure**

The following figure shows the traffic generated by the source applications during a camera failure with standard Ethernet:

.. figure:: media/TrafficSourceApplications_standard_ethernet_broken_camera.png
   :align: center
   :width: 100%

The front left camera generates excessive traffic (10x normal rate), potentially
overwhelming the network.

**Traffic Sink Applications - Camera Failure**

The following figure shows the traffic received by the sink applications during a camera failure with standard Ethernet:

.. figure:: media/TrafficSinkApplications_standard_ethernet_broken_camera.png
   :align: center
   :width: 100%

With standard Ethernet, the excessive traffic from the camera affects other traffic
flows. In contrast, the TSN configurations with per-stream filtering and policing
contain the impact of the camera failure.

**Analysis**

The results demonstrate several key aspects of TSN in automotive networks:

1. **Deterministic Communication**: TSN provides deterministic latency guarantees
   for critical traffic, ensuring that control messages are delivered within their
   timing constraints.

2. **Traffic Isolation**: Different traffic classes are isolated from each other,
   preventing interference between them even during failure scenarios.

3. **Reliability**: Stream redundancy and failure protection ensure that critical
   traffic continues to be delivered despite link failures.

4. **Fault Containment**: Per-stream filtering and policing contain the impact of
   component failures, preventing them from affecting the entire network.

5. **Efficient Resource Utilization**: Credit-based shaping and time-aware
   scheduling ensure efficient use of network resources while meeting the
   requirements of different traffic classes.

Practical Applications
---------------------

The TSN features demonstrated in this showcase have several practical applications
in automotive networks:

1. **Advanced Driver Assistance Systems (ADAS)**: Camera and sensor data must be
   delivered with low latency and high reliability for functions like automatic
   emergency braking and lane keeping assistance.

2. **Autonomous Driving**: Fully autonomous vehicles require deterministic
   communication between sensors, processing units, and actuators to ensure safe
   and reliable operation.

3. **Drive-by-Wire Systems**: Electronic control of steering, braking, and
   acceleration requires deterministic and reliable communication to replace
   mechanical linkages.

4. **Infotainment and Telematics**: Non-critical functions like entertainment and
   navigation can share the same network as critical functions without interference.

5. **Over-the-Air Updates**: Software updates can be delivered over the network
   without affecting critical vehicle functions.

These applications benefit from:

- **Determinism**: Guaranteed maximum latency for critical traffic
- **Reliability**: Continued operation despite component failures
- **Bandwidth**: High-speed communication for data-intensive applications
- **Integration**: Single network for multiple functions with different requirements
- **Scalability**: Ability to add new functions without compromising existing ones

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`InVehicleNetworkShowcase.ned <../InVehicleNetworkShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/combiningfeatures/invehicle`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/combiningfeatures/invehicle && inet'

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

To deepen your understanding of TSN in automotive networks, consider experimenting with the following modifications:

1. **Different Failure Scenarios**: Create new failure scenarios, such as multiple
   simultaneous failures, to test the resilience of the network.

2. **Traffic Pattern Variations**: Modify the traffic patterns of different
   applications to see how the network handles varying loads.

3. **Network Topology Changes**: Modify the network topology to explore different
   redundancy options and their impact on reliability.

4. **TSN Feature Combinations**: Enable or disable specific TSN features to
   understand their individual and combined effects on network performance.

5. **Performance Metrics**: Add additional statistics collection to measure
   metrics like end-to-end delay, jitter, and packet loss for different traffic
   classes.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/785>`__ page in the GitHub issue tracker for commenting on this showcase.
