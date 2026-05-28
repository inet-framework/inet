Multicast Routing with PIM
==========================

Goals
-----

When a server needs to deliver the same content — such as a live video stream —
to many receivers at once, the straightforward approach is to send a separate
unicast copy to each one. This works, but it wastes bandwidth: the server's
uplink and every shared link along the way must carry N identical copies for N
receivers.

IP multicast solves this by letting the *network* replicate packets. The source
sends each packet once, and routers along the path duplicate it only where the
tree branches. Each link carries only one copy, regardless of how many
receivers are downstream. On the local network segment, the last-hop router
sends the packet as a multicast frame — all interested hosts receive it
directly. Hosts signal their interest to their local router using IGMP
(Internet Group Management Protocol).

Protocol Independent Multicast (PIM) is the protocol that builds and maintains
these distribution trees. It is called "protocol independent" because it does
not include its own topology discovery mechanism — instead it relies on the
unicast routing table (populated by any unicast routing protocol or static
configuration).

In this showcase, we demonstrate two PIM modes available in INET — Dense Mode
(PIM-DM) and Sparse Mode (PIM-SM) — using simple example networks. We also
show a more realistic IPTV streaming scenario using PIM-SM over a grid
network.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/routing/pim2 <https://github.com/inet-framework/inet/tree/master/showcases/routing/pim2>`__

About PIM
---------

PIM operates in two primary modes, each suited to a different multicast traffic
pattern.

**PIM Dense Mode (PIM-DM)** assumes that multicast group members are densely
distributed throughout the network. It uses a *flood-and-prune* strategy:

- Multicast traffic is initially **flooded** to all parts of the network.
- Routers with no interested downstream receivers send **Prune** messages back
  upstream to stop receiving the traffic. This prune state is temporary — it
  expires after a *prune timer* (default 180s), at which point the router
  resumes flooding. This creates a periodic flood-and-prune cycle.
- If a new receiver appears on a previously pruned branch *before* the prune
  timer expires, its router sends a **Graft** message for immediate
  re-attachment — without waiting for the next flood cycle.
- To avoid the periodic flood-and-prune cycle, the source's first-hop router (also called the *designated router* or DR)
  can periodically send **State Refresh** messages (every 60s by default)
  downstream to maintain prune state without re-flooding.

PIM-DM is defined in RFC 3973 (Experimental).

**PIM Sparse Mode (PIM-SM)** assumes receivers are sparsely distributed and
uses an explicit join model. Unlike PIM-DM, traffic does not flow until a
receiver asks for it — but receivers don't know where the source is, and
sources don't know where the receivers are. The protocol solves this by
designating one router as the *Rendezvous Point (RP)* — a well-known meeting
point. Receivers send Join messages *towards* the RP, and sources send their
traffic *to* the RP. The RP connects the two sides, and the tree of paths that
forms around it is called the *shared tree*. All routers in the PIM domain
must know the RP's address — in real networks this is typically learned via the
PIM Bootstrap mechanism, but in INET it is configured statically on every
router.

PIM uses a standard notation for multicast forwarding state. Each router's
multicast forwarding table contains entries that determine what to forward:

- ``(*,G)`` — "any source, group G": forward traffic for group G regardless of
  who sent it.
- ``(S,G)`` — "source S, group G": only forward traffic from a specific source.

The same notation is used in PIM control messages: a ``(*,G)`` Join message
asks each router along the path to create a ``(*,G)`` forwarding entry, while
an ``(S,G)`` Join asks for an ``(S,G)`` forwarding entry to be created. Receivers typically send
``(*,G)`` Joins because they just want the stream without caring who sends it,
while the RP uses ``(S,G)`` Joins when it needs to pull traffic from a
particular source.

The basic operation works as follows:

- Receivers send ``(*,G)`` **Join** messages towards the RP to subscribe to a
  multicast group. Traffic only flows along branches that have been explicitly
  requested.
- When a source starts sending, it simply sends a normal multicast IP packet
  onto its local LAN — it has no knowledge of PIM. The source's first-hop
  router (the *designated router* or DR) picks up this multicast packet and
  wraps it inside a unicast **Register** message
  addressed to the RP. This *registers* the source with the RP — informing it
  that a new source is active for this group, while simultaneously delivering
  the data.
  It is a temporary bootstrap mechanism — it gets traffic flowing immediately,
  but adds overhead: each packet carries an extra header, and the RP must
  individually decapsulate every packet from every source. With many
  simultaneous sources this can turn the RP into a bottleneck, which is why
  the protocol transitions to native multicast as quickly as possible:
- The RP strips the unicast envelope, recovers the original multicast packet,
  and forwards it down the shared tree to receivers. At the same time, it
  sends an (S,G) Join back towards the source to build a native multicast path
  (source → DR → ... → RP).
- Once native multicast forwarding is established on that path, the RP sends a
  **Register-Stop** to the DR. The DR stops encapsulating, and from this point
  on, traffic flows as plain multicast end-to-end — the RP is just another
  router in the tree, not a unicast relay.

.. TODO: The Register-Null bullet below might be too detailed for the About
   section. Consider removing it if it distracts from the main flow.

- After receiving Register-Stop, the DR starts a *Register-Stop Timer*. When
  the timer nears expiry, the DR sends a **Register-Null** message (a Register
  with no data) to the RP to check whether receivers still exist. If they do,
  the RP replies with another Register-Stop and the cycle repeats. This
  probing keeps the source's DR in sync with the RP without sending actual
  data.

When a receiver leaves the group, its router sends a **Prune** message towards
the RP to remove that branch from the shared tree.

PIM-SM is defined in RFC 4601.

.. note::

   The INET PIM-SM implementation simplifies the Register-Stop timing: the RP
   sends the Register-Stop immediately upon receiving the first Register
   message — in the same step as the (S,G) Join — rather than waiting for the
   native path to be established. This means only one Register-encapsulated
   packet is ever delivered, followed by a brief gap until the native (S,G)
   path propagates.

The Model
---------

We use two network topologies in this showcase. The first, ``PimShowcaseNetwork``,
is a simple tree topology used for both the PIM-DM and PIM-SM configurations.
A :ned:`StandardHost` (``source``) sends UDP multicast traffic to the group
address ``239.1.1.1`` through four :ned:`MulticastRouter` nodes (R0–R3) to two
receivers. R0 is the source's directly connected router, R1 is the tree root,
and R2/R3 branch towards the receivers. The receivers run :ned:`UdpSink`
applications configured to join this multicast group.

.. figure:: media/PimShowcaseNetwork.png

The second network, ``PimIptvNetwork``, models a more realistic scenario. Nine
:ned:`MulticastRouter` nodes are arranged in a 3×3 grid, with an IPTV server
(:ned:`StandardHost`) attached at one corner and a client at the opposite corner.

.. figure:: media/PimIptvNetwork.png

Both networks extend :ned:`WiredNetworkBase` (which provides an
:ned:`Ipv4NetworkConfigurator`). The configurator automatically assigns IP
addresses and populates static unicast routes. PIM needs these unicast routes for
*Reverse Path Forwarding (RPF) checks* — when a multicast packet arrives at a
router, PIM verifies that it arrived on the interface that the unicast routing
table considers the shortest path back towards the source (or the RP, for shared
trees). This check prevents forwarding loops and ensures packets travel along the
correct distribution tree. All routers are :ned:`MulticastRouter` nodes —
:ned:`Router` nodes with PIM enabled (``hasPim = true``) and multicast forwarding
turned on by default. Every router along the path must run PIM so it can
process Join and Prune messages and build its own multicast forwarding state
(i.e., learn which interfaces to forward each group's traffic to).

.. note::

   The INET PIM-SM implementation has some limitations compared to the full
   RFC 4601 specification: only a single global RP is supported (configured
   statically), switchover to the Shortest Path Tree (SPT) is not implemented,
   and PIM Bootstrap / RP discovery mechanisms are not available.

PIM configuration
~~~~~~~~~~~~~~~~~

PIM is configured on router interfaces using the ``pimConfig`` parameter. This is
an XML string that specifies which mode to use:

.. code-block:: ini

   # For Dense Mode:
   **.pimConfig = xml("<config><interface mode='dense'/></config>")

   # For Sparse Mode:
   **.pimConfig = xml("<config><interface mode='sparse'/></config>")

For PIM-SM, the Rendezvous Point must also be configured by setting the ``RP``
parameter of the :ned:`PimSm` module to an IP address belonging to the designated
RP router. The RP router identifies itself by checking whether the configured RP
address is local to one of its interfaces. A convenient way to assign a
well-known address is to configure the RP router's loopback interface in the
:ned:`Ipv4NetworkConfigurator` XML — the configurator will then generate routes
to it from all other routers:

.. code-block:: ini

   **.configurator.config = xml("<config>..." \
       "<interface hosts='R1' names='lo0' address='10.99.0.1' netmask='255.255.255.255'/>" \
       "...</config>")
   **.pim.pimSM.RP = "10.99.0.1"

PIM-DM Configuration
~~~~~~~~~~~~~~~~~~~~~

The ``PimDm`` configuration uses the ``PimShowcaseNetwork`` with PIM operating
in Dense Mode. ``receiver1`` joins the multicast group at 5s, while ``receiver2``
joins later at 50s. The source begins sending at 10s.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PimDm]
   :end-before: [Config PimSm]

