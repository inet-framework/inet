Path MTU Discovery
==================

Goals
-----

Every link has a limit on how large a packet it will carry, its Maximum Transmission
Unit (MTU). A path made of several links can carry no more than its smallest link, and
the sender has no way of knowing that number in advance.

IPv6 handles this differently from IPv4, and the difference is strict. A router may
not split a packet it is forwarding. Only the original sender may split a packet.
So when a packet is too large for the next link, the router has no way to deliver it:
it discards the packet and sends an ICMPv6 Packet Too Big message back to the sender,
naming the size that would have fitted. The sender remembers that number and sends
smaller packets from then on. This exchange is Path MTU Discovery, defined in RFC 8201.

The whole scheme rests on one message travelling backwards along the path. Everything
else in IPv6 forwarding only needs packets to travel forwards. That asymmetry is what
makes Path MTU Discovery fragile, and it is why the failure it produces — traffic that
vanishes silently while small packets work perfectly — is a familiar one.

This showcase creates a path whose limit is lower than the sender's own link, and then
shows four outcomes: paying for fragmentation, losing everything when the message is
filtered, recovering when the message gets through, and the quiet case where the
sender was sized correctly from the start.

A tunnel is used to make the path narrower, because that is the most common way a path
ends up narrower than its links. What the tunnel is for does not matter here; the
:doc:`../../tunneling/doc/index` showcase covers that.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/ipv6/pathmtudiscovery <https://github.com/inet-framework/inet/tree/master/showcases/ipv6/pathmtudiscovery>`__

About Path MTU Discovery
------------------------

Why a tunnel narrows the path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A tunnel wraps each packet in a second IPv6 header before sending it on. That header
is 40 bytes. A packet that exactly filled a 1500-byte link before it was wrapped
becomes 1540 bytes afterwards, and no longer fits.

The node that does the wrapping has two roles at once, and the rules for them differ:

- For the **inner** packet it is a *router*. It is forwarding somebody else's packet,
  so it may not split it. If the inner packet is larger than the tunnel will accept,
  the packet is discarded and a Packet Too Big message goes back to the sender.
- For the **outer** packet it is the *sender*. It built that packet itself, so it may
  split it. If the outer packet is too large for the link, it is fragmented and the
  far end of the tunnel puts the pieces back together.

Which of the two limits is exceeded first therefore decides whether an oversized
packet is fragmented or refused. Both behaviours appear in this showcase.

What goes wrong
~~~~~~~~~~~~~~~

Path MTU Discovery works when the Packet Too Big message reaches the sender. The most
common reason it does not is that a firewall discards it. Administrators often block
ICMP as a matter of habit, which in IPv4 mostly broke diagnostic tools. In IPv6 it
breaks much more, because IPv6 depends on ICMPv6 for Neighbor Discovery, Router
Discovery and Path MTU Discovery alike. The practice is common enough that RFC 4890
was written to tell firewall administrators which ICMPv6 messages they must not
filter; Packet Too Big is on that list.

When the message does not arrive, the sender never learns and keeps sending the same
size. Every one of those packets is discarded. Small packets continue to work, so the
network looks healthy while large transfers hang. This is called a Path MTU Discovery
black hole.

Because the mechanism cannot be relied on, network operators fall back on a cruder
one: they configure tunnel routers to rewrite the Maximum Segment Size option inside
passing TCP handshakes, so the two endpoints agree on a smaller segment and never need
the feedback at all. That technique only helps TCP. UDP and QUIC get nothing from it.

Path MTU Discovery in INET
--------------------------

The tunnel's limit
~~~~~~~~~~~~~~~~~~

``Ipv6TunnelInterface`` has an ``mtu`` parameter that sets the largest inner packet the
tunnel will accept. It defaults to 1500 bytes, the same as an Ethernet link, which is
the value that guarantees fragmentation: the outer header always pushes an inner
packet of that size 40 bytes over the link's limit.

The value that avoids fragmentation is the link's limit minus the outer header:

.. code-block:: none

    1500 (Ethernet)  -  40 (outer IPv6 header)  =  1460

