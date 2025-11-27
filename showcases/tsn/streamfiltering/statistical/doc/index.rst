Statistical Policing
====================

Goals
-----

In shared network environments, misbehaving traffic sources can disrupt other
applications. When a client generates traffic that exceeds available bandwidth,
it can interfere with other clients sharing the same infrastructure. This is
particularly problematic in Time-Sensitive Networking (TSN) environments where
predictable and reliable communication is essential.

Per-stream filtering and policing addresses this problem by limiting excessive
traffic and ensuring fair resource allocation. This showcase demonstrates an
alternative approach to per-stream policing using statistical methods:

- **Token bucket policing:** Provides deterministic rate limiting through token
  management—packets pass if tokens are available, otherwise they are dropped.
- **Statistical policing:** Uses a sliding window rate meter to continuously
  measure traffic rates, combined with a statistical rate limiter that
  probabilistically drops packets when rates exceed configured limits. This
  simpler method gradually increases drop probability as traffic exceeds limits.

Both approaches effectively limit excessive traffic while allowing normal
traffic to flow unimpeded, but with different characteristics in their
enforcement mechanisms.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/statistical>`__

Background
----------

Statistical per-stream policing operates through a three-stage pipeline:
streams are first identified and classified, then metered to measure their
traffic rates, and finally filtered based on whether they exceed configured
limits. This section explains the metering and filtering components specific to
statistical policing.

Stream Filtering and Policing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Per-stream filtering and policing in TSN limits excessive traffic to prevent
network disruption. This functionality is implemented in the bridging layer of
TSN switches. See the :doc:`/showcases/tsn/streamfiltering/tokenbucket/doc/index`
for detailed background on the filtering architecture and the role of meters and
filters within the :ned:`StreamFilterLayer`.

Sliding Window Rate Meter
~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`SlidingWindowRateMeter` measures traffic rates by continuously tracking
packets over a configurable time window, providing a smoothed, time-averaged
view of the traffic rate.

Key parameter:

- :par:`timeWindow`: The duration over which traffic is measured (e.g., 15 ms).
  This parameter controls a fundamental trade-off: longer windows allow shorter
  bursts to pass while introducing more measurement lag (~15 ms in this case);
  shorter windows reduce lag but filter bursts more aggressively.

Statistical Rate Limiter
~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`StatisticalRateLimiter` uses the measured rate from the meter to make
probabilistic dropping decisions. It compares the measured data rate to a
configured maximum data rate and drops packets with a probability that increases
as the measured rate exceeds the limit.

Key parameter:

- :par:`maxDatarate`: The target rate limit (e.g., 40 Mbps)

This probabilistic approach differs fundamentally from token bucket policing:

- **Token bucket:** Deterministic behavior—packets pass if tokens are available,
  fail otherwise, creating sharp cutoffs.
- **Statistical limiter:** Gradual enforcement—drop probability increases
  progressively as traffic exceeds limits, allowing some fluctuation around the
  configured rate.

The Model
---------

We use the following network topology:

.. figure:: media/Network.png
   :align: center
   :width: 100%

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

**Traffic Patterns**

Two distinct traffic patterns are generated:

- **client1 (misbehaving):** Produces traffic at an average rate of 40 Mbps
  using small 25-byte packets with a dual-frequency sinusoidal pattern for
  packet intervals. This creates highly variable, bursty traffic that
  oscillates between different rates.

- **client2 (normal):** Generates steady traffic at 20 Mbps using 500-byte
  packets at regular intervals.

The misbehaving traffic uses small packets to provide finer temporal resolution
in the data rate plots for demonstration purposes. However, this means the
network data rate will be significantly higher than the 40 Mbps application data
rate due to protocol overhead (each 25-byte payload requires 62 bytes of
overhead: 12B interframe gap + 8B preamble + 14B MAC header + 20B IP header +
8B UDP header = 62B). As shown in the Results section, the combined traffic will
frequently exceed the 100 Mbps channel capacity between the switch and server.

.. note:: **Why Filtering Instead of Shaping?**
   
   Both traffic streams belong to the same traffic class. If they were in
   different classes, traffic shaping could solve the problem. However, since
   they share the same class, both normal and misbehaving streams are mixed in
   the shaper, requiring per-stream filtering to distinguish and limit the
   misbehaving traffic.

The bridging layer identifies outgoing packets by their UDP destination port.

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
   :end-before: SlidingWindowRateMeter
   :language: ini

Statistical Meter and Limiter Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We use a :ned:`SlidingWindowRateMeter` combined with a :ned:`StatisticalRateLimiter` for both streams. 
The meter continuously measures the traffic rate over a sliding time window, while the limiter 
probabilistically drops packets when the measured rate exceeds the configured maximum.

Key parameters:

- :par:`timeWindow`: The duration over which traffic is measured (15ms for both streams)
- :par:`maxDatarate`: The target rate limit (40 Mbps for misbehaving traffic, 20 Mbps for normal traffic)

As a rule of thumb, bursts significantly smaller than the time window are passed.

.. note:: Generally in policing, we want to allow some short bursts, and penalize sustained misbehaving traffic, while always staying below link capacity.

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: SlidingWindowRateMeter

The following figure illustrates the per-stream filtering architecture, showing
how traffic is classified, metered, and filtered inside the bridging layer of the switch:

.. figure:: media/StreamFiltering_SimpleIeee8021qFilter2.png
   :align: center
   :width: 100%


Results
-------

The Problem
~~~~~~~~~~~

This section demonstrates the problem that statistical policing solves: without
policing, misbehaving traffic disrupts well-behaved traffic sharing the same
network infrastructure.

**Application-Level Traffic**

The first chart shows the application-level traffic generated by both clients
(client1 with misbehaving sinusoidal pattern, client2 with normal steady rate):

.. figure:: media/client.png
   :align: center

The combined application-level traffic averages approximately 60 Mbps. However,
due to the high protocol overhead of client1's misbehaving traffic—each 25-byte
payload requires 62 bytes of overhead (12B interframe gap + 8B preamble + 14B
MAC header + 20B IP header + 8B UDP header)—the misbehaving stream alone is
sufficient to saturate a 100 Mbps channel at the network level.

**Network-Level Traffic**

The following chart shows the outgoing network-level traffic at both clients,
illustrating the protocol overhead impact:

.. figure:: media/networkdatarate.png
   :align: center

The misbehaving traffic stream saturates the channel between client1 and the
switch when its rate peaks, causing the visible clipping of the sinusoid
pattern at around 86 Mbps. This clipping occurs below the 100 Mbps channel
capacity because with small 25-byte packets, a significant portion of the
channel is consumed by the mandatory 12-byte interframe gap between packets.

The combined network traffic frequently exceeds the 100 Mbps channel capacity
between the switch and the server.

**Traffic at Server (Without Policing)**

The next chart shows the traffic received at the server without policing, where
both streams compete for bandwidth:

.. figure:: media/server_nopolicing.png
   :align: center

The misbehaving traffic competes with normal traffic for bandwidth, slowing
normal traffic during congestion periods and occasionally speeding it up when
the misbehaving traffic has lower rates.

**End-to-End Delay (Without Policing)**

The end-to-end delay of the normal stream reveals the severity of the problem,
with significant delay spikes without policing:

.. figure:: media/delay_nopolicing.png
   :align: center

When the combined traffic exceeds the link capacity, packets queue at the
switch, causing delays. Without policing, the normal traffic experiences
significant delay spikes reaching almost 40 ms when the misbehaving traffic
peaks. This demonstrates the core problem: well-behaved traffic is disrupted by
excessive traffic sources sharing the same network infrastructure.

The Solution
~~~~~~~~~~~~

This section shows how enabling statistical policing in the switch solves the
problem by limiting misbehaving traffic while protecting normal traffic.

**Application-Level Traffic at Server (With Policing)**

The following charts compare application-level traffic at the server with and
without policing. With statistical policing enabled in the switch's ingress
filter, the misbehaving traffic is limited to approximately 40 Mbps
(network-level), while the normal traffic flows at its expected 20 Mbps rate,
unaffected by the misbehaving stream:

.. figure:: media/server_excess.png
   :align: center

.. figure:: media/server_normal.png
   :align: center

Due to the small packets and large overhead, the application-level data rate at
the server is lower (approximately 20 Mbps) than the 40 Mbps network-level
limit. The statistical rate limiter probabilistically drops excess packets,
enforcing the rate limit.

The normal traffic continues to flow at its expected rate of 20 Mbps
(network-level), unaffected by the misbehaving traffic. The total traffic to the
server stays well within the link capacity.

**Characteristics of Statistical Policing**

Notice that the policed misbehaving traffic fluctuates around the 40 Mbps limit
rather than being strictly capped. This is characteristic of statistical
policing: the sliding window rate meter measures traffic over a 15 ms time
window, and the statistical rate limiter probabilistically drops packets based on
the measured rate. This creates gradual, probabilistic enforcement that allows
some variation while maintaining the average rate at the configured limit. This
contrasts with token bucket policing, which creates sharper cutoffs when tokens
are depleted.

**End-to-End Delay (With Policing)**

The end-to-end delay comparison demonstrates the effectiveness of policing, with
normal traffic experiencing consistently low delays (~0.1 ms) without
significant spikes:

.. figure:: media/delay_normal.png
   :align: center

With policing enabled, the normal traffic experiences consistently low delays,
typically around 0.1 ms, without any significant spikes. The statistical
policing mechanism successfully protects the normal traffic from disruption by
the misbehaving traffic. This demonstrates a key benefit of per-stream policing
in TSN environments: ensuring predictable performance for well-behaved streams
even when sharing network infrastructure with excessive traffic sources.

Additional Details
~~~~~~~~~~~~~~~~~~

This section provides deeper insights into the policing mechanism's internal
operation and explains some observed behaviors in the delay measurements.

**Delay Variation and VLAN Tag Overhead**

The following chart shows a zoomed view of the end-to-end delay of the normal
stream during periods when delays are low in both scenarios, showing delay
variation and consistent offset between scenarios:

.. figure:: media/delay_normal_zoomed.png
   :align: center

The delay variation occurs because both traffic streams are mixed in the
switch's output queue. This is standard queueing behavior: when packets from
both streams are queued simultaneously, a packet from one stream may need to
wait for a packet from the other stream to complete transmission, introducing
variable delays depending on the remaining transmission duration.

Note the consistent offset between the two delay values: the scenario without
policing shows slightly lower delays than the scenario with policing. This
difference is due to the absence of VLAN tags (IEEE 802.1Q headers) in the
non-policing configuration.

**Filter Operation: Misbehaving Traffic**

The following chart shows the incoming, outgoing, and dropped data rates in the
misbehaving traffic's filter, along with the measured data rate. The filter
starts dropping packets when the measured rate exceeds 40 Mbps:

.. figure:: media/excess_filter.png
   :align: center

Key observations:

- **Packet dropping threshold:** The filter starts dropping packets when the
  measured data rate (from the sliding window meter) exceeds 40 Mbps.

- **Measurement lag:** The measured data rate shows a lag of approximately 15 ms
  behind the incoming traffic. This delay occurs because the meter must
  accumulate a full time window (15 ms) of packet data before producing a
  measurement.

- **Burst handling:** The 15 ms time window allows short, high-frequency bursts
  to pass through largely intact (visible at the start of the chart). These
  bursts are comparable in duration to the time window, so the sliding window
  meter doesn't immediately register them as exceeding the limit. However, when
  traffic remains persistently high, the filter limits the outgoing rate to
  approximately 40 Mbps by dropping excess packets.

- **Time window trade-off:** A shorter time window would reduce measurement lag
  but would make the meter more reactive to short bursts, filtering them more
  aggressively. A longer window allows more bursts to pass but introduces more
  lag. The 15 ms window strikes a balance between these competing concerns.

**Filter Operation: Normal Traffic**

The filter behavior for normal traffic is straightforward, with all packets
passing through without any drops:

.. figure:: media/normal_filter.png
   :align: center

All packets pass through with no drops since the traffic operates well within
its configured 20 Mbps limit. The statistical rate limiter has no reason to drop
packets when the measured rate stays below the threshold.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/streamfiltering/statistical`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/showcases/tsn/streamfiltering/statistical && inet'

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

Use `this page in the GitHub issue tracker <https://github.com/inet-framework/inet/discussions/794>`__ for commenting on this showcase.
