Token Bucket based Policing
===========================

Goals
-----

In this example, we demonstrate per-stream policing using token buckets in
Time-Sensitive Networking (TSN). This approach provides a deterministic method
for traffic policing that enforces bandwidth limits while
allowing for controlled traffic bursts.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

Background
----------

Token bucket policing is one of the most widely used mechanisms for traffic
regulation in computer networks.

**Token Bucket Algorithm**

The token bucket algorithm is a deterministic traffic policing mechanism that
works as follows:

1. A virtual "bucket" holds tokens, which are added at a constant rate (the Committed Information Rate or CIR)
2. The bucket has a maximum capacity (the Committed Burst Size or CBS)
3. When the bucket is full, newly generated tokens are discarded
4. Each packet requires a number of tokens equal to its size to be forwarded
5. If there are enough tokens in the bucket, they are removed and the packet is allowed to pass
6. If there are not enough tokens, the packet is either dropped or marked as non-conforming

This approach allows for:

- Long-term average rate control (determined by the CIR)
- Accommodation of short traffic bursts up to the size of the bucket (determined by the CBS)
- Deterministic behavior with clear pass/fail criteria for each packet

**Single Rate Two Color Metering**

The Single Rate Two Color Meter is a specific implementation of the token bucket
algorithm that:

1. Uses a single token bucket with parameters CIR and CBS
2. Labels packets as either "green" (conforming) or "red" (non-conforming)
3. Green packets are forwarded, while red packets are typically dropped

This simple but effective approach is widely used in network traffic management
to enforce bandwidth limits while allowing for controlled bursts.

The Model
---------

In this configuration, we implement token bucket policing using the following
components:

1. :ned:`SingleRateTwoColorMeter`: Measures and labels packets based on token availability
2. :ned:`LabelFilter`: Drops packets that are labeled as non-conforming (red)

The following figure illustrates the per-stream filtering architecture, showing
how traffic is classified, metered, and filtered:

.. figure:: media/filter2.png
   :align: center
   :width: 100%

Here is the network topology:

.. figure:: media/Network.png
   :align: center

The network consists of three nodes:

- A client generating two traffic streams (best effort and video)
- A switch implementing the token bucket policing mechanism
- A server receiving the traffic

There are four applications in the network creating two independent data streams
between the client and the server. The average data rates are 40 Mbps and 20
Mbps, but both vary over time using a sinusoidal packet interval to create
complex traffic patterns with natural bursts.

**Network Configuration**

The client and the server are :ned:`TsnDevice` modules, and the switch is a
:ned:`TsnSwitch` module. The links between them use 100 Mbps :ned:`EthernetLink`
channels.

The two streams have two different traffic classes: best effort and video. The
bridging layer identifies the outgoing packets by their UDP destination port.
The client encodes and the switch decodes the streams using the IEEE 802.1Q PCP
field.

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: server applications
   :language: ini

**Stream Identification and Classification**

The client identifies and marks outgoing packets with PCP, while the switch decodes these
markings to apply the appropriate policies:

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: ingress per-stream filtering
   :language: ini

**Per-Stream Filtering**

The per-stream ingress filtering dispatches the different traffic classes to separate metering and filter paths:

.. literalinclude:: ../omnetpp.ini
   :start-at: ingress per-stream filtering
   :end-before: SingleRateTwoColorMeter
   :language: ini

**Token Bucket Implementation**

We use a :ned:`SingleRateTwoColorMeter` for both streams. This meter contains a
single token bucket with two key parameters:

1. :par:`committedInformationRate` (CIR): The rate at which tokens are added to the bucket (40Mbps for best effort, 20Mbps for video)
2. :par:`committedBurstSize` (CBS): The maximum capacity of the token bucket (10kB for best effort, 5kB for video)

Packets are labeled green or red by the meter based on token availability, and
red packets are dropped by the filter.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SingleRateTwoColorMeter

This mechanism ensures that the long-term average rate does not exceed the CIR,
while still allowing for short bursts up to the size of the CBS.

Results
-------

The simulation results demonstrate how the token bucket policing mechanism
affects the traffic streams as they pass through the switch. The following
figures show the traffic patterns at different points in
the network.

**Client Application Traffic**

.. figure:: media/ClientApplicationTraffic.png
   :align: center

This figure shows the traffic generated by the client applications. The blue
line represents the best effort traffic (~40Mbps), while the red line shows the
video traffic (~20Mbps). Note the sinusoidal variations in both traffic streams,
creating bursts that exceed the target rates at times.

**Best Effort Traffic Class**

.. figure:: media/BestEffortTrafficClass.png
   :align: center

This figure shows the operation of the per-stream filter for the best-effort
traffic class.
When the incoming traffic rate exceeds the CIR and the token bucket is depleted,
packets are dropped. The outgoing traffic rate is effectively capped at the CIR
over the long term, though short bursts can exceed this rate when the token
bucket has accumulated tokens.

**Video Traffic Class**

.. figure:: media/VideoTrafficClass.png
   :align: center

Similarly, this figure shows the operation of the per-stream filter for the
video traffic class. The video stream has a lower CIR
and CBS, resulting in more aggressive policing during traffic peaks.

**Token Bucket States**

.. figure:: media/TokenBuckets.png
   :align: center

This figure provides a direct view of the token bucket state for both traffic
classes. The lines show the number of tokens available in each bucket over time.
The filled areas indicate periods of rapid token consumption and refilling as
packets pass through.

Several key observations:

1. When the token count is high, packets pass through without being dropped
2. When the token count approaches zero, packets begin to be dropped
3. The token count increases during periods of low traffic and decreases during bursts
4. The data rate is at its maximum when the token count is near the minimum (high token consumption)
5. The best effort bucket (blue) has a higher capacity than the video bucket (red), corresponding to their respective CBS values

**Server Application Traffic**

.. figure:: media/ServerApplicationTraffic.png
   :align: center

This figure shows the traffic as received by the server applications after
passing through the policing mechanism. The traffic patterns are now more
controlled, with peaks effectively limited by the token bucket parameters. The
slight difference between this and the outgoing traffic from the per-stream
filter is due to measurements being taken at different protocol layers.

**Analysis**

The results demonstrate several key aspects of token bucket policing:

1. **Rate Limiting**: The long-term average rate is effectively limited to the CIR for each traffic class.

2. **Burst Tolerance**: Short bursts are allowed to pass through as long as tokens are available in the bucket, providing flexibility for naturally bursty traffic.

3. **Deterministic Behavior**: Unlike statistical policing, token bucket policing provides deterministic guarantees on traffic conformance.

4. **Different Treatment for Different Classes**: The best effort and video traffic classes are policed according to their respective CIR and CBS values, demonstrating the per-stream filtering capability.

Practical Applications
---------------------

Token bucket policing is particularly useful in the following scenarios:

1. **Hard Real-Time Systems**: Where deterministic guarantees on bandwidth usage are required

2. **Network Admission Control**: To ensure new traffic flows don't exceed their allocated bandwidth

3. **Service Level Agreements (SLAs)**: To enforce contractual bandwidth limits between service providers and customers

4. **Congestion Prevention**: To prevent network congestion by limiting traffic at ingress points

In TSN environments, token bucket policing is often used for critical traffic
classes that require strict bandwidth guarantees.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/streamfiltering/tokenbucket`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/streamfiltering/tokenbucket && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.
