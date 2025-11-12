Token Bucket based Policing
===========================

Goals
-----

In shared network environments, misbehaving or excessive traffic sources can 
disrupt other applications. For example, a client generates excessive 
traffic that exceeds the available bandwidth, potentially disrupting traffic 
from other clients sharing the same network infrastructure.

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
a :ned:`StreamFilterLayer` component containing the default :ned:`SimpleIeee8021qFilter` modules as ingress and/or egress filters.

.. TODO az egyik tartalmazza a masikat

Here is a :ned:`StreamFilterLayer` module with an ingress filter (in most cases, only the ingress filter is used):

.. figure:: media/filteringlayer2.png
   :align: center

For example, a :ned:`SimpleIeee8021qFilter` with 2 streams looks like the following:

.. figure:: media/SimpleIeee8021qFilter3.png
   :align: center

The :ned:`SimpleIeee8021qFilter` contains three key submodules:

- **classifier**: A :ned:`StreamClassifier` module by default that identifies incoming packets and 
  dispatches them to the appropriate stream-specific processing path
- **meter**: A traffic meter for each stream that measures traffic rates and 
  labels packets as either "green" (conforming) or "red" (non-conforming) 
  based on predefined traffic parameters. A :ned:`DualRateThreeColorMeter` module by default.
- **filter**: A :ned:`LabelFilter` module by default for each stream that drops red packets while 
  allowing green packets to pass through

The type and configuration of the **meter** and **filter** modules implement the
specific filtering mechanism (e.g. token-bucket-based, statistical). This
architecture ensures that each stream is independently metered and policed
according to its specific traffic parameters.


Token Bucket Algorithm
~~~~~~~~~~~~~~~~~~~~~~

The token bucket algorithm is a deterministic traffic policing mechanism that
works as follows:

- A virtual "bucket" holds tokens, which are added at a constant rate (the Committed Information Rate)
- The bucket has a maximum capacity (the Committed Burst Size)
- When the bucket is full, newly generated tokens are discarded
- Each packet requires a number of tokens equal to its size to be forwarded
- If sufficient tokens are available in the bucket, they are removed and the packet is allowed to pass through
- If insufficient tokens are available, the packet is either dropped or marked as non-conforming

This approach allows for:

- Long-term average rate control (determined by the Committed Information Rate)
- Accommodation of short traffic bursts up to the size of the bucket (determined by the Committed Burst Size)
- Deterministic behavior with clear pass/fail criteria for each packet

Single Rate Two Color Metering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The single rate two color meter is a specific implementation of the token bucket
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
normal traffic with a steady data rate, the other misbehaving traffic with varying, sometimes excessive data rate. 
Both streams get forwarded to the server by
the switch, while the combined traffic at times exceeds the capacity of the link between
the switch and the server. We explore what happens with and without policing in
the switch.

Traffic Configuration
~~~~~~~~~~~~~~~~~~~~~

The clients and the server are :ned:`TsnDevice` modules, and the switch is a
:ned:`TsnSwitch` module. The links between them use 100 Mbps :ned:`EthernetLink`
channels.

Two distinct traffic patterns are generated. ``client1`` produces misbehaving
traffic at an average rate of 40 Mbps with sinusoidal packet intervals that
create complex, bursty traffic patterns. Meanwhile, ``client2`` generates steady
normal traffic at 20 Mbps.

The two streams are of the same traffic class. This is important because if they 
were in different classes, the problem could be solved by the traffic shaper. 
However, since they are the same class, the well-behaving and misbehaving streams 
are mixed in the shaper, so the problem must be solved through filtering.

The
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

- :par:`committedInformationRate`: The rate at which tokens are added to the bucket (40Mbps for misbehaving, 20Mbps for normal)
- :par:`committedBurstSize`: The maximum capacity of the token bucket (10kB for misbehaving, 5kB for normal)

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
- :ned:`LabelFilter`: Drops packets that are labeled as non-conforming

The following figure illustrates the per-stream filtering architecture, showing
how traffic is classified, metered, and filtered inside the bridging layer of the switch:

.. figure:: media/StreamFiltering_SimpleIeee8021qFilter2.png
   :align: center
   :width: 100%

Results
-------

The Problem
~~~~~~~~~~~

The first three charts demonstrate the problem we are addressing: excessive,
misbehaving traffic disrupting normal traffic in a shared network
environment, without policing. Here is the traffic generated by the client:

.. figure:: media/client.png
   :align: center

``client1`` generates the misbehaving traffic stream with an average rate around 40 Mbps but with
significant variation due to the sinusoidal packet interval pattern. The traffic
rate oscillates creating natural bursts that exceed the configured committed
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

Without policing, the normal traffic suffers significant delay spike reaching
up to ~25 ms during the period when the misbehaving traffic is high. This clearly
demonstrates the problem: well-behaved traffic is disrupted by misbehaving or
excessive traffic sources sharing the same network infrastructure.

The Solution
~~~~~~~~~~~~

The next two charts show how token bucket policing solves this problem by
limiting excessive traffic while protecting normal traffic. Here is the traffic at the server, with and without policing:

.. figure:: media/server.png
   :align: center

With token bucket policing enabled in the switch's ingress filter, the misbehaving traffic is now limited to
approximately its configured committed information rate of 40 Mbps - the peaks
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
traffic bursts, packets are dropped when tokens are insufficient. The
solid line shows the packets that passed through - these correspond to periods
when the token bucket had sufficient tokens. The committed burst size of 10 kB
allows some short-term bursts to pass, but sustained excess traffic is dropped
to enforce the 40 Mbps committed information rate. Now let's see the same for the normal traffic's filter:

.. figure:: media/normal_filter.png
   :align: center

The normal traffic filter shows that virtually all packets pass through with no drops. Since the normal traffic operates well within its
configured limit (20 Mbps committed information rate with 5 kB committed burst
size), the token bucket always has sufficient tokens available.

This chart shows the number of tokens available in each bucket over time:

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
