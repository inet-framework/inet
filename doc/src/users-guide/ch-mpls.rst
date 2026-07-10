.. _ug:cha:mpls:

The MPLS Models
===============

.. _ug:sec:mpls:overview:

Overview
--------

Multi-Protocol Label Switching (MPLS) is a "layer 2.5" protocol for high-performance telecommunications networks. MPLS data is directed from one network node to the next using numeric labels instead of network addresses. This avoids complex lookups in a routing table and allows for traffic engineering. The labels identify virtual links (label-switched paths or LSPs, also called MPLS tunnels) between distant nodes, rather than endpoints. The routers that make up a label-switched network are called label-switching routers (LSRs) inside the network ("transit nodes"), and label edge routers (LER) on the edges of the network ("ingress" or "egress" nodes).

A fundamental MPLS concept is that the meaning of the labels used to forward traffic between and through two LSRs must be agreed upon. This common understanding is achieved by using signaling protocols through which one LSR informs another of label bindings it has made. Such signaling protocols are also called label distribution protocols. The two main label distribution protocols used with MPLS are LDP and RSVP-TE.

INET provides basic support for building MPLS simulations. It provides models for the MPLS, LDP, and RSVP-TE protocols and their associated data structures, and preassembled MPLS-capable router models.

.. _ug:sec:mpls:core-modules:

Core Modules
------------

The core modules are:

- :ned:`Mpls` implements the MPLS protocol

- :ned:`LibTable` holds the LIB (Label Information Base)

- :ned:`Ldp` implements the LDP signaling protocol for MPLS

- :ned:`RsvpTe` implements the RSVP-TE signaling protocol for MPLS

- :ned:`Ted` contains the Traffic Engineering Database

- :ned:`LinkStateRouting` is a simple link-state routing protocol

- :ned:`RsvpClassifier` is a configurable ingress classifier for MPLS

.. _ug:sec:mpls:mpls:

Mpls
~~~~

The MPLS protocol is implemented by the :ned:`Mpls` module. MPLS is situated between layer 2 and 3, and its main function is to switch packets based on their labels. For that purpose, it relies on the data structure called LIB (Label Information Base). LIB is essentially a table with the following columns: *input-interface*, *input-label*, *output-interface*, *label-operation(s)*.

When receiving a labeled packet from another LSR, MPLS first extracts the incoming interface and incoming label pair, and then looks it up in the local LIB. If a matching entry is found, it applies the prescribed label operations and forwards the packet to the output interface.

The label operations can be one of the following:

- *Push* adds a new MPLS label to a packet. (A packet may contain multiple labels, acting as a stack.) When a normal IP packet enters an LSP, the new label will be the first label on the packet.

- *Pop* removes the topmost MPLS label from a packet. This is typically done at either the penultimate or the egress router.

- *Swap* replaces the topmost label with a new label.

In INET, the local LIB is stored in a :ned:`LibTable` module in the router.

When receiving an unlabeled (e.g., plain IPv4) packet, MPLS first determines the forwarding equivalence class (FEC) for the packet using an ingress classifier, and then inserts one or more labels in the packet's newly created MPLS header. The packet is then passed on to the next hop router for the LSP.

The ingress classifier is also a separate module; it is selected depending on the choice of the signaling protocol.

.. _ug:sec:mpls:libtable:

LibTable
~~~~~~~~

The :ned:`LibTable` module stores the LIB (Label Information Base), as described in the previous section. The router is expected to have one instance of the :ned:`LibTable` module.

LIB is normally filled and maintained by label distribution protocols (RSVP-TE, LDP), but in INET, it is possible to preload it with initial contents.

The :ned:`LibTable` module accepts an XML config file whose structure follows the contents of the LIB table. An example configuration:

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
       </libentry>
   </libtable>

There can be multiple ``<libentry>`` elements, each describing a row in the table. Columns are given as child elements: ``<inLabel>``, ``<inInterface>``, ``<outInterface>``, and ``<outLabel>``. The ``<inInterface>`` element may also be the literal string ``any``, meaning the entry matches regardless of the incoming interface.