Set the tunnel to 1460 and the two checks line up. Any inner packet the tunnel accepts
still fits the link once wrapped, so nothing is ever fragmented. Anything larger is
refused at the first check and never reaches the second.

RFC 2473 expects a tunnel to derive this number from the path between its endpoints
and to keep it up to date as that path changes. INET's tunnel interface has a fixed
value that you set.

Discovering the path limit
~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``Ipv6`` module carries out Path MTU Discovery and has two parameters for it:

- ``pathMtuDiscovery`` — whether the node acts on incoming Packet Too Big messages.
  Enabled by default. Turning it off restores the behaviour of a node that never
  learns.
- ``pathMtuAgingTime`` — how long a learned value is kept before the node tries a
  larger size again, ten minutes by default. A path can widen, and nothing would tell
  the sender if it never retried.

A learned value is never raised by an incoming message, only lowered, and it is never
taken below 1280 bytes, which RFC 8201 fixes as the smallest MTU any IPv6 link must
support.

What this means for an application: the sending program is not involved and does not
change. It keeps writing the same amount of data. The IPv6 layer underneath it splits
each write to fit what it has learned. A TCP connection would instead reduce its
segment size and avoid splitting anything, but this showcase uses UDP, which has no
equivalent, so the source has to fragment.

Filtering the message
~~~~~~~~~~~~~~~~~~~~~

To show what happens when the Packet Too Big message never arrives, one node discards
it. INET has no firewall module, but it does have a Security Policy Database, which is
a packet filter by definition — it matches traffic against selectors and applies one
of three verdicts, one of which is to discard. Enabling it on a node and giving it a
policy is enough:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: *.firewall.ipv6.hasIpsec
   :end-at: *.firewall.ipv6.ipsec.spdConfig
   :language: ini

.. literalinclude:: ../firewall.xml
   :caption: firewall.xml
   :language: xml

Three things about that policy are worth knowing before adapting it.

The first entry discards ICMPv6 — protocol number 58 — travelling from anywhere beyond
the firewall towards ``hostA``'s network. The selector matches on addresses rather
than on the message type, because the type selector only works for IPv4's ICMP. So
this filter drops *all* ICMPv6 between those two address ranges, not only Packet Too
Big. That is realistic: firewalls that cause this problem block ICMP broadly.

The two ``BYPASS`` entries are not optional. When no policy matches, the default is to
discard, so a policy file with only the first entry would silence the node completely.

The address ranges are chosen so that neither side can be one of the firewall's own
addresses. Neighbor Discovery sends its unicast messages from a node's global address,
so a filter written in terms of address *scope* would discard the firewall's own
Neighbor Advertisements and break the network. Multicast Neighbor Discovery is never
filtered, so Router Advertisements and Duplicate Address Detection are unaffected
either way.

The Model
---------

.. figure:: media/network.png
   :align: center

   The network, with the address each interface holds. The ``configurator`` and
   ``visualizer`` modules have no part in the protocol: the first assigns addresses
   and routes, and the second draws the address labels.

.. FIGURE RECIPE: launch `inet -u Qtenv -c Fragmentation --mcp-server-address
   localhost:8799`, then over the MCP server: run_simulation time_limit 1.9s mode
   express, then get_canvas_image with module_path "<root>", area "module_rectangle",
   margin 5. The ini filters tun* out of the interface table labels, because a tunnel
   interface has no address of its own and would only add an "fe80::" label.

``hostA`` and ``hostB`` are the two endpoints. ``borderA`` and ``borderB`` are joined
by an IPv6-in-IPv6 tunnel, and ``transit`` is a router between them that carries only
the two addresses of the links it is attached to. ``firewall`` sits between ``hostA``
and ``borderA``; in three of the four configurations it does nothing at all.

The tunnel is not part of the topology — it is an interface on each border router plus
a route leading to it:

.. figure:: media/tunnel-overlay.png
   :align: center

   Where the tunnel runs. The dashed line is drawn on the figure; it is not a link in
   the model.

.. FIGURE RECIPE: derived from media/network.png with PIL — a quadratic Bezier from
   (505,218) through (695,88) to (893,218) in the 1194x344 image, 3px wide, colour
   (0,90,200), dashes of 9 samples out of 400, arrow heads at both ends, and a white
   label box centred at x=695, y=96. Redraw whenever network.png is recaptured.

