Step 6. Adding Packet Resending
============================

Goals
-----

In the sixth step, we enhance our network model by adding a mechanism for
resending lost or corrupted packets. In real networks, packet loss is inevitable
due to various factors such as congestion, hardware failures, and transmission
errors. To ensure reliable communication, many protocols implement a resending
mechanism where packets that are not acknowledged by the receiver are resent. We
add a Resending module to model this behavior.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step6.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network6.ned
   :language: ned
   :start-at: network Network6
   :end-at: }

The network still consists of a client and a server, but the client now includes
an additional module for resending packets that are lost or corrupted.

The Hosts
~~~~~~~~~

The client host (``ClientHost6``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketQueue`` module that buffers packets
- An ``InstantServer`` module that processes packets
- A ``Resending`` module that handles packet resending
- An ``EthernetFcsHeaderInserter`` module that adds FCS headers for error detection
- An ``InterpacketGapInserter`` module that enforces a minimum time gap between packets
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost6``) remains the same as in the previous step, with:

- A ``PassivePacketSink`` module that consumes packets
- An ``EthernetFcsHeaderChecker`` module that checks FCS headers for errors
- A ``PacketReceiver`` module that handles the reception of packets

Packet Resending
~~~~~~~~~~~~~~

The ``Resending`` module in the client implements a simple automatic repeat request
(ARQ) mechanism. When a packet is sent, the module waits for an acknowledgment
(ACK) from the receiver. If an ACK is not received within a certain timeout
period, the module assumes the packet was lost or corrupted and resends it.

In this model, the ``Resending`` module is configured to retry sending a packet up
to 3 times if it is not acknowledged. This means that a packet can be sent up to
4 times in total (the original transmission plus 3 retries) before it is
considered permanently lost.

This resending mechanism is essential for reliable communication in networks
where packet loss can occur. Without it, lost packets would never reach the
receiver, potentially causing issues in higher-layer protocols and applications.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network6]
   :end-before: [Config Network7]

The configuration sets the network to ``Network6`` and extends the ``Base4``
configuration, which includes the ``Base3`` settings and adds the number of
retries configuration for the resending module.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are placed in the ``PacketQueue``, processed by
the ``InstantServer``, and then passed to the ``Resending`` module.

The ``Resending`` module sends each packet and waits for an ACK. If an ACK is not
received within the timeout period, the module resends the packet. This process
continues until either an ACK is received or the maximum number of retries (3) is
reached.

After a packet is successfully sent (or the maximum number of retries is
reached), the packet is passed to the ``EthernetFcsHeaderInserter``,
``InterpacketGapInserter``, and ``PacketTransmitter`` as in the previous steps.

When a packet arrives at the server, it is processed as in the previous steps. If
the packet is not corrupted (as determined by the FCS check), the server sends an
ACK back to the client. If the packet is corrupted, no ACK is sent, and the
client will eventually resend the packet.

This model introduces the concept of packet resending, which is a fundamental
aspect of reliable network communication. In the next step, we'll add sequence
numbering and reordering to handle out-of-order packet delivery, which can occur
in real networks due to various factors such as routing changes and packet
retransmissions.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network6.ned <../Network6.ned>`
