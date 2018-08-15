.. _dg:cha:introduction:

Introduction
============

.. _dg:sec:introduction:what-is-inet:

What is INET Framework
----------------------

INET Framework is an open-source model library for the OMNeT++
simulation environment. It provides protocols, agents and other models
for researchers and students working with communication networks. INET
is especially useful when designing and validating new protocols, or
exploring new or exotic scenarios.

INET supports a wide class of communication networks, including wired,
wireless, mobile, ad hoc and sensor networks. It contains models for the
Internet stack (TCP, UDP, IPv4, IPv6, OSPF, BGP, etc.), link layer
protocols (Ethernet, PPP, IEEE 802.11, various sensor MAC protocols,
etc), refined support for the wireless physical layer, MANET routing
protocols, DiffServ, MPLS with LDP and RSVP-TE signalling, several
application models, and many other protocols and components. It also
provides support for node mobility, advanced visualization, network
emulation and more.

Several other simulation frameworks take INET as a base, and extend it
into specific directions, such as vehicular networks,
overlay/peer-to-peer networks, or LTE.

.. _dg:sec:introduction:scope-of-this-manual:

Scope of this Manual
--------------------

This manual is written for developers who intend to extend INET with new
components, written in C++. This manual is accompanied by the INET
Reference, which is generated from NED and MSG files using OMNeT++’s
documentation generator, and the documentation of the underlying C++
classes, generated from the source files using Doxygen. A working
knowledge of OMNeT++ and the C++ language is assumed.