.. _ug:sec:mpls:ldp:

Ldp
~~~

The :ned:`Ldp` module implements the Label Distribution Protocol (LDP). LDP is used to establish LSPs in an MPLS network when traffic engineering is not required. LDP establishes LSPs that follow the existing IP routing table, and is particularly well suited for establishing a full mesh of LSPs between all routers on the network.

LDP relies on the underlying routing information provided by a routing protocol to forward label packets. The router's forwarding information base, or FIB, determines the hop-by-hop path through the network.

In INET, the :ned:`Ldp` module takes routing information from the :ned:`Ted` module. The :ned:`Ted` instance in the network is filled and maintained by a :ned:`LinkStateRouting` module. Unfortunately, it is currently not possible to use other routing protocol implementations such as :ned:`Ospfv2` in conjunction with :ned:`Ldp`.

When :ned:`Ldp` is used as the signaling protocol, it also serves as the ingress classifier for :ned:`Mpls`.

.. _ug:sec:mpls:lening:

Ted
~~~

The :ned:`Ted` module contains the Traffic Engineering Database (TED). In INET, :ned:`Ted` contains a link-state database, including reservations for each link by RSVP-TE.

.. _ug:sec:mpls:linkstaterouting:

LinkStateRouting
~~~~~~~~~~~~~~~~

The :ned:`LinkStateRouting` module provides a simple link-state routing protocol. It uses :ned:`Ted` as its link-state database. Unfortunately, the :ned:`LinkStateRouting` module cannot operate independently; it can only be used inside an MPLS router.

.. _ug:sec:mpls:rsvpte:

RsvpTe
~~~~~~

The :ned:`RsvpTe` module implements RSVP-TE (Resource Reservation Protocol – Traffic Engineering) as the signaling protocol for MPLS. RSVP-TE handles bandwidth allocation and allows traffic engineering across an MPLS network. Like LDP, RSVP uses discovery messages and advertisements to exchange LSP path information between all hosts. However, whereas LDP is restricted to using the configured IGP's shortest path as the transit path through the network, RSVP can take into consideration network constraint parameters such as available bandwidth and explicit hops. RSVP uses a combination of the Constrained Shortest Path First (CSPF) algorithm and Explicit Route Objects (EROs) to determine how traffic is routed through the network.

When :ned:`RsvpTe` is used as the signaling protocol, :ned:`Mpls` requires a separate ingress classifier module, which is usually a :ned:`RsvpClassifier`.

The :ned:`RsvpTe` module allows LSPs to be specified statically in an XML config file. An example ``traffic.xml`` file:

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
               </path>
           </paths>
       </session>
   </sessions>

In the route, ``<node>`` stands for a strict hop, and ``<lnode>`` stands for a loose hop.

Paths can also be set up and torn down dynamically with :ned:`ScenarioManager` commands (see chapter :doc:`ch-scenario-scripting`). :ned:`RsvpTe` understands the ``<add-session>`` and ``<del-session>`` :ned:`ScenarioManager` commands. The contents of the ``<add-session>`` element can be the same as the ``<session>`` element for the ``traffic.xml`` above. The ``<del-session>`` element syntax is also similar, but only ``<endpoint>``, ``<tunnel_id>``, and ``<lspid>`` need to be specified.

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

The :ned:`RsvpClassifier` module implements an ingress classifier for :ned:`Mpls` when using :ned:`RsvpTe` for signaling. The classifier can be configured with an XML config file.

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

INET provides the following preassembled MPLS routers:

- :ned:`LdpMplsRouter` is an MPLS router with the LDP signaling protocol

- :ned:`RsvpMplsRouter` is an MPLS router with the RSVP-TE signaling protocol

.. _ug:sec:mpls:conformance:

Conformance Status
------------------

