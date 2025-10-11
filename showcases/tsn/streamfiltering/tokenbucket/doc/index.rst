Token Bucket based Policing
===========================

Goals
-----

In shared network environments, misbehaving or excessive traffic sources can 
disrupt other applications. For example, one client generates excessive 
traffic that exceeds the available bandwidth, potentially disrupting the traffic 
flow from other clients sharing the same network infrastructure.

This is particularly problematic in Time-Sensitive Networking (TSN) environments 
where predictable and reliable communication is essential. One solution to this 
problem is per-stream filtering and policing. This approach allows us to limit 
excessive traffic and protect well-behaved streams from disruption, ensuring 
fair resource allocation across different streams.

In this showcase, we demonstrate per-stream policing in TSN using a token bucket
mechanism. This approach provides a deterministic method for traffic policing
that enforces bandwidth limits while allowing for controlled traffic bursts. We
implement a scenario where one client generates excessive traffic while another
generates normal traffic, and show how token bucket policing can effectively
limit the excessive traffic while allowing the normal traffic to flow unimpeded.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

Background
----------

Stream Filtering and Policing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Stream filtering and policing limits excessive traffic from misbehaving network 
devices to prevent disruption of other devices. Unlike traffic shaping, which 
stores excess packets for later transmission, policing drops excessive packets 
that exceed predefined limits.

This functionality is implemented in the bridging layer of :ned:`TsnSwitch` modules through 
a :ned:`StreamFilterLayer` component containing ingress and/or egress filter modules. 
INET uses the :ned:`SimpleIeee8021qFilter` module by default to ensure misbehaving 
traffic cannot monopolize network resources or degrade well-behaved traffic streams.

Here is a streamFilterLayer module with an ingress filter (in most cases, only the ingress filter is used):

.. figure:: media/filteringlayer2.png
   :align: center

For example, a SimpleIeee8021qFilter with 2 streams looks like the following:

.. figure:: media/SimpleIeee8021qFilter3.png
   :align: center

The :ned:`SimpleIeee8021qFilter` contains three key submodules:

- **classifier**: A :ned:`StreamClassifier` that identifies incoming packets and 
  dispatches them to the appropriate stream-specific processing path
- **meter**: A traffic meter for each stream that measures traffic rates and 
  labels packets as either "green" (conforming) or "red" (non-conforming) 
  based on predefined traffic parameters
- **filter**: A :ned:`LabelFilter` for each stream that drops red packets while 
  allowing green packets to pass through

The type and configuration of the **meter** and **filter** modules implement the
specific filtering mechanism (e.g. token-bucket-based, statistical). This
architecture ensures that each stream is independently metered and policed
according to its specific traffic parameters.


.. meter and filter modules implement the filtering mechanisms

Token Bucket Algorithm
~~~~~~~~~~~~~~~~~~~~~~

The token bucket algorithm is a deterministic traffic policing mechanism that
works as follows:

- A virtual "bucket" holds tokens, which are added at a constant rate (the Committed Information Rate)
- The bucket has a maximum capacity (the Committed Burst Size)
- When the bucket is full, newly generated tokens are discarded
- Each packet requires a number of tokens equal to its size to be forwarded
- If there are enough tokens in the bucket, they are removed and the packet is allowed to pass
- If there are not enough tokens, the packet is either dropped or marked as non-conforming

This approach allows for:

- Long-term average rate control (determined by the Committed Information Rate)
- Accommodation of short traffic bursts up to the size of the bucket (determined by the Committed Burst Size)
- Deterministic behavior with clear pass/fail criteria for each packet

Single Rate Two Color Metering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Single Rate Two Color Meter is a specific implementation of the token bucket
algorithm that works as follows:

- Uses a single token bucket with parameters `Committed Information Rate` and `Committed Burst Size`
- Labels packets as either "green" (conforming) or "red" (non-conforming)
- Green packets are forwarded, while red packets are typically dropped

The Model
---------

We use the following network topology:

.. figure:: media/Network4.png
   :align: center

Two client devices each generate a traffic stream, one generating
normal, the other excess traffic. Both streams get forwarded to the server by
the switch, while the combined traffic at times exceeds the capacity of the link between
the switch and the server. We explore what happens with and without policing in
the switch.

Traffic Configuration
~~~~~~~~~~~~~~~~~~~~~