This configuration demonstrates all three key PIM-DM behaviors:

1. **Flood**: When the source starts sending at 10s, multicast packets are
   flooded to all downstream router interfaces. R0 forwards to R1, and R1 floods
   to both R2 and R3.

2. **Prune**: Since ``receiver2`` has not yet joined the group, R3 has no
   downstream members. R3 sends a Prune message upstream to R1, which stops
   forwarding traffic towards R3. Only ``receiver1`` (via R2) receives the stream.

3. **Graft**: At 50s, ``receiver2`` joins the multicast group. R3 detects the
   new local member (via IGMP) and sends a Graft message to R1. R1 resumes
   forwarding to R3, and ``receiver2`` begins receiving the stream immediately.

PIM-SM Configuration
~~~~~~~~~~~~~~~~~~~~~

The ``PimSm`` configuration also uses ``PimShowcaseNetwork``, but with PIM in
Sparse Mode. Router ``R1`` is configured as the Rendezvous Point.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PimSm]
   :end-before: [Config Iptv]

In PIM-SM, no traffic flows until a receiver explicitly joins. The sequence of
events is:

1. At 5s, ``receiver1`` joins the multicast group. Its directly connected router
   R2 (acting as the *designated router* for that LAN — the router responsible
   for forwarding multicast to/from the segment) sends a (*,G) Join message
   towards the RP (R1). This builds the shared tree branch R1→R2.

2. At 10s, the source starts sending. Its designated router R0 receives the
   multicast packets but has no direct path to receivers. R0 encapsulates each
   packet in a PIM *Register* message and unicasts it to the RP (R1). R1
   decapsulates the packet and forwards it down the shared tree to ``receiver1``.

3. Simultaneously, R1 sends an (S,G) Join towards the source (to R0) to
   establish native multicast forwarding on the path source→R0→R1. Once this
   path is active, R1 sends a *Register-Stop* to R0, which stops the
   encapsulation overhead.

4. At 30s, ``receiver2`` joins. Its designated router R3 sends a (*,G) Join to
   R1, extending the shared tree to include the R1→R3 branch.

IPTV Configuration
~~~~~~~~~~~~~~~~~~

The ``Iptv`` configuration uses ``PimIptvNetwork``, a 3×3 grid of routers with an
IPTV server at one corner and a client at the opposite corner.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Iptv]

The :ned:`Ipv4NetworkConfigurator` automatically populates the unicast routing
tables, which PIM then uses for RPF checks to determine the shortest path
towards the RP and to build the multicast distribution tree.

The IPTV server sends high-rate multicast traffic (one 1000-byte packet every
40ms, i.e. 25 packets/s) starting at 25s. The client joins the multicast group
at 30s. At that point, R22 (the client's designated router) sends a (*,G) Join
towards the RP (R10). The join propagates through the grid, and the multicast
tree is established along the shortest path from the server's corner (R00)
through the RP (R10) to the client's corner (R22). Once the tree is built,
the client begins receiving the stream.

Results
-------

After running each configuration, the simulation produces result files that can
be analyzed in the OMNeT++ IDE. Key observations:

- **PIM-DM**: ``receiver1`` receives all 80 packets sent between 10–90s.
  ``receiver2`` joins at 50s and receives approximately 40 packets (from 50s
  onward). Between 10–50s, the R3 branch is in the pruned state — no multicast
  traffic is forwarded towards ``receiver2``. The Graft message from R3 at 50s
  restores the flow.

- **PIM-SM**: ``receiver1`` gets traffic from 10s onward (it joined at 5s,
  source starts at 10s) and receives all 80 packets. ``receiver2`` starts
  receiving at 30s when it joins the group, receiving 60 packets. PIM Register
  messages from R0 to R1 can be observed in the event log at 10s, followed by a
  Register-Stop once the (S,G) state is established.

- **IPTV**: The IPTV server sends 750 packets total (25 packets/s for 30s). The
  client joins at 30s — 5s after the source starts at 25s — and receives 625
  packets. The 125 missing packets correspond exactly to the 5s window between
  the source starting and the client's group membership being established (the
  client simply had not joined yet during that interval).

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PimShowcase.ned <../PimShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/routing/pim2`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/routing/pim2 && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__
on GitHub for commenting on this showcase.
