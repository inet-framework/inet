IPv6-in-IPv6 Tunneling
======================

Goals
-----

A network can only forward a packet if it has a route for the destination address.
Some addresses are, by design, ones that no public network will ever carry. A site
that numbers its internal networks from such a range cannot simply send those packets
across the Internet to another site.

Tunneling solves this by hiding the packet. The site's border router wraps the whole
packet inside a second IPv6 header that uses addresses the public network *does*
carry. The network forwards the outer packet normally and never inspects what is
inside. The router at the far end removes the outer header and delivers the original
packet into the second site.

This showcase demonstrates IPv6-in-IPv6 tunneling as defined in RFC 2473. Two sites
use addresses the network between them cannot route, and a tunnel between the site
border routers carries their traffic across. The showcase then varies one thing at a
time: which traffic the tunnel carries, and what happens when the routing that feeds
the tunnel is wrong.

One point of scope, because the term is ambiguous. "IPv6 tunneling" often means
carrying IPv6 across an IPv4-only network — the transition mechanisms 6in4, 6to4, 6rd
and Teredo. This page is not about those, and INET does not model them. This page is
about carrying IPv6 inside IPv6, which is a different mechanism solving a different
problem.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/ipv6/tunneling <https://github.com/inet-framework/inet/tree/master/showcases/ipv6/tunneling>`__

About IPv6-in-IPv6 tunneling
----------------------------

A tunnel puts one packet inside another. The node that adds the outer header is the
*tunnel entry point*, and the node that removes it is the *tunnel exit point*. The
original packet is the *inner packet*; the packet that actually travels is the *outer
packet*.

The outer header is an ordinary IPv6 header. Its source address is the entry point,
its destination address is the exit point, and its Next Header field says ``IPv6``.
That last field is the whole trick: it tells the exit point that the payload is
another IPv6 datagram rather than a transport protocol. Every router between the two
endpoints treats the outer packet as normal traffic addressed to the exit point, and
never looks at the inner packet at all.

Nothing about this is specific to IPv6-in-IPv6. Putting a packet inside another packet
and routing the outer one is the mechanism underneath every overlay network in use
today. IPv6-in-IPv6 is that mechanism in its simplest form, because the outer header
is the same IPv6 header the reader already knows, with nothing added.

Addresses the network will not carry
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IPv6 reserves a range called Unique Local Addresses, ``fc00::/7``, defined in
RFC 4193. In practice organisations use the ``fd00::/8`` half of it. Unique Local
Addresses are the rough equivalent of the private address ranges in IPv4: they are
meant for traffic inside an organisation, and they never appear in the global routing
table.

They are not merely discouraged from the global routing table — they cannot be in it.
An organisation creates its own Unique Local Address prefix by choosing 40 random
bits. There is no registry and no allocation authority, so no record exists of which
organisation holds which prefix, and two organisations may choose the same one. A
router receiving a route for such a prefix would have no way to decide whose network
it leads to. Public networks therefore discard these addresses at their borders.

So an organisation with two sites, both numbered from Unique Local Addresses, has a
concrete problem. Each site works internally. Neither site can reach the other across
a public network, because that network will not carry the addresses. A tunnel between
the two border routers solves it: the inner packet keeps the Unique Local Addresses,
and the outer packet uses the border routers' public addresses.

A tunnel is not a Virtual Private Network
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The arrangement in this showcase looks like a Virtual Private Network, and the shape
is indeed the same. The difference matters: a plain IPv6-in-IPv6 tunnel gives
*reachability*, not *confidentiality*. Nothing in it is encrypted or authenticated.
Anyone who can observe the transit network can read every inner packet, and anyone who
can inject traffic toward the exit point can inject inner packets. In a real
deployment the encapsulation is what a security protocol such as IPsec is layered on
top of; the tunnel by itself carries traffic, it does not protect it.

IPv6-in-IPv6 tunneling in INET
------------------------------

In INET a tunnel is not a protocol module. It is a *virtual network interface* called
``Ipv6TunnelInterface``. A packet routed to that interface is wrapped in an outer IPv6
header and handed back to IPv6, which forwards the resulting datagram toward the exit
point like any other locally originated packet. At the far end, IPv6 sees a datagram
addressed to itself whose Next Header field says ``IPv6``, removes the outer header,
and processes the inner packet as if it had just arrived from the network.

