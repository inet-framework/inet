Multicast Routing with PIM
==========================

Goals
-----

IP multicast enables efficient one-to-many data delivery by sending a single
copy of each packet that is replicated only where paths diverge. Protocol
Independent Multicast (PIM) is the standard family of multicast routing
protocols used to build distribution trees for multicast traffic across IP
networks. It is called "protocol independent" because it does not include its
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
pattern:

- **PIM Dense Mode (PIM-DM)** assumes that multicast group members are densely
  distributed throughout the network. It uses a *flood-and-prune* strategy:
  multicast traffic is initially flooded to all parts of the network, and
  branches that have no interested receivers send Prune messages back upstream
  to stop receiving the traffic. If a previously pruned host wants to rejoin,
  its router can send a *Graft* message for immediate re-attachment without
  waiting for the prune timer to expire. PIM-DM also supports *State Refresh*
  messages to maintain prune state without periodic re-flooding. PIM-DM is
  defined in RFC 3973.

- **PIM Sparse Mode (PIM-SM)** assumes receivers are sparsely distributed and
  uses an explicit join model. A special router called the *Rendezvous Point
  (RP)* serves as the root of a shared multicast distribution tree. Receivers
  send Join messages towards the RP to receive traffic. When a source starts
  sending, its first-hop router encapsulates multicast packets in PIM *Register*
  messages and unicasts them to the RP. The RP decapsulates these packets and
  forwards them down the shared tree, while simultaneously sending an (S,G) Join
  towards the source to establish native multicast forwarding. Once native
  forwarding is established, the RP sends a *Register-Stop* to the source's
  first-hop router. PIM-SM is defined in RFC 4601.

The Model
---------

We use two network topologies in this showcase. The first, ``PimShowcaseNetwork``,
is a simple tree topology used for both the PIM-DM and PIM-SM configurations.
A :ned:`StandardHost` (``source``) sends UDP multicast traffic to the group
address ``239.1.1.1`` through three :ned:`MulticastRouter` nodes (R1–R3) to two
receivers. The receivers run :ned:`UdpSink` applications configured to join this
multicast group.

.. figure:: media/PimShowcaseNetwork.png

The second network, ``PimIptvNetwork``, models a more realistic scenario. Nine
:ned:`MulticastRouter` nodes are arranged in a 3×3 grid, with an IPTV server
(:ned:`StandardHost`) attached at one corner and a client at the opposite corner.

.. figure:: media/PimIptvNetwork.png

Both networks extend :ned:`WiredNetworkBase` (which inherits an
:ned:`Ipv4NetworkConfigurator` from :ned:`NetworkBase`). The configurator assigns
IP addresses and populates static unicast routes, which PIM uses for Reverse
Path Forwarding (RPF) checks. All routers are :ned:`MulticastRouter` nodes —
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
parameter of the :ned:`PimSm` module to an IP address that is assigned to one of
the RP router's interfaces. The RP router identifies itself by checking whether
the configured RP address is local to one of its interfaces. A convenient way to
achieve this is to set the router's ``routerId`` to the desired RP address — this
causes the address to be assigned to the router's loopback interface:

.. code-block:: ini

   **.R1.**.routerId = "10.0.0.1"
   **.pim.pimSM.RP = "10.0.0.1"

PIM-DM Configuration
~~~~~~~~~~~~~~~~~~~~~

The ``PimDm`` configuration uses the ``PimShowcaseNetwork`` with PIM operating
in Dense Mode. Both receivers join the multicast group at simulation time 5s,
and the source begins sending at 10s.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PimDm]
   :end-before: [Config PimSm]

Since PIM-DM uses flood-and-prune, the multicast traffic is initially forwarded
to all router interfaces. Routers connected to networks with no group members
send Prune messages upstream. In this configuration, both receivers are present,
so no branches are pruned and the traffic flows through all three routers to both
receivers. PIM Hello messages are exchanged periodically between all neighboring
routers to maintain adjacency.

PIM-SM Configuration
~~~~~~~~~~~~~~~~~~~~~

The ``PimSm`` configuration also uses ``PimShowcaseNetwork``, but with PIM in
Sparse Mode. Router ``R1`` is configured as the Rendezvous Point.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config PimSm]
   :end-before: [Config Iptv]

In PIM-SM, no traffic flows until a receiver explicitly joins. When ``receiver1``
starts at 5s, its designated router (R2) sends a (*,G) Join message towards the
RP (R1). When the source starts sending at 10s, its first-hop router (also R1 in
this topology) receives the multicast packets and — since R1 is itself the RP —
immediately forwards them down the shared tree to ``receiver1``.

When ``receiver2`` joins at 30s, its designated router (R3) also sends a Join to
the RP, extending the shared tree to include that branch as well.

IPTV Configuration
~~~~~~~~~~~~~~~~~~

The ``Iptv`` configuration uses ``PimIptvNetwork``, a 3×3 grid of routers with an
IPTV server at one corner and a client at the opposite corner.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Iptv]

The :ned:`Ipv4NetworkConfigurator` automatically populates the unicast routing
tables, which PIM then uses to determine the shortest path towards the RP and to
build the multicast distribution tree.

The IPTV server sends high-rate multicast traffic (one 1000-byte packet every
40ms, simulating a ~200 kbps video stream) starting at 25s. The client joins
the multicast group at 30s. At that point, the client's designated router sends
a (*,G) Join towards the RP (R10), and once the shared tree is established, the
client begins receiving the stream.

Results
-------

After running each configuration, the simulation produces result files that can
be analyzed in the OMNeT++ IDE. Key observations:

- **PIM-DM**: Multicast traffic reaches both receivers immediately after the
  source starts sending at 10s, as PIM-DM floods traffic to all branches by
  default. Both receivers receive all 80 packets sent during the simulation.
  PIM Hello messages are exchanged periodically between all neighboring routers.

- **PIM-SM**: Traffic only flows to a receiver after it has joined the group.
  ``receiver1`` gets traffic from 10s onward (it joined at 5s, source starts
  at 10s) and receives all 80 packets. ``receiver2`` starts receiving after 30s
  when it joins the group, receiving 60 packets. PIM Register, Join/Prune, and
  Hello messages can be observed in the logs.

- **IPTV**: The IPTV server starts streaming at 25s (750 packets total at 25
  packets/s). The client joins at 30s and receives 625 packets — missing those
  sent during the 5s before its Join propagated through the network. The
  multicast stream traverses the grid of routers from the server's corner to the
  client's corner via the shared tree rooted at R10.

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
