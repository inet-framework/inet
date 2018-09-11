:orphan:

.. _dg:cha:cross-layer-communication:

Cross-Layer Communication
=========================

Overview
--------

In the INET Framework, when an upper-layer protocol wants to send a data
packet over a lower-layer protocol, the upper-layer module just sends
the message object representing the packet to the lower-layer module,
which will in turn encapsulate it and send it. The reverse process takes
place when a lower layer protocol receives a packet and sends it up
after decapsulation.

It is often necessary to convey extra information with the packet. For
example, when an application-layer module wants to send data over TCP,
some connection identifier needs to be specified for TCP. When TCP sends
a segment over IP, IP will need a destination address and possibly other
parameters like TTL. When IP sends a datagram to an Ethernet interface
for transmission, a destination MAC address must be specified. This
extra information is attached to the message object in the form as
*message tags*.

Message tags are small value objects, which are attached to packets
using the ... C++ calls... TODO complete

Tags
----

TODO

The Dispatching Mechanism
-------------------------

TODO

Typical Tags Understood By Various Protocols
--------------------------------------------

TODO list the tag names understood, by laters!
