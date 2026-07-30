:orphan:

.. _dg:cha:mpls:

The MPLS Models
===============

This chapter describes the internals of INET's MPLS/LDP/RSVP-TE/TED models:
the data-plane packet flow through :ned:`Mpls` and :ned:`LibTable`, how the
two signaling protocols program the LIB, and the wire formats used by their
serializers.

Packet and LIB Data-Plane Flow
------------------------------

The data plane is implemented by two cooperating modules: :ned:`Mpls` (the
forwarding logic) and :ned:`LibTable` (the Label Information Base, or LIB,
that :ned:`Mpls` consults on every packet).

- An unlabeled (plain IPv4) packet arriving from L3 is handed to the
  configured ingress classifier, an object implementing
  ``IIngressClassifier`` (its ``lookupLabel()`` method). The concrete
  classifier is whichever signaling protocol module is in use: the ``Ldp``
  module implements ``IIngressClassifier`` directly, while ``RsvpTe`` uses a
  separate ``RsvpClassifier`` module (via the narrower ``IRsvpClassifier``
  interface). The classifier determines the packet's forwarding equivalence
  class (FEC) and returns the label operation(s) to apply plus the outgoing
  interface.
- The label operations returned by the classifier (and, for already-labeled
  packets arriving from L2, the label operations found by looking up the
  incoming (interface, label) pair in ``LibTable``) are then applied by
  ``Mpls::doStackOps()``, which pushes, swaps, or pops labels on the
  packet's MPLS label stack, doing the actual work described by RFC 3031's
  reference forwarding model.
- Along the way, ``Mpls`` also enforces the RFC 3443 TTL model (uniform or
  pipe, selected by the ``ttlModel`` parameter) and an RFC 5462 Traffic
  Class mapping between the IP DSCP and the MPLS TC field; penultimate-hop
  popping is available via the reserved implicit-null label (RFC 3032).

The following algorithm is carried out by the MPLS module (this is the
model's original 2005 design summary; see the paragraphs above for how the
classifier and LIB fit into it in the current code):



::

   Step 1: - Check from which layer the packet is coming
   Alternative 1: From layer 3
       Step 1: Find and check the next hop of this packet
       Alternative 1: Next hop belongs to this MPLS cloud
           Step 1: Encapsulate the packet in an MPLS packet using the label
           operations returned by the ingress classifier
           Step 2: Send it to the next hop
           Step 3: Return
       Alternative 2: Next hop does not belong to this MPLS cloud
           Step 1: Send the packet to the next hop
   Alternative 2: From layer 2
       Step 1: Record the packet's incoming interface
       Step 2: - Check if the packet is for this LSR
       Alternative 1: Yes
           Step 1: Check if the packet has a label
           Alternative 1: Yes
               Step 1: Strip off all labels and send the packet to L3
               Step 2: Return
           Alternative 2: No
               Step 1: Send the packet to L3
               Step 2: Return
       Alternative 2: No
           Step 1: Continue to the next step
       Step 3: Check the packet type
       Alternative 1: The packet is native IP
           Step 1: Check the LSR type
           Alternative 1: The LSR is an Ingress Router
               Step 1: Look up the LIB for the outgoing label
               Alternative 1: Label cannot be found
                   Step 1: Check if the label for this FEC is being requested
                   Alternative 1: Yes
                       Step 1: Return
                   Alternative 2: No
                       Step 1: Store the packet with FEC id
                       Step 2: Request the signalling component
                       Step 3: Return
               Alternative 2: Label found
                   Step 1: Carry out the label operation on the packet
                   Step 2: Forward the packet to the outgoing interface found
                   Step 3: Return
           Alternative 2: The LSR is not an Ingress Router
               Step 1: Print out the error
               Step 2: Delete the packet and return
       Alternative 2: The packet is MPLS
           Step 1: Check the LSR type
           Alternative 1: The LSR is an Egress Router
               Step 1: POP the top label
               Step 2: Forward the packet to the outgoing interface found
               Step 3: Return
           Alternative 2: The LSR is not an Egress Router
               Step 1: Look up the LIB for the outgoing label
               Alternative 1: Label cannot be found
                   Step 1: Check if the label for this FEC is being requested
                   Alternative 1: Yes
                       Step 1: Return
                   Alternative 2: No
                       Step 1: Store the packet with FEC id
                       Step 2: Request the signalling component
                       Step 3: Return
               Alternative 2: Label found
                   Step 1: Carry out the label operation on the packet
                   Step 2: Forward the packet to the outgoing interface found
                   Step 3: Return
   Step 2: Return

Programming the LIB
-------------------

Neither ``Ldp`` nor ``RsvpTe`` writes to ``LibTable`` directly through its
own storage; both go through the same ``LibTable::installLibEntry()`` API,
which allocates (or updates) one LIB row given an incoming interface/label
and the outgoing interface plus label operation vector, and returns the
label actually assigned (useful when the caller passes ``-1`` to request a
fresh, locally-allocated label).

