QUIC Bandwidth Sharing on a Bottleneck Link
===========================================

Goals
-----

This showcase demonstrates how QUIC's congestion control mechanism ensures fair
bandwidth sharing among multiple competing connections on a shared bottleneck link.
Congestion control is a critical component of transport protocols, responsible
for detecting available network capacity and adjusting transmission rates to
avoid overwhelming the network while maximizing throughput.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/quic/linksharing <https://github.com/inet-framework/inet/tree/master/showcases/quic/linksharing>`__

About QUIC
----------

QUIC (Quick UDP Internet Connections) is a modern transport layer protocol that
operates over UDP to deliver faster and more reliable connections than TCP.
About a third of all web traffic today uses QUIC, including HTTP/3.

It significantly reduces latency by combining the connection handshake with TLS 1.3 encryption,
allowing data to be exchanged almost immediately while ensuring robust security.
A key advantage is support for multiple streams in a single connection, which helps to
eliminate head-of-line blocking. Furthermore,
QUIC supports connection migration, allowing devices to seamlessly switch between networks
(such as moving from Wi-Fi to cellular data) without interrupting active downloads or calls.

QUIC was designed to fairly coexist with TCP and provide similar network-friendly
behavior, so it must use a congestion control algorithm compatible with TCP.
The QUIC implementation in INET uses NewReno.

The Model
---------

This simulation uses a dumbbell topology, where three QUIC senders transmit
data to their respective receivers through a shared bottleneck link. The
senders start and stop at different times, creating a dynamic scenario where
the number of active connections changes during the simulation. The showcase
illustrates several important aspects of QUIC's congestion control:

- **Fair bandwidth allocation**: When multiple connections compete for the same
  bottleneck, each active connection should receive an approximately equal share
  of the available bandwidth.
- **Dynamic adaptation**: As connections come and go, the remaining ones should
  quickly adapt to utilize the newly available (or newly reduced) capacity.
- **Efficient utilization**: The congestion control should keep the bottleneck
  link well-utilized without causing excessive queuing or packet loss.

Additionally, random (non-QUIC) background UDP traffic is present throughout the
simulation to model realistic network conditions and test the robustness of the
congestion control under variable network loads.

The Network
~~~~~~~~~~~

The network consists of three QUIC senders and their corresponding receivers,
connected through two routers. The link between the two routers serves as the
bottleneck, with limited bandwidth and a configured delay.

.. figure:: media/network.png
   :align: center
   :width: 70%

The bottleneck link has the following characteristics:

- **Bandwidth**: 10 Mbps (full-duplex)
- **Delay**: 10 ms (one-way)
- **MTU** (Maximum Transmission Unit): 1280 bytes
- **Queue capacity**: 200 kilobit (10 Mbps * 2 * 10ms = 200 kb = 25 kB), which equals the Bandwidth-Delay Product (BDP).
  This allows the link to be fully utilized while minimizing excess buffering.
- **Queueing discipline**: FIFO, DropTail (i.e., new packets are dropped when the queue is full, without any prioritization or active queue management)

All other links are configured as high-speed channels with 1 Gbps datarate and
no delay to ensure the bottleneck is the only limiting factor in the network.

Configuration and Behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~

The three QUIC senders transmit data with a staggered timing pattern:

.. code-block:: text

   Seconds 0    5    10   15   20   25   30   35   40   45   50
           |    |    |    |    |    |    |    |    |    |    |
   Sender1      xxxxxxxxxxxxxxxxxxxxxxxxxx
   Sender2           xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   Sender3                     xxxxxxxxxxxxxxxxxxxxxxxxxx

- **Sender1** starts at 5s, stops at 30s (25 seconds active)
- **Sender2** starts at 10s, stops at 40s (30 seconds active)
- **Sender3** starts at 20s, stops at 45s (25 seconds active)

Each sender is configured with an application that continuously provides data
for QUIC to transmit.

Key QUIC configuration parameters:

- **Congestion control**: NewReno (a classic congestion control algorithm)
- **Flow control**: Effectively disabled with large window sizes (4 GB) to isolate
  and study congestion control behavior without flow control limitations

The background UDP traffic generator sends packets of varying sizes (500-1000 bytes)
at random intervals (10-20 ms) throughout the simulation to add some variability
to the network conditions.

Results
-------

Throughput Over Time
~~~~~~~~~~~~~~~~~~~~

The following plot shows the achieved throughput at all receivers over the
simulation duration. It is measured at the application layer.

.. figure:: media/receiver_throughput.png
   :align: center
   :width: 70%

It can be observed that the total throughput closely approaches the bottleneck
capacity of 10 Mbps whenever there is active QUIC traffic, and the bandwidth is
fairly shared among the active connections.