Every link is Ethernet with the usual 1500-byte limit. Nothing in this network has a
narrow link. The only thing that narrows the path is the tunnel, and how much it
narrows it is what the configurations vary.

The addresses are from ``2001:db8::/32``, the range RFC 3849 reserves for
documentation. Routes are written out by hand rather than computed, so ``transit``
holds only the two prefixes it is attached to and the tunnel is genuinely needed.

The rest of the scenario is the same in every configuration:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: [General]
   :end-before: [Config Fragmentation]
   :language: ini

``hostA`` sends UDP packets to ``hostB`` twice a second from 2 s to 9.75 s, which is 16
packets, and ``hostB`` runs ``UdpSink``. Traffic starts at 2 s so that Neighbor
Discovery finishes first.

Only two things differ between the configurations: how much data the application
writes, and what the tunnel will accept.

.. list-table::
   :header-rows: 1

   * - Configuration
     - Application writes
     - Inner packet
     - Tunnel ``mtu``
     - ICMPv6
   * - ``Fragmentation``
     - 1452 B
     - 1500 B
     - 1500 B
     - delivered
   * - ``BlackHole``
     - 1452 B
     - 1500 B
     - 1460 B
     - filtered
   * - ``Discovery``
     - 1452 B
     - 1500 B
     - 1460 B
     - delivered
   * - ``SizedToFit``
     - 1412 B
     - 1460 B
     - 1460 B
     - delivered

The inner packet is the application's data plus 8 bytes of UDP header and 40 bytes of
IPv6 header, so 1452 bytes of data makes a 1500-byte packet.

Fragmentation Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The tunnel keeps its default limit, so it accepts the 1500-byte inner packet:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: [Config Fragmentation]
   :end-before: [Config BlackHole]
   :language: ini

Wrapping it produces a 1540-byte outer packet, which does not fit the link to
``transit``. ``borderA`` is the sender of that outer packet, so it is allowed to split
it, and does. ``borderB`` reassembles the pieces before unwrapping.

Everything arrives. The cost is paid on the wire and at ``borderB``, not by the
application.

BlackHole Configuration
~~~~~~~~~~~~~~~~~~~~~~~

The tunnel's limit is lowered to 1460 so that nothing is ever fragmented, which is the
correct thing to configure. The firewall discards ICMPv6 heading towards ``hostA``:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: [Config BlackHole]
   :end-before: [Config Discovery]
   :language: ini

Now the 1500-byte inner packet is larger than the tunnel accepts. ``borderA`` is
forwarding it, so it may not split it: it discards the packet and sends Packet Too Big
back towards ``hostA``. The firewall discards that message. ``hostA`` learns nothing,
sends the next packet at the same size, and the cycle repeats for the whole run.

Discovery Configuration
~~~~~~~~~~~~~~~~~~~~~~~

The same network and the same tunnel limit, with the firewall doing nothing:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: [Config Discovery]
   :end-before: [Config SizedToFit]
   :language: ini

The first packet is discarded exactly as before, but this time the Packet Too Big
message reaches ``hostA``, which records that the path to ``hostB`` accepts 1460
bytes. From the next packet onwards, ``hostA`` splits its own datagrams to fit, and
everything gets through.

One packet is lost — the one that taught the sender. That loss is the mechanism
working, not failing.

SizedToFit Configuration
~~~~~~~~~~~~~~~~~~~~~~~~

The application writes 1412 bytes instead of 1452, which makes a 1460-byte inner
packet:

.. literalinclude:: ../omnetpp.ini
   :caption: omnetpp.ini
   :start-at: [Config SizedToFit]
   :language: ini

The tunnel accepts it, wrapping it produces exactly 1500 bytes, and that fits the
link. Nothing is discarded, nothing is split, and nothing has to be discovered.

Results
-------

.. list-table::
   :header-rows: 1

   * - Configuration
     - Delivered, of 16
     - Packets ``hostA`` → ``firewall``
     - Packets ``borderA`` → ``transit``
   * - ``Fragmentation``
     - 16
     - 20
     - 36
   * - ``BlackHole``
     - 0
     - 21
     - 4
   * - ``Discovery``
     - 15
     - 36
     - 34
   * - ``SizedToFit``
     - 16
     - 20
     - 20

