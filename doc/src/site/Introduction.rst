=======================
What Is INET Framework?
=======================

A Network Simulator
~~~~~~~~~~~~~~~~~~~

INET Framework is an open-source model library for the `OMNeT++ simulation
environment <http://omnetpp.org>`__.
It provides protocols, agents, and other models for researchers and
students working with communication networks. INET is especially useful
when designing and validating new protocols or exploring new or exotic
scenarios.

INET contains models for the Internet stack (TCP, UDP, IPv4, IPv6, OSPF,
BGP, etc.), wired and wireless link-layer protocols (Ethernet, PPP, IEEE
802.11, etc.), support for mobility, MANET protocols, DiffServ, MPLS with
LDP and RSVP-TE signaling, several application models, and many other
protocols and components.

Several other simulation frameworks take INET as a base and extend it
in `specific directions <Protocols.html>`__, such as vehicular networks,
overlay/peer-to-peer networks, or LTE.

Designed for Experimentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

INET is built around the concept of modules that communicate by message
passing. Agents and network protocols are represented by components, which
can be freely combined to form hosts, routers, switches, and other
networking devices. New components can be programmed by the user, and
existing components have been written so that they are easy to understand
and modify.

INET benefits from the infrastructure provided by OMNeT++. Beyond making
use of the services provided by the OMNeT++ simulation kernel and library
(component model, parameterization, result recording, etc.), this also
means that models may be developed, assembled, parameterized, run, and
their results evaluated from the comfort of the OMNeT++ Simulation IDE or
from the command line.

Some features:

- OSI layers implemented (physical, link-layer, network, transport, application)
- Pluggable protocol implementations for various layers
- IPv4/IPv6 network stack (or build your own network layer)
- Transport layer protocols: TCP, UDP, SCTP
- Routing protocols (ad-hoc and wired)
- Wired/wireless interfaces (Ethernet, PPP, IEEE 802.11, etc.)
- Physical layer with scalable level of detail (unit disc radio to detailed propagation models, frame level to bit/symbol level representation, etc.)
- Wide range of application models
- Network emulation support
- Mobility support
- Supports the modeling of the physical environment (obstacles for radio propagation, etc.)
- Separation of concerns
- Visualization support

Driven By the Community
~~~~~~~~~~~~~~~~~~~~~~~

INET Framework is maintained by the OMNeT++ team for the community,
utilizing patches and new models contributed by members of the community.
