Mobile IPv6
===========

Goals
-----

An IPv6 address plays two roles at once: routers treat it as a *location* (the
prefix says which link the node is on), and transport connections treat it as
an *identity* (a connection is pinned to the address pair). When a device moves
to a network with a different prefix, it gets a new, routable address — but
nobody can reach it at the address they know, so its open connections stall
until it comes back.

Mobile IPv6 (RFC 6275) solves this by splitting the two roles into two
addresses, anchored by a *home agent*. This showcase demonstrates the whole
mechanism in one scenario: a wireless node moves from its home network to a
foreign one and back, while a peer keeps pinging it at its stable address.
Without Mobile IPv6 the node cannot be reached while it is away; with it, the
traffic keeps flowing — first through a tunnel, then, with route optimization,
on the direct path.

| Verified with INET version: ``TODO``
| Source files location: `inet/showcases/ipv6/mipv6 <https://github.com/inet-framework/inet/tree/master/showcases/ipv6/mipv6>`__

.. todo::

   The showcase needs INET source changes that are not in any release yet: ac1db6c244 (a ``WirelessHost6`` mobile node without Mobile IPv6), 8b08688968 and 7e6083f7f1 (the first-registration Binding Update timer, pull request #1134), 979eb60440 (pull request #1152), aeee20a40d (Mobile IPv6 signaling addressed to the next hop), and pull requests #1247, #1195, #1141, #1183, #1157 and #1260. Without them the
   ``WithoutMipv6`` configuration still runs Mobile IPv6 and the registration
   takes two Binding Updates. Fill in the version above, and change the
   ``inet-4.7`` release in the Try It Yourself ``opp_env`` commands, once a
   release contains them.

About Mobile IPv6
-----------------

Terminology
~~~~~~~~~~~

Mobile IPv6 is dense with abbreviations, so here is the cast of characters:

- **Mobile node (MN)** — the device that moves between networks.
- **Home network** — the network hosting the mobile node's permanent address;
  "home" means an administrative relationship (a campus network, an ISP), not
  a physical place.
- **Home address (HoA)** — the mobile node's stable address, formed from the
  home network's prefix. This is the identity: applications bind to it, and
  peers use it to reach the mobile node, wherever it is.
- **Care-of address (CoA)** — the temporary address the mobile node gets in
  whatever network it is visiting. This is the location; it changes with every
  move.
- **Home agent (HA)** — a function on a router on the home link. It stands in
  for the mobile node while it is away.
- **Correspondent node (CN)** — simply the peer the mobile node talks to: a
  server, another host, anything. It can be anywhere in the internet, and it
  needs Mobile IPv6 support only if it takes part in route optimization.
- **Binding** — the association between a home address and a care-of address,
  valid for a limited lifetime; creating, refreshing and deleting bindings is
  what all Mobile IPv6 signaling does.

The mobile node acquires both of its addresses by *stateless address
autoconfiguration* (SLAAC): routers periodically multicast Router
Advertisements carrying the link's prefix (a freshly attached host solicits
one immediately with a Router Solicitation), the host appends an interface
identifier of its own making, and the address is ready — no server involved.
This is what makes mobility work in networks that have never heard of the
mobile node: it can build itself a care-of address anywhere.

An address built that way is a guess until it is checked, so the host may not
use it yet — the standard calls such an address *tentative* — and runs
*duplicate address detection* (DAD, RFC 4862): it multicasts a *Neighbor
Solicitation* naming that very address and waits. A Neighbor Solicitation is
the question IPv6's *Neighbor Discovery* protocol asks about an address — the
replacement for the ARP request — and a *Neighbor Advertisement* is the answer
to it. Silence means the address is free and becomes usable; an answer means
the guess collided and the address must be abandoned. Nothing may be sent from
an address still being checked, and the wait is a fixed timeout rather than a
round trip, so a host arriving on a new link pays duplicate address detection
in full before it can do anything — which is why it dominates the handover
outage measured below.

What happens on a move
~~~~~~~~~~~~~~~~~~~~~~

When the mobile node (MN) walks out of its home network into a foreign one:

1. **Layer 2 handover** — the wireless interface re-associates with the new
   access point.
2. **Movement detection** — a Router Advertisement on the new link reveals an
   unfamiliar prefix: "I have moved."
3. **Care-of address formation** — normal SLAAC on the new link. Before the
   node uses its new addresses, duplicate address detection checks both its
   link-local address and the new care-of address; the two checks may run at
   the same time.
4. **Registration** — the mobile node sends a *Binding Update (BU)* to its
   home agent (HA): "my home address is now reachable at this care-of
   address, for this lifetime." The home agent confirms with a *Binding
   Acknowledgement (BA)*. Every Binding Update carries a sequence number
   that counts up, and the acknowledgement echoes the number of the update it
   answers; each end discards anything below the highest number it has already
   seen as stale, which is how a retransmission is told apart from a newer
   registration. No security handshake is needed here — the mobile
   node and its home agent trust each other by prior arrangement (the standard
   protects this signaling with an IPsec association set up in advance).
5. **Delivery resumes** — the home agent now intercepts every packet addressed
   to the home address and forwards it to the care-of address inside an
   IPv6-in-IPv6 tunnel: it wraps the whole original packet inside a new one
   addressed to the care-of address, and the mobile node unwraps
   (decapsulates) it again. The mobile node sends its own traffic back through
   the same tunnel in reverse.

Bindings are soft state: they expire unless the mobile node refreshes them
with further Binding Updates, so a crashed or vanished mobile node simply ages
out of the home agent's *binding cache* (the table of home address → care-of
address mappings).

Why must the reverse direction also be tunneled, instead of the mobile node
answering the correspondent node directly? Ingress filtering: a packet leaving
the foreign network with a home-network source address looks spoofed and gets
dropped. So base Mobile IPv6 is *bidirectional tunneling* — both directions
detour through the home agent, and the correspondent never learns that its
peer moved.

On the home link itself, the home agent impersonates the absent mobile node:
it answers Neighbor Solicitations for the home address (the IPv6 equivalent of
an ARP request — "the node that *is* this address: report your MAC") with its
own MAC address, so frames for the home address land on it. This is proxy
Neighbor Discovery — the same mechanism an attacker would call address
spoofing, used here with authorization. While the mobile node is at home, the
home agent does nothing at all.

Route optimization
~~~~~~~~~~~~~~~~~~

The tunnel detour costs latency (the path stretches through the home network)
and 40 bytes of outer header per packet. *Route optimization* removes both:
the mobile node registers its binding directly with the correspondent node
(CN), after which packets flow on the direct path in both directions — the
home agent drops out entirely.

The interesting part is security. A forged "I moved, send my traffic here"
would be a session-hijacking primitive, and correspondent and mobile node are
strangers — there is no pre-arranged trust to lean on. Mobile IPv6's answer is
the *return-routability procedure*, which is pure authentication and carries
no routing information at all:

- The mobile node sends two probes to the correspondent node: a *Home Test
  Init (HoTI)* routed through the home-agent tunnel, and a *Care-of Test Init
  (CoTI)* sent directly.
- The correspondent node returns a token to each source: a *Home Test (HoT)*
  via the home agent, and a *Care-of Test (CoT)* on the direct path.
- Only a node reachable at **both** the home address and the care-of address
  collects both tokens, and only their combination yields the key that
  authenticates the *Binding Update* to the correspondent node.

Note the built-in ordering: the mobile node sends the Home Test Init and the
Care-of Test Init at the same time, but the home half travels through the home
agent, so the mobile node must first have sent its Binding Update to the home
agent. The Care-of Test half needs no home agent. The tokens are
deliberately short-lived (three and a half minutes; the binding they authorize
at a correspondent node lasts at most seven), so long sessions re-run the
procedure periodically; a correspondent can also prompt a refresh with a
*Binding Refresh Request*.

After route optimization, packets carry both addresses: the care-of address
where routers look, and the home address in an extension header — a *type 2
routing header* toward the mobile node, a *Home Address destination option*
from it. The network layer swaps the home address back in at each end, so
transport connections stay pinned to the stable home address and never notice
that the packets took a different path.

Route optimization is optional, per correspondent. A mobile node may prefer
tunneling on purpose: route optimization reveals the care-of address — that
is, the mobile node's current location — to every correspondent, while
tunneling hides it behind the home agent.

When the mobile node returns home, it de-registers: a Binding Update with
lifetime zero deletes the binding at the home agent and at every correspondent
node, the tunnel disappears, and the home agent stops answering for an address
whose owner is back. The mobile node then announces its return on the home
link with an unsolicited Neighbor Advertisement, so its neighbors switch
their caches back to it immediately instead of waiting to re-resolve the
address. Everything collapses to plain IPv6.

A final property worth noticing: only the mobile node, its home agent, and
(optionally) the correspondent nodes know that mobility is happening. The
visited network sees an ordinary host with an ordinary local address; every
router in between forwards ordinary IPv6 packets.

Mobile IPv6 in INET
-------------------

INET implements the mobile node, home agent, and correspondent node roles of
RFC 6275 in the ``Mipv6`` module, an optional submodule of the IPv6 network
layer. The ``hasMipv6`` parameter of ``Ipv6NetworkLayer`` creates that module,
together with the two data modules it works with.

Which role a node plays is decided by two boolean parameters on ``Mipv6``:
``isMobileNode`` and ``isHomeAgent``. They are not three switches for three
roles. They select between two kinds of memory. A mobile node keeps a
*Binding Update List*, the record of the bindings it has registered
elsewhere. Every other Mobile IPv6 node keeps a *Binding Cache*, the record of
bindings it holds for others. A node that sets neither flag is a
correspondent node: it does not select that role, it falls through to it.

What each role does follows from that. The mobile node registers. It forms a
care-of address, sends the Binding Update, and runs the return-routability
exchange. The home agent answers. It acknowledges every Binding Update, holds
the binding, and builds the tunnel to the care-of address. A correspondent
node only accepts a binding and answers the return-routability test. No node
type sets both flags.

A model rarely sets the flags itself. The four node types below are ordinary
IPv6 nodes that switch Mobile IPv6 on and then fix them, so choosing a node
type chooses a role:

- ``WirelessHost6`` — a ``StandardHost6`` with one wireless interface, in the
  mobile-node role. This is the node that moves.
