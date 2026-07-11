.. _ug:cha:mpls:

The MPLS Models
===============

.. _ug:sec:mpls:overview:

Overview
--------

Multi-Protocol Label Switching (MPLS) is a "layer 2.5" protocol for high-performance telecommunications networks. MPLS data is directed from one network node to the next using numeric labels instead of network addresses. This avoids complex lookups in a routing table and allows for traffic engineering. The labels identify virtual links (label-switched paths or LSPs, also called MPLS tunnels) between distant nodes, rather than endpoints. The routers that make up a label-switched network are called label-switching routers (LSRs) inside the network ("transit nodes"), and label edge routers (LER) on the edges of the network ("ingress" or "egress" nodes).

A fundamental MPLS concept is that the meaning of the labels used to forward traffic between and through two LSRs must be agreed upon. This common understanding is achieved by using signaling protocols through which one LSR informs another of label bindings it has made. Such signaling protocols are also called label distribution protocols. The two main label distribution protocols used with MPLS are LDP and RSVP-TE.

Segment Routing over the MPLS data plane (SR-MPLS, RFC 8660) takes a different approach: instead of a signaling protocol establishing per-hop label bindings, every router derives its own labels from a network-wide map of router identities to small integers ("segment identifiers", or SIDs), flooded by extensions to the IGP (OSPF-SR, IS-IS-SR). An ingress router can then steer a packet along an explicit, loosely specified path by pushing an ordered stack of these labels ("segments"), without any LSP being signaled or torn down anywhere in the network.

INET provides basic support for building MPLS simulations. It provides models for the MPLS, LDP, RSVP-TE, and SR-MPLS protocols and their associated data structures, and preassembled MPLS-capable router models.

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

- :ned:`StaticIngressClassifier` is a static, XML-configured ingress classifier for MPLS, matching on IPv4 or IPv6 destination prefixes

- :ned:`SegmentRouting` derives node-SID and adjacency-SID label programming from the TED, signaling-free, for SR-MPLS

- :ned:`SrPolicy` is a configurable ingress classifier for MPLS that imposes SR-MPLS segment-list label stacks

- :ned:`Pce` is a stateful Path Computation Element (PCEP server) that computes and updates RSVP-TE ingress paths on behalf of a :ned:`Pcc`

- :ned:`Pcc` is a Path Computation Client, co-located with an ingress :ned:`RsvpTe`, that delegates path computation and (optionally) ongoing path control to a :ned:`Pce`

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

.. _ug:sec:mpls:segmentrouting:

SegmentRouting
~~~~~~~~~~~~~~

The :ned:`SegmentRouting` module implements the SR-MPLS (RFC 8660) control plane: for every other router, it derives an :ned:`LibTable` entry from the router's own view of the network, with no signaling protocol and no per-hop label negotiation.

In a real deployment, every router advertises a small integer node identifier (a "node SID") via extensions to the IGP (OSPF-SR, IS-IS-SR, see RFC 8665/8667), and all routers agree on a common "Segment Routing Global Block" (SRGB) of label values from which node-SID labels are drawn. INET does not model IGP flooding of SID advertisements; instead, :ned:`SegmentRouting` takes this router-id-to-SID map as a static, network-wide-consistent XML configuration file (the ``sidTable`` parameter), and takes the network topology itself from the :ned:`Ted` module (playing the same role here that it plays for :ned:`Ldp`). All routers are expected to be configured with the same ``srgbBase`` and ``srgbSize`` (a homogeneous SRGB); this is what makes a node-SID label mean the same thing at every router.

An example ``sidTable`` XML file:

.. code-block:: xml

   <sr>
       <node router="10.1.1.1" sid="1"/>
       <node router="10.1.1.2" sid="2"/>
   </sr>

For every other router N with node-SID index ``sid``, :ned:`SegmentRouting` computes N's node-SID label as ``srgbBase + sid``, computes the shortest path to N over the :ned:`Ted`, and installs one :ned:`LibTable` entry for that label: a *swap* to the same label (the homogeneous-SRGB property) if N is not a direct neighbor, or a *pop* (penultimate-hop-popping, RFC 8660 Section 2) if N is a direct neighbor. Whenever the TED changes, :ned:`SegmentRouting` recomputes and reinstalls its own entries, without disturbing any coexisting :ned:`Ldp` or :ned:`RsvpTe` entries in the same :ned:`LibTable`.