- ``Ldp`` calls ``installLibEntry()`` from ``updateFecListEntry()`` (egress:
  installs the local pop, or implicit-null, entry and advertises it upstream
  as a Label Mapping), from ``processLABEL_REQUEST()`` (transit: installs a
  swap entry for a FEC a peer explicitly requested a label for), and from
  ``processLABEL_MAPPING()`` (ingress/transit: installs a fresh swap or push
  entry for a label mapping received from the downstream peer).
- ``RsvpTe`` calls ``installLibEntry()`` from ``commitResv()`` when a Resv
  message is processed, both for the LSP's ingress mapping and, on
  transit/egress routers, for the label distributed by the immediate
  downstream neighbor.

Both protocols therefore reduce, from the data plane's point of view, to
"populate ``LibTable`` rows and act as the ingress classifier" -- the actual
forwarding decision is always made by ``Mpls``/``LibTable`` as described
above, regardless of which signaling protocol installed the entry.

LDP Message Processing
----------------------

The simulation follows the message processing rules specified in RFC 3036
(LDP Specification). A summary of the algorithm used in the RFC is
presented below.

Label Request Message processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An LSR may transmit a Request message under any of the following conditions:

- The LSR recognizes a new FEC via the forwarding table, and the next
   hop is its LDP peer. The LIB of this LSR does not have a mapping from
   the next hop for the given FEC.

- Network topology changes, and the next hop to the FEC is no longer valid
   and new mapping is not available.

- The LSR receives a Label Request for a FEC from an upstream LDP, and
   it does not have label binding information for this FEC. The FEC next
   hop is an LDP peer.

Upon receiving a Label Request message, the following procedures will be
performed:



::

   Step 1: Extract the FEC from the message and locate the incoming interface
           of the message.
   Step 2: Check whether the FEC is an outstanding FEC.
       Alternative 1: This FEC is outstanding
           Step 1: Return
       Alternative 2: This FEC is not outstanding
           Step 1: Continue
   Step 3: Check if there is an exact match of the FEC in the routing table.
       Alternative 1: There is an exact match
           Step 1: Continue
       Alternative 2: There is no match
           Step 1: Construct a Notification message of No route and
                   send this message back to the sender.
   Step 4: Make a query to the local LIB to find out the corresponding label.
       Alternative 1: The label was found
           Step 1: Construct a Label Mapping message and send it over
                   the incoming interface.
       Alternative 2: The label cannot be found for this FEC
           Step 1: Construct a new Label Request message and send
                   the message out using L3 routing.
           Step 2: Construct a Notification message indicating that the
                   label cannot be found.

Label Mapping Message processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Upon receiving a Label Mapping message, the following procedures will be
performed:



::

   Step 1: Extract the FEC and the label from the message.
   Step 2: Check whether this is an outstanding FEC
       Alternative 1: This FEC is outstanding
           Step 1: Continue
       Alternative 2: This FEC is not outstanding
           Step 1: Send back the server a Notification of Error message.
   Step 3: Install the new label to the local LIB using the extracted label,
           FEC, and the message's incoming interface.

The CSPF Algorithm
------------------