- ``MobileHost6`` — the same role on a wired host (not used here).
- ``HomeAgent6`` — a ``Router6`` in the home-agent role.
- ``CorrespondentNode6`` — a ``StandardHost6`` that is neither a mobile node
  nor a home agent. It differs from a plain ``StandardHost6`` in nothing but
  the presence of the Mobile IPv6 modules, and that presence is what lets it
  accept a binding and take part in route optimization.

The screenshot below shows the mobile node's IPv6 network layer. Mobile IPv6
is not a separate protocol layer: the ``mipv6`` module sits beside ``ipv6``,
``icmpv6``, and ``neighbourDiscovery`` and implements the mobility signaling,
with two data modules — ``buList`` (the mobile node's record of bindings it
has registered elsewhere) and ``bindingCache`` (bindings this node holds for
others) — beside it. The entire protocol footprint is visible in this one
image:

.. figure:: media/networklayer.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas
   config:   RouteOptimization   # ../omnetpp.ini
   seed:     default (seed-set=1)
   shows:    Ipv6NetworkLayer interior of mobileNode: mipv6 beside ipv6/icmpv6/
             neighbourDiscovery, plus buList + bindingCache
   anchor:   structural (t=0..12s, any time before handover). If the submodule
             set differs, Ipv6NetworkLayer or the mipv6 integration changed.
   capture:  set_canvas_view zoom=1.0 -> get_canvas_image module_rectangle
             margin=5 on Mipv6Showcase.mobileNode.ipv6; was 968x615. Known
             blemish: routingTable status text overlaps configurator label
             (the module's own layout).
   stamp:    captured 2026-08, INET 4.7

Every tunable parameter lives in the ``mipv6`` module:

- ``isMobileNode`` and ``isHomeAgent`` — the two role flags, fixed by the node
  type as described above.
- ``useRouteOptimization`` (default ``true``) — route-optimize with
  correspondent nodes, or always tunnel through the home agent.
- ``maxHaBindingLifeTime`` (default 3600 s) — the lifetime the mobile node
  requests for a home registration; the home agent accepts it unchanged.
- ``maxRrBindingLifeTime`` (default 420 s) — the lifetime the mobile node
  requests for a binding at a correspondent node, which accepts it unchanged.

One level up, ``hasMipv6`` on ``Ipv6NetworkLayer`` decides whether these
modules exist at all. The same network layer also has a ``hasPmipv6``
parameter for Proxy Mobile IPv6 (RFC 5213), a different approach in which the
network moves the node and the mobile node runs no mobility software of its
own. This showcase does not use it.

Neither lifetime expires inside this showcase's 80 second run. ``Mipv6`` also
emits two signals: ``mipv6RoCompleted`` when route optimization finishes, and
``packetDropped``. ``Mipv6.ned`` declares no statistic for them, so a study
must declare a ``@statistic`` before it can record them. The losses that the
Results section walks through do not happen in ``Mipv6`` and emit no ``Mipv6``
drop signal: the access points drop pings after their retry limit, and in the
run without Mobile IPv6 the router ``homeAgent`` drops the pings it cannot
deliver, and its interface on the home link discards the misaddressed replies.

Configuration notes:

- ``hasMipv6`` (on the node's ``ipv6`` submodule) creates or omits the whole
  Mobile IPv6 footprint. The ``WithoutMipv6`` configuration sets it to
  ``false`` on the mobile node, which leaves an ordinary wireless IPv6 host and
  is how this showcase measures what mobility support is worth.
- ``useRouteOptimization`` on the mobile node is what separates two of the
  three configurations: the same scenario runs both ways with this one flag.
- Movement detection does not depend on frequent Router Advertisements: on
  every layer-2 association the IPv6 neighbour discovery module immediately
  sends a Router Solicitation (its ``detectL2Movement`` parameter, default
  ``true``), so the mobile node never waits for a periodic advertisement.
- The mobile node autoconfigures its addresses, so the address configurator
  must leave hosts alone: the network sets ``assignAddressesToHosts = false``
  on the ``Ipv6NetworkConfigurator``, which then assigns addresses to routers
  only (it still adds static routes to hosts). The home agent is recognized
  from its Router Advertisements (via the home-agent flag the standard defines
  for them), which is how the mobile node learns its home agent's address — at
  home, before ever leaving. The standard's remote-discovery mechanisms are not
  modeled, so a mobile node must start the simulation in its home network.

Implementation notes and simplifications, so the simulation is read for what
it is:

- The IPsec protection that RFC 6275 mandates between mobile node and home
  agent is not modeled (standard practice in simulation).
- The return-routability *message exchange* — sequence, paths, sizes, timing,
  and the mobile node's periodic token refresh — is faithful, but the
  cryptographic content is schematic: the tokens are constants rather than
  keyed, nonce-indexed values, and the correspondent never invalidates them.
- The home agent intercepts traffic by virtue of being the home network's
  router; answering Neighbor Solicitations on behalf of the absent mobile
  node (proxy Neighbor Discovery) is not implemented. In this scenario the
  distinction is invisible — no other host lives on the home link — but a
  host on the home link could not reach an away mobile node.

  .. todo::

     The home agent now runs duplicate address detection on the home address
     before it acknowledges a first registration (pull request #1195), but it
     still neither announces its claim with a multicast Neighbor Advertisement
     nor answers Neighbor Solicitations for the away mobile node. This part of
     gap 1 of MIPV6_IMPLEMENTATION_GAPS.md was not filed as of 2026-08-27.

- Before it acknowledges a first registration, the home agent runs duplicate
  address detection on the mobile node's home address on the home link, as the
  standard requires. It creates the binding and the tunnel as soon as the
  Binding Update arrives, and sends the acknowledgement when the check
  completes.
- Until that acknowledgement lands the mobile node's reverse tunnel is not up,
  so the node holds the packets it sends from its home address — ping replies,
  and the first Home Test Init of route optimization — and sends them through
  the reverse tunnel as soon as the acknowledgement arrives. It does not emit a
  packet whose home-address source would look spoofed outside the home network
  — its private version of the ingress filtering discussed earlier. The held
  replies arrive late instead of being lost.
- The mobile node waits 1.5 s for the acknowledgement of a first home
  registration before it retransmits the Binding Update. This is the standard's
  first-registration timeout (InitialBindackTimeoutFirstReg), chosen to leave
  the home agent time for its duplicate address detection. The acknowledgement
  arrives about one second after the Binding Update (1.04 s in the run shown),
  so the home registration takes one Binding Update, and the binding cache
  records sequence number 1.
- After a move, the mobile node runs duplicate address detection first on its
  link-local address and then on its new care-of address, one after the other;
  the standard requires both checks and allows them to run at the same time.
  Before each probe the node waits a random delay, as RFC 4862 describes.

The Model
---------

The network puts the three Mobile IPv6 locations in three corners: a home
network (``apHome`` + ``homeAgent``), a foreign network (``apForeign`` +
``foreignRouter``), and a correspondent node, all meeting at a backbone
router:

.. figure:: media/network.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas
   config:   RouteOptimization   # ../omnetpp.ini
   seed:     default (seed-set=1)
   shows:    topology + the mobile node associated at home ("AP: HOME/at home"
             status label, green SLAAC address label on the right)
   anchor:   t=12s: associated, SLAAC done, label shows the home address
             2001:db8:0:1:8aa:ff:fe00:d. If the address differs, the MAC
             assignment order changed -> re-pin destAddr in the ini too.
   capture:  express-run to 12s -> set_canvas_view fit (zoom ~1.106) ->
             get_canvas_image module_rectangle margin=5; was 844x722. A route
             visualizer arrow may still be fading at some capture times;
             capture at ~12.2s (mid ping interval) for a clean shot.
   stamp:    captured 2026-08, INET 4.7

The status text above the mobile node is live: it shows the associated
SSID and the mobility state (at home / away / route-optimized), and it updates
as the simulation runs. The green label on the right is the node's preferred
address, which stays the home address throughout, even while the node is away.
Note that the foreign network's router is a plain
``Router6`` — the visited network needs no Mobile IPv6 support at all, just
as promised above.

The WAN link delays are the scenario's one deliberate design choice: 5 ms
between home agent and backbone, 8 ms between foreign router and backbone,
1 ms to the correspondent node. Real mobility spans real distances, and these
delays make each forwarding mode land at a distinct, predictable round-trip
time — the sum of the link delays along its path:

.. literalinclude:: ../Mipv6Showcase.ned
   :start-at: connections:
   :end-at: correspondentNode.ethg
   :language: ned

Predicted round-trip times, adding up the link delays along each path (the
access LANs' 0.1 µs is too small to matter):

- **at home**: 2 × (1 + 5) = 12 ms
- **tunneled**: 2 × (1 + 5) + 2 × (5 + 8) = 38 ms — every packet crosses the
  backbone twice, once to the home agent and once through the tunnel
- **route-optimized**: 2 × (1 + 8) = 18 ms

Those are the wired links only. The wireless hop is crossed twice per round
trip — once carrying the request to the mobile node, once carrying the reply
back — and each crossing costs about 1 ms: at 2 Mbps a ping frame takes that
long to clock out, plus the wait for a free channel. Adding the resulting 2 ms
gives the three plateaus the Results section measures: 14 ms at home, 40 ms
tunneled, 20 ms route-optimized.

Note that the route-optimized value is below anything a path through the home
agent could achieve: even the cheapest conceivable detour — out through the
tunnel (1 + 5 + 5 + 8 = 19 ms) and straight back (8 + 1 = 9 ms) — costs 28 ms
in link delays alone. Measuring 20 ms is proof by arithmetic that the home
agent is out of the loop.

The mobile node's movement has three acts (``movement.xml``): it dwells at
home for 15 s, dashes to the foreign network in 3 s, dwells there for 30 s,
and returns the same way, settling at home for the rest of the 80 s run. The
two access points operate on different wireless channels, and the mobile node
scans both.

Throughout the run, the correspondent node pings the mobile node's *home
address* every 0.5 s — the address is hardcoded in the ini file because it is
the stable identity a peer would know (it is also predictable: the home
prefix plus the interface identifier derived from the mobile node's MAC
address):

.. literalinclude:: ../omnetpp.ini
   :start-at: correspondentNode.numApps
   :end-at: app[0].startTime
   :language: ini

The mobile node is a ``WirelessHost6`` in all three configurations below.
They differ only in whether its Mobile IPv6 (MIPv6) engine is present at all
and, when it is, whether route optimization is enabled; everything else —
network, movement, traffic — is identical.

WithoutMipv6 configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config WithoutMipv6]
   :end-at: hasMipv6
   :language: ini

Setting ``hasMipv6 = false`` removes the ``mipv6`` module from the mobile
node's IPv6 network layer, leaving an ordinary wireless IPv6 host. It
associates with the foreign access point and stateless address
autoconfiguration (SLAAC) gives it a perfectly good new address — but nobody
sends anything to that address. The pings keep targeting the old (home)
address, which now leads to a network where nobody answers Neighbor
Solicitations for it. Every ping from the moment it leaves home coverage
until it walks back is lost. This is the problem Mobile IPv6 (MIPv6) exists
to solve, measured.

BidirectionalTunneling configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config BidirectionalTunneling]
   :end-at: useRouteOptimization
   :language: ini

The mobile node runs Mobile IPv6 with route optimization switched off — the
base protocol, nothing else. After the handover it registers its care-of
address with the home agent, and the pings resume: into the home network,
intercepted by the home agent, tunneled to the care-of address, answered
through the reverse tunnel. The round-trip time settles on the tunneling
plateau (~40 ms) for the whole stay abroad.

RouteOptimization configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config RouteOptimization]
   :end-at: useRouteOptimization
   :language: ini

The configuration states ``useRouteOptimization`` explicitly so that the one
parameter separating this scenario from the previous one is visible in both
listings. The first tunneled ping that reaches the mobile node triggers the
return-routability procedure with the correspondent node, the Binding Update
installs a binding there, and once that registration completes, the traffic
takes the direct path (~20 ms) — until the return home tears everything down
again.

Results
-------

The round-trip time of every ping tells the whole story. The three
configurations are plotted separately on identical axes, so the phases can be
compared panel by panel; the shaded band marks the interval the mobile node
spends away from its home network.

.. figure:: media/pingrtt-without.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      Mipv6Showcase.anf   chart "Ping round-trip time (without Mobile IPv6)"
   inputs:   results/WithoutMipv6-#0.vec (re-run the config first, seed-set 1)
   shows:    RTT of every ping without Mobile IPv6: the 14 ms home plateau, no reply from 17.014 s to 54.514 s (37.5 s), the home plateau again; "away from home" span shaded 15..51 s
   anchor:   axes are pinned (x 0..80 s, y 0..45 ms) so the three panels compare
             directly -- keep all three identical if any one is redone.
             boot: ping7 35.5 ms at 4.536 s is the only boot reply on scale; home median 14.05 ms; last reply before the move ping32 at 17.014 s;
             first reply after the return ping107 at 54.514 s (14.07 ms); no reply above the axis at ~52 s;
             elevated ping117 15.73 ms at 59.5 s (waited behind the mobile node's NUD probe).
             If the gap is not 37.5 s, the return timing changed.
             Replies above 45 ms are NOT drawn (no marker, no label; the y axis stays pinned).
             In this run that is ping4/ping5/ping6 at 4.536 s (1535.5 / 1035.5 / 535.5 ms, held at boot by homeAgent).
   export:   opp_charttool imageexport Mipv6Showcase.anf -n "(without Mobile IPv6)" -f png --dpi 150
             -d doc/media   (8x6 in -> 1200x900; filename from image_export_filename)
   stamp:    captured 2026-09-30, INET HEAD 8c94b616cd (topic/gy/mipv6-showcase), OMNeT++ 6.4.0aipre2
             verified 2026-10-01 at ba7c6038bf: the WithoutMipv6 run is identical (same event count 42620, same pings), so this image is kept.
             re-exported 2026-10-01 without off-scale markers and labels (user ruling).

Without Mobile IPv6 the node is unreachable the whole time it is away — no
replies at all for 37.5 s (34.5–37.5 s over ten seeds), resuming only when it
re-enters home coverage on the way back. Its home address means nothing on the
foreign link. The first three replies after boot, at about 4.5 s, are not drawn
in any of the three charts: they arrive 0.5 to 1.5 s late, too late to fit the
0–45 ms axis, because the router ``homeAgent`` holds the first pings while the
mobile node is still checking its home address.

.. figure:: media/pingrtt-bidirectional.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      Mipv6Showcase.anf   chart "Ping round-trip time (bidirectional tunneling)"
   inputs:   results/BidirectionalTunneling-#0.vec (re-run the config first, seed-set 1)
   shows:    RTT of every ping with route optimization off: 14 ms at home, no point drawn from 17.014 s until ping44 at 23.040 s (the held ping42/ping43 replies at 22.96 s are above the axis), the 40 ms tunneled plateau from ping44, gap 50.040-51.514 s, 14 ms at home again; "away from home" span shaded 15..51 s
   anchor:   axes are pinned (x 0..80 s, y 0..45 ms) so the three panels compare
             directly -- keep all three identical if any one is redone.
             first replies after the move ping42 at 22.964 s
             (963.9 ms) and ping43 at 22.965 s (465.2 ms), both above the axis (not drawn); away median 40.32 ms
             (40.26-40.40, from ping44); last reply away ping98 at 50.040 s; first reply at home ping101
             at 51.514 s (return 1.474 s); elevated ping111 16.04 ms at 56.5 s (MN NUD probe).
             Replies above 45 ms are NOT drawn (no marker, no label; the y axis stays pinned).
             In this run that is ping4/ping5/ping6 at 4.536 s (1535.5 / 1035.5 / 535.5 ms) and ping42/ping43 at 22.964 / 22.965 s (963.9 / 465.2 ms, held by the mobile node until the Binding Acknowledgement).
   export:   opp_charttool imageexport Mipv6Showcase.anf -n "(bidirectional tunneling)" -f png --dpi 150
             -d doc/media   (8x6 in -> 1200x900; filename from image_export_filename)
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ 6.4.0aipre2
             re-exported 2026-10-01 without off-scale markers and labels (user ruling).

Bidirectional tunneling restores reachability, at the cost of a detour. After a
6.0 s outage replies resume on the 40 ms plateau and stay there, every packet
taking the long way through the home agent. The first two replies, to
``ping42`` and ``ping43``, are not drawn: with round trips of 963.9 ms and
465.2 ms they do not fit the 0–45 ms axis. The mobile node held them until its
binding was active, as the registration sequence chart below shows.

Where those seconds go, from this run's event log — the last reply at home
arrives at t = 17.014 s, the first tunneled one at t = 22.964 s:

- **0.39 s** — the ping spacing, not the handover: the node still hears
  ``apHome`` until 17.407 s, but the next ping is due only at 17.5 s.
- **0.35 s** — still associated with the home access point, already out of
  range: the node declares the access point lost after 3.5 beacon intervals
  without a beacon (0.1 s each; the 3.5 is fixed in INET). The requests sent
  from 17.5 s on are simply lost.
- **0.65 s** — scanning both channels, then authenticating and associating with
  ``apForeign``.
- **0.45 s** — waiting for a Router Advertisement on the new link, which
  reveals the unfamiliar prefix; the router delays its answer by a random time
  of up to 0.5 s.
- **1.90 s** — duplicate address detection on the link-local address: the node
  waits a random 0.90 s, sends the probe, and then waits 1 s for an answer, as
  RFC 4862 describes. The home address is tentative meanwhile.
- **1.15 s** — duplicate address detection on the new care-of address, the same
  way: a random 0.15 s, the probe, and 1 s. The *Binding Update* leaves at the
  same instant as this check completes.
- **1.04 s** — the Binding Update's way to the home agent (30 ms, including 16
  ms for ``foreignRouter`` to resolve the backbone router's address), the home
  agent's own duplicate address detection on the home address (the probe at
  once, and 1 s) before it sends the *Binding Acknowledgement*, and the
  acknowledgement's way back (14 ms). The home agent creates the binding and
  the tunnel at once, before its check completes.
- **0.02 s** — the first reply's way back. The pings sent at 22.0 s and 22.5 s
  reached the mobile node through the tunnel; it held their replies until its
  binding became active at 22.944 s and then sent them through the reverse
  tunnel. The pings sent before 22.0 s never reached it: the home agent had no
  binding for them yet.

More than half the outage — 4.04 s of 5.95 s — is duplicate address detection,
at one end or the other. It is a correctness check whose entire cost lands in
handover latency, which is what motivates optimizations such as RFC 4429
Optimistic DAD. All three terms are timeouts rather than round trips, so none
of them shrinks on a faster link.

Those values are this seed's, not constants: before each of the mobile node's
probes INET waits a random delay, as RFC 4862 describes, and after it
``retransTimer`` (1 s); the home agent probes at once. With these random delays
and the random delay of the Router Advertisement, the outage ranges over
5.3–9.3 s across ten seeds (median 6.1 s); three runs of the ten are shorter
than this one.

.. figure:: media/pingrtt-routeopt.png
   :align: center

..
   FIGURE RECIPE (redo via the "inet-showcase-charts" skill)
   type:     chart (matplotlib)
   anf:      Mipv6Showcase.anf   chart "Ping round-trip time (route optimization)"
   inputs:   results/RouteOptimization-#0.vec (re-run the config first, seed-set 1)
   shows:    RTT of every ping with route optimization on: 14 ms at home, no point drawn from 17.014 s until ping44 at 23.020 s (the held ping42/ping43 replies at 22.97 s are above the axis), no 40 ms point, the 20 ms direct plateau from ping44, gap 50.020-51.514 s, 14 ms at home again; "away from home" span shaded 15..51 s
   anchor:   axes are pinned (x 0..80 s, y 0..45 ms) so the three panels compare
             directly -- keep all three identical if any one is redone.
             ping42 at 22.965 s (965.1 ms) and ping43 at
             22.966 s (466.3 ms) above the axis (not drawn); NO 40 ms point; ping44 at 23.020 s (20.22 ms) is the first
             direct reply; direct median 20.16 ms (20.08-20.22); last reply away ping98 at 50.020 s;
             first reply at home ping101 at 51.514 s (return 1.494 s, Router Advertisement not held).
             Replies above 45 ms are NOT drawn (no marker, no label; the y axis stays pinned).
             In this run that is ping4/ping5/ping6 at 4.536 s (1535.5 / 1035.5 / 535.5 ms) and ping42/ping43 at 22.965 / 22.966 s (965.1 / 466.3 ms, held by the mobile node until the Binding Acknowledgement).
   export:   opp_charttool imageexport Mipv6Showcase.anf -n "(route optimization)" -f png --dpi 150
             -d doc/media   (8x6 in -> 1200x900; filename from image_export_filename)
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ 6.4.0aipre2
             re-exported 2026-10-01 without off-scale markers and labels (user ruling).

Route optimization removes the detour right away. The same outage, then the two
held replies at 22.97 s, too late to be drawn on this axis, and the direct path
at 20 ms from ``ping44`` on. No reply shows the 40 ms tunnel path: the
correspondent node has its binding at 22.994 s, just before ``ping44`` leaves.

Up to the handover the three runs are identical: while the node is at home
Mobile IPv6 has nothing to do, so the three configurations are the same
simulation, sample for sample, on the 14 ms baseline. Plotted on one pair of
axes the three curves coincided exactly and hid one another, which is why they
are shown separately here.

On the way back (t≈50 s) a second outage covers re-association and
de-registration: 1.49 s with route optimization and 1.47 s with bidirectional
tunneling, both far shorter than the 5.95 s outbound outage (the return video
explains why). All three configurations converge on the 14 ms baseline again —
for the plain host only at 54.514 s: its home router's address resolution
reaches it at 52.006 s, but the router's Router Advertisement, held by the 3 s
limit on multicast Router Advertisements, arrives only at 54.311 s, and until
then the host addresses its replies to the foreign router, so the router's
interface on the home link discards them.

Details worth noticing rather than worrying about: the first replies arrive
only at t≈4.5 s — the first three (``ping4`` to ``ping6``, 1.5, 1.0 and 0.5 s
late) are too late to be drawn, and the isolated dot at about 35 ms at the same
moment is the fourth, ``ping7``, the last of the pings the router held —
because the home agent cannot resolve the home address while the mobile node is
still checking it with duplicate address detection after boot, and holds the
pings meanwhile; the few isolated elevated dots (``ping111`` at t=56.5 s with
bidirectional tunneling and ``ping117`` at t=59.5 s without Mobile IPv6) are
not 802.11 retransmissions but replies that waited behind a Neighbor
Unreachability Detection probe; and after the return, the two Mobile IPv6 runs
are answered again at t=51.514 s, 20 µs apart, and the plain host only at
54.514 s — which host comes back first depends on the run's random timing, not
on Mobile IPv6.

The handover, live
~~~~~~~~~~~~~~~~~~

The video below shows the outbound handover in the ``RouteOptimization``
configuration (t = 16.5 s to 23.6 s). The colored polylines are drawn by INET's
network-route visualizer: each traces the path a ping reply actually took.
Watch the sequence: the home path (correspondent → backbone → home agent →
mobile node) while at home; the dash to the foreign network; the registration,
which is not drawn but shows in the status label; then two replies, to
``ping42`` and ``ping43``, taking the detour through the home agent together —
and finally the direct path through the foreign router, with the home agent out
of the loop. The status label steps from "at home" through a brief "away (via
home agent)" to "away (route-optimized, 1 CN)", while the address label keeps
showing the home address — the node's identity — for the whole stay; the
care-of address is used underneath.