The senders actively regulate their output data rate to prevent excess traffic,
ensuring that capacity on the upstream links is not wasted for packets that
would be dropped at the bottleneck. This is shown in the following plot:

.. figure:: media/sender_throughput.png
   :align: center
   :width: 70%

The output data rate of ``sender1`` initially overshoots the 10 Mbps bottleneck
capacity for a short time, causing some packets to be dropped, until the
congestion control algorithm corrects and adapts to the bottleneck. Similar
brief overshoots occur when ``sender2`` and ``sender3`` start transmitting,
as the existing connections adjust to share the available bandwidth. Notably,
the background UDP sender maintains a relatively constant transmission rate
throughout the simulation, as it has no congestion control mechanism to react
to network conditions. Therefore, it simply acts as a constant reduction in
the overall available bandwidth for QUIC on the bottleneck.

Averaging out short-term fluctuations with a smoothing window provides a
clearer view of the long-term behavior:

.. figure:: media/sender_throughput_smoothed.png
   :align: center
   :width: 70%

The plot clearly demonstrates fair bandwidth sharing:

- **0-5s**: No QUIC traffic yet
- **5-10s**: Only ``sender1`` is active, utilizing the full bottleneck capacity (~10 Mbps).
- **10-20s**: ``sender1`` and ``sender2`` are active, each achieving approximately 5 Mbps (fair share)
- **20-30s**: All three senders are active, each achieving approximately 3.3 Mbps (fair share)
- **30-40s**: ``sender1`` has stopped, ``sender2`` and ``sender3`` share bandwidth (~5 Mbps each)
- **40-45s**: Only ``sender3`` remains, utilizing the full capacity
- **45-50s**: No QUIC traffic, only background UDP

The congestion control algorithms successfully detect the available bandwidth
and adjust transmission rates so that each active sender receives an equal share.
Minor variations are due to the random background traffic and the dynamic nature
of congestion control.

.. note::

   Bandwidth is measured by adding up the bytes received at each receiver
   over time in fixed intervals (0.5s), divided by the length of the interval.

Packet Loss
~~~~~~~~~~~

Packet loss at the bottleneck is minimal, demonstrating effective congestion
control:

.. figure:: media/packet_drop_rate.png
   :align: center
   :width: 70%

The low packet loss rate confirms that QUIC's congestion control successfully
prevents queue overflow while maintaining high link utilization.
Occasional packet losses occur as part of the normal congestion control feedback
mechanism - when the queue fills and packets are dropped, senders detect congestion
and reduce their transmission rates. There are distinct spikes when each sender
starts transmitting (at 5s, 10s, and 20s) as they probe for available bandwidth.

Congestion Control
~~~~~~~~~~~~~~~~~~

The congestion window (cwnd) evolution of the three senders illustrates how
QUIC's NewReno congestion control adapts to changing network conditions,
while each connection continuously tries to probe for additional bandwidth:
This results in a typical sawtooth pattern in the value of the congestion window
over time.

.. figure:: media/congestion_window.png
   :align: center
   :width: 70%

The initial ramp-up reflects the slow-start phase where the congestion window grows
exponentially until it detects the network capacity.

Whenever a packet is dropped, the congestion window is decreased accordingly,
and the queue can then be drained, preventing further losses. However, the
senders quickly ramp up their congestion windows again to utilize the available
bandwidth.

This can be seen in the following zoomed-in plot:

.. figure:: media/cwnd_queue_drops.png
   :align: center
   :width: 70%

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`linksharing.ned <../linksharing.ned>`

Conclusions
-----------

This showcase demonstrates several key principles of congestion control in QUIC:

1. **Fair resource allocation**: Multiple competing flows automatically converge to
   equal bandwidth shares without any centralized coordination, with each flow
   receiving approximately 1/N of the bottleneck capacity when N flows are active.

2. **Dynamic adaptability**: The congestion control mechanism quickly responds to
   changes in network conditions, rapidly redistributing bandwidth when flows join
   or leave the network.

3. **Efficient utilization**: The bottleneck link remains well-utilized throughout
   the simulation while maintaining stable queue occupancy and minimal packet loss.

4. **Robust operation**: The presence of random background traffic does not
   significantly impact the fairness or stability of the bandwidth sharing.

These results confirm that QUIC's NewReno congestion control (inherited from TCP)
successfully balances the competing goals of high throughput, low latency, and
fair resource sharing in multi-flow scenarios.

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/quic/linksharing`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/quic/linksharing && inet'

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

Use `this page <https://github.com/inet-framework/inet/discussions/1070>`__ in
the GitHub issue tracker for commenting on this showcase.
