Step 5. Adding Interpacket Gap
===========================

Goals
-----

In the fifth step, we enhance our network model by adding interpacket gap
insertion. In real networks, especially in protocols like Ethernet, there is a
minimum time gap between consecutive packet transmissions, known as the
interpacket gap (IPG) or interframe spacing. This gap allows network devices to
prepare for the next packet and helps prevent congestion. We add an
InterpacketGapInserter module to model this behavior.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step5.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network5.ned
   :language: ned
   :start-at: network Network5
   :end-at: }

The network still consists of a client and a server, but the client now includes
an additional module for inserting interpacket gaps.

The Hosts
~~~~~~~~~

The client host (``ClientHost5``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketQueue`` module that buffers packets
- An ``InstantServer`` module that processes packets
- An ``EthernetFcsHeaderInserter`` module that adds FCS headers for error detection
- An ``InterpacketGapInserter`` module that enforces a minimum time gap between packets
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost5``) remains the same as in the previous step, with:

- A ``PassivePacketSink`` module that consumes packets
- An ``EthernetFcsHeaderChecker`` module that checks FCS headers for errors
- A ``PacketReceiver`` module that handles the reception of packets

Interpacket Gap Insertion
~~~~~~~~~~~~~~~~~~~~~~~

The ``InterpacketGapInserter`` module in the client enforces a minimum time gap
between consecutive packet transmissions. It delays the transmission of a packet
if the previous packet was transmitted too recently, ensuring that there is at
least the configured gap duration between the end of one packet's transmission
and the start of the next.

In this model, the interpacket gap is set to 1ms, which means there will be at
least a 1ms gap between the end of one packet's transmission and the start of the
next. This is much larger than the typical interpacket gap in real Ethernet
networks (which is usually around 96 bit times, or 9.6us for 10Mbps Ethernet),
but it makes the effect more noticeable in the simulation.

The interpacket gap is an important aspect of many network protocols, as it
provides time for network devices to process packets and helps prevent
congestion. Without it, back-to-back packet transmissions could overwhelm
receivers and intermediate devices.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network5]
   :end-before: [Config Network6]

The configuration sets the network to ``Network5`` and extends the ``Base3``
configuration, which includes the ``Base2`` settings and adds the interpacket gap
duration configuration.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are placed in the ``PacketQueue``, processed by
the ``InstantServer``, and then passed to the ``EthernetFcsHeaderInserter`` which
adds an FCS header for error detection.

The packets are then passed to the ``InterpacketGapInserter``, which enforces a
minimum time gap of 1ms between consecutive packet transmissions. If a packet
arrives at the ``InterpacketGapInserter`` less than 1ms after the previous packet
was transmitted, it will be delayed until the gap duration has elapsed.

After the interpacket gap is enforced, the packets are transmitted over the lossy
channel, where some bits may be flipped due to the configured bit error rate.
When a packet arrives at the server, it is processed as in the previous step.

This model introduces the concept of interpacket gap, which is an important
aspect of many network protocols. In the next step, we'll add a mechanism for
resending packets that are lost or corrupted during transmission, making our
communication more reliable.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network5.ned <../Network5.ned>`