This leads to the single most useful idea on this page:

    A tunnel is one network interface plus one route.

The interface says *where the tunnel goes*. The route says *what goes into it*. The
two are configured independently, and changing only the route changes what the tunnel
is used for, without touching the tunnel itself.

Configuring the interface
~~~~~~~~~~~~~~~~~~~~~~~~~

Host and router node types have a ``tun`` submodule vector for tunnel interfaces.
Setting ``numTunInterfaces`` creates the slots, and each slot's type and parameters
are set individually:

.. literalinclude:: ../omnetpp.ini
   :start-at: *.borderA.numTunInterfaces
   :end-at: *.borderA.tun[0].destination
   :language: ini

The interface takes three parameters:

- ``source`` — the tunnel entry point, which must be an address of this node. It
  becomes the source address of the outer header.
- ``destination`` — the tunnel exit point. It becomes the destination address of the
  outer header.
- ``mtu`` — the largest packet the tunnel will accept, 1500 bytes by default.

The interface is created with the name ``tun0``, which is how routes refer to it.

Each end is configured separately, and each end describes only its own direction. The
tunnel in this showcase is used in both directions, so both border routers declare an
interface, with the ``source`` and ``destination`` values swapped. Configuring only
one end would give a tunnel that carries traffic one way and nothing back.

Steering traffic into the tunnel
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The tunnel interface has no idea which traffic it is meant to carry. It wraps whatever
arrives and sends it to its far end. What decides which traffic arrives is an ordinary
route whose output interface is the tunnel:

.. literalinclude:: ../tunnel.xml
   :start-at: <route hosts="borderA" destination="fd00:b::/64"
   :end-at: <route hosts="borderB" destination="fd00:a::/64"
   :language: xml

Because the route is ordinary, the usual longest-prefix rule applies, and the *shape*
of the route decides what the tunnel is for. A route for a single address sends one
host's traffic through it. A route for a prefix sends a whole remote site. A default
route sends everything. This showcase uses the first two.

The cost of the outer header
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The outer header is 40 bytes, and those bytes are added to every packet the tunnel
carries. A packet that exactly filled the outgoing link before encapsulation no longer
fits after it, and the entry point must then split it into fragments that the exit
point reassembles. Setting the tunnel's ``mtu`` to the link's limit minus 40 avoids
this, by refusing oversized packets instead of splitting them. This showcase keeps the
default and uses small packets, so no fragmentation occurs.

The Model
---------

All four configurations use the same network:

.. figure:: media/network.png
   :align: center

.. FIGURE RECIPE: launch `inet -u Qtenv -c NoTunnel --mcp-server-address localhost:8799`,
   then over the MCP server: run_simulation time_limit 1.9s mode express (so addresses are
   assigned and no packet is in flight), then get_canvas_image with module_path "<root>",
   area "module_rectangle", margin 5. NoTunnel is used deliberately: in the tunnel configs
   the tunnel interface adds an "fe80::" label that means nothing to a reader.

``hostA`` is in site A and ``hostB1`` and ``hostB2`` are in site B. ``borderA`` and
``borderB`` are the two site border routers, and ``transit`` is a router in the public
network between them. ``switchB`` is an Ethernet switch that puts both site B hosts on
one link. The hosts and routers are ``StandardHost6`` and ``Router6``, the IPv6
variants of INET's generic host and router.

The addresses in the figure are the point of the network:

.. list-table::
   :header-rows: 1

   * - Link
     - Prefix
     - Kind
   * - ``hostA`` – ``borderA`` (site A)
     - ``fd00:a::/64``
     - Unique Local Address
   * - ``borderA`` – ``transit``
     - ``2001:db8:1::/64``
     - public
   * - ``transit`` – ``borderB``
     - ``2001:db8:2::/64``
     - public
   * - ``borderB`` – site B hosts
     - ``fd00:b::/64``
     - Unique Local Address

Both sites use Unique Local Addresses. Both border routers also have a public address
on the link toward ``transit``, and those two public addresses are the tunnel
endpoints.

Routing is configured by hand rather than computed, using
``*.configurator.addStaticRoutes = false`` and an explicit route for every node. This
is more verbose than letting the configurator compute shortest paths, but it makes the
premise of the showcase visible instead of merely asserted. The transit router's
routing table is this, and nothing else:

.. code-block:: none

    transit:
      2001:db8:1::/64 via <unspec> dev eth0
      2001:db8:2::/64 via <unspec> dev eth1

Two entries, both for public prefixes. There is no route for ``fd00::/8``, so any
packet carrying those addresses that reaches ``transit`` is discarded. In a real
network the absence is not a configuration choice — it is unavoidable, for the reasons
given above. Here it has to be arranged deliberately, because a shortest-path
configurator would happily install routes that reality would not.

In every configuration, ``hostA`` sends 100-byte UDP packets to both site B hosts,
twice a second, from 2 s to the end of the 10 s simulation. That is 16 packets to each
host. The two receiving hosts run ``UdpSink``.

NoTunnel Configuration
~~~~~~~~~~~~~~~~~~~~~~

The ``NoTunnel`` configuration is the baseline. There is no tunnel, and the two sites
are left to reach each other directly:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config NoTunnel]
   :end-before: [Config Tunnel]
   :language: ini

``hostA`` sends its packets to ``borderA``. ``borderA`` has no route for
``fd00:b::/64``, so it forwards them along its default route toward ``transit``, the
way a site router hands anything it does not recognise to its provider. ``transit``
has no route for those addresses and discards them.

``transit`` does try to report the failure, and cannot. The ICMPv6 Destination
Unreachable message it generates is addressed to ``hostA``, whose address
``fd00:a::a`` is a Unique Local Address as well — so the error is discarded for exactly
the same reason as the packet that caused it. ``hostA`` learns nothing. Both sites are
simply invisible to the network between them.

Tunnel Configuration
~~~~~~~~~~~~~~~~~~~~

The ``Tunnel`` configuration adds a tunnel interface to each border router and a route
for the whole remote site pointing into it:

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config Tunnel]
   :end-before: [Config SelectiveTunnel]
   :language: ini

The tunnel endpoints are the two public addresses, ``2001:db8:1::1`` and
``2001:db8:2::2``. The routes that feed the tunnel are the two shown earlier: on
``borderA``, everything for ``fd00:b::/64``; on ``borderB``, everything for
``fd00:a::/64``.

SelectiveTunnel Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``SelectiveTunnel`` configuration keeps exactly the same tunnel and changes one
route. Instead of a prefix route for the whole of site B, ``borderA`` gets a route for
one host address:

.. literalinclude:: ../selective.xml
   :start-at: <route hosts="borderA" destination="fd00:b::b1"
   :end-at: <route hosts="borderA" destination="fd00:b::b1"
   :language: xml

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config SelectiveTunnel]
   :end-before: [Config RecursiveRouting]
   :language: ini

Traffic to ``hostB1`` now matches that route and enters the tunnel. Traffic to
``hostB2`` matches nothing that leads into the tunnel, so it takes the ordinary path
and dies at ``transit`` exactly as in the baseline.

RecursiveRouting Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because the tunnel is fed by ordinary routes, an ordinary routing mistake can feed it
the wrong traffic. The ``RecursiveRouting`` configuration adds one wrong route on
``borderB``: a host route for ``hostB1``, a host that is directly attached to
``borderB``, pointing into the tunnel back to ``borderA``:

.. literalinclude:: ../recursive.xml
   :start-at: <route hosts="borderB" destination="fd00:b::b1"
   :end-at: <route hosts="borderB" destination="fd00:b::b1"
   :language: xml

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config RecursiveRouting]
   :language: ini

A packet for ``hostB1`` now travels to ``borderB`` through the tunnel, and ``borderB``
sends it straight back through the tunnel to ``borderA``, which sends it forward
again. The packet bounces between the two border routers.

What stops it is the hop limit. Each border router forwards the inner packet, and
forwarding decrements its hop limit. INET starts an IPv6 packet with a hop limit of
30, and each round trip costs two decrements, so a packet survives about fifteen round
trips before it is discarded. The loop is self-limiting, which is why this
configuration terminates rather than running forever.

Real routers detect this situation, which is called *recursive routing*, and shut the
tunnel down rather than let it consume the link. INET has no such protection.

Results
-------

The first question is whether the traffic arrives. This is the number of packets each
site B host received, out of 16 sent to each:

.. list-table::
   :header-rows: 1

   * - Configuration
     - ``hostB1``
     - ``hostB2``
   * - ``NoTunnel``
     - 0
     - 0
   * - ``Tunnel``
     - 16
     - 16
   * - ``SelectiveTunnel``
     - 16
     - 0
   * - ``RecursiveRouting``
     - 0
     - 16

The baseline delivers nothing, which confirms that the transit network really cannot
carry these addresses and that the tunnel is doing the work in the other
configurations.

``SelectiveTunnel`` delivers to one host and not the other. Both hosts are on the same
link in the same site, and both are reached through the same tunnel — the only
difference is that one of them is named by a route and the other is not. This is the
clearest demonstration on the page that the tunnel carries whatever the routing gives
it, and nothing else.

``RecursiveRouting`` inverts the pattern: the host with the wrong route is the one that
gets nothing, while ``hostB2``, whose routing is untouched, keeps receiving all 16
packets. A tunnel does not "break"; one route breaks one destination.

What an encapsulated packet looks like
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following is an Ethernet frame captured on the link between ``borderA`` and
``transit`` in the ``Tunnel`` configuration, shown as the chunk list of INET's packet
inspector:

.. figure:: media/encapsulated-packet.png
   :align: center

.. FIGURE RECIPE: launch `inet -u Qtenv -c Tunnel --mcp-server-address localhost:8799`,
   then over the MCP server: run_simulation to 2.6s in "fast" mode, list_logged_packets with
   module_path "borderA" and pick a 214-byte EthernetSignal whose hop_modules run borderA ->
   transit, open_inspector on its "logged:<treeId>" path with type "object",
   expand_inspector_tree depth 4, then get_inspector_screenshot width 1500 height 3800 and
   crop the "chunks[7]" block (it sits under the "dissection" node, below the raw bin/raw hex
   listings). Depth 3 leaves the chunk list collapsed; depth 4 also expands the raw bit dump,
   which is why the screenshot must be tall and then cropped.

The two middle entries are the point. Chunk ``[2]`` is the outer IPv6 header: it runs
from ``2001:db8:1::1`` to ``2001:db8:2::2``, the two public tunnel endpoints, and its
protocol field says ``ipv6``, meaning the payload is another IPv6 datagram. Chunk
``[3]`` is the inner header, still carrying the original ``fd00:a::a`` and
``fd00:b::b2`` addresses, with its protocol field naming ``udp``.

The transit network forwards this frame using the addresses in chunk ``[2]``, which it
has routes for. The addresses in chunk ``[3]``, which it has no routes for, are just
payload bytes to it.

The other entries are the ordinary framing around the packet. ``[0]`` and ``[1]`` are
the Ethernet physical and MAC headers, ``[4]`` is the UDP header, ``[5]`` is the
application's own data, and ``[6]`` is the Ethernet frame check sequence.

The cost of the loop
~~~~~~~~~~~~~~~~~~~~

Delivery counts do not show the whole effect of ``RecursiveRouting``. Counting the
packets that crossed the link between ``transit`` and ``borderA`` does:

.. list-table::
   :header-rows: 1

   * - Configuration
     - packets sent by ``transit`` toward ``borderA``
   * - ``Tunnel``
     - 4
   * - ``RecursiveRouting``
     - 244

Sixteen packets, each making about fifteen round trips before its hop limit runs out,
put roughly 240 extra packets onto the transit link — traffic that is never delivered
to anyone. This is why recursive routing matters in practice. The visible symptom is
that one destination is unreachable; the expensive consequence is that the link
between the sites carries many times its normal load to accomplish nothing.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Ipv6TunnelingShowcase.ned <../Ipv6TunnelingShowcase.ned>`,
:download:`no-tunnel.xml <../no-tunnel.xml>`,
:download:`tunnel.xml <../tunnel.xml>`,
:download:`selective.xml <../selective.xml>`,
:download:`recursive.xml <../recursive.xml>`

Try It Yourself
---------------

If you already have INET installed, write the following into the command line from the
INET root directory:

.. code-block:: bash

    $ cd showcases/ipv6/tunneling
    $ inet

Otherwise, there is an easy way to install INET and OMNeT++ using ``opp_env``, and run
the simulation interactively.

Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.7 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.7.*/showcases/ipv6/tunneling && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.7
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

.. TODO: replace the issue-tracker link below once the showcase discussion issue is opened

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues>`__ in
the GitHub issue tracker for commenting on this showcase.