:ned:`SegmentRouting` additionally allocates one dynamically-numbered *adjacency SID* (RFC 8660 Section 3.3) per currently-up local link, each an entry that pops the label and forwards out that one specific interface. Adjacency SIDs are locally significant (not drawn from the SRGB, not flooded to other routers); they let an :ned:`SrPolicy` policy pin traffic onto one specific physical link, e.g., a non-shortest-path parallel link, regardless of what the IGP would otherwise choose.

.. _ug:sec:mpls:srpolicy:

SrPolicy
~~~~~~~~

The :ned:`SrPolicy` module is an ingress classifier for :ned:`Mpls` that generalizes :ned:`RsvpClassifier`'s single-label FEC table to a full, ordered *segment list*: an SR policy names a sequence of node SIDs and/or adjacency SIDs, and :ned:`SrPolicy` pushes the corresponding label stack, steering the packet along that explicit (if loosely specified) path -- SR traffic engineering with no signaling protocol and no LSP to establish or tear down.

An example ``policies`` XML file:

.. code-block:: xml

   <policies>
       <policy dest="10.3.3.0" netmask="255.255.255.0" segments="node:3 node:5"/>
       <policy dest="10.9.9.0" netmask="255.255.255.0" segments="node:3 adj:ppp2"/>
   </policies>

