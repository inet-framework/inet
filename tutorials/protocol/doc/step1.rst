Step 1. Direct Communication Between Hosts
=======================================

Goals
-----

In this first step, we want to create a simple network that contains two hosts,
with one host sending a data stream directly to the other. Our goal is to
demonstrate the most basic form of communication between network nodes without
any protocol processing.

The Model
---------

In this step, we'll use the model depicted in Network1.ned:

.. figure:: media/step1.png
   :width: 80%
   :align: center

The network contains two hosts: a client and a server. The client generates
packets and sends them directly to the server, which consumes them.

The hosts are defined as modules in the same NED file:

.. literalinclude:: ../Network1.ned
   :language: ned
   :start-at: module ClientHost1
   :end-at: }

.. literalinclude:: ../Network1.ned
   :language: ned
   :start-at: module ServerHost1
   :end-at: }

The client host contains an ``ActivePacketSource`` module that generates packets
at random intervals with exponential distribution. The mean interval is
configured to be 500us, and the packet length is set to 1000 bytes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Base1]
   :end-at: packetLength

The server host contains a ``PassivePacketSink`` module that simply consumes
the packets it receives. There is no protocol processing at this stage - packets
are directly transferred from the source to the sink.

The connection between the client and server is a simple link with a 1us delay.
This represents the most basic form of communication channel, without any
consideration for bandwidth limitations, transmission errors, or other
real-world factors that will be introduced in later steps.

Results
-------

When we run the simulation, the client's ``ActivePacketSource`` generates
packets at random intervals according to the configured parameters. These
packets are sent directly to the server's ``PassivePacketSink``, which consumes
them immediately upon receipt.

The direct connection means that packets are transferred instantaneously (aside
from the configured 1us delay), without any consideration for:

- Transmission time based on packet size and channel capacity
- Queueing of packets when they are generated faster than they can be transmitted
- Bit errors that might occur during transmission
- Proper spacing between consecutive packet transmissions
- Reliability mechanisms to handle packet loss
- Ordering of packets that might arrive out of sequence

These aspects of real-world network protocols will be introduced in subsequent
steps of this tutorial, gradually building a more complete and realistic
protocol stack.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network1.ned <../Network1.ned>`