.. note::
   The algorithm described in this section is **not implemented** by the
   current model. ``RsvpTe`` resolves loose ERO hops with a plain
   routing-table lookup, one hop at a time; there is no online
   constraint-based path computation at the ingress. A bandwidth-pruned
   shortest-path helper (``Ted::calculateShortestPath``) exists in the
   ``Ted`` module but has no caller anywhere in the codebase. Explicit
   routes (strict and/or loose hops) must be specified by hand, either in
   the ``traffic`` XML configuration or via ``add-session``
   :ned:`ScenarioManager` commands (see the users guide's MPLS chapter).
   The rest of this section documents the design that was originally
   envisioned for this feature, kept here for anyone implementing it.

CSPF stands for Constraint Shortest Path First. This constraint-based
routing is executed online by the Ingress Router. The CSPF calculates an
optimum explicit route (ER), based on specific constraints. CSPF relies
on a Traffic Engineering Database (TED) to do those calculations. The
resulting route is then used by RSVP-TE.

The CSPF, in particular, and any constraint-based routing process require
the following inputs:

- Attributes of the traffic trunks, e.g., bandwidth, link affinities

- Attributes of the links of the network, e.g. bandwidth, delay

- Attributes of the LSRs, e.g. types of signaling protocols supported

- Other topology state information.

There has been no standard for CSPF so far. The implementation of CSPF
in the simulation is based on the concept of "induced graph" introduced
in RFC 2702. An induced graph is analogous to a virtual topology in an
overlay model. It is logically mapped onto the physical network through
the selection of LSPs for traffic trunks. CSPF is similar to a normal
SPF, except during link examination, it rejects links without capacity
or links that do not match color constraints or configured policy. The
CSPF algorithm used in the simulation has two phases. In the first
phase, all the links that do not satisfy the constraints of bandwidth
are excluded from the network topology. The link affinity is also
examined in this phase. In the second phase, the Dijkstra algorithm is
performed.

Dijkstra Algorithm:



::

   Dijkstra(G, w, s):
      Initialize-single-source(G,s);
      S = empty set;
      Q = V[G];
      While Q is not empty {
          u = Extract-Min(Q);
          S = S union {u};
          for each vertex v in Adj[u] {
              relax(u, v, w);
          }
      }

In which:

-  G: the graph, represented in some way (e.g. adjacency list)

-  w: the distance (weight) for each edge (u,v)

-  s (small s): the starting vertex (source)

-  S (big S): a set of vertices whose final shortest path from s have
   already been determined

-  Q: set of remaining vertices, Q union S = V

Wire Formats and Serializers
----------------------------

Three chunk-serializer classes make every packet exchanged by this
subsystem convertible to and from actual bytes (a prerequisite for the
``~tND`` fingerprint ingredient, for pcap recording, and for emulation).
Each follows the usual ``FieldsChunkSerializer`` pattern: one class
registered, via ``Register_Serializer()``, for every concrete chunk type it
handles.

LDP (``LdpPacketSerializer``, RFC 5036)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Encodes the 10-byte PDU header (version, PDU length, LSR-Id, label space)
followed by exactly one message (8-byte message header plus TLVs) --- see
the "one message per PDU" simplification noted in :ned:`Ldp`'s
documentation. The TLV type codes used are the standard RFC 5036 Section
3.4 values: FEC TLV (0x0100), Address List TLV (0x0101), Generic Label TLV
(0x0200), Status TLV (0x0300), Common Hello Parameters TLV (0x0400), and
Common Session Parameters TLV (0x0500). PDU/message lengths are derived
from the chunk's actual serialized length, not stored as separate msg
fields.

RSVP-TE (``RsvpTeSerializer``, RFC 2205 + RFC 3209)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Encodes the 8-byte RSVP common header followed by the message's objects in
RFC Class-Num/C-Type order (SESSION, RSVP_HOP, TIME_VALUES,
LABEL_REQUEST/LABEL, ERO, SENDER_TEMPLATE, SENDER_TSPEC, STYLE, FLOWSPEC,
FILTER_SPEC, RRO, ERROR_SPEC, HELLO, depending on message type). The
model's internal ``RsvpConstants`` message-type enum was never written to
match the RFC 2205 Section 3.1.1 wire numbering, so the serializer
translates between the two rather than renumbering the model enum (which
would have a much larger blast radius across every ``switch`` in
``RsvpTe.cc``):

.. list-table::
   :header-rows: 1
   :widths: 40 30

   * - Model ``rsvpKind``
     - RFC wire message type
   * - ``PATH_MESSAGE`` (1)
     - Path (1)
   * - ``RESV_MESSAGE`` (2)
     - Resv (2)
   * - ``PTEAR_MESSAGE`` (3)
     - PathTear (5)
   * - ``RTEAR_MESSAGE`` (4)
     - ResvTear (6)
   * - ``PERROR_MESSAGE`` (5)
     - PathErr (3)
   * - ``RERROR_MESSAGE`` (6)
     - ResvErr (4)
   * - ``HELLO_MESSAGE`` (7)
     - Hello (20, per the RFC 3209 appendix)

A few model fields have no RFC wire representation and therefore do not
survive a serialize/deserialize round trip: the session's setup/holding
priority (a real implementation would carry these in a SESSION_ATTRIBUTE
object, which this model never emits) and the internal PathTear "force"
flag. The Tspec/Flowspec objects likewise only carry a single bandwidth
value in the model; the serializer writes it into the RFC 2210 token
bucket's rate, bucket size, and peak rate fields alike, with fixed
placeholder values for the (unmodeled) minimum policed unit and maximum
packet size parameters.

TED link-state protocol (``LinkStateRoutingSerializer``, model-internal format v1)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``LinkStateMsg`` is **not an RFC wire format** -- it is this model's own
flooding protocol, used by ``LinkStateRouting`` to mirror the local TED
contents to peers; no standard defines its bytes. ``LinkStateRoutingSerializer``
fixes one deterministic, documented layout for it (explicitly versioned as
"format v1" in its header comment, in case a future change needs a v2): a
4-byte message header (command, flags, a 16-bit record count) followed by
that many fixed-width ``TeLinkStateInfo`` records, all fields in network
byte order. Because this is model-internal, any future change to the
record layout only needs to keep the model and the serializer in sync with
each other -- there is no external interoperability constraint.
