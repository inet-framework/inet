Step 2. Adding Packet Transmitter and Receiver
=========================================

Goals
-----

In the second step, we enhance our network by adding proper packet transmission
and reception capabilities. We want to model the physical transmission of
packets over a communication channel with a specific data rate, which introduces
transmission delay based on packet size.

The Model
---------

In this step, we'll use the model depicted in Network2.ned:

.. figure:: media/step2.png
   :width: 80%
   :align: center

The network still contains a client and a server, but now we've added
transmitter and receiver modules to handle the physical transmission of packets.


The hosts are defined as modules in the Network2.ned file. The client host now
contains a ``PacketTransmitter`` module in addition to the ``ActivePacketSource``:

.. literalinclude:: ../Network2.ned
   :language: ned
   :start-at: module ClientHost2
   :end-at: }
   :emphasize-lines: 9-11

The ``PacketTransmitter`` is responsible for converting packets into signals that
can be transmitted over the physical medium. It handles the transmission process,
including the timing based on packet size and channel data rate.

The server host now contains a ``PacketReceiver`` module in addition to the
``PassivePacketSink``:

.. literalinclude:: ../Network2.ned
   :language: ned
   :start-at: module ServerHost2
   :end-at: }
   :emphasize-lines: 9-11

The ``PacketReceiver`` is responsible for receiving signals from the physical
medium and converting them back into packets that can be processed by the upper
layers.

The connection between the client and server now has a data rate of 100Mbps in
addition to the 1us delay:

.. literalinclude:: ../Network2.ned
   :language: ned
   :start-at: network Network2
   :end-at: }

The data rate is also configured for the transmitter and receiver modules:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Base2]
   :end-at: datarate

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates
packets at random intervals as before. However, now these packets are passed to
the ``PacketTransmitter`` module, which converts them into signals and
transmits them over the physical medium.

The key difference from Step 1 is that packet transmission now takes time based
on the packet size and the channel data rate. For example, a 1000-byte packet
transmitted over a 100Mbps channel takes approximately 80 microseconds to
transmit (8000 bits / 100Mbps = 80µs).

This introduces a more realistic model of network communication, where:

- Packet transmission time is proportional to packet size and inversely
  proportional to channel data rate
- The transmitter can only transmit one packet at a time
- If packets are generated faster than they can be transmitted, they will
  experience queueing delay (although in this step we don't yet have a queue)

The ``PacketReceiver`` at the server side receives the signals from the physical
medium and converts them back into packets, which are then passed to the
``PassivePacketSink`` for consumption.

This step introduces the concept of transmission delay, which is a fundamental
aspect of network communication. In subsequent steps, we'll add more protocol
elements to handle other aspects of real-world network protocols, such as
queueing, error detection, and reliability mechanisms.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`
