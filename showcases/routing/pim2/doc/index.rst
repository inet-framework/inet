Multicast Routing with PIM
==========================

Goals
-----

IP multicast enables efficient one-to-many data delivery by sending a single
copy of each packet that is replicated only where paths diverge. Each link in
the tree carries only one copy of each packet. On the local network segment,
the last-hop router sends the packet as a multicast frame — all interested
hosts receive it without individual copies. Hosts signal their interest to the
router using IGMP (Internet Group Management Protocol).

Protocol Independent Multicast (PIM) is the standard family of multicast
routing protocols used to build distribution trees for multicast traffic across
IP networks. It is called "protocol independent" because it does not include its
own topology discovery mechanism — instead it relies on the unicast routing table
(populated by any unicast routing protocol or static configuration).

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
  upstream to stop receiving the traffic.
- If a previously pruned host later wants to rejoin, its router sends a
  **Graft** message for immediate re-attachment — without waiting for the prune
  timer to expire.

PIM-DM is defined in RFC 3973.

**PIM Sparse Mode (PIM-SM)** assumes receivers are sparsely distributed and
uses an explicit join model. A special router called the *Rendezvous Point
(RP)* serves as the root of a shared multicast distribution tree. All routers
in the PIM domain must know the RP's address — in real networks this is
typically learned via the PIM Bootstrap mechanism, but in INET it is configured
statically on every router.

The basic operation works as follows:

- Receivers send **Join** messages towards the RP to subscribe to a group.
  Traffic only flows along branches that have been explicitly requested.
- When a source starts sending, its first-hop router (the *designated router*
  or DR) encapsulates multicast packets in **Register** messages and unicasts
  them to the RP.
- The RP decapsulates these packets and forwards them down the shared tree. At
  the same time, it sends an (S,G) Join towards the source to establish native
  multicast forwarding.
- Once native forwarding is established, the RP sends a **Register-Stop** to
  the DR, ending the encapsulation overhead.

PIM-SM is defined in RFC 4601.

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
turned on by default.

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