The MPLS/LDP/RSVP-TE models implement a substantial part of their reference
RFCs, but are not complete protocol implementations. This section lists,
mechanism by mechanism, what is modeled, what is only partially modeled, and
what is deliberately left out, so that simulation results can be interpreted
against the actual feature set rather than against the RFCs themselves. Every
row marked "Not modeled" is an intentional simplification, not an oversight;
none of it silently changes behavior in a way that would surprise a user who
reads this table first.

LDP (RFC 5036)
~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Mechanism
     - Status
     - Notes
   * - Basic discovery (UDP 646 multicast Hello, hold timer)
     - Modeled
     - Received ``holdTime`` is honored via the RFC's min(local, peer) rule.
   * - Extended/targeted discovery (remote sessions)
     - Not modeled
     - Only directly-connected peers discovered via basic discovery are
       supported.
   * - Session initialization (Initialization message, parameter
       negotiation)
     - Not modeled
     - A successful TCP connection stands in for session establishment;
       there is no Initialization/parameter-negotiation exchange.
   * - KeepAlive
     - Not modeled
     - Session liveness relies solely on the Hello adjacency's hold timer.
   * - Label distribution mode (DU / DoD)
     - Partially modeled
     - Downstream-on-Demand with ordered control only; Downstream
       Unsolicited and independent control are not implemented, and there
       is no parameter to select a mode.
   * - Label retention mode (liberal / conservative)
     - Partially modeled
     - Behavior is conservative-like by construction, but not a
       configurable, spec-named retention mode.
   * - Label Mapping / Label Request / Label Withdraw / Label Release
     - Modeled
     - Includes the full label lifecycle (mapping, request, withdraw,
       release) with a wire-faithful PDU/TLV encoding.
   * - Notification
     - Partially modeled
     - Only the ``NO_ROUTE`` status is generated; other or unrecognized
       status codes on a received Notification are logged and ignored
       rather than causing an error.
   * - Address / Address Withdraw messages
     - Not modeled
     - Received Address/Address Withdraw messages are logged and ignored
       (an unsupported message is not a protocol error); the peer's
       next-hop mapping is instead derived from the local routing table.
   * - Loop detection (hop count / path vector TLV)
     - Not modeled
     - n/a
   * - Wire format (PDU header, LSR-ID, label space, TLV encoding)
     - Modeled
     - One message per PDU (a documented model simplification); every
       message type round-trips through a registered serializer with the
       exact RFC-computed length.
   * - Active/passive role selection
     - Partially modeled
     - Exactly one side of each adjacency ends up active, so LSP setup
       works, but the comparison used is the opposite of RFC 5036 Section
       2.5.2's rule (the LSR with the smaller, not greater, transport
       address is chosen active).
   * - Session-failure recovery
     - Modeled
     - A TCP close/failure cleans up bindings and reconnects; this is
       hello/socket-event-driven recovery, not a spec-defined session FSM.
   * - Graceful restart
     - Not modeled
     - n/a
   * - Generalized TTL Security Mechanism (GTSM)
     - Not modeled
     - n/a
   * - IPv6 (RFC 7552)
     - Not modeled
     - The whole subsystem is IPv4-only.
   * - Pseudowire signaling
     - Not modeled
     - n/a
   * - Penultimate-hop popping / implicit null label
     - Modeled
     - Controlled by the ``advertiseImplicitNull`` parameter.

