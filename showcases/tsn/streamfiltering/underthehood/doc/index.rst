Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the filtering and policing modules in INET can
work outside the context of a complete network node. This approach facilitates
assembling and validating specific complex filtering and policing behaviors in
isolation, which can be difficult to replicate and debug in a complete network
simulation.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/underthehood <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/underthehood>`__

Background
----------

In network simulations, traffic filtering and policing mechanisms are typically
embedded within network nodes such as switches or routers. However, this
integration can make it challenging to isolate and test specific filtering
behaviors, especially when troubleshooting complex configurations.

**Component Testing vs. Integration Testing**

Testing components in isolation before integrating them into a larger system is
a fundamental principle in software engineering and system design. This approach
offers several advantages:

1. **Simplified Debugging**: Issues can be identified and resolved more easily when components are tested in isolation
2. **Focused Testing**: Specific behaviors and edge cases can be tested without interference from other system components
3. **Rapid Iteration**: Changes can be made and tested quickly without reconfiguring an entire network
4. **Validation**: Component behavior can be validated against specifications before integration

**Stream Filtering and Policing Components**

The INET framework provides several modular components for stream filtering and
policing that can be used both within network nodes and in standalone
configurations:

1. **Stream Identifier**: Maps packets to streams based on configurable criteria
2. **Stream Classifier**: Assigns packets to specific processing paths based on their stream
3. **Token Bucket Meter**: Measures traffic against configured rates and burst sizes
4. **Packet Filter**: Makes forwarding decisions based on metering results

These components can be connected in various ways to implement different
filtering and policing behaviors, from simple rate limiting to complex
multi-rate, multi-color schemes.

The Model
---------

In this configuration, we directly connect a per-stream filtering module to
multiple packet sources, bypassing the complexity of a full network simulation.
This allows us to focus solely on the behavior of the filtering mechanism.

Here is the network:

.. figure:: media/Network.png
   :align: center
   :width: 100%

The network consists of the following components:

1. **Multiple Packet Sources**: Generate traffic with varying patterns
2. **Packet Multiplexer**: Combines traffic from multiple sources
3. **Stream Identifier**: Maps packets to streams based on their source
4. **IEEE 802.1Q Filter**: Applies filtering and policing to each stream
5. **Passive Packet Sink**: Collects packets that pass through the filter

**Component Implementation**

The key components in this showcase are:

1. **ActivePacketSource** (:ned:`ActivePacketSource`): Generates packets at configurable intervals
2. **StreamIdentifier** (:ned:`StreamIdentifier`): Maps packets to streams based on configurable criteria
3. **SimpleIeee8021qFilter** (:ned:`SimpleIeee8021qFilter`): Implements per-stream filtering using token bucket meters

**Traffic Configuration**

The showcase includes three traffic sources, each with different sinusoidal
patterns to create varying traffic rates:

1. **Source 0**: Complex pattern with multiple frequency components
2. **Source 1**: Simple sinusoidal pattern with medium frequency
3. **Source 2**: Simple sinusoidal pattern with low frequency

Each source generates 100-byte packets at intervals determined by their
respective sinusoidal functions, creating realistic traffic patterns with
natural variations.

**Stream Identification and Filtering**

The stream identifier maps packets to streams based on their source:

- Packets from source[0] are mapped to stream "s0"
- Packets from source[1] are mapped to stream "s1"
- Packets from source[2] are mapped to stream "s2"

The IEEE 802.1Q filter then applies token bucket metering to each stream using a
:ned:`SingleRateTwoColorMeter` with the following parameters:

- :par:`committedInformationRate`: 8Mbps for all streams
- :par:`committedBurstSize`: 100kB for all streams

Here is the complete configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

**Network Definition**

The network is defined in the NED file:

.. literalinclude:: ../PeekingUnderTheHoodShowcase.ned
   :language: ned
   :start-at: package inet

Results
-------

The simulation results demonstrate how the token bucket filtering mechanism
affects each traffic stream. The following figures show the traffic patterns and
token bucket states for each stream.

**Traffic Class 1**

.. figure:: media/TrafficClass1.png
   :align: center

This figure shows the traffic and token bucket state for the first stream (s0).
The blue area represents the incoming traffic, which has a complex pattern with
multiple frequency components. The red area shows the dropped traffic, and the
green line indicates the outgoing traffic after filtering. The yellow line shows
the token bucket state.

When the incoming traffic rate exceeds the committed information rate and the
token bucket is depleted, packets are dropped (shown in red). The token bucket
state fluctuates as tokens are added at the committed rate and consumed by
passing packets.

**Traffic Class 2**

.. figure:: media/TrafficClass2.png
   :align: center

This figure shows the traffic and token bucket state for the second stream (s1).
The traffic pattern is a simple sinusoidal wave with medium frequency. The token
bucket state (yellow line) shows how tokens accumulate during periods of low
traffic and are consumed during traffic peaks.

**Traffic Class 3**

.. figure:: media/TrafficClass3.png
   :align: center

This figure shows the traffic and token bucket state for the third stream (s2).
The traffic pattern is a simple sinusoidal wave with low frequency. The longer
period of this pattern allows the token bucket to accumulate more tokens during
low traffic periods, resulting in fewer dropped packets during peaks.

**Analysis**

The results demonstrate several key aspects of token bucket filtering:

1. **Rate Adaptation**: The token bucket mechanism adapts to varying traffic rates, allowing short bursts to pass while limiting long-term average rates.

2. **Burst Tolerance**: The size of the token bucket (committed burst size) determines how large a traffic burst can be before packets are dropped.

3. **Pattern Sensitivity**: Different traffic patterns result in different filtering behaviors, even with the same token bucket parameters.

4. **Visualization Benefits**: Testing the filter in isolation makes it easier to visualize and understand the relationship between traffic patterns, token bucket states, and packet dropping.

Practical Applications
---------------------

This "under the hood" approach to testing filtering and policing mechanisms has
several practical applications:

1. **Component Development**: When developing new filtering or policing algorithms, testing them in isolation allows for faster iteration and validation.

2. **Parameter Tuning**: Finding the optimal parameters for token bucket meters (CIR, CBS) is easier when the components are tested directly.

3. **Educational Tool**: Understanding the behavior of complex filtering mechanisms is simpler when they are isolated from other network components.

4. **Troubleshooting**: When issues arise in a full network simulation, recreating the problem in an isolated environment can help identify the root cause.

5. **Performance Testing**: The performance characteristics of filtering components can be measured more accurately when tested in isolation.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/streamfiltering/underthehood`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/streamfiltering/underthehood && inet'

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

