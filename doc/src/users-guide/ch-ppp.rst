.. _ug:cha:ppp:

Point-to-Point Links
====================

.. _ug:sec:ppp:overview:

Overview
--------

For simulating wired point-to-point links, the INET Framework contains a
minimal implementation of the PPP protocol and a corresponding network
interface module.

-  :ned:`Ppp` is a simple module that performs encapsulation of network
   datagrams into PPP frames and decapsulation of the incoming PPP
   frames. It can be connected to the network layer directly or can be
   configured to get the outgoing messages from an output queue. The
   module collects statistics about the transmitted and dropped
   packages.

-  :ned:`PppInterface` is a compound module that complements the
   :ned:`Ppp` module with an output queue. It implements the
   :ned:`IWiredInterface` interface. Input and output hooks can be
   configured for further processing of the network messages.

PPP (RFC 1661) is a complex protocol which, in addition to providing a
method for encapsulating multi-protocol datagrams, also contains control
protocols for establishing, configuring, and testing the data-link
connection (LCP) and for configuring different network-layer protocols
(NCP).

The INET implementation only covers encapsulation and decapsulation of
data into PPP frames. Control protocols, which do not have a significant
effect on the links’ capacity and latency during normal link operation,
are not simulated. In addition, header field compressions (PFC and ACFC)
are also bot supported, so a simulated PPP frame always contains 1-byte
Address and Control fields and a 2-byte Protocol field.

.. _ug:sec:ppp:the-ppp-module:

The PPP module
--------------

The PPP module receives packets from the upper layer in the
:gate:`netwIn` gate, adds a :msg:`PppHeader`, and send
it to the physical layer through the :gate:`phys` gate. The packet with
:msg:`PppHeader` is received from the :gate:`phys` and sent to the upper
layer immediately through the :gate:`netwOut` gate.

Incoming datagrams are waiting in a queue if the line is currently busy.
In routers, PPP relies on an external queue module (implementing
:ned:`IPacketQueue`) to model finite buffer, implement QoS and/or RED,
and requests packets from this external queue one-by-one. The name of
this queue is given as the :par:`queueModule` parameter.

In hosts, no such queue is used, so :ned:`Ppp` contains an internal
queue to store packets wainting for transmission.
Conceptually the queue is of inifinite size, but for better diagnostics
one can specify a hard limit in the :par:`packetCapacity` parameter – if
this is exceeded, the simulation stops with an error.

The module can be used in simulations where the nodes are connected and
disconnected dinamically. If the channel between the PPP modules is
down, the messages received from the upper layer are dropped (including
the messages waiting in the queue). When the connection is restored it
will poll the queue and transmits the messages again.

The PPP module registers itself in the interface table of the node. The
:var:`mtu` of the entry can be specified by the :par:`mtu` module
parameter. The module checks the state of the physical link and updates
the entry in the interface table.

.. _ug:sec:ppp:pppinterface:

PppInterface
------------

:ned:`PppInterface` is a compound module that implements the
:ned:`IWiredInterface` interface. It contains a :ned:`Ppp` module and a
passive queue for the messages received from the network layer.

The queue type is specified by the :par:`typename` parameter of the queue
submodule. It can be set to ``PacketQueue`` or to a module type implementing
the :ned:`IPacketQueue` interface. There are implementations with QoS and
RED support.

In typical use of the :ned:`Ppp` module it is augmented with other nodes
that monitor the traffic or simulate package loss and duplication. The
:ned:`PppInterface` module abstract that usage by adding :ned:`IHook`
components to the network input and output of the :ned:`Ppp` component.
Any number of hook can be added by specifying the :par:`numOutputHooks`
and :par:`numInputHooks` parameters and the types of the
:var:`outputHook` and :var:`inputHook` components. The hooks are chained
in their numeric order.