RSVP-TE (RFC 2205 + RFC 3209)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Mechanism
     - Status
     - Notes
   * - Path / Resv with LABEL_REQUEST and LABEL objects
     - Modeled
     - n/a
   * - SESSION / RSVP_HOP / TIME_VALUES / SENDER_TEMPLATE / SENDER_TSPEC
       objects
     - Modeled
     - Serialized with their RFC Class-Num/C-Type on the wire.
   * - Explicit Route Object (ERO): strict and loose hops
     - Partially modeled
     - Strict and loose hop forwarding both work, but loose hops are
       resolved with a plain routing-table lookup; there is no
       constraint-based computation, so EROs that need bandwidth or
       affinity awareness must be hand-written by the user.
   * - Constrained Shortest Path First (CSPF) at the ingress
     - Not modeled
     - A bandwidth-pruned shortest-path helper exists in :ned:`Ted` but has
       no caller; nothing computes an ERO automatically.
   * - Record Route Object (RRO)
     - Partially modeled
     - Carried in the Resv direction only; not consulted for loop
       detection or fast-reroute merge points.
   * - Soft-state refresh and timeout
     - Modeled
     - Refresh period, state-lifetime factor, and per-refresh
       uniform(0.5, 1.5) jitter follow RFC 2205 Section 3.7, all
       configurable via NED parameters.
   * - Refresh reduction (RFC 2961)
     - Not modeled
     - Every refresh is a full-state message; no MESSAGE_ID/Ack/Srefresh.
   * - PathTear / PathErr
     - Modeled
     - n/a
   * - ResvTear / ResvErr
     - Modeled
     - ResvTear is sent on soft-state teardown; ResvErr is sent (with a
       model-local error code) when a reservation is preempted.
   * - Hello (instances, request/ack)
     - Modeled
     - A Hello timeout directly flips the corresponding TED link's state
       and triggers a routing-table rebuild rather than going through an
       independent IGP/interface-state layer; see the TED table below.
   * - Setup/holding priority and preemption
     - Modeled
     - Bandwidth sharing for admission control is computed per outgoing
       link.
   * - Make-before-break re-route / re-optimization
     - Not modeled
     - Path teardown happens before the replacement LSP is signaled
       (break-before-make).
   * - Fast reroute (RFC 4090)
     - Not modeled
     - n/a
   * - Session attribute affinities / admin groups
     - Not modeled
     - The TED carries no affinity/admin-group bits to match against.
   * - Traversal of non-RSVP clouds (Router Alert)
     - Not modeled (by design)
     - Path/Resv messages are addressed hop-by-hop directly to the
       next-hop router's address.
   * - Token-bucket Tspec (RFC 2210)
     - Partially modeled
     - The model tracks a single bandwidth value; the serializer writes
       it into the rate, bucket size, and peak rate fields alike, and the
       minimum policed unit/maximum packet size fields are fixed
       placeholders that are not modeled state.
   * - Wire-format serialization
     - Modeled
     - The model's internal message-type numbering is translated to the
       RFC wire numbers on serialization; ``setup_pri``/``holding_pri``
       and the internal PathTear "force" flag have no RFC wire
       representation and do not survive a serialize/deserialize round
       trip.

MPLS data plane (RFC 3031 + RFC 3032 + RFC 3443 + RFC 5462)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Mechanism
     - Status
     - Notes
   * - Label stack push/swap/pop
     - Modeled
     - n/a
   * - Shim header serialization
     - Modeled
     - n/a
   * - TTL processing (RFC 3443)
     - Modeled
     - Uniform and pipe models, selected with the ``ttlModel`` parameter;
       TTL expiry hands the datagram up to L3 for ICMP Time Exceeded
       processing.
   * - Traffic Class / EXP handling (RFC 5462)
     - Modeled
     - DSCP-to-TC mapping on push/pop; writing the (lossy) TC value back
       into the IP DSCP on pop is opt-in via ``writeTcBackOnPop`` (off by
       default).
   * - Reserved labels 0-15 / penultimate-hop popping
     - Modeled
     - The implicit null label (3) can be advertised by both LDP and
       RSVP-TE via their respective ``advertiseImplicitNull`` parameters.
   * - Payload protocol at egress
     - Partially modeled
     - Popping the last label always re-tags the payload as IPv4; there
       is no per-LIB-entry payload-protocol field, so non-IPv4 payloads
       (e.g., for pseudowires) cannot be carried.
   * - ECMP / entropy label over FECs
     - Not modeled
     - n/a
   * - Label allocation reuse
     - Not modeled
     - Labels are allocated from a monotonically increasing counter;
       freed labels are never reclaimed.