.. video:: media/handover.mp4
   :align: center

..
   VIDEO RECIPE (redo via the "video-recording" skill)
   config:   RouteOptimization
   seed:     default (seed-set=1)
   shows:    home path -> the move -> AP label HOME -> FOREIGN, status "at home" while both
             duplicate address detections run -> "away (via home agent)" -> two reply paths through
             the reverse tunnel (the held ping42/ping43 replies, drawn along homeAgent-backbone) ->
             "away (route-optimized, 1 CN)" -> ping44 and later pings on the direct path through
             foreignRouter
   anchors:  association with apForeign 18.408 s; care-of DAD done + BU 21.900 s (status "away (via
             home agent)"); BA at the MN 22.944 s releases the held replies: ping42 and ping43 reply
             paths drawn at 22.96 s; CN's BA at the MN 23.003 s (status "away (route-optimized, 1 CN)");
             ping44 direct 23.020 s, ping45 23.520 s. The green address label stays at the home address
             2001:db8:0:1:8aa:ff:fe00:d throughout (it shows the interface's preferred address, which is
             the home address in this model version; the Mipv6 watch careOfAddress holds
             2001:db8:0:3:8aa:ff:fe00:d). No reply path is drawn between 17.014 s and 22.96 s.
   window:   express to 16.5 s -> step 1 event (normal) -> record to 23.6 s.
             Route visualizer fades in simulation time (0.6 s) -> no settle wait.
   launch:   private prefs copy so the user's Qtenv prefs stay untouched: XDG_CONFIG_HOME=
             /var/tmp/mipv6-video/xdg (omnetpp/.qtenvrc copied from ~/.config/omnetpp/.qtenvrc with
             animation_enabled=false); opp_run_release -l <wt>/src/INET -u Qtenv -c RouteOptimization
             -n <wt>/src:<wt>/showcases '--*.visualizer.osgVisualizer.typename=""'
             --mcp-server-address localhost:8777 --qtenv-default-run=0 omnetpp.ini
   anim:     set_animation_parameters profile=normal playback_speed=1 min_animation_speed=0.1
             (0.05 s simulation time per frame at fps=2; encoded at 6 fps = 0.3 s per video second)
   capture:  record_video fps=2, crop_area=with_padding, time_limit 23.6; 142 frames (0000-0141);
             frames kept in /var/tmp/final/video/frames_handover
   encode:   ffmpeg -r 6 -f image2 -i <prefix>_%04d.png -filter:v "crop=830:684:923:123"
             -vcodec libx264 -pix_fmt yuv420p <name>.mp4  (the crop keeps only the canvas interior,
             x 923-1752, y 123-806. The Qtenv window position varies between launches -- re-measure the
             black canvas border on a frame before encoding: 2026-10-01 crop_rect was 854x732 at 911,87,
             border columns 921-922 / 1753-1754); 830x684, fits the 834 px column without scaling
   post:     none
   stamp:    recorded 2026-10-01, INET HEAD ba7c6038bf, OMNeT++ 6.4.0aipre2