Four packets on each link are Neighbor Discovery rather than application traffic;
``SizedToFit`` shows that baseline plainly, at 16 data packets plus 4 on both links.

Read down the last two columns and the counts say where the fragmentation happened.

In ``SizedToFit`` the two numbers match: one packet leaves the sender and one packet
crosses the tunnel. In ``Fragmentation`` the sender's link carries 20 and the tunnel's
carries 36 — the extra sixteen appear only after ``borderA``, because that is where
the splitting happens. In ``Discovery`` the doubling has moved to the other end: 36 on
the sender's link and 34 beyond it, because ``hostA`` is now doing the splitting and
``borderA`` merely forwards what it is given.

That is the real effect of Path MTU Discovery here. It did not remove the
fragmentation, because the application still writes 1452 bytes and UDP has no way to
write less. What it changed is *who does the work*: a router in the forwarding path
before, the sending host afterwards. That is where IPv6 wants it, and it is why
routers are forbidden to fragment in the first place.

``SizedToFit`` is the only configuration with no fragmentation anywhere, and it is the
one an operator should aim for.

``BlackHole`` carries almost nothing on the tunnel link — 4 packets, all Neighbor
Discovery. No application data ever gets past ``borderA``. The sender's link still
carries its 16 packets, so from ``hostA``'s point of view everything is being sent
normally.

What a fragment looks like
~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the second of the two fragments ``borderA`` produces in the ``Fragmentation``
configuration, taken from the link to ``transit``:

.. figure:: media/fragment.png
   :align: center

.. FIGURE RECIPE: launch `inet -u Qtenv -c Fragmentation --mcp-server-address
   localhost:8799`, run_simulation to 2.6s in "fast" mode, list_logged_packets with
   module_path "borderA" and pick the 126-byte packet named "...-frag-1448-last" (the
   small last fragment; the first fragment is 1522 B and its raw bit dump makes the
   inspector tree far too tall to render). open_inspector on its "logged:<treeId>"
   path with type "object", expand_inspector_tree depth 4, get_inspector_screenshot
   width 1500 height 5000, then crop the "chunks[6]" block under the "dissection" node.

Chunk ``[2]`` is the outer header, addressed from ``2001:db8:3::1`` to
``2001:db8:4::2`` — the two tunnel endpoints. Chunk ``[3]`` is an IPv6 Fragment header,
which only appears on a packet that has been split. Its ``fragmentOffset`` of 1448
says this piece starts 1448 bytes into the original, and its ``nextHeaderProtocol`` of
41 says that what was split was an IPv6 datagram, which is what a tunnel carries.
Chunk ``[4]`` is the tail of the original packet, and ``[0]``, ``[1]`` and ``[5]`` are
the Ethernet framing around it.

The packet names in the simulation show the same thing more briefly. One
``UdpBasicAppData-1`` of 1526 bytes arrives at ``borderA``, and two packets leave it:
``UdpBasicAppData-1-frag-0`` of 1522 bytes and ``UdpBasicAppData-1-frag-1448-last`` of
126 bytes.

That second one is the point of the whole exercise. It carries 52 bytes of data in a
126-byte frame. Splitting a packet does not cost much in bytes — about 4% here — but it
doubles the number of packets, and a runt like this one costs a router as much to
handle as a full-size one.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`PathMtuDiscoveryShowcase.ned <../PathMtuDiscoveryShowcase.ned>`,
:download:`configurator.xml <../configurator.xml>`,
:download:`firewall.xml <../firewall.xml>`

Try It Yourself
---------------

If you already have INET installed, write the following into the command line from the
INET root directory:

.. code-block:: bash

    $ cd showcases/ipv6/pathmtudiscovery
    $ inet

``inet`` with no arguments offers a list of the four configurations to choose from.

Otherwise, there is an easy way to install INET and OMNeT++ using ``opp_env``, and run
the simulation interactively.

Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.7 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.7.*/showcases/ipv6/pathmtudiscovery && inet'

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
