Token-Bucket-Based Policing
===========================

Goals
-----

In shared network environments, misbehaving traffic sources can
disrupt other applications. For example, when a client generates excessive
traffic that exceeds available bandwidth, it can disrupt traffic from other
clients sharing the same network infrastructure.

This problem is particularly critical in Time-Sensitive Networking (TSN) environments
where predictable and reliable communication is essential for real-time applications.
Per-stream filtering and policing provides an effective solution by limiting
excessive traffic and protecting well-behaved streams from disruption, ensuring
fair resource allocation across different streams.

This showcase demonstrates per-stream policing in TSN using a token bucket
mechanism. Token bucket policing provides a deterministic method for traffic control
that enforces long-term bandwidth limits while allowing controlled short-term traffic bursts.
The scenario implements two clients: one generating excessive traffic and another
generating normal traffic. We show how token bucket policing effectively
limits the excessive traffic while allowing normal traffic to flow unimpeded.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/tsn/streamfiltering/tokenbucket <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/tokenbucket>`__

Background
----------

Stream Filtering and Policing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Stream filtering and policing limit excessive traffic from misbehaving network
devices to prevent disruption of well-behaved devices. Unlike traffic shaping, which
buffers excess packets for later transmission, policing immediately drops packets
that exceed predefined rate limits.

In INET, this functionality is implemented within the bridging layer of :ned:`TsnSwitch` modules
through a :ned:`StreamFilterLayer` component. This layer contains :ned:`SimpleIeee8021qFilter`
modules that can be configured as ingress filters, egress filters, or both.

Here is a :ned:`StreamFilterLayer` module with an ingress filter (in most cases, only the ingress filter is used):

.. figure:: media/filteringlayer2.png
   :align: center

For example, a :ned:`SimpleIeee8021qFilter` with 2 streams looks like the following:

.. figure:: media/SimpleIeee8021qFilter3.png
   :align: center

The :ned:`SimpleIeee8021qFilter` contains the following key submodules:

- **classifier**: A :ned:`StreamClassifier` module by default that identifies incoming packets and
  dispatches them to the appropriate stream-specific processing path.
- **meter**: A traffic meter for each stream that measures traffic rates and
  labels packets as either "green" (conforming) or "red" (non-conforming)
  based on predefined traffic parameters. They are :ned:`DualRateThreeColorMeter` by default.
- **filter**:  A traffic filter for each stream that drops red packets while
  allowing green packets to pass through. They are :ned:`LabelFilter` by default.

The type and configuration of the **meter** and **filter** modules implement the
specific filtering mechanism (e.g. token-bucket-based, statistical). This
architecture ensures that each stream is independently metered and policed
according to its specific traffic parameters.


Token Bucket Algorithm
~~~~~~~~~~~~~~~~~~~~~~

The token bucket algorithm is a deterministic traffic policing mechanism that
operates using the following principles:

- A virtual "bucket" stores tokens that are replenished at a constant rate called the
  `Committed Information Rate` (CIR)
- The bucket has a maximum capacity equal to the `Committed Burst Size` (CBS)
- When the bucket reaches full capacity, newly generated tokens overflow and are discarded
- Each incoming packet consumes tokens proportional to its size from the bucket
- Packets are forwarded only when sufficient tokens are available; consumed tokens are removed from the bucket
- When insufficient tokens are available, packets are either dropped (policing) or marked as non-conforming

This mechanism provides:

- **Long-term rate control**: Average traffic rate is limited to the CIR over time
- **Burst accommodation**: Short bursts up to CBS bytes can exceed the CIR temporarily
- **Deterministic operation**: Each packet has predictable pass/fail behavior based on current token availability

Single Rate Two Color Metering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The single rate two color meter is a specific implementation of the token bucket
algorithm that works as follows:

- Uses a single token bucket with the `Committed Information Rate` and `Committed Burst Size` parameters
- Labels packets as either "green" (conforming) or "red" (non-conforming)
- Green packets are forwarded, while red packets are typically dropped

The Model
---------

This showcase uses a simple four-node network topology to demonstrate token bucket policing:

.. figure:: media/Network4.png
   :align: center

The network consists of two client devices that send traffic streams to a server through a TSN switch:

- **client1**: Generates misbehaving traffic with varying, sometimes excessive data rates
- **client2**: Generates well-behaved traffic with a steady data rate
- **switch**: A :ned:`TsnSwitch` that implements token bucket policing to control traffic flows
- **server**: Receives traffic from both clients

The combined traffic from both clients occasionally exceeds the link capacity between
the switch and the server, creating congestion. This scenario allows us to compare network
behavior with and without token bucket policing enabled in the switch.

Traffic Configuration
~~~~~~~~~~~~~~~~~~~~~

The network topology consists of :ned:`TsnDevice` modules for clients and server,
connected through a :ned:`TsnSwitch` via 100 Mbps :ned:`EthernetLink` channels.

**Traffic Patterns:**

- ``client1``: Generates **misbehaving traffic** at an average rate of 40 Mbps using sinusoidal
  packet intervals, creating variable, bursty traffic patterns that periodically exceed the committed rate
- ``client2``: Generates **well-behaved traffic** at a steady 20 Mbps with consistent packet intervals

.. literalinclude:: ../omnetpp.ini
   :start-at: client applications
   :end-before: server applications
   :language: ini

Stream Identification and Classification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Traffic Classification:** Both streams belong to the same traffic class, which is important for this
demonstration. If they belonged to different traffic classes, TSN traffic shaping could resolve
the issue by prioritizing traffic. Since both streams share the same class, they compete equally
for bandwidth, making per-stream policing necessary to protect the well-behaved stream.
This requires no explicit configuration.

**Stream Identification:** The system uses a two-stage identification process:

- Clients identify outgoing packets by UDP destination port and encode them with VLAN IDs
- The switch decodes VLAN IDs to classify incoming packets and apply appropriate policing policies

.. literalinclude:: ../omnetpp.ini
   :start-at: outgoing streams
   :end-before: ingress per-stream filtering
   :language: ini

Per-Stream Filtering
~~~~~~~~~~~~~~~~~~~~

The per-stream ingress filtering dispatches the different streams to separate metering and filter paths:

.. literalinclude:: ../omnetpp.ini
   :start-at: streamFilter.ingress.numStreams
   :end-before: SingleRateTwoColorMeter
   :language: ini

Token Bucket Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~

Each stream uses a :ned:`SingleRateTwoColorMeter` configured with stream-specific parameters
to implement token bucket policing. The meter operates with two key parameters:

- :par:`committedInformationRate` (CIR): Token replenishment rate

  - Misbehaving stream: 40 Mbps (matches average traffic rate)
  - Normal stream: 20 Mbps (matches steady traffic rate)

- :par:`committedBurstSize` (CBS): Maximum token bucket capacity

  - Misbehaving stream: 10 kB (allows moderate bursts)
  - Normal stream: 5 kB (smaller burst allowance for steady traffic)

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SingleRateTwoColorMeter

The meter continuously replenishes tokens at the CIR rate. When packets arrive:

- **Green packets**: Sufficient tokens available - packet forwarded, tokens consumed
- **Red packets**: Insufficient tokens - packet marked for dropping by the :ned:`LabelFilter`

This configuration ensures long-term traffic rates do not exceed the CIR, while
the CBS allows temporary bursts that consume accumulated tokens. The different
CBS values reflect the distinct burst characteristics of each stream type.

The following figure illustrates the per-stream filtering architecture, showing
how traffic is classified, metered, and filtered inside the bridging layer of the switch:

.. figure:: media/StreamFiltering_SimpleIeee8021qFilter2.png
   :align: center
   :width: 100%

Results
-------

Without Policing
~~~~~~~~~~~~~~~~

The first three charts demonstrate the problem we are addressing: excessive,
misbehaving traffic disrupting normal traffic in a shared network
environment, without policing. Here is the traffic generated by the client:

.. figure:: media/client.png
   :align: center

``client1`` generates the misbehaving traffic stream with an average rate around 40 Mbps but with
significant variation due to the sinusoidal packet interval pattern. The traffic
rate oscillates, creating natural bursts that exceed the configured committed
information rate. ``client2`` generates steady normal traffic at approximately
20 Mbps. Together, the combined traffic can momentarily exceed the 100 Mbps link
capacity between the switch and server.

Here is the traffic received at the server:

.. figure:: media/server_nopolicing.png
   :align: center

When the combined traffic exceeds the link capacity between the switch and the
server, packets must queue at the switch, leading to delays. The
misbehaving traffic competes with normal traffic for bandwidth, slowing it down.

The next chart shows the end-to-end delay of the normal traffic stream:

.. figure:: media/delay_nopolicing.png
   :align: center

Without policing, the normal traffic suffers a significant delay spike reaching
up to ~25 ms during the period when the misbehaving traffic is high. This clearly
demonstrates the problem: well-behaved traffic is disrupted by misbehaving or
excessive traffic sources sharing the same network infrastructure.

With Policing
~~~~~~~~~~~~~

The following chart compares traffic at the server with and without policing, showing how token bucket policing solves this problem:

.. figure:: media/server.png
   :align: center

With token bucket policing enabled in the switch's ingress filter, the misbehaving traffic is now limited to
approximately its configured committed information rate of 40 Mbps -- the peaks
that previously exceeded the limit are clipped by the token bucket mechanism.
The normal traffic continues to flow at its expected rate of 20 Mbps, unaffected
by the excess traffic. The total traffic to the server stays well within the
link capacity.

The next chart shows the end-to-end delay of normal traffic, with and without policing:

.. figure:: media/delay_normal.png
   :align: center

With policing enabled, the normal traffic experiences consistently low delays,
typically under 1 ms, with no significant spikes. The token bucket mechanism has
successfully protected the normal traffic from being disrupted by the misbehaving
traffic stream. This demonstrates a key benefit of per-stream policing in TSN
environments: ensuring predictable performance for well-behaved streams.

Additional Details
~~~~~~~~~~~~~~~~~~

The remaining charts provide insight into how the token bucket mechanism
operates internally. Here are the incoming, outgoing, and dropped data rates in the misbehaving traffic's filter:

.. figure:: media/misbehaving_filter.png
   :align: center

The misbehaving traffic filter shows substantial packet drops when the
incoming traffic rate exceeds what the token bucket can accommodate. During
traffic bursts, packets are dropped when tokens are insufficient. The green
solid line shows the packets that passed through -- these correspond to periods
when the token bucket had sufficient tokens. The committed burst size of 10 kB
allows some short-term bursts to pass, but sustained excess traffic is dropped
to enforce the 40 Mbps committed information rate. Now let's see the same for the normal traffic's filter:

.. figure:: media/normal_filter.png
   :align: center

The normal traffic filter shows that virtually all packets pass through with no drops. Since the normal traffic operates well within its
configured limit (20 Mbps committed information rate with 5 kB committed burst
size), the token bucket always has sufficient tokens available.

The next chart shows the number of tokens available in each bucket over time.

.. figure:: media/tokens.png
   :align: center

The
misbehaving traffic bucket (blue line) shows significant variation, frequently
dropping to low levels during traffic bursts when packets consume tokens faster
than the committed information rate replenishes them. When the token count is
low or zero, incoming packets are labeled red and subsequently dropped by the
filter.

The normal traffic bucket (orange line) maintains a relatively higher token
count throughout the simulation. Since the normal traffic rate is well below its
configured committed information rate, tokens accumulate to the committed burst
size limit during quiet periods and are available during traffic bursts. This
explains why the normal traffic filter showed no packet drops.

The maximum token count for each bucket corresponds to its committed burst size:
10 kB (10,000 Bytes) for misbehaving traffic and 5 kB (5,000 Bytes) for normal
traffic.

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

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/tsn/streamfiltering/tokenbucket && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/795>`__ page in the GitHub issue tracker for commenting on this showcase.