The signaling, message by message
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The sequence charts below follow the same handover on the eventlog level, one
exchange at a time, filtered to the messages that matter in each stretch. Each
horizontal line is one node, and each arrow is one hop of one packet. An arrow
that bends at a line stops at that node, which then sends the packet on; an
arrow that only crosses a line passes that node's position on the chart
without touching the node. That is how the charts show whether the home agent
is in the path or not. A strip under each chart gives the absolute simulation
time at the named events.

The ping names carry the ICMPv6 sequence number, which starts at zero: the
correspondent node sends ``ping0`` at t = 1 s and one more every 0.5 s, so
``ping42`` leaves at t = 22.0 s.

**Arriving in the foreign network.** This part is the same in all three
configurations; the chart is taken from the ``WithoutMipv6`` run, so it ends
before any Binding Update. Its time axis is linear, so the waits appear at
their true length:

.. figure:: media/seqchart-p1-movement.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   WithoutMipv6, seed-set 1, --record-eventlog=true
             --eventlog-recording-intervals=18.3s..22.0s,51.3s..54.6s
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-8c94.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range. In NONLINEAR mode the zoom
             only sets the left edge; the scale follows the event count, the panel width is set with
             resize_window, and the right edge ends at the last filtered event.
   axes:     mobileNode, apForeign, foreignRouter (MANUAL order)
   filter:   message_names RouterSolicitation, RouterAdvertisement, NeighbourSolicitation
   view:     NETWORK_COMMUNICATION, SIMULATION_TIME (linear); resize_window 1250x900 (widget 1036x508);
             goto_event 10756, zoom 18.30..22.00 (viewport left 18.3011594516, 280 px/s)
   anchor:   RS 18.408302 (x 30); solicited RA at the MN 18.857696 (#11091, x 156); periodic RA
             19.352899 (x 294); link-local DAD probe 19.754759 (#11626, x 407; random 0.897 s before it);
             link-local check done 20.754759 (#12221, no message); care-of DAD probe 20.900354 (x 728;
             random 0.146 s); RS again 20.994583 (#12346, x 754); care-of check done 21.900354 (#12938,
             no message, x 1006). Second RS/NS labels in the apForeign band = the AP's copy into its cell.
             If the RA and the NS coincide again, the delay-before-probe fix is gone.
   capture:  screenshot 1036x508 (raw: seqchart-raw-8c94/p1.png) -> crop (0,150)-(1036,470)
   post:     below the chart a magenta wait strip (x=(t-18.3011594516)*280): "random delay 0.90 s"
             18.858-19.755, "link-local check 1 s" 19.755-20.755, "0.15 s" 20.755-20.900, "care-of
             check 1 s" 20.900-21.900, caption "duplicate address detection on the mobile node"; then a
             linear tick strip (18.408, 18.858, 19.353, 19.755, 20.900, 20.995, 21.900), note "time [s],
             linear". No zoom strip any more (the RA and the NS no longer coincide). Result 1036x455.
             Script research/analyst-data/pn8c94.py p1.
   stamp:    captured 2026-09-30, INET HEAD 8c94b616cd (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre
             verified 2026-10-01 at ba7c6038bf: the WithoutMipv6 run is identical (same event count 42620, same pings), so this image is kept.

The Router Solicitation goes from ``mobileNode`` to ``foreignRouter`` at once,
and the Router Advertisement comes back about 0.45 s later. The node then waits
a random 0.90 s before it sends the Neighbor Solicitation that checks its
link-local address, and 1 s more for an answer. Right after that check, a
second Neighbor Solicitation checks the new care-of address, again after a
random delay (0.15 s), and again 1 s passes in silence. The Router Solicitation
in between, at 20.995 s, is the node restarting router discovery, and the
Router Advertisement at 19.353 s is a periodic one. The strip below the chart
marks the four waits. The end of a check is not a message, so no arrow marks
it. The second Router Solicitation and Neighbor Solicitation labels in the
``apForeign`` band are not second messages: they are the access point relaying
the multicast copy back into its cell.

**Registration, and replies held back.** This chart is taken from the
``BidirectionalTunneling`` run; with the messages shown here, the
``RouteOptimization`` run looks the same. The bracket marks the home agent's
duplicate address detection on the home address, and the red bars mark how long
the mobile node holds each of the two replies. The time axis is not linear
here: busy stretches get more room than idle ones, so the bracket's label gives
the true length of the check:

.. figure:: media/seqchart-p2-registration.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   BidirectionalTunneling, seed-set 1, --record-eventlog=true
             --eventlog-recording-intervals=21.8s..23.1s,51.3s..52.1s
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-ba7c.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range. In NONLINEAR mode the zoom
             only sets the left edge; the scale follows the event count, the panel width is set with
             resize_window, and the right edge ends at the last filtered event.
   axes:     mobileNode, apForeign, foreignRouter, backbone, homeAgent, correspondentNode
   filter:   message_names "Binding Update", "Binding Acknowledgement", ping42, ping42-reply, ping43,
             ping43-reply
   view:     NETWORK_COMMUNICATION, NONLINEAR; resize_window 1400x1000 (widget 1160x578);
             goto_event 12939, zoom 21.895..22.9655
   anchor:   BU leaves the MN 21.900354 (#12940, x 24), at homeAgent 21.930018 (#13024, x 151); the home
             agent's DAD probe of the home address at once, 21.930018 (not drawn; home link); ping42 leaves the CN
             22.000 (x 205), reaches the MN via homeAgent 22.037710 (x 407.5), reply held; ping43 leaves the
             CN 22.500 (x 481), at the MN 22.519974 (x 633.5), reply held; BA leaves homeAgent 22.930018
             (#13594, x 706), at the MN 22.943581 (x 801.5); held replies leave at 22.943581 (x 861 / 920)
             through the reverse tunnel; at the CN 22.963943 (x 1107) and 22.965183 (x 1146)
   capture:  screenshot 1160x578 (raw: seqchart-raw-ba7c/p2.png) -> crop (0,62)-(1160,556) + tick strip
             -> 1160x571. The last labels at the right edge are cut (the last filtered event ends the view).
   post:     magenta bracket on the homeAgent lifeline 151-706, bar at cropped y 425, two-line label "home
             agent checks the / home address: 1.000 s"; red hold bars above the mobileNode axis:
             407.5-861 at y 20 "ping42 reply held 0.906 s" (label left) and 633.5-920 at y 44 "ping43
             reply held 0.424 s" (label right). Ticks 21.900, 21.930, 22.000, 22.038, 22.500, 22.520,
             22.930, 22.944, 22.964, 22.965.
             Composition script research/analyst-data/pnba7c.py (helpers compose.py; raw screenshots
             in research/analyst-data/seqchart-raw-ba7c/): run from analyst-data/ with an outba7c/
             directory. Overlay font DejaVu Sans 17 px. The IDE's rulers are cut off; the IDE's dotted
             cursor column is removed (decursor()); an absolute-time strip gives a tick at each anchor
             event's x (read from the arrow ends in the screenshot), labelled with the event's time, and
             the note "time [s] at the marked events; the axis between them is not linear".
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre

At the left edge the *Binding Update* descends from the mobile node (top
lifeline) through ``apForeign``, ``foreignRouter`` and ``backbone`` to
``homeAgent``. No acknowledgement follows it at once: the home agent first
checks the home address with duplicate address detection on the home link — a
Neighbor Solicitation at once, then 1 s of silence — and sends the *Binding
Acknowledgement* when the check ends, at 22.930 s.

Meanwhile ``ping42`` and ``ping43`` reach the mobile node through the
home-agent detour — their arrows bend at the ``homeAgent`` lifeline — but **no
reply travels back yet**. Until the binding is active the mobile node holds its
own home-address-sourced replies, the same implementation note as before. (The
pings sent between 20.0 s and 21.5 s never reach the mobile node: the home
agent had no binding for them yet.) After the bracket, the *Binding
Acknowledgement* travels to the mobile node and activates the binding: one
Binding Update, one acknowledgement. At that instant, 22.944 s, the node sends
both held replies through the reverse tunnel, back through the ``homeAgent``
lifeline; the reply to ``ping42`` is the first one to arrive, at 22.964 s.

**The care-of test.** In the ``RouteOptimization`` run, the arrival of
``ping42`` at 22.038 s also starts return routability. The node sends the Home
Test Init (HoTI) and the Care-of Test Init (CoTI) at the same time. The Home
Test Init has the home address as its source, so the node holds it, like the
ping replies; the red marker shows where:

.. figure:: media/seqchart-p3-careoftest.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   RouteOptimization, seed-set 1, --record-eventlog=true
             --eventlog-recording-intervals=21.8s..23.6s,51.3s..52.1s
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-ba7c.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range. In NONLINEAR mode the zoom
             only sets the left edge; the scale follows the event count, the panel width is set with
             resize_window, and the right edge ends at the last filtered event.
   axes:     mobileNode, apForeign, foreignRouter, backbone, homeAgent, correspondentNode
   filter:   message_names CoTI, CoT, ping42
   view:     NETWORK_COMMUNICATION, NONLINEAR; resize_window 1250x1000 (widget 1036x578);
             goto_event 13065, zoom 21.9995..22.0575 (a zoom to 22.058 cuts the CoT's last hop)
   anchor:   ping42 leaves the CN 22.000 (#13065, x 22), bends at homeAgent, reaches the MN 22.037710
             (#13214, x 463.5); HoTI (#13218) held, CoTI (#13219) leaves at the same instant (x 561);
             CoTI at the CN 22.047638 (#13285, x 757.5); CoT at the MN 22.057252 (#13327, x 987.5).
             CoTI and CoT do not touch homeAgent.
   capture:  screenshot 1036x578 (raw: seqchart-raw-ba7c/p3.png) -> crop (0,62)-(1036,556) + tick strip
             -> 1036x550
   post:     red "held" marker (stem + pause sign) on the mobileNode axis at x 561, label left "HoTI held
             until the Binding Acknowledgement at 22.944 s". Ticks 22.000, 22.0377, 22.0476, 22.0573.
             Composition script research/analyst-data/pnba7c.py (helpers compose.py; raw screenshots
             in research/analyst-data/seqchart-raw-ba7c/): run from analyst-data/ with an outba7c/
             directory. Overlay font DejaVu Sans 17 px. The IDE's rulers are cut off; the IDE's dotted
             cursor column is removed (decursor()); an absolute-time strip gives a tick at each anchor
             event's x (read from the arrow ends in the screenshot), labelled with the event's time, and
             the note "time [s] at the marked events; the axis between them is not linear".
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre

The *Care-of Test Init* and the *Care-of Test (CoT)* run straight between the
two nodes, without touching ``homeAgent``. The care-of half of the test is
finished, while the home half has not left the mobile node.

**The home test, and the correspondent registration.** The node sends the held
Home Test Init when the Binding Acknowledgement arrives, at 22.944 s, together
with the held replies. Now the binding is active, so the message goes through
the reverse tunnel:

.. figure:: media/seqchart-p4-hometest.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   RouteOptimization (same eventlog as P3)
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-ba7c.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range. In NONLINEAR mode the zoom
             only sets the left edge; the scale follows the event count, the panel width is set with
             resize_window, and the right edge ends at the last filtered event.
   axes:     mobileNode, apForeign, foreignRouter, backbone, homeAgent, correspondentNode
   filter:   message_names HoTI, HoT, "Binding Update", "Binding Acknowledgement"
   view:     NETWORK_COMMUNICATION, NONLINEAR; resize_window 1330x1000 (widget 1102x578);
             goto_event 13704, zoom 22.929..23.0035
   anchor:   the home agent's BA leaves homeAgent 22.930018 (#13704, x 22), at the MN 22.943581 (#13740,
             x 148.5); the held HoTI leaves at the same instant (#13745, x 210), bends at homeAgent
             22.957738 (#13909, x 352), at the CN 22.963751 (#13937, x 423.5); HoT via homeAgent at the MN
             22.983549 (#14030, x 666.5); BU to the CN leaves at once (#14033, x 729), at the CN
             22.993650 (#14096, x 856.5); the CN's BA at the MN 23.003281 (#14151, x 1004.5). No HoTI
             retransmission. The MN-side "Binding Acknowledgement" label is cut at the right edge.
   capture:  screenshot 1102x578 (raw: seqchart-raw-ba7c/p4.png) -> crop (0,62)-(1102,556) + tick strip
             -> 1102x550
   post:     red "held" marker at x 210, label right "the held HoTI leaves with the Binding
             Acknowledgement". Ticks 22.930, 22.944, 22.958, 22.964, 22.984, 22.994, 23.003.
             Composition script research/analyst-data/pnba7c.py (helpers compose.py; raw screenshots
             in research/analyst-data/seqchart-raw-ba7c/): run from analyst-data/ with an outba7c/
             directory. Overlay font DejaVu Sans 17 px. The IDE's rulers are cut off; the IDE's dotted
             cursor column is removed (decursor()); an absolute-time strip gives a tick at each anchor
             event's x (read from the arrow ends in the screenshot), labelled with the event's time, and
             the note "time [s] at the marked events; the axis between them is not linear".
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre

The *Home Test Init* and the *Home Test (HoT)* both bend at the ``homeAgent``
line: the home half of the test travels through the tunnel, as it must. The
Home Test reaches the mobile node at 22.984 s, and return routability is
complete. At the same instant the *Binding Update* goes straight to the
correspondent node, and its *Binding Acknowledgement* comes back at 23.003 s.
(INET requests an acknowledgement on every Binding Update; asking is the mobile
node's choice, but a correspondent node that is asked must answer.) Route
optimization completes after ``ping43`` has left the correspondent node at 22.5
s, but the correspondent node has its binding from 22.994 s, so ``ping44``,
sent at 23.0 s, already takes the direct path.

**Route optimization takes effect.**

.. figure:: media/seqchart-p5-direct.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   RouteOptimization (same eventlog as P3)
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-ba7c.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range. In NONLINEAR mode the zoom
             only sets the left edge; the scale follows the event count, the panel width is set with
             resize_window, and the right edge ends at the last filtered event.
   axes:     mobileNode, apForeign, foreignRouter, backbone, homeAgent, correspondentNode
   filter:   message_names ping43-reply, ping44, ping44-reply, ping45, ping45-reply (ping43-reply keeps the
             homeAgent axis on the chart: an axis without filtered events is hidden)
   view:     NETWORK_COMMUNICATION, NONLINEAR; resize_window 1330x1000 (widget 1102x578);
             goto_event 13923, zoom 22.960..23.5205
   anchor:   left edge: the last tunneled reply (ping43-reply) bends at homeAgent and reaches the CN
             22.966311 (x 93.5); ping44 leaves the CN 23.000 (x 190), at the MN 23.009885 (#14202,
             x 343.5), reply at the CN 23.020218 (#14271, x 583.5); ping45 leaves 23.500 (x 686), at the
             MN 23.509885 (#14457, x 839.5), reply at the CN 23.520078 (#14523, x 1077.5). ping44 and
             ping45 cross the homeAgent band without bending.
   capture:  screenshot 1102x578 (raw: seqchart-raw-ba7c/p5.png) -> crop (0,62)-(1102,556) + tick strip
             -> 1102x550
   post:     no marks; ticks 22.966, 23.000, 23.010, 23.020, 23.500, 23.510, 23.520.
             Composition script research/analyst-data/pnba7c.py (helpers compose.py; raw screenshots
             in research/analyst-data/seqchart-raw-ba7c/): run from analyst-data/ with an outba7c/
             directory. Overlay font DejaVu Sans 17 px. The IDE's rulers are cut off; the IDE's dotted
             cursor column is removed (decursor()); an absolute-time strip gives a tick at each anchor
             event's x (read from the arrow ends in the screenshot), labelled with the event's time, and
             the note "time [s] at the marked events; the axis between them is not linear".
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre

From ``ping44`` onward the arrows run **directly between correspondent and
mobile node** — no arrow bends at the ``homeAgent`` lifeline any more. The
arrows merely *cross* its band on the way past, which is the visual difference
between a packet the home agent forwards and one that simply passes its
position on the chart. That is route optimization in one glance.

Inside the packets
~~~~~~~~~~~~~~~~~~

The two forwarding modes are distinguishable inside a single packet. Below are
the two IPv6 header chunks of a tunneled ping request, captured on the home
agent's backbone link and expanded field by field in Qtenv's object inspector:
**two stacked IPv6 headers** — the outer one from the home agent
(``2001:db8:0:1:...:1``) to the care-of address (``2001:db8:0:3:...:d``) with
``protocol = ipv6``, carrying the untouched inner packet from the correspondent
to the *home* address, 40 bytes of overhead in all. Notice that the two
destination addresses share their interface identifier (``8aa:ff:fe00:d``)
under different prefixes — the identity/location split, visible inside one
packet. (The numbers after the protocol names in the figure, like ``ipv6(40)``,
are INET's internal protocol identifiers, not the IANA protocol numbers; the
``protocolId`` fields, 41 and 58, are the genuine wire values.)

.. figure:: media/tunneled_packet.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   BidirectionalTunneling
   seed:     default (seed-set=1)
   shows:    IPv6-in-IPv6 on the home agent's backbone link: chunks[2] = outer
             Ipv6Header (2001:db8:0:1:8aa:ff:fe00:1 -> care-of 2001:db8:0:3:8aa:ff:fe00:d,
             protocol ipv6(40), protocolId 41) and chunks[3] = inner Ipv6Header
             (correspondent 2001:db8:0:5:8aa:ff:fe00:8 -> home 2001:db8:0:1:8aa:ff:fe00:d,
             protocol icmpv6(18), protocolId 58), each with its fields
   launch:   Qtenv with the sim's own MCP server: opp_run_release -l <wt>/src/INET -u Qtenv
             -c BidirectionalTunneling -n <wt>/src:<wt>/showcases
             '--*.visualizer.osgVisualizer.typename=""' --mcp-server-address localhost:8777
             --qtenv-default-run=0 omnetpp.ini   (port 8765 is taken by the showcase
             previewer; the OSG override is needed only while VisualizationOsg is off)
   target:   run_simulation mode=express time_limit=25.4, then mode=fast time_limit=25.52
             (stops at 25.519985, event #15352; was #14747) -> list_logged_packets
             module_path=Mipv6Showcase.homeAgent name_pattern=ping49* -> the 170 B
             EthernetSignal "ping49" sent by homeAgent.eth[1] at 25.5060208 (event #15293; was #14688),
             path logged:<id> (was logged:12223; 2026-09-30: logged:40780)
   anchor:   tunneled pings are 170 B on the HA-backbone wire (130 B + 40 B outer header)
             throughout the away phase; chunks[1] EthernetMacHeader src 0A-AA-00-00-00-02 ->
             dst 0A-AA-00-00-00-05. A single Ipv6Header = the tunnel was not up.
   capture:  open_inspector type=object -> expand_inspector_tree depth=5 ->
             get_inspector_screenshot width=1400 height=4400 -> PIL-crop (60,1776)-(800,2567):
             the chunks[2] and chunks[3] Ipv6Header rows with all their fields; 740x791.
             depth=5 unfolds raw bin/raw hex above the chunks; the crop starts below them.
             The row headers are cut at the right edge on purpose (the fields below repeat them).
   stamp:    captured 2026-09-30, INET HEAD 8c94b616cd, OMNeT++ 6.4.0aipre2 (chunk ids now 11261 / 11260; only the two "id =" rows differ from the 2026-09-29 image)
             re-verified 2026-10-01 at ba7c6038bf: re-captured, pixel-identical (same chunk ids); image kept.

The same ping in the route-optimized mode, captured at the correspondent node
and expanded the same way: **one** IPv6 header, addressed to the care-of
address directly, followed by a *type 2 routing header* (``extensionType =
43``, ``routingType = 2``, ``segmentsLeft = 1``) whose address field —
collapsed here, but opened in the Wireshark dissection below — carries the home
address: 24 bytes instead of 40, and no detour. (Replies in the other direction
carry the home address in a *Home Address destination option* instead; not
shown.)

.. figure:: media/ropacket.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   RouteOptimization
   seed:     default (seed-set=1)
   shows:    direct-path ping on the correspondent node's wire: chunks[2] = a single Ipv6Header
             (correspondent 2001:db8:0:5:8aa:ff:fe00:8 -> care-of 2001:db8:0:3:8aa:ff:fe00:d,
             chunkLength 40 B, payloadLength 88 B, hopLimit 30, protocolId 43) and chunks[3] =
             Ipv6RoutingHeader (chunkLength 24 B, nextHeaderProtocol 58, routingType 2,
             segmentsLeft 1, address[1] collapsed)
   launch:   Qtenv with the sim's own MCP server: opp_run_release -l <wt>/src/INET -u Qtenv
             -c RouteOptimization -n <wt>/src:<wt>/showcases
             '--*.visualizer.osgVisualizer.typename=""' --mcp-server-address localhost:8777
             --qtenv-default-run=0 omnetpp.ini   (8765 is taken by the showcase previewer)
   target:   run_simulation mode=express time_limit="25.4", then mode=fast time_limit="25.52"
             (stops at 25.519886, event #15579) -> list_logged_packets
             module_path=Mipv6Showcase.correspondentNode name_pattern=ping49* -> the 154 B
             EthernetSignal "ping49" sent by correspondentNode.eth[0] at 25.5 (event #15470),
             path logged:<id> (2026-09-30: logged:53815). Not ping49-reply: that is also 154 B, but it
             carries a Home Address destination option instead of the routing header.
   anchor:   route-optimized pings are 154 B on the CN wire (130 B + 24 B type 2 routing header)
             from ping44 on. The chunk ids shown (Ipv6Header id = 11403, Ipv6RoutingHeader
             id = 11402) are run-specific; if they change, the model or the run changed. A single
             Ipv6Header without a routing header = route optimization did not complete.
   capture:  open_inspector object_path=logged:<id> type=object -> expand_inspector_tree depth=5
             -> get_inspector_screenshot width=1400 height=4400 -> PIL-crop (60,1688)-(800,2358):
             740x670, the same crop as the 2026-08 image. address[1] stays collapsed at depth=5;
             depth=6 opens it but also unfolds the hex dumps and shifts every row.
   compare:  against the 2026-08 image the only differing pixels are the two "id =" rows
             (was 11202 / 11201); every address, length and field is identical.
   stamp:    captured 2026-09-30, INET HEAD 8c94b616cd, OMNeT++ 6.4.0aipre2 (was ids 11080 / 11079)
             re-verified 2026-10-01 at ba7c6038bf: re-captured, pixel-identical (same chunk ids); image kept.

The same two kinds of packet, off the wire
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

INET can also write a packet capture (PCAP) file, so the same two kinds of
packet can be handed to Wireshark. The captured pings are different ones
(``ping42`` tunneled, ``ping44`` route-optimized, where the inspector showed
``ping49``), but their header fields are the same. That is worth doing as a
cross-check: Wireshark knows nothing about INET and dissects the recorded bytes
on their own terms, so whatever it reports is a property of the packet rather
than of the simulator's own view of it.

.. figure:: media/tunneled_packet_wireshark.png
   :align: center

..
   FIGURE RECIPE (redo with INET's PcapRecorder + the Wireshark GUI)
   type:     wireshark GUI screenshot of the packet-detail pane
   config:   BidirectionalTunneling
   seed:     default (seed-set=1)
   shows:    Frame 1: 162 bytes on wire; Ethernet II 0a:aa:00:00:00:02 -> 0a:aa:00:00:00:05
             (homeAgent -> backbone); outer IPv6 2001:db8:0:1:8aa:ff:fe00:1 -> care-of
             2001:db8:0:3:8aa:ff:fe00:d, Payload Length 104, Next Header IPv6 (41), Hop Limit 30;
             inner IPv6 2001:db8:0:5:8aa:ff:fe00:8 -> home 2001:db8:0:1:8aa:ff:fe00:d, Payload
             Length 64, Next Header ICMPv6 (58), Hop Limit 28; ICMPv6 collapsed
   pcap:     opp_run_release -l <wt>/src/INET -u Cmdenv -c BidirectionalTunneling
             -n <wt>/src:<wt>/showcases '--*.visualizer.osgVisualizer.typename=""'
             '--*.homeAgent.numPcapRecorders=1'
             '--*.homeAgent.pcapRecorder[0].pcapFile="<dir>/tunneled.pcap"'
             '--*.homeAgent.pcapRecorder[0].fileFormat="pcap"'
             '--**.fcsMode="computed"' '--**.crcMode="computed"' '--**.checksumMode="computed"'
             omnetpp.ini   (the computed modes are required: with the default declared FCS the
             recorder aborts and writes an empty file)
   frame:    ping42 = frame 189, relative 21.514055 s (sim 22.006): the first tunneled echo
             request since 8c94b616cd (tshark -Y 'ipv6.nxt==41 && icmpv6.type==128' -> first match;
             ping38-ping41 are never tunneled any more: the home agent has no binding before 21.930 s).
             Cut it out: editcap -r tunneled.pcap one.pcap 189
   gui:      Xephyr :77 -screen 1500x1150 -ac -noreset &
             DISPLAY=:77 QT_QPA_PLATFORM=xcb wireshark -r one.pcap
             (the desktop is Wayland; XWayland refuses synthetic input, so drive a nested X
             server; set gui.byte_view_show and gui.packet_diagram_show to false in the profile's
             "recent" file so the detail tree gets the full width)
   expand:   window 1500x900; click the first tree row, then per header Home, Down x N, Right,
             N = 3 then 2 (the inner IPv6 header first, so the outer row does not move)
   capture:  import -window <id>, crop (0,487)-(772,836): 772x349
   compare:  2026-09-30, not recaptured: tshark -V of frame 189 (ping42, current run, pcap in
             /var/tmp/update6/pcap/) and of the frame behind the image (old pcap frame 159, ping39) are
             identical line for line from "Ethernet II" to "Internet Control Message Protocol" except
             the Frame Check Sequence, which the shot does not show; the image still shows the current
             run.
   stamp:    captured 2026-08; verified against INET HEAD 8c94b616cd, OMNeT++ 6.4.0aipre2,
             Wireshark 4.6.4
             re-verified 2026-10-01 at ba7c6038bf: regenerated pcap, the same frame (189 / 100) and tshark -V
             identical line for line including the FCS; image kept.

Wireshark independently finds the two stacked IPv6 headers — outer from the
home agent to the care-of address with ``Next Header: IPv6 (41)``, inner from
the correspondent to the home address with ``Next Header: ICMPv6 (58)``. These
are the IANA protocol numbers, the same ones the object inspector above shows
in its ``protocolId`` fields; only the numbers in parentheses after its protocol
names are INET's own identifiers.

.. figure:: media/ropacket_wireshark.png
   :align: center

..
   FIGURE RECIPE (redo with INET's PcapRecorder + the Wireshark GUI)
   type:     wireshark GUI screenshot of the packet-detail pane
   config:   RouteOptimization   # ../omnetpp.ini
   seed:     default (seed-set=1)
   pcap:     opp_run_release ... -c RouteOptimization
             --"*.correspondentNode.numPcapRecorders=1"
             --'*.correspondentNode.pcapRecorder[0].pcapFile="results/routeopt.pcap"'
             --'*.correspondentNode.pcapRecorder[0].fileFormat="pcap"'
             --'**.fcsMode="computed"' --'**.crcMode="computed"'
             --'**.checksumMode="computed"'
             The computed modes are required (declared FCS -> empty file).
   frame:    tshark -Y 'ipv6.routing.type==2 && icmpv6.type==128' -> first match =
             frame 100 = ping44 (sent at sim 23.000, capture-relative 22.149633; since 8c94b616cd
             ping41 is lost and ping44 is the first route-optimized request);
             editcap -r routeopt.pcap frame.pcap 100  (one-frame file, shows as "Frame 1")
   gui:      Xephyr :77 -screen 1500x1150 -ac -noreset &
             env -u WAYLAND_DISPLAY QT_QPA_PLATFORM=xcb DISPLAY=:77 HOME=<tmp>
             XDG_CONFIG_HOME=<tmp>/.config wireshark -r frame.pcap
   layout:   <tmp>/.config/wireshark/recent: gui.byte_view_show: FALSE,
             gui.packet_diagram_show: FALSE (the geometry keys there are ignored with a
             warning; the window comes up 1500x900 at 0,0 anyway)
   expand:   XTest via ctypes (libXtst): click the first detail row (200,493), then
             Home, Down x2, Right (IPv6 header), Home, Down x12, Right (routing header).
   capture:  import -window root on :77, crop (0,487)-(772,806); 772x319
   anchor:   one IPv6 root with Next Header: Routing Header for IPv6 (43), destination
             2001:db8:0:3:8aa:ff:fe00:d (care-of) and Address[1]: 2001:db8:0:1:8aa:ff:fe00:d
             (home); [Length: 24 bytes] -- the overhead the page quotes. The Frame row
             reads 146 bytes on wire (kept in the crop; the page cites it).
             No routing header = route optimization did not complete.
   compare:  2026-09-30, not recaptured: tshark -V of frame 100 (ping44, current run) and of frame
             97 (ping41) of the aeee20a40d run differ only in the Frame Check Sequence, which the shot
             does not show.
   stamp:    captured 2026-09-29 (aeee20a40d); verified against INET HEAD 8c94b616cd, Wireshark 4.6.4
             re-verified 2026-10-01 at ba7c6038bf: regenerated pcap, the same frame (189 / 100) and tshark -V
             identical line for line including the FCS; image kept.

And here the routing header gives up the field the object inspector kept
collapsed: ``Address[1]: 2001:db8:0:1:8aa:ff:fe00:d`` — the home address,
carried alongside a destination of ``2001:db8:0:3:8aa:ff:fe00:d``, the care-of
address. One packet, both halves of the identity/location split, and no home
agent anywhere on its path.

One detail to reconcile: Wireshark reports these frames as 162 and 146 bytes,
eight fewer than the 170 B and 154 B the simulation reports for the same kinds
of packet. INET counts the Ethernet preamble and start-of-frame delimiter,
which a capture file does not store.

Meanwhile the home agent's binding cache holds exactly one entry — the mapping
this whole protocol exists to maintain. The 3600 s lifetime is the
home-registration default (``maxHaBindingLifeTime``) — unlike the seven-minute
correspondent bindings, home bindings are long-lived. INET refreshes one 2 s
before it expires, a fixed value, where the standard suggests refreshing well
before; in this 80 s run no refresh happens. And the sequence number is 1: one
Binding Update, answered by one acknowledgement:

.. figure:: media/bindingcache.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   BidirectionalTunneling (RouteOptimization looks the same)
   seed:     default (seed-set=1)
   shows:    homeAgent.ipv6.bindingCache with one entry: HoA 2001:db8:0:1:8aa:ff:fe00:d =>
             CoA 2001:db8:0:3:8aa:ff:fe00:d, lifetime 3600, home registration, BU sequence 1
   launch:   Qtenv with the sim's own MCP server (--mcp-server-address localhost:8777; setup_config
             config_name=BidirectionalTunneling run_number=0 switches an open session)
   target:   run_simulation mode=express time_limit=22.5 (stops at 22.480866, event #13365): inside the
             home agent's duplicate address detection (entry created at the BU 21.930018, BA only at
             22.930018) -> open_inspector Mipv6Showcase.homeAgent.ipv6.bindingCache type=object ->
             expand_inspector_tree depth=4. The entry is the same before and after the BA.
   anchor:   exactly 1 entry, BU_Sequence#: 1, from 21.930018 (BU received, #13025) until 51.471164
             (BT de-registration). Sequence 2 = the BU was retransmitted (none of the ten seeds at
             ba7c6038bf).
   capture:  get_inspector_screenshot width=1300 height=900 -> PIL-crop (14,370)-(830,430): 816x60.
             Pixel-identical to the 2026-09-29 image, which is therefore kept unchanged.
   stamp:    verified 2026-09-30, INET HEAD 8c94b616cd, OMNeT++ 6.4.0aipre2
             re-verified 2026-10-01 at ba7c6038bf: re-captured at 22.48 s; same text, only the tree's
             expand-arrow shade differs (UI focus state); image kept.

Coming home
~~~~~~~~~~~

The second video shows the return (t = 49.5 s to 54.6 s), still in the
``RouteOptimization`` configuration: direct-path pings, the walk home, and —
0.08 s after re-association, when the home agent's Router Advertisement arrives
— the de-registration Binding Updates (lifetime zero) to the home agent and the
correspondent node. The status label returns to "at home" and the pings to the
14 ms home path; the address label has shown the home address, the node's
identity, all along. The binding cache empties, and the node is an ordinary
IPv6 host again.

.. video:: media/returnhome.mp4
   :align: center

..
   VIDEO RECIPE (redo via the "video-recording" skill)
   config:   RouteOptimization
   seed:     default (seed-set=1)
   shows:    last direct pings (red route through foreignRouter) -> the walk home -> AP label
             FOREIGN -> HOME ("Associated with AP" bubble) and almost at once (0.08 s) status "at home" ->
             ping101 and the following pings on the home path through homeAgent
   anchors:  last reply away ping98 at 50.020 s; beacon loss 50.721 s; association with apHome
             51.372 s; the HA's solicited RA leaves 51.453 s after the random 0.081 s (NOT held: last
             home-link multicast RA 47.847 s); de-registration BUs to HA and CN 51.454 s, BAs at the MN
             51.457 / 51.469 s; first home reply ping101 at 51.514 s. The address label shows the home
             address throughout. If the status stays "away" for 2-3 s after the association, the
             Router Advertisement is held by the 3 s multicast limit again (timeline moved).
   window:   same Qtenv session after handover.mp4: express to 49.5 s -> step 1 event -> record to
             54.6 s in one go (about 3 s of home-path pings after the return).
   launch:   private prefs copy so the user's Qtenv prefs stay untouched: XDG_CONFIG_HOME=
             /var/tmp/mipv6-video/xdg (omnetpp/.qtenvrc copied from ~/.config/omnetpp/.qtenvrc with
             animation_enabled=false); opp_run_release -l <wt>/src/INET -u Qtenv -c RouteOptimization
             -n <wt>/src:<wt>/showcases '--*.visualizer.osgVisualizer.typename=""'
             --mcp-server-address localhost:8777 --qtenv-default-run=0 omnetpp.ini
   anim:     set_animation_parameters profile=normal playback_speed=1 min_animation_speed=0.1
             (0.05 s simulation time per frame at fps=2; encoded at 6 fps = 0.3 s per video second)
   capture:  record_video fps=2, crop_area=with_padding, time_limit 54.6; 102 frames (0142-0243);
             frames kept in /var/tmp/final/video/frames_returnhome
   encode:   ffmpeg -r 6 -start_number 142 -f image2 -i <prefix>_%04d.png -filter:v "crop=830:684:923:123"
             -vcodec libx264 -pix_fmt yuv420p <name>.mp4  (the crop keeps only the canvas interior,
             x 923-1752, y 123-806. The Qtenv window position varies between launches -- re-measure the
             black canvas border on a frame before encoding: 2026-10-01 crop_rect was 854x732 at 911,87,
             border columns 921-922 / 1753-1754); 830x684, fits the 834 px column without scaling
   post:     none
   stamp:    recorded 2026-10-01, INET HEAD ba7c6038bf, OMNeT++ 6.4.0aipre2

Why the return is short: the home agent answers the node's Router Solicitation
with a multicast Router Advertisement, as INET's routers always do, and a
router may multicast at most one Router Advertisement every 3 s. In this run
its last one, at 47.847 s, was more than 3 s earlier, so the answer leaves
after its random delay only, 0.08 s, at 51.453 s. The node then skips duplicate
address detection — it must not probe its own home address while its binding is
alive — and the home agent acknowledges the de-registration at once. So the
return is far shorter than the way out: 1.49 s with route optimization and 1.47
s with bidirectional tunneling, against 5.95 s, the shortest returns of the ten
seeds. In most other seeds the last multicast advertisement was less than 3 s
old, and the limit held the answer back by up to 3 s; that is what spreads the
returns over 1.5–5.0 s across ten seeds (1.5–4.5 s with bidirectional
tunneling).

Here is the return in the ``RouteOptimization`` run as a sequence chart. It
shows only the signaling and the first ping answered at home; the pings that
still go to the care-of address are left out. The bracket marks the home
agent's random delay before its Router Advertisement, 0.08 s, which no limit
holds back here:

.. figure:: media/seqchart-p6-return.png
   :align: center
   :width: 100%

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   RouteOptimization, seed-set 1, --record-eventlog=true
             --eventlog-recording-intervals=21.8s..23.6s,51.3s..52.1s
   ide:      OMNeT++ IDE (omnetpp-aipre-GUI, MCP server on 127.0.0.1:5077), started with
             ~/omnetpp-aipre-GUI/ide/opp_ide -data ~/omnetpp-aipre-GUI/samples. The .elog must be
             inside a workspace project: copied to /home/user/inet/tmp-mipv6-seq/<Config>-ba7c.elog
             (project "inet") and opened by absolute path.
   nav:      goto_event <first event> BEFORE zoom_to_simulation_time_range; NONLINEAR right edge = last
             filtered event.
   axes:     mobileNode, apHome, homeAgent, backbone, correspondentNode
   filter:   message_names RouterSolicitation, RouterAdvertisement, "Binding Update", "Binding
             Acknowledgement", NeighbourAdvertisement, ping101, ping101-reply, ping102 (ping102 only
             extends the view; it is cropped off)
   view:     NETWORK_COMMUNICATION, NONLINEAR; resize_window 1400x1100 (widget 1160x649);
             goto_event 29590, zoom 51.3715..52.01
   anchor:   RS leaves the MN 51.371929 (#29590), at homeAgent 51.372709 (#29625, x 52.5); the solicited
             RA leaves homeAgent 51.453272 (#29684, x 129) after the random 0.081 s -- NOT held: the last
             home-link multicast RA was at 47.847 s, more than 3 s earlier; RA at the MN 51.454035 (#29706,
             x 158.5); two de-registration BUs in that event (#29710/#29711); BU at homeAgent 51.454857
             (#29736); HA BA at the MN 51.456814 (#29810, x 360.5); BU at the CN 51.461970 (#29882, x 501);
             the CN's BA reaches the MN via homeAgent 51.468601 (#29929, x 602.5); ping101 leaves the CN
             51.500 (#29962, x 665), reply at the CN 51.513875 (#30076, x 875). If a bracket of ~2-3 s
             appears, the Router Advertisement is held again (the 3 s multicast limit).
   capture:  screenshot 1160x649 (raw: seqchart-raw-ba7c/p6.png) -> crop (0,62)-(920,627): drops the
             IDE rulers and the ping102 arrows; + tick strip -> 920x642
   post:     magenta bracket BELOW the homeAgent axis from 52.5 to 129, bar at cropped y 342, label
             left-aligned under it "RA delay 0.08 s (not held)". Ticks 51.373, 51.453, 51.457, 51.462,
             51.469, 51.500, 51.514.
             Composition script research/analyst-data/pnba7c.py (helpers compose.py; raw screenshots
             in research/analyst-data/seqchart-raw-ba7c/), run from analyst-data/ with an outba7c/
             directory. Overlay font DejaVu Sans 17 px; IDE cursor column removed (decursor()).
   stamp:    captured 2026-10-01, INET HEAD ba7c6038bf (topic/gy/mipv6-showcase), OMNeT++ IDE 6.4.0aipre

The Router Solicitation reaches ``homeAgent`` right after the association, and
the Router Advertisement follows 0.08 s later. After it, the two
de-registration Binding Updates leave the node's IPv6 layer in the same
instant; on the chart they leave the ``mobileNode`` line one after the other,
because the radio sends the frames in turn. Then both acknowledgements come
back within milliseconds, the correspondent node's through the home agent. The
last arrows are the round trip of ``ping101`` on the home path.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Mipv6Showcase.ned <../Mipv6Showcase.ned>`,
:download:`movement.xml <../movement.xml>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, navigate to the
``inet/showcases/ipv6/mipv6`` folder in the `Project Explorer`, and double-click the
``omnetpp.ini`` file to open it. Select a configuration and press **Run**.

If you don't have INET and OMNeT++ installed, you can quickly set them up
using `opp_env <https://omnetpp.org/opp_env>`__, and run the simulation
interactively. Ensure that ``opp_env`` is installed on your system, then
execute:

.. code-block:: bash

    $ opp_env run inet-4.7 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.7.*/showcases/ipv6/mipv6 && inet'

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

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.

.. TODO: create the tracker issue for this showcase and replace the link above

The handover outage in this showcase is set mostly by protocol timers. For a
care-of address, Mobile IPv6 (RFC 6275) prefers duplicate address detection
without a random delay; INET waits a random delay before this probe too, as for
its link-local address (0.15 s in this run).

A measured 802.11 testbed (Cabellos-Aparicio et al., 2005) found a mean Mobile
IPv6 handover of 2.1 s, 87 % of it in the IPv6 phase. That phase, 1.84 s on
average, is shorter than this run's Router Advertisement wait and duplicate
address detection together, 3.49 s. The testbed spent it on duplicate address
detection and on Neighbor Unreachability Detection, which finds out that the
old router no longer answers; this run detects the move from the link layer
instead.

This run's 5.95 s is 3.84 s longer. The largest part of the difference is the
IPv6 phase, 1.66 s longer here, because this run checks both its link-local and
its care-of address, each after a random delay. The home agent's
first-registration exchange takes 1.04 s here, with a real duplicate address
detection, and 4 ms in the testbed. This run counts 0.74 s before the access
point is lost (the first two terms of the budget in the Results), which the
testbed's clock, started at the scan, leaves out, and its scan and association
take 0.65 s against 0.26 s. The remaining 0.01 s is this run's 20 ms for the
held first reply against the testbed's 9 ms for the registration at the
correspondent node.

The 802.11 terms of this scenario depend on its scan settings. Fast roaming
(IEEE 802.11k and 802.11r) changes access points in tens of milliseconds, but
only between access points of one Wi-Fi network, where the network normally
also keeps the node's address and Mobile IPv6 is not needed. A move to a
different network, as here, needs a full scan and association, and in a secured
network also a full authentication, which this scenario leaves out. So the
802.11 terms of a real move can be shorter or longer than here.

Whether a router answers a Router Solicitation by unicast, which the 3 s limit
does not hold back, is a setting: some routers do so by default, others
multicast unless configured. RFC 7772 asks routers to support unicast answers,
and asks networks with many battery-powered devices to turn them on, to save
energy.

Optimizations such as Optimistic Duplicate Address Detection (RFC 4429) let a
node use a new address while the check still runs. This removes the mobile
node's own waits, 1.90 s for the link-local and 1.15 s for the care-of address
here, but not the home agent's 1 s check before a first registration, which the
standard requires separately.

Route optimization does not shorten the outage. The standard lets the Binding
Update to the correspondent node go only after the home agent has acknowledged
the home registration. INET does not check this rule, but it keeps the same
order, because the Home Test Init can leave only through the reverse tunnel,
which the acknowledgement creates. The gain of route optimization is on the
path afterward, 20 ms instead of 40 ms. That gain comes from where the home
agent sits in this network, 5 ms off the backbone; with a home agent close to
the correspondent node, the gain shrinks.

The two modes also cost differently per packet. The tunnel adds at least 40
bytes to every packet, more when the tunnel is protected with IPsec, which this
model leaves out. On a path that carries 1500-byte packets, the node can then
send packets of at most 1460 bytes without fragmentation. Route optimization
adds only 24 bytes, but in IPv6 extension headers, which some networks filter
out.

In practice, the pattern of the ``BidirectionalTunneling`` run, an anchor that
keeps the node's address and a tunnel to wherever the node is, is what mobile
operator networks, enterprise Wi-Fi controllers and Wi-Fi calling use, under
other names and protocols. In mobile operator core networks and enterprise
Wi-Fi controllers, the network, not the node, sends the registration, and the
node keeps its address on the new link, so the Router Advertisement wait and
duplicate address detection of this showcase do not occur there. Wi-Fi calling
is closer to this showcase: the phone first gets a new local address on the
Wi-Fi network, then builds an IPsec tunnel from it to the operator's gateway,
which keeps the phone's inner address. What carries over to all three is the
anchor, the tunnel, its detour and its per-packet overhead.

Route optimization did not spread. It needs support in every correspondent
node, firewalls and filters on some paths drop its headers, and it reveals the
node's location to each correspondent node it route-optimizes with. End hosts
that need their own sessions to survive a change of network more often solve
this in the transport layer, with Multipath TCP or QUIC connection migration;
being reachable at a fixed address for sessions that others start still needs
an anchor.
