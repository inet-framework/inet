.. _ug:cha:mpls:

The MPLS Models
===============

.. _ug:sec:mpls:overview:

Overview
--------

Multi-Protocol Label Switching (MPLS) is a “layer 2.5” protocol for
high-performance telecommunications networks. MPLS directs data from one
network node to the next based on numeric labels instead of network
addresses, avoiding complex lookups in a routing table and allowing
traffic engineering. The labels identify virtual links (label-switched
paths or LSPs, also called MPLS tunnels) between distant nodes rather
than endpoints. The routers that make up a label-switched network are
called label-switching routers (LSRs) inside the network (“transit
nodes”), and label edge routers (LER) on the edges of the network
(“ingress” or “egress” nodes).

A fundamental MPLS concept is that two LSRs must agree on the meaning of
the labels used to forward traffic between and through them. This common
understanding is achieved by using signaling protocols by which one LSR
informs another of label bindings it has made. Such signaling protocols
are also called label distribution protocols. The two main label
distribution protocols used with MPLS are LDP and RSVP-TE.

INET provides basic support for building MPLS simulations. It provides
models for the MPLS, LDP and RSVP-TE protocols and their associated data
structures, and preassembled MPLS-capable router models.

.. _ug:sec:mpls:core-modules:

Core Modules
------------

The core modules are:

-  :ned:`Mpls` implements the MPLS protocol

-  :ned:`LibTable` holds the LIB (Label Information Base)

-  :ned:`Ldp` implements the LDP signaling protocol for MPLS

-  :ned:`RsvpTe` implements the RSVP-TE signaling protocol for MPLS

-  :ned:`Ted` contains the Traffic Engineering Database

-  :ned:`LinkStateRouting` is a simple link-state routing protocol

-  :ned:`RsvpClassifier` is a configurable ingress classifier for MPLS

.. _ug:sec:mpls:mpls:

Mpls
~~~~

The :ned:`Mpls` module implements the MPLS protocol. MPLS is situated
between layer 2 and 3, and its main function is to switch packets based
on their labels. For that, it relies on the data structure called LIB
(Label Information Base). LIB is fundamentally a table with the
following columns: *input-interface*, *input-label*, *output-interface*,
*label-operation(s)*.

Upon receiving a labelled packet from another LSR, MPLS first extracts
the incoming interface and incoming label pair, and then looks it up in
local LIB. If a matching entry is found, it applies the prescribed label
operations, and forwards the packet to the output interface.

Label operations can be the following:

-  *Push* adds a new MPLS label to a packet. (A packet may contain
   multiple labels, acting as a stack.) When a normal IP packet enters
   an LSP, the new label will be the first label on the packet.

-  *Pop* removes the topmost MPLS label from a packet. This is typically
   done at either the penultimate or the egress router.

-  *Swap*: Replaces the topmost label with a new label.

In INET, the local LIB is stored in a :ned:`LibTable` module in the
router.

Upon receiving an unlabelled (e.g. plain IPv4) packet, MPLS first
determines the forwarding equivalence class (FEC) for the packet using
an ingress classifier, and then inserts one or more labels in the
packet’s newly created MPLS header. The packet is then passed on to the
next hop router for the LSP.

The ingress classifier is also a separate module; it is selected
depending on the choice of the signaling protocol.

.. _ug:sec:mpls:libtable:

LibTable
~~~~~~~~

:ned:`LibTable` stores the LIB (Label Information Base), as described in
the previous section. :ned:`LibTable` is expected to have one instance
in the router.

LIB is normally filled and maintained by label distribution protocols
(RSVP-TE, LDP), but in INET it is possible to preload it with initial
contents.

The :ned:`LibTable` module accepts an XML config file whose structure
follows the contents of the LIB table. An example configuration:

.. code-block:: xml

   <libtable>
       <libentry>
           <inLabel>203</inLabel>
           <inInterface>ppp1</inInterface>
           <outInterface>ppp2</outInterface>
           <outLabel>
               <op code="pop"/>
               <op code="swap" value="200"/>
               <op code="push" value="300"/>
           </outLabel>
           <color>200</color>
       </libentry>
   </libtable>

There can be multiple ``<libentry>`` elements, each describing a row
in the table. Colums are given as child elements: ``<inLabel>``,
``<inInterface>``, etc. The ``<color>`` element is optional, and
it only exists to be able to color LSPs on the GUI. It is not used by
the protocols.

.. _ug:sec:mpls:ldp:

Ldp
~~~

The :ned:`Ldp` module implements the Label Distribution Protocol (LDP).
LDP is used to establish LSPs in an MPLS network when traffic
engineering is not required. It establishes LSPs that follow the
existing IP routing table, and is particularly well suited for
establishing a full mesh of LSPs between all of the routers on the
network.

LDP relies on the underlying routing information provided by a routing
protocol in order to forward label packets. The router’s forwarding
information base, or FIB, is responsible for determining the hop-by-hop
path through the network.