The clients and the server are :ned:`TsnDevice` modules, and the switch is a
:ned:`TsnSwitch` module. The links between them use 100 Mbps :ned:`EthernetLink`
channels.

Client1 generates excess traffic at an average rate of 40 Mbps, while client2
generates normal traffic at 20 Mbps. Both streams vary over time, with client1
using a sinusoidal packet interval to create complex traffic patterns with
natural bursts. TODO not here?

The two streams are of the same traffic class. The
bridging layer identifies the outgoing packets by their UDP destination port.
The clients encode and the switch decodes the streams using VLAN IDs.

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: server applications
   :language: ini

Stream Identification and Classification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The clients identify and mark outgoing packets with VLAN IDs, while the switch decodes these
markings to apply the appropriate policies:

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: ingress per-stream filtering
   :language: ini

Per-Stream Filtering
~~~~~~~~~~~~~~~~~~~~

The per-stream ingress filtering dispatches the different traffic classes to separate metering and filter paths:

.. literalinclude:: ../omnetpp.ini
   :start-at: streamFilter.ingress.numStreams
   :end-before: SingleRateTwoColorMeter
   :language: ini

Token Bucket Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~

We use a :ned:`SingleRateTwoColorMeter` for both streams. This meter contains a
single token bucket with two key parameters:

- :par:`committedInformationRate`: The rate at which tokens are added to the bucket (40Mbps for best effort, 20Mbps for video)
- :par:`committedBurstSize`: The maximum capacity of the token bucket (10kB for best effort, 5kB for video)

Packets are labeled green or red by the meter based on token availability, and
red packets are dropped by the filter.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SingleRateTwoColorMeter

This mechanism ensures that the long-term average rate does not exceed the Committed Information Rate,
while still allowing for short bursts up to the size of the Committed Burst Size.

In this configuration, we implement token bucket policing using the following
components:

- :ned:`SingleRateTwoColorMeter`: Measures and labels packets based on token availability
- :ned:`LabelFilter`: Drops packets that are labeled as non-conforming (red)

The following figure illustrates the per-stream filtering architecture, showing
how traffic is classified, metered, and filtered in the bridging layer of the switch:

.. figure:: media/SimpleIeee8021qFilter.png
   :align: center
   :width: 100%

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
line represents the best effort traffic from client1 (~40Mbps), while the red line shows the
video traffic from client2 (~20Mbps). Note the sinusoidal variations in both traffic streams,
creating bursts that exceed the target rates at times.

**Best Effort Traffic Class**

.. figure:: media/BestEffortTrafficClass.png
   :align: center

This figure shows the operation of the per-stream filter for the best-effort
traffic class.
When the incoming traffic rate exceeds the Committed Information Rate and the token bucket is depleted,
packets are dropped. The outgoing traffic rate is effectively capped at the Committed Information Rate
over the long term, though short bursts can exceed this rate when the token
bucket has accumulated tokens.

**Video Traffic Class**

.. figure:: media/VideoTrafficClass.png
   :align: center

Similarly, this figure shows the operation of the per-stream filter for the
video traffic class. The video stream has a lower Committed Information Rate
and Committed Burst Size, resulting in more aggressive policing during traffic peaks.

**Token Bucket States**

.. figure:: media/TokenBuckets.png
   :align: center

This figure provides a direct view of the token bucket state for both traffic
classes. The lines show the number of tokens available in each bucket over time.
The filled areas indicate periods of rapid token consumption and refilling as
packets pass through.

Several key observations:

- When the token count is high, packets pass through without being dropped
- When the token count approaches zero, packets begin to be dropped
- The token count increases during periods of low traffic and decreases during bursts
- The data rate is at its maximum when the token count is near the minimum (high token consumption)
- The best effort bucket (blue) has a higher capacity than the video bucket (red), corresponding to their respective Committed Burst Size values

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

- **Rate Limiting**: The long-term average rate is effectively limited to the Committed Information Rate for each traffic class.

- **Burst Tolerance**: Short bursts are allowed to pass through as long as tokens are available in the bucket, providing flexibility for naturally bursty traffic.

- **Deterministic Behavior**: Unlike statistical policing, token bucket policing provides deterministic guarantees on traffic conformance.

- **Different Treatment for Different Classes**: The best effort and video traffic classes are policed according to their respective Committed Information Rate and Committed Burst Size values, demonstrating the per-stream filtering capability.

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

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/streamfiltering/tokenbucket && inet'

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

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.
