Step 1. Basic Application-to-Application Communication
=================================================

Goals
-----

In the first step, we want to create a network that contains two hosts, with one
host sending a data stream to the other. Our goal is to keep the protocol stack
as simple as possible, with direct application-to-application communication.

We'll make the model more realistic in later steps by adding various protocol
elements.

The Model
---------

In this step, we'll use the model depicted below:

.. figure:: media/step1.png
   :width: 70%
   :align: center

Here is the NED source of the network:

.. literalinclude:: ../Network1.ned
   :language: ned
   :start-at: network Network1
   :end-at: }

The network consists of two hosts: a client and a server. The client generates
packets that are sent directly to the server. The connection between them has a
delay of 1us.

The Hosts
~~~~~~~~~

The client host (``ClientHost1``) contains an ``ActivePacketSource`` module that
generates packets at random intervals. The server host (``ServerHost1``) contains
a ``PassivePacketSink`` module that receives and consumes the packets.

The ``ActivePacketSource`` module in the client generates packets of a fixed size
at random intervals with exponential distribution. The mean interval and packet
size are configured in the ``omnetpp.ini`` file.

The ``PassivePacketSink`` module in the server simply discards the received
packets.

Direct Connection
~~~~~~~~~~~~~~~~

In this simple model, packets are sent directly from the client's application to
the server's application without any intermediate protocol layers. The connection
between the hosts has a delay of 1us, which represents the transmission delay
over the physical medium.

This direct connection is a simplification that will be replaced with more
realistic protocol elements in later steps.

Configuration
------------

Here is the configuration for this step:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Network1]
   :end-before: [Config Network2]

The configuration sets the network to ``Network1`` and extends the ``Base1``
configuration, which defines the packet production interval and packet length.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates packets
at random intervals with an exponential distribution, with a mean of 500us. Each
packet is 1000 bytes in size. These packets are sent directly to the server's
``PassivePacketSink``, which consumes them.

The packets travel over the connection with a delay of 1us, which is a very
simplified model of transmission delay.

This basic model demonstrates the simplest form of communication between two
hosts, without any protocol overhead or complexity. In the next steps, we'll
gradually add more realistic protocol elements to build a complete protocol
stack.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network1.ned <../Network1.ned>`
