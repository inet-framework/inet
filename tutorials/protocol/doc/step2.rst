Step 2. Adding Packet Transmitter and Receiver
=========================================

Goals
-----

In the second step, we enhance our network model by adding packet transmitter and
receiver modules. These modules represent the physical layer functionality of
transmitting and receiving signals over a communication medium. This brings our
model closer to real-world network communication by introducing the concept of
data rate and transmission time.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step2.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network2.ned
   :language: ned
   :start-at: network Network2
   :end-at: }

The network still consists of a client and a server, but now each host includes
additional modules for handling packet transmission and reception. The connection
between them now has both a delay (1us) and a data rate (100Mbps).

The Hosts
~~~~~~~~~

The client host (``ClientHost2``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost2``) now contains:

- A ``PassivePacketSink`` module that consumes packets
- A ``PacketReceiver`` module that handles the reception of packets

Packet Transmission and Reception
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``PacketTransmitter`` module in the client receives packets from the
application as a whole and transmits them to the physical medium. It models the
process of converting packets into signals that can be transmitted over the
physical medium. The transmission takes time based on the packet size and the
configured data rate.

The ``PacketReceiver`` module in the server receives signals from the physical
medium and converts them back into packets that can be processed by the
application. It models the process of demodulating signals received from the
physical medium.

Physical Medium
~~~~~~~~~~~~~~

The connection between the hosts now has both a delay (1us) and a data rate
(100Mbps). The delay represents the propagation delay over the physical medium,
while the data rate determines how long it takes to transmit a packet based on
its size.

With a data rate of 100Mbps, a 1000-byte packet takes 80us to transmit
(1000 bytes * 8 bits/byte / 100Mbps = 80us). This transmission time is in
addition to the propagation delay of 1us.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network2]
   :end-before: [Config Network3]

The configuration sets the network to ``Network2`` and extends the ``Base2``
configuration, which includes the ``Base1`` settings and adds the data rate
configuration for the transmitter and receiver.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are passed to the ``PacketTransmitter``, which
transmits them over the physical medium at the configured data rate of 100Mbps.

The packets travel over the connection with a propagation delay of 1us and a
transmission time that depends on the packet size and data rate. For the 1000-byte
packets used in this simulation, the transmission time is 80us.

The server's ``PacketReceiver`` receives the signals from the physical medium and
converts them back into packets, which are then passed to the ``PassivePacketSink``
for consumption.

This model introduces the concept of transmission time, which is a more realistic
representation of how data is transmitted over a physical medium. In the next
step, we'll add queueing functionality to handle situations where packets arrive
faster than they can be transmitted.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`
