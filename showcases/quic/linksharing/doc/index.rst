QUIC Bandwidth Sharing on a Bottleneck Link
===========================================

Goals
-----

This showcase demonstrates how QUIC's congestion control mechanism ensures fair
bandwidth sharing among multiple competing flows on a shared bottleneck link.
Congestion control is a critical component of transport protocols, responsible
for detecting available network capacity and adjusting transmission rates to
avoid overwhelming the network while maximizing throughput.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/quic/linksharing <https://github.com/inet-framework/inet/tree/master/showcases/quic/linksharing>`__

The Model
---------

In this simulation, three QUIC senders transmit data to their respective receivers
through a shared bottleneck link. The senders start and stop at different times,
creating a dynamic scenario where the number of active flows changes during the
simulation. The showcase illustrates several important aspects of QUIC's
congestion control:

- **Fair bandwidth allocation**: When multiple flows compete for the same
  bottleneck, each active flow should receive an approximately equal share of
  the available bandwidth.
- **Dynamic adaptation**: As flows join or leave, the remaining flows should
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
   :width: 70%
   :align: center

The bottleneck link has the following characteristics:

- **Bandwidth**: 10 Mbps
- **Delay**: 10 ms
- **MTU**: 1280 bytes
- **Queue capacity**: 200 kilobit, which equals the Bandwidth-Delay Product (BDP)

All other links are configured as high-speed channels with 1 Gbps datarate to
ensure the bottleneck is the only limiting factor in the network.

Configuration and Behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~

The three QUIC senders transmit data with a staggered timing pattern:

.. code-block:: text

   Seconds 0   10   20   30   40   50   60   70   80   90   100
           |   |    |    |    |    |    |    |    |    |    |
   Sender1     xxxxxxxxxxxxxxxxxxxxxxxxxx
   Sender2          xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
   Sender3                    xxxxxxxxxxxxxxxxxxxxxxxxxx

- **Sender1** starts at 10s, stops at 60s (50 seconds active)
- **Sender2** starts at 20s, stops at 80s (60 seconds active)
- **Sender3** starts at 40s, stops at 90s (50 seconds active)

Each sender uses the ``TrafficgenCompound`` application with ``TrafficgenSimple``
generator configured to keep feeding the connections with data to send.

Key QUIC configuration parameters:

- **Congestion control**: NewReno
- **Flow control**: Effectively disabled with large window sizes (4 GB)
- **Send queue limit**: 100 KB per sender
- **ACK policy**: Bundle ACKs for non-ACK-eliciting packets

The random UDP traffic generator sends packets of varying sizes (100-1000 bytes)
at random intervals (25-100 ms) throughout the simulation to add some variability
to the network conditions.

Results
-------

Throughput Over Time
~~~~~~~~~~~~~~~~~~~~

The following plot shows the achieved throughput of all senders over the
simulation duration, measured every 100 ms:

.. figure:: media/throughput.png
   :width: 100%

The plot clearly demonstrates fair bandwidth sharing:

- **0-10s**: No QUIC traffic yet
- **10-20s**: Only ``sender1`` is active, utilizing the full bottleneck capacity (~10 Mbps)
- **20-40s**: ``sender1`` and ``sender2`` are active, each achieving approximately 5 Mbps (fair share)
- **40-60s**: All three senders are active, each achieving approximately 3.3 Mbps (fair share)
- **60-80s**: ``sender1`` has stopped, ``sender2`` and ``sender3`` share bandwidth (~5 Mbps each)
- **80-90s**: Only ``sender3`` remains, utilizing the full capacity

The congestion control algorithms successfully detect the available bandwidth
and adjust transmission rates so that each active sender receives an equal share.
Minor variations are due to the random background traffic and the dynamic nature
of congestion control.

Queue Occupancy
~~~~~~~~~~~~~~~

The bottleneck router's queue occupancy shows how congestion control prevents
queue overflow:

.. figure:: media/queue_occupancy.png
   :width: 100%

The queue remains stable, hovering around the BDP value, which indicates that
senders are efficiently utilizing the available bandwidth without causing
excessive buffering. The queue capacity is set to accommodate the BDP:
``10 Mbps * 2 * 10 ms = 25 KB``.

Packet Loss
~~~~~~~~~~~

Packet loss at the bottleneck is minimal, demonstrating effective congestion
control:

.. figure:: media/packet_loss.png
   :width: 100%

The low packet loss rate confirms that QUIC's congestion control successfully
prevents queue overflow while maintaining high link utilization. Occasional
packet losses occur as part of the normal congestion control feedback mechanism.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`shared_link.ned <../shared_link.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/quic/shared_link`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/quic/shared_link && inet'

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

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TBD>`__ in
the GitHub issue tracker for commenting on this showcase.