Each ``<policy>`` matches a destination prefix (longest-prefix match across all policies, like :ned:`RsvpClassifier`) and gives an ordered, whitespace-separated segment list. Each segment is either ``node:<sidIndex>`` (a node SID, resolved against the sibling :ned:`SegmentRouting` module's ``sidTable``) or ``adj:<interfaceName>`` (one of *this* router's own adjacency SIDs, resolved against that same module's dynamically allocated adjacency-SID table). Segments are listed left to right in network-processing order, i.e., the first segment is the outermost (active) label on the pushed stack -- the opposite of the order in which :ned:`LibTable`'s push operations are appended, so :ned:`SrPolicy` builds the operation list in reverse.

If the first segment's label would be immediately popped again by the very next router (a node segment whose target is this router's direct neighbor, or any adjacency segment, which by construction always names one of this router's own local links), that label is not pushed at all; the packet is simply forwarded out the resolved interface with the remaining labels underneath. Because adjacency-SID advertisement between routers is not modeled, an ``adj:`` segment only has well-defined meaning as a policy's first (or sole) segment, naming one of the ingress router's own links; see :ned:`SrPolicy`'s NED documentation for the full reasoning and its consequences if used later in a segment list.

.. _ug:sec:mpls:tilfa:

TI-LFA (Topology-Independent Loop-Free Alternate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Setting :ned:`SegmentRouting`'s ``tiLfa`` parameter to true (the default is false, so existing networks are unaffected) enables RFC 9855 local link protection: for every other router D whose current shortest path from this router (the Point of Local Repair, or PLR) leaves via one of this router's own links, :ned:`SegmentRouting` precomputes a backup label stack that steers D's traffic around that one link, and installs it (inactive) into :ned:`LibTable` alongside the primary node-SID entry.

The computation follows RFC 9855's P-space/Q-space construction: P-space is the set of nodes still reachable from the PLR (and, in "extended" P-space, from each of the PLR's other neighbors) at *unchanged* cost once the protected link is excluded; Q-space of D is the set of nodes whose own shortest path to D does not depend on that link either. If a node lies in both (a "PQ node"), a single extra node-SID segment naming it is enough: pushed on top of the packet's own, already-present node-SID(D) label, it detours the packet to the PQ node, which forwards it on toward D exactly as it would anyone else's node-SID traffic. If no single PQ node exists, :ned:`SegmentRouting` looks for a P-space node directly adjacent to a Q-space node and bridges the two with two node-SID segments instead of one (bounded by the ``maxRepairStackDepth`` parameter, default 3; beyond that the destination is left unprotected and an ``EV_WARN`` is logged). Either way, the protected link's own far end is never itself chosen as a repair segment, even where the P/Q-space math would technically allow it: routing a *different* destination's traffic through the node whose adjacency to the PLR just changed would depend on that node's own view of the network already being consistent, which is exactly the assumption RFC 9855's P/Q-space rules exist to avoid leaning on.

This deliberately diverges from RFC 9855 Section 5's textbook two-segment form, which bridges via the P-node's own *adjacency* SID rather than a second node SID: since adjacency-SID advertisement between routers is not modeled here (see :ref:`ug:sec:mpls:srpolicy`), a PLR could never resolve a label value for a third-party P-node's adjacency in the first place. Two globally-resolvable node SIDs sidestep that gap entirely, at the cost of one extra label on the wire versus the RFC's own encoding.

The backup activates the instant the protected interface goes down (:ned:`Ted`'s ``trackInterfaceState`` hook, the same physical-carrier-loss signal :ned:`Ldp`/:ned:`RsvpTe` rely on for their own failure detection) and reverts the instant it comes back up; no IGP reconvergence has to complete first. In this model, however, the ordinary node-SID recompute this same local failure triggers (see `SegmentRouting`_ above) *also* runs synchronously in the very same simulation event, and for the PLR's own traffic already reroutes to the very same post-convergence path the backup would have used. The backup mechanism's own, distinct value here is therefore in the data plane it exercises -- a genuine, RFC-shaped repair label stack pushed for any packet a downstream router still forwards to the PLR as a transit hop during the failure window -- not a measurable reduction in the convergence gap, which this discrete-event model doesn't otherwise incur. Node protection (surviving the failure of a whole neighboring router, not just one link), SRLG-aware protection, and formal micro-loop-freedom analysis (RFC 8333) are not modeled; protection is single-link only.

.. _ug:sec:mpls:ipv6:

IPv6 Support
------------

The MPLS data plane carries IPv6 payloads end-to-end, alongside IPv4, in the same network: each :ned:`LibTable` entry records the payload protocol its label ultimately carries, and :ned:`Mpls` stamps the popped datagram with that entry's own protocol at the true egress (rather than always assuming IPv4, as earlier versions of this model did). Both :ned:`Mpls`'s ingress path and its plain (unlabeled) from-L2 path offer IPv6 datagrams to the ingress classifier and up to L3 exactly as they already did for IPv4; TTL/Traffic-Class handling under the ``ttlModel``/``writeTcBackOnPop`` parameters dispatches to the IPv6 header's hop-limit/DSCP fields the same way it already did for IPv4's TTL/DSCP. The RFC 3032 IPv6 explicit-null label (value 2) is recognized at egress alongside the existing (IPv4) explicit-null label (0).

Ingress classification is dual-stack wherever a FEC table can name a destination at all: :ned:`StaticIngressClassifier` (a static, XML-configured ingress classifier -- one ``<fecentry dest="..." prefixLength="..." label="..." interface="..."/>`` per FEC, with the destination matched by longest prefix) and :ned:`SrPolicy` (see above) both match a policy's ``dest``/prefix against either an IPv4 or an IPv6 destination, resolved from whichever native header the packet actually carries. A :ned:`StaticIngressClassifier`/:ned:`SrPolicy` table may freely mix IPv4 and IPv6 entries; each incoming packet is only ever matched against entries of its own family. :ned:`StaticIngressClassifier`'s XML also still accepts the older, IPv4-only ``netmask="a.b.c.d"`` attribute in place of ``prefixLength`` for backward compatibility; an IPv6 ``dest`` requires ``prefixLength``.

:ned:`Ldp` additionally advertises and learns IPv6 FECs dynamically: if a router has an :ned:`Ipv6RoutingTable` (``hasIpv6 = true``), its IPv6 routes are turned into FECs and advertised/withdrawn via ordinary DU-mode Label Mapping messages exactly like IPv4 routes are, using RFC 7552's Address Family 2 encoding of the FEC TLV's Address Prefix element. This is real wire-format support (Address Family 1 vs. 2 selects a 4-byte or 16-byte prefix on an actual Label Mapping/Request/Notification message), not a static-configuration workaround -- but the LDP session itself (Hello discovery, TCP transport, LSR-ID, loop-detection path vectors) stays IPv4-only in this model: RFC 7552 allows a session's own transport address family to be independent of which FEC families it advertises, and that is the only part of RFC 7552 this model exploits. Because DU-mode advertisement only ever concerns FECs a router's own routing table already knows about, every transit hop along a dynamically-signaled IPv6 LSP -- not just the ingress/egress edges -- needs its own IPv6 route to the destination, exactly as a transit hop needs its own IPv4 route for an ordinary IPv4 LDP LSP.

A "6PE-lite" example under ``examples/mpls/ipv6pe/`` demonstrates IPv6-addressed edge islands reachable across a small, purely IPv4-addressed, IPv4-signaled SR-MPLS core -- the transit routers carry no IPv6 configuration at all. Because an SR-MPLS node SID is allocated per router and shared by both address families (unlike an LDP- or static-XML-configured FEC, which is inherently family-specific), :ned:`SrPolicy` pushes an extra, innermost IPv6 explicit-null label under the node-SID label stack whenever a matched policy's destination is IPv6, so that the true egress -- not any of the family-agnostic transit routers doing penultimate-hop-popping along the way -- is the one that decides the payload is IPv6. See that example's README for the full walkthrough and for how this differs from a real 6PE (RFC 4798) deployment.

Explicitly out of scope: BGP labeled-unicast / a real 6PE-6VPE control plane (this model's edge-to-edge IPv6-prefix-to-label mapping is static/SR-policy configuration, not a routing protocol); RSVP-TE sessions for IPv6 FECs (RFC 3209's IPv6 objects are not modeled; :ned:`RsvpTe`/:ned:`Ted`/:ned:`RsvpClassifier` remain IPv4-only throughout); LDP-over-IPv6 transport (RFC 7552 Section 6's dual-stack transport-connection and preference rules -- TCP over an IPv6 session -- are not modeled: only the FEC TLV itself is dual-stack); an IPv6-native :ned:`Ted`/:ned:`LinkStateRouting` (both stay IPv4-keyed regardless of which payload families ride over the LSPs they help establish); and SRv6 (a different data plane encoding entirely from the MPLS-based SR-MPLS this chapter describes).

.. _ug:sec:mpls:pcep:

PCEP / Stateful PCE
-------------------

:ned:`Pce` and :ned:`Pcc` implement a Path Computation Element (PCE) controller and its client, computing and (optionally) actively controlling RSVP-TE ingress paths from a central point rather than leaving every ingress router to compute its own routes locally. A :ned:`Pcc`, co-located with an ingress :ned:`RsvpTe`, connects to a statically configured :ned:`Pce` over TCP port 4189 (RFC 5440's assigned PCEP port); the session comes up via an Open/Keepalive exchange (KeepAlive Time negotiated as the smaller of both sides' proposals; DeadTimer-based liveness, each side monitoring the *other's own* advertised value). PCEP has no discovery phase (unlike :ned:`Ldp`'s Hello): the PCE's address is configured directly on the PCC, which always plays the active (connecting) role.

Architecture: the omniscient TED
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A real PCE typically learns network topology via BGP-LS or IGP peering. This model's :ned:`Pce` instead holds a direct module-path reference to the SAME :ned:`Ted` module every router already maintains via :ned:`LinkStateRouting` -- an "omniscient" simplification in the same category as :ned:`Ted`'s own pre-existing topology omniscience elsewhere in this codebase (e.g. :ned:`RsvpTe`'s own local CSPF, :ned:`SegmentRouting`'s node-SID computation). There is no BGP-LS feed, no partial/incremental topology view, and no separate PCE-side TED synchronization protocol to model.

Two modes: stateless computation and stateful delegation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- **Stateless (RFC 5440 Sections 6.5-6.6, PCReq/PCRep).** Setting :ned:`RsvpTe`'s ``computationMode`` to ``"pce"`` (default ``"local"``, fully backward compatible) redirects its existing CSPF hook (``computeEro``, see the RSVP-TE conformance table below) from a local :ned:`Ted` call to a PCReq/PCRep round trip with the configured :ned:`Pcc` (``pccModule``): the PCE runs the IDENTICAL bandwidth-pruning CSPF (``Ted::calculateShortestPath()``) that :ned:`RsvpTe`'s own local computation already uses, just rooted at the requesting PCC and computed centrally instead of in-process. A PCReq that finds no PCE session yet OPERATIONAL, or gets a NO-PATH reply, falls through to :ned:`RsvpTe`'s existing path-retry machinery unchanged -- there is no separate failure path to learn.
- **Stateful (RFC 8231, PCRpt/PCUpd).** Setting :ned:`RsvpTe`'s ``delegate`` parameter to true (meaningful only alongside ``computationMode="pce"``) additionally reports every ingress LSP's establishment, make-before-break replacement, and teardown to the PCE (a PCRpt, keyed by a stable PLSP-ID that survives every later path replacement for that LSP). The PCE may then push an unsolicited PCUpd carrying a new ERO for a delegated LSP -- via a ScenarioManager ``<pce-reoptimize plspId="N"/>`` command in this model (a TED-change-triggered version was considered and rejected as harder to make deterministic for testing) -- which the PCC applies via the SAME make-before-break machinery an internally-triggered reroute already uses, using the PCE-supplied ERO directly (no fresh CSPF/PCReq round-trip, since the PCE's ERO is already a full strict route).

Controller-death fallback
~~~~~~~~~~~~~~~~~~~~~~~~~

If a stateful PCEP session is lost while LSPs remain delegated, the :ned:`Pcc` waits ``stateTimeout`` (RFC 8231 Section 5.2, default 60 s) for the PCE to come back. If the session is still down once the timer expires, every affected LSP falls back to LOCAL path computation (:ned:`Ted`'s CSPF, the SAME code path ``computationMode="local"`` already uses) -- a one-time forced reroute, not merely a passive "next time it happens to need one" -- and is never delegated/reported again. If the session recovers before the timeout, delegation simply continues; no RFC 8231 Section 5.6 State Synchronization Sequence is performed (a documented scope simplification -- the PCE does not re-learn which LSPs were delegated to it before a session bounce).

Explicitly out of scope: BGP-LS topology feed; multi-PCE/H-PCE (RFC 6805); PCEP-over-TLS (RFC 8253); P2MP paths (RFC 8306); association groups (RFC 8697/8745); PCEP objective-function TLV negotiation (RFC 5541 -- the fixed objective is always minimum TE metric subject to a bandwidth constraint); PCE-initiated LSPs (RFC 8281 -- every LSP here is PCC-initiated and merely delegated after the fact); and SR-mode PCUpd application (RFC 8664-style segment-list updates via :ned:`SrPolicy` -- only RSVP-TE-mode delegation, a full strict hop-by-hop ERO, is modeled). A runnable example combining stateless computation, stateful delegation, and a genuine PCE-initiated reroute is provided under ``examples/mpls/pcep``.

.. _ug:sec:mpls:mpls-enabled-router-models:

MPLS-Enabled Router Models
--------------------------

INET provides the following preassembled MPLS routers:

- :ned:`LdpMplsRouter` is an MPLS router with the LDP signaling protocol

- :ned:`RsvpMplsRouter` is an MPLS router with the RSVP-TE signaling protocol

- :ned:`SrMplsRouter` is an MPLS router with SR-MPLS node-SID/adjacency-SID label programming (:ned:`SegmentRouting`) and an :ned:`SrPolicy` ingress classifier

A complete, runnable example demonstrating node-SID forwarding, an explicit-path policy that overrides the IGP-preferred route, and an adjacency-SID pin onto a non-preferred parallel link is provided under ``examples/mpls/sr``.

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
     - Partially modeled
     - IPv6 FECs (Address Family 2 in the FEC TLV's Address Prefix element)
       are advertised and learned dynamically via ordinary DU-mode Label
       Mapping messages, and label-switch real IPv6 traffic end-to-end (see
       :ref:`ug:sec:mpls:ipv6`). The session itself -- Hello discovery, TCP
       transport, LSR-ID, loop-detection path vectors -- remains IPv4-only;
       RFC 7552 Section 6's dual-stack transport-connection rules are not
       modeled.
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

SR-MPLS (RFC 8660)
~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Mechanism
     - Status
     - Notes
   * - Node segments (node SIDs) and PHP
     - Modeled
     - :ned:`SegmentRouting` installs a swap or (penultimate-hop-popping)
       pop entry per remote router, computed from :ned:`Ted`.
   * - Adjacency segments (adjacency SIDs)
     - Partially modeled
     - Dynamically allocated per local link and usable via :ned:`SrPolicy`'s
       ``adj:`` segments, but only as a policy's first/sole segment; see
       :ned:`SrPolicy`'s NED documentation for why.
   * - IGP flooding of SID advertisements (OSPF-SR/IS-IS-SR, RFC
       8665/8667)
     - Not modeled
     - The router-id-to-SID map is static, network-wide-consistent
       configuration (the ``sidTable`` XML), standing in for what a real
       deployment would flood.
   * - Segment Routing Global Block (SRGB)
     - Partially modeled
     - ``srgbBase``/``srgbSize`` are NED parameters; a homogeneous SRGB across
       the network is the supported case and is not itself checked
       network-wide.
   * - Explicit-path / segment-list steering
     - Modeled
     - :ned:`SrPolicy` pushes an ordered label stack per a static XML
       policy table; recomputation on topology change is inherited from
       :ned:`SegmentRouting`'s own per-node-SID recomputation.
   * - SR-DB / BGP-LS, PCEP-driven policies
     - Not modeled
     - n/a
   * - TI-LFA (RFC 9855) local link protection
     - Partially modeled
     - :ned:`SegmentRouting` precomputes a per-destination P/Q-space
       backup label stack for each of its own links and activates it via
       :ned:`LibTable`'s backup fields the instant that link's own
       interface goes down (see below); node protection, SRLG awareness,
       and formal micro-loop-freedom analysis are not modeled.
       Adjacency-SID pins are not protected at all: a pinned link's
       failure blackholes that policy's traffic.
   * - SRv6
     - Not modeled
     - Only the MPLS data plane encoding (RFC 8660) is implemented.
   * - Entropy labels / ECMP over segment lists
     - Not modeled
     - n/a

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
     - Modeled
     - Each :ned:`LibTable` entry records its own payload protocol
       (IPv4 or IPv6; default IPv4, preserving pre-existing
       configurations byte-for-byte); popping the last label re-tags
       the payload according to that entry rather than always assuming
       IPv4. See :ref:`ug:sec:mpls:ipv6`.
   * - ECMP / entropy label over FECs
     - Not modeled
     - n/a
   * - Label allocation reuse
     - Not modeled
     - Labels are allocated from a monotonically increasing counter;
       freed labels are never reclaimed.

PCEP / Stateful PCE (RFC 5440 + RFC 8231)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 32 14 54

   * - Mechanism
     - Status
     - Notes
   * - Session establishment (Open/Keepalive, KeepAlive Time negotiation, DeadTimer liveness)
     - Partially modeled
     - Minimal FSM (mirrors :ned:`Ldp`'s own level of fidelity); no Open error
       negotiation/retry (RFC 5440 Section 6.3).
   * - Stateless path computation (PCReq/PCRep)
     - Modeled
     - The PCE runs the identical bandwidth-pruning CSPF :ned:`RsvpTe`'s own
       local computation uses (Workstream C6), rooted at the requesting PCC.
       Fixed objective function only (minimum TE metric, bandwidth-feasible);
       no objective-function TLV negotiation (RFC 5541).
   * - Stateful delegation (PCRpt/PCUpd)
     - Modeled
     - A delegated LSP is identified by a stable PLSP-ID across every later
       make-before-break replacement. PCE-initiated reoptimization is
       triggered by an explicit ScenarioManager command in this model, not
       autonomously on every TED change.
   * - RFC 8231 Section 5.6 State Synchronization Sequence
     - Not modeled
     - A PCEP session that bounces and reconnects does not re-establish which
       LSPs were previously delegated.
   * - Controller-death fallback (state timeout)
     - Modeled
     - A PCC whose session stays down past ``stateTimeout`` (RFC 8231 Section
       5.2) forces every delegated LSP back to local (non-PCE) computation.
   * - SR-mode path updates (RFC 8664-style segment-list PCUpd via :ned:`SrPolicy`)
     - Not modeled
     - Only RSVP-TE-mode delegation (a full strict hop-by-hop ERO) is
       modeled; applying a PCUpd through :ned:`SrPolicy` would first need a
       runtime policy-update API :ned:`SrPolicy` does not currently have.
   * - BGP-LS topology feed
     - Not modeled
     - The PCE's :ned:`Ted` view is a direct, omniscient module-path
       reference, not a learned topology feed.
   * - Multi-PCE / Hierarchical PCE (RFC 6805)
     - Not modeled
     - Exactly one PCE, one PCC-to-PCE session per PCC.
   * - PCEP-over-TLS (RFC 8253)
     - Not modeled
     - Sessions run over plain TCP.
   * - P2MP paths (RFC 8306)
     - Not modeled
     - Every LSP modeled is point-to-point.
   * - Association groups (RFC 8697/8745)
     - Not modeled
     - n/a
   * - PCE-initiated LSPs (RFC 8281)
     - Not modeled
     - Every LSP is PCC-initiated and merely delegated to the PCE after the
       fact.
   * - Wire format (Common Header, Open/Keepalive/PCReq/PCRep/PCRpt/PCUpd)
     - Modeled
     - Every message type round-trips through a registered serializer; the
       LSP/SRP/RP objects are written as separate byte-aligned fields rather
       than RFC 5440/8231's compact bit-packed layout (a documented
       simplification).
