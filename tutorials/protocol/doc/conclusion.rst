Conclusion
==========

In this tutorial, we have built a protocol stack from the ground up, starting
with simple application-to-application communication and gradually adding more
complex features. Through seven progressive steps, we have explored various
aspects of network protocols and their implementation in the INET Framework.

Here's a summary of what we've learned:

1. **Basic Application-to-Application Communication**: We started with a simple
   model where packets are sent directly from one application to another, with
   just a propagation delay in between.

2. **Packet Transmission and Reception**: We added packet transmitter and
   receiver modules to model the physical layer functionality of converting
   packets into signals and vice versa, introducing the concept of transmission
   time based on packet size and data rate.

3. **Queueing and Serving**: We added a packet queue and a server to model the
   buffering and processing of packets, which is essential in real network
   devices to handle situations where packets arrive faster than they can be
   transmitted.

4. **Error Detection**: We added Ethernet FCS header inserter and checker modules
   to detect transmission errors, and introduced a lossy channel to simulate bit
   errors that can occur during transmission.

5. **Interpacket Gap**: We added an interpacket gap inserter to enforce a minimum
   time gap between consecutive packet transmissions, which is an important
   aspect of many network protocols to prevent congestion and allow devices to
   prepare for the next packet.

6. **Packet Resending**: We added a resending module to handle packet loss and
   corruption by resending packets that are not acknowledged, which is essential
   for reliable communication in networks where packet loss can occur.

7. **Sequence Numbering and Reordering**: We added sequence numbering and
   reordering modules to handle out-of-order packet delivery, which can occur in
   real networks due to various factors such as routing changes and packet
   retransmissions.

These steps have given us a solid understanding of the fundamental concepts and
mechanisms that underlie network protocols. By building a protocol stack
incrementally, we have seen how each component contributes to the overall
functionality and reliability of network communication.

The INET Framework provides a rich set of protocol elements that can be combined
in various ways to model different network protocols and scenarios. This tutorial
has only scratched the surface of what's possible. You can continue to explore
and experiment with more complex protocol stacks by adding additional elements
from the INET Framework, such as:

- MAC address handling for link-layer addressing
- IP addressing and routing for network-layer functionality
- Transport protocols like TCP and UDP for end-to-end communication
- Application protocols for specific use cases

We encourage you to use this tutorial as a starting point for your own
explorations and simulations of network protocols using the INET Framework.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network1.ned <../Network1.ned>`,
:download:`Network2.ned <../Network2.ned>`,
:download:`Network3.ned <../Network3.ned>`,
:download:`Network4.ned <../Network4.ned>`,
:download:`Network5.ned <../Network5.ned>`,
:download:`Network6.ned <../Network6.ned>`,
:download:`Network7.ned <../Network7.ned>`
