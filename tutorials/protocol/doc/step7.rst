Step 7. Adding Sequence Numbering and Reordering
==========================================

Goals
-----

In the seventh step, we enhance our network model by adding sequence numbering
and packet reordering capabilities. In real networks, packets can arrive out of
order due to various factors such as routing changes, packet retransmissions, and
network congestion. To ensure correct processing of packets, many protocols
implement sequence numbering and packet reordering. We add SequenceNumbering and
Reordering modules to model this behavior.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step7.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network7.ned
   :language: ned
   :start-at: network Network7
   :end-at: }

The network still consists of a client and a server, but now includes modules for
sequence numbering and packet reordering.

The Hosts
~~~~~~~~~

The client host (``ClientHost7``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketQueue`` module that buffers packets
- An ``InstantServer`` module that processes packets
- A ``SequenceNumbering`` module that adds sequence numbers to packets
- A ``Resending`` module that handles packet resending
- An ``EthernetFcsHeaderInserter`` module that adds FCS headers for error detection
- An ``InterpacketGapInserter`` module that enforces a minimum time gap between packets
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost7``) now contains:

- A ``PassivePacketSink`` module that consumes packets
- A ``Reordering`` module that reorders out-of-sequence packets
- An ``EthernetFcsHeaderChecker`` module that checks FCS headers for errors
- A ``PacketReceiver`` module that handles the reception of packets

Sequence Numbering and Reordering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``SequenceNumbering`` module in the client adds a sequence number to each
packet before it is sent. The sequence number is a unique identifier that
indicates the order in which packets were sent. This allows the receiver to
determine if packets have arrived out of order.

The ``Reordering`` module in the server uses these sequence numbers to reorder
packets that arrive out of sequence. It buffers out-of-sequence packets until the
missing packets arrive, ensuring that packets are delivered to the application in
the correct order.

This sequence numbering and reordering mechanism is essential for reliable
communication in networks where packets can arrive out of order. Without it,
out-of-sequence packets would be processed in the wrong order, potentially
causing issues in higher-layer protocols and applications.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network7]
   :end-before: [Config Network8]

The configuration sets the network to ``Network7`` and extends the ``Base4``
configuration, which includes the settings for the resending module.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are placed in the ``PacketQueue``, processed by
the ``InstantServer``, and then passed to the ``SequenceNumbering`` module, which
adds a sequence number to each packet.

The packets are then passed to the ``Resending`` module, which handles packet
resending as in the previous step. After that, the packets are processed by the
``EthernetFcsHeaderInserter``, ``InterpacketGapInserter``, and
``PacketTransmitter`` as before.

When a packet arrives at the server, it is processed by the ``PacketReceiver`` and
``EthernetFcsHeaderChecker`` as in the previous steps. If the packet is not
corrupted, it is passed to the ``Reordering`` module.

The ``Reordering`` module checks the sequence number of the packet. If the packet
is in sequence (i.e., its sequence number is the next expected one), it is passed
directly to the ``PassivePacketSink``. If the packet is out of sequence, it is
buffered until the missing packets arrive.

This model introduces the concepts of sequence numbering and packet reordering,
which are fundamental aspects of reliable network communication. These mechanisms
ensure that packets are processed in the correct order, even if they arrive out
of sequence due to network conditions.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network7.ned <../Network7.ned>`
