Step 4. Adding Error Detection
==========================

Goals
-----

In the fourth step, we enhance our network model by adding error detection
capabilities. In real networks, transmission errors can occur due to various
factors such as noise, interference, and hardware issues. We add Ethernet FCS
(Frame Check Sequence) header inserter and checker modules to detect these
errors, and we introduce a lossy channel to simulate transmission errors.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step4.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network4.ned
   :language: ned
   :start-at: network Network4
   :end-at: }

The network still consists of a client and a server, but now includes modules for
error detection and a lossy channel to simulate transmission errors.

The Hosts
~~~~~~~~~

The client host (``ClientHost4``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketQueue`` module that buffers packets
- An ``InstantServer`` module that processes packets
- An ``EthernetFcsHeaderInserter`` module that adds FCS headers for error detection
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost4``) now contains:

- A ``PassivePacketSink`` module that consumes packets
- An ``EthernetFcsHeaderChecker`` module that checks FCS headers for errors
- A ``PacketReceiver`` module that handles the reception of packets

Error Detection
~~~~~~~~~~~~~~

The ``EthernetFcsHeaderInserter`` module in the client adds a Frame Check Sequence
(FCS) header to each packet before transmission. The FCS is a checksum calculated
from the packet data, which allows the receiver to detect if the packet was
corrupted during transmission.

The ``EthernetFcsHeaderChecker`` module in the server checks the FCS of each
received packet. If the calculated checksum doesn't match the one in the FCS
header, it means the packet was corrupted during transmission, and the packet is
discarded.

This error detection mechanism is essential in real networks to ensure data
integrity. Without it, corrupted packets would be processed as if they were
valid, potentially causing issues in higher-layer protocols and applications.

Lossy Channel
~~~~~~~~~~~~

The connection between the hosts now has a bit error rate (BER) of 1E-5, which
means that, on average, 1 out of every 100,000 bits will be flipped during
transmission. This simulates a lossy channel where transmission errors can occur.

When a bit error occurs in a packet, the FCS check at the receiver will fail, and
the packet will be discarded. This models the behavior of real networks where
corrupted packets are detected and dropped.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network4]
   :end-before: [Config Network5]

The configuration sets the network to ``Network4`` and extends the ``Base2``
configuration, which includes the data rate settings for the transmitter and
receiver.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are placed in the ``PacketQueue``, processed by
the ``InstantServer``, and then passed to the ``EthernetFcsHeaderInserter`` which
adds an FCS header for error detection.

The packets are then transmitted over the lossy channel, where some bits may be
flipped due to the configured bit error rate. When a packet arrives at the
server, the ``PacketReceiver`` converts the signal back into a packet, which is
then passed to the ``EthernetFcsHeaderChecker``.

The ``EthernetFcsHeaderChecker`` calculates the checksum of the received packet
and compares it with the one in the FCS header. If they match, the packet is
considered valid and is passed to the ``PassivePacketSink`` for consumption. If
they don't match, the packet is considered corrupted and is discarded.

This model introduces the concept of error detection, which is a fundamental
aspect of reliable network communication. In the next step, we'll add interpacket
gap insertion to model the spacing between consecutive packet transmissions in
real networks.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network4.ned <../Network4.ned>`
