.. _ug:cha:introduction:

Introduction
============

.. _ug:sec:introduction:what-is-inet-framework:

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

.. _ug:sec:introduction:designed-for-experimentation:

Designed for Experimentation
----------------------------

INET is built around the concept of modules that communicate by message
passing. Agents and network protocols are represented by components,
which can be freely combined to form hosts, routers, switches, and other
networking devices. New components can be programmed by the user, and
existing components have been written so that they are easy to
understand and modify.

INET benefits from the infrastructure provided by OMNeT++. Beyond making
use of the services provided by the OMNeT++ simulation kernel and
library (component model, parameterization, result recording, etc.),
this also means that models may be developed, assembled, parameterized,
run, and their results evaluted from the comfort of the OMNeT++
Simulation IDE, or from the command line.

INET Framework is maintained by the OMNeT++ team for the community,
utilizing patches and new models contributed by members of the
community.

.. _ug:sec:introduction:scope-of-this-manual:

Scope of this Manual
--------------------

This manual is written for users who are interested in assembling
simulations using the components provided by the INET Framework. (In
contrast, if you are interested in modifing INET’s components or plan to
extend INET with new protocols or other components using C++, we
recommend the *INET Developers Guide*.)

This manual does not attempt to be a reference for INET. It concentrates
on conveying the big picture, and does not attempt to cover all
components, or try to document the parameters, gates, statistics or
precise operation of individual components. For such information, users
should refer to the *INET Reference*, a web-based cross-referenced
documentation generated from NED and MSG files.

A working knowledge of OMNeT++ is assumed.
