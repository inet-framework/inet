Step 3. Adding Queue and Server
===========================

Goals
-----

In the third step, we enhance our network model by adding queueing functionality.
In real networks, packets often arrive faster than they can be transmitted,
requiring a buffer to store packets temporarily. We add a packet queue and a
server to model this behavior, bringing our simulation closer to real-world
network devices.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step3.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network3.ned
   :language: ned
   :start-at: network Network3
   :end-at: }

The network still consists of a client and a server, but the client now includes
additional modules for queueing and serving packets before transmission.

The Hosts
~~~~~~~~~

The client host (``ClientHost3``) now contains:

- An ``ActivePacketSource`` module that generates packets
- A ``PacketQueue`` module that buffers packets
- An ``InstantServer`` module that processes packets
- A ``PacketTransmitter`` module that handles the transmission of packets

The server host (``ServerHost3``) remains the same as in the previous step, with:

- A ``PassivePacketSink`` module that consumes packets
- A ``PacketReceiver`` module that handles the reception of packets

Queueing and Serving
~~~~~~~~~~~~~~~~~~~

The ``PacketQueue`` module in the client buffers packets when they arrive faster
than they can be transmitted. It acts as a FIFO (First In, First Out) queue,
storing packets in the order they are received and releasing them in the same
order.

The ``InstantServer`` module processes packets from the queue. In this model, the
server processes packets instantaneously (hence the name), but in more complex
models, the server could introduce processing delays or implement more
sophisticated packet handling.

This queueing mechanism is essential in network devices like routers and switches,
which need to buffer packets during periods of congestion or when the outgoing
link is busy.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network3]
   :end-before: [Config Network4]

The configuration sets the network to ``Network3`` and extends the ``Base2``
configuration, which includes the data rate settings for the transmitter and
receiver.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals. These packets are first placed in the ``PacketQueue``, where
they wait if necessary. The ``InstantServer`` then processes the packets from the
queue and passes them to the ``PacketTransmitter`` for transmission.

If packets arrive faster than they can be transmitted (which can happen due to
the random generation intervals and the fixed transmission time based on the data
rate), they will accumulate in the queue. This models the behavior of real
network devices during periods of high traffic.

The server's ``PacketReceiver`` receives the signals from the physical medium and
converts them back into packets, which are then passed to the ``PassivePacketSink``
for consumption.

This model introduces the concept of queueing, which is a fundamental aspect of
network communication. In the next step, we'll add error detection mechanisms to
handle transmission errors that can occur in real networks.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network3.ned <../Network3.ned>`
