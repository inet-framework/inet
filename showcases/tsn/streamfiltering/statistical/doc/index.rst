Statistical Policing
====================

Goals
-----

In shared network environments, misbehaving or excessive traffic sources can
disrupt other applications. For example, a client generates excessive traffic
that exceeds the available bandwidth, potentially disrupting traffic from other
clients sharing the same network infrastructure.

This is particularly problematic in Time-Sensitive Networking (TSN) environments
where predictable and reliable communication is essential. One solution to this
problem is per-stream filtering and policing. This approach allows us to limit
excessive traffic and protect well-behaved streams from disruption, ensuring
fair resource allocation across different streams.

Building on the token bucket policing showcase, this showcase demonstrates an
alternative approach to per-stream policing using statistical methods. While
token bucket policing provides deterministic rate limiting through token
management, this approach uses a sliding window rate meter to continuously
measure traffic rates combined with a statistical rate limiter that
probabilistically drops packets when rates exceed configured limits. This is a simpler method
that gradually increases drop probability as traffic exceeds limits. We implement the
same scenario where one client generates excessive traffic while another
generates normal traffic, and show how statistical policing can effectively
limit the excessive traffic while allowing the normal traffic to flow unimpeded.

| Verified with INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/statistical <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/statistical>`__

Background
----------

Stream Filtering and Policing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Per-stream filtering and policing in TSN limits excessive traffic to prevent
network disruption. This functionality is implemented in the bridging layer of
TSN switches. See the `token bucket policing showcase <../tokenbucket/doc/index.html>`_
for detailed background on the filtering architecture and the role of meters and
filters within the StreamFilterLayer.

Sliding Window Rate Meter
~~~~~~~~~~~~~~~~~~~~~~~~~

The sliding window rate meter measures traffic rates by continuously tracking
packet bytes over a configurable time window. Unlike token buckets that make
instantaneous pass/fail decisions based on token availability, the sliding
window provides a smoothed, time-averaged view of the traffic rate.

Key parameter:

- :par:`timeWindow`: The duration over which traffic is measured (e.g., 15ms)

Statistical Rate Limiter
~~~~~~~~~~~~~~~~~~~~~~~~

The statistical rate limiter uses the measured rate from the meter to make
probabilistic dropping decisions. It compares the measured data rate to a
configured maximum data rate and drops packets with a probability that increases
as the measured rate exceeds the limit.

Key parameter:

- :par:`maxDatarate`: The target rate limit (e.g., 40 Mbps)

Unlike token bucket policing which provides deterministic behavior (packet
passes if tokens available, fails otherwise), statistical policing provides
gradual rate enforcement through probabilistic packet dropping.

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

Two distinct traffic patterns are generated. ``client1`` produces misbehaving
traffic at an average rate around 40 Mbps using small 25-byte packets with a
complex dual-frequency sinusoidal pattern for packet intervals. This creates
highly variable, bursty traffic that oscillates between different rates.
Meanwhile, ``client2`` generates steady normal traffic at 20 Mbps using 500-byte
packets at regular intervals.

The misbehaving traffic uses small packets with high packet rates to provide
finer temporal resolution in the data rate plots. However, this also means that
the network data rate will be significantly higher than the 40 Mbps application
data rate due to protocol overhead. The combined traffic from both streams
at times will exceed the 100Mbps channel capacity between the switch and the server,
as we'll see in the Results section.

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

Unlike token bucket policing with its deterministic pass/fail decisions, the statistical approach 
gradually increases drop probability as traffic exceeds limits, providing more graceful rate enforcement.
.. not sure this should be here

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

The first charts demonstrate the problem we are addressing: excessive,
misbehaving traffic disrupting normal traffic in a shared network environment,
without policing. 

Here is the application traffic generated by the clients:

.. figure:: media/client.png
   :align: center

The combined application-level traffic averages approximately 60 Mbps, but due
to high protocol overhead of the misbehaving traffic—each 25-byte packet
requires approximately 50 bytes of overhead (14B MAC header, 8B PHY preamble, 8B
interframe gap, 20B IP/UDP headers)—the misbehaving stream alone is enough to
saturate a 100 Mbps channel.
The following chart shows the outgoing network-level traffic at client1
(misbehaving) and client2 (normal):

.. figure:: media/networkdatarate.png
   :align: center

The combined network traffic frequently exceeds the 100 Mbps channel capacity
between the switch and the server. The next chart shows the traffic received at
the server without policing:

.. figure:: media/server_nopolicing.png
   :align: center

When the combined traffic exceeds the link capacity, packets must queue at the
switch, leading to delays. The misbehaving traffic competes with normal traffic
for bandwidth, slowing down the normal stream.

Here is the delay of the normal stream:

.. figure:: media/delay_nopolicing.png
   :align: center

.. figure:: media/server.png
   :align: center

.. figure:: media/delay_normal.png
   :align: center

.. figure:: media/excess_filter.png
   :align: center

.. figure:: media/normal_filter.png
   :align: center

.. figure:: media/StreamFiltering_SimpleIeee8021qFilter2.png
   :align: center
   :width: 100%

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

Use `this <https://github.com/inet-framework/inet/discussions/794>`__ page in the GitHub issue tracker for commenting on this showcase.