In INET, the :ned:`Ldp` module takes routing information from :ned:`Ted`
module. The :ned:`Ted` instance in the network is filled and maintained
by a :ned:`LinkStateRouting` module. Unfortunately, it is currently not
possible to use other routing protocol implementations such as
:ned:`Ospfv2` in conjunction with :ned:`Ldp`.

When :ned:`Ldp` is used as signaling protocol, it also serves as ingress
classifier for :ned:`Mpls`.

.. _ug:sec:mpls:ted:

Ted
~~~

The :ned:`Ted` module contains the Traffic Engineering Database (TED).
In INET, :ned:`Ted` contains a link state database, including
reservations for each link by RSVP-TE.

.. _ug:sec:mpls:linkstaterouting:

LinkStateRouting
~~~~~~~~~~~~~~~~

The :ned:`LinkStateRouting` module provides a simple link state routing
protocol. It uses :ned:`Ted` as its link state database. Unfortunately,
the :ned:`LinkStateRouting` module cannot operate independently, it can
only be used inside an MPLS router.

.. _ug:sec:mpls:rsvpte:

RsvpTe
~~~~~~

The :ned:`RsvpTe` module implements RSVP-TE (Resource Reservation
Protocol – Traffic Engineering), as signaling protocol for MPLS. RSVP-TE
handles bandwidth allocation and allows traffic engineering across an
MPLS network. Like LDP, RSVP uses discovery messages and advertisements
to exchange LSP path information between all hosts. However, whereas LDP
is restricted to using the configured IGP’s shortest path as the transit
path through the network, RSVP can take taking into consideration
network constraint parameters such as available bandwidth and explicit
hops. RSVP uses a combination of the Constrained Shortest Path First
(CSPF) algorithm and Explicit Route Objects (EROs) to determine how
traffic is routed through the network.

When :ned:`RsvpTe` is used as signaling protocol, :ned:`Mpls` needs a
separate ingress classifier module, which is usually a
:ned:`RsvpClassifier`.

The :ned:`RsvpTe` module allows LSPs to be specified statically in an
XML config file. An example ``traffic.xml`` file:

.. code-block:: xml

   <sessions>
       <session>
           <endpoint>host3</endpoint>
           <tunnel_id>1</tunnel_id>
           <paths>
               <path>
                   <lspid>100</lspid>
                   <bandwidth>100000</bandwidth>
                   <route>
                       <node>10.1.1.1</node>
                       <lnode>10.1.2.1</lnode>
                       <node>10.1.4.1</node>
                       <node>10.1.5.1</node>
                   </route>
                   <permanent>true</permanent>
                   <color>100</color>
               </path>
           </paths>
       </session>
   </sessions>

In the route, ``<node>`` stands for strict hop, and ``<lnode>``
for loose hop.

Paths can also be set up and torn down dynamically with
:ned:`ScenarioManager` commands (see chapter :doc:`ch-scenario-scripting`).
:ned:`RsvpTe` understands the ``<add-session>`` and ``<del-session>``
:ned:`ScenarioManager` commands. The contents of the
``<add-session>`` element can be the same as the ``<session>``
element for the ``traffic.xml`` above. The ``<del-command>``
element syntax is also similar, but only ``<endpoint>``,
``<tunnel_id>`` and ``<lspid>`` need to be specified.

The following is an example ``scenario.xml`` file:

.. code-block:: xml

   <scenario>
       <at t="2">
           <add-session module="LSR1.rsvp">
               <endpoint>10.2.1.1</endpoint>
               <tunnel_id>1</tunnel_id>
               <paths>
                   ...
               </paths>
           </add-session>
       </at>
       <at t="2.4">
           <del-session module="LSR1.rsvp">
               <endpoint>10.2.1.1</endpoint>
               <tunnel_id>1</tunnel_id>
               <paths>
                   <path>
                       <lspid>100</lspid>
                   </path>
               </paths>
           </del-session>
       </at>
   </scenario>

.. _ug:sec:mpls:classifier:

Classifier
----------

The :ned:`RsvpClassifier` module implements an ingress classifier for
:ned:`Mpls` when using :ned:`RsvpTe` for signaling. The classifier can
be configured with an XML config file.

.. code-block:: ini

   **.classifier.config = xmldoc("fectable.xml");

An example ``fectable.xml`` file:

.. code-block:: xml

   <fectable>
       <fecentry>
           <id>1</id>
           <destination>host5</destination>
           <source>host1</source>
           <tunnel_id>1</tunnel_id>
           <lspid>100</lspid>
       </fecentry>
   </fectable>

.. _ug:sec:mpls:mpls-enabled-router-models:

MPLS-Enabled Router Models
--------------------------

INET provides the following pre-assembled MPLS routers:

-  :ned:`LdpMplsRouter` is an MPLS router with the LDP signaling
   protocol

-  :ned:`RsvpMplsRouter` is an MPLS router with the RSVP-TE signaling
   protocol