To deepen your understanding of filtering and policing mechanisms, consider experimenting with the following modifications:

1. **Different Traffic Patterns**: Modify the production intervals of the traffic sources to create different patterns and observe how the filtering mechanism responds.

2. **Different Meter Types**: Replace the SingleRateTwoColorMeter with other meter types such as MultiTokenBucketMeter or DualRateThreeColorMeter.

3. **Different Filter Types**: Try different filter implementations to see how they affect the traffic.

4. **Parameter Sensitivity**: Adjust the committedInformationRate and committedBurstSize parameters to see how they affect the filtering behavior.

5. **Compare with Network Integration**: Create a similar configuration within a network node and compare the results to see if there are any differences.

References
----------

1. `IEEE 802.1Q-2018 <https://standards.ieee.org/ieee/802.1Q/7098/>`__ - IEEE Standard for Local and Metropolitan Area Networks--Bridges and Bridged Networks

2. `RFC 2697 <https://datatracker.ietf.org/doc/html/rfc2697>`__ - A Single Rate Three Color Marker

3. `RFC 2698 <https://datatracker.ietf.org/doc/html/rfc2698>`__ - A Two Rate Three Color Marker

4. Szigeti, T., Hattingh, C., Barton, R., & Briley, K. (2013). `End-to-End QoS Network Design: Quality of Service for Rich-Media & Cloud Networks <https://www.ciscopress.com/store/end-to-end-qos-network-design-quality-of-service-for-9781587143694>`__. Cisco Press.

5. Varga, A., & Hornig, R. (2008). `An overview of the OMNeT++ simulation environment <https://dl.acm.org/doi/10.4108/ICST.SIMUTOOLS2008.3027>`__. Proceedings of the 1st International Conference on Simulation Tools and Techniques for Communications, Networks and Systems.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/796>`__ page in the GitHub issue tracker for commenting on this showcase.
