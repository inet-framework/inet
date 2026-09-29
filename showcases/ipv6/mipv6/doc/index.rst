Mobile IPv6
===========

Goals
-----

An IPv6 address plays two roles at once: routers treat it as a *location* (the
prefix says which link the node is on), and transport connections treat it as
an *identity* (a connection is pinned to the address pair). When a device moves
to a network with a different prefix, it gets a new, routable address — but
every open connection breaks, and nobody can reach it at the address they know.

Mobile IPv6 (RFC 3775, later RFC 6275) solves this by splitting the two roles
into two addresses, anchored by a *home agent*. This showcase demonstrates the
whole mechanism in one scenario: a wireless node moves from its home network to
a foreign one and back, while a peer keeps pinging it at its stable address.
Without Mobile IPv6 the session dies; with it, the traffic keeps flowing —
first through a tunnel, then, with route optimization, on the direct path.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/ipv6/mipv6 <https://github.com/inet-framework/inet/tree/master/showcases/ipv6/mipv6>`__

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
- ``maxHaBindingLifeTime`` (default 3600 s) — the ceiling on a home
  registration.
- ``maxRrBindingLifeTime`` (default 420 s) — the ceiling on a binding held at
  a correspondent node.

One level up, ``hasMipv6`` on ``Ipv6NetworkLayer`` decides whether these
modules exist at all.

Neither lifetime expires inside this showcase's 80 second run, but a study of
re-registration reaches the 420 second one first. ``Mipv6`` also emits two
signals a study can record: ``mipv6RoCompleted`` when route optimization
finishes, and ``packetDropped``.

Configuration notes:

- ``hasMipv6`` (on the node's ``ipv6`` submodule) creates or omits the whole
  Mobile IPv6 footprint. The ``WithoutMipv6`` configuration sets it to
  ``false`` on the mobile node, which leaves an ordinary wireless IPv6 host
  and is how this showcase measures what mobility support is worth.
- ``useRouteOptimization`` on the mobile node is what separates two of the
  three configurations: the same scenario runs both ways with this one flag.
- Movement detection does not depend on frequent Router Advertisements: on
  every layer-2 association the IPv6 neighbour discovery module immediately
  sends a Router Solicitation (its ``detectL2Movement`` parameter, default
  ``true``), so the mobile node never waits for a periodic advertisement.
- The mobile node autoconfigures its addresses, so the address configurator
  must leave hosts alone: the network sets ``assignAddressesToHosts = false``
  on the ``Ipv6NetworkConfigurator``, which then assigns addresses and routes
  to routers only. The home agent is recognized from its Router
  Advertisements (via the home-agent flag the standard defines for them),
  which is how the mobile node learns its home agent's address — at home,
  before ever leaving. The standard's remote-discovery mechanisms are not
  modeled, so a mobile node must start the simulation in its home network.

Implementation notes and simplifications, so the simulation is read for what
it is:

- The IPsec protection that RFC 3775 mandates between mobile node and home
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

  .. admonition:: TODO

     Merge this bullet with the next one: the one-second delay described there
     is a consequence of this same missing capability, not a separate
     shortcoming.

     This is gap 1 of MIPV6_IMPLEMENTATION_GAPS.md, and unlike the other gaps
     the page refers to it is **not filed** — no issue, no branch, no fix in
     progress as of 2026-08-27.

     RFC 6275 Section 10.4.1 asks the home agent to impersonate the absent
     mobile node on the home link: claim its home address with duplicate
     address detection, announce the claim with a multicast Neighbor
     Advertisement so neighbors replace their cache entries, and then answer
     Neighbor Solicitations for it. INET does none of the three — there is no
     on-link agent code anywhere in ``src/inet/networklayer/mipv6/``, the only
     "proxy" matches being Proxy Mobile IPv6 message fields. They are missing
     together because they are one capability, which is why the home agent's
     duplicate address detection cannot be added on its own.

     The next bullet's one-second delay is the workaround for the missing
     first step, hardcoded as ``sendTime = existingBinding ? 0 : 1`` at
     ``Mipv6.cc:867`` and applied at ``:656``, which carries its own
     ``// TODO solve the HA DAD problem in a different way``. Fixing this gap
     replaces that literal with a real probe, so the delay would then vary per
     seed the way the mobile node's own duplicate address detection does, and
     the handover budget in the Results section would need re-deriving.
- The home agent delays its first Binding Acknowledgement by one second, as a
  stand-in for the duplicate address detection the standard requires it to
  perform on the home address before answering. The delay itself is not the
  deviation: a compliant home agent waits about as long for real duplicate
  address detection.

  .. admonition:: TODO

     This bullet exists only because the previous one's capability is missing.
     Implementing proxy Neighbor Discovery gives the home agent a real
     duplicate address detection probe, and this bullet is then deleted: the
     wait stops being a stand-in and becomes the timing of a check that
     actually runs. The wait itself does not go away -- INET's duplicate
     address detection costs ``retransTimer`` (1 s) plus a random 0--1 s, so
     it stays in the same range and starts varying per seed -- but it then
     belongs beside the handover budget in the Results section, which is where
     the reader meets it as a number, rather than in a list of things the
     simulation does not do.
- Until that acknowledgement lands the mobile node's reverse tunnel is not up,
  and the packets it sends from its home address — ping replies, and the
  first Home Test Init of route optimization — are dropped instead of being
  queued or tunneled. The node refuses to emit a packet whose home-address
  source would look spoofed outside the home network — its private version of
  the ingress filtering discussed earlier. This is the part that departs from
  the standard, and it costs one or two extra lost pings at the tail of each
  handover outage.
- The mobile node waits 1.5 s for the acknowledgement of a first home
  registration before it retransmits the Binding Update. This is the
  standard's first-registration timeout (InitialBindackTimeoutFirstReg), chosen
  to leave the home agent time for its duplicate address detection. The held
  acknowledgement arrives about one second after the Binding Update, so every
  home registration here takes one Binding Update, and the binding cache
  records sequence number 1.
- After a move, the mobile node runs duplicate address detection on its
  link-local address only, and assigns the care-of address without probing
  it. The standard requires both checks; a node that runs them at the same
  time waits about as long.
- Routers in this network have ICMPv6 Redirect generation disabled
  (``sendRedirects = false``). A router sends a Redirect when it forwards a
  packet back out of the very interface that packet arrived on, to tell the
  sender about a better first hop. Traffic intercepted *toward* the mobile
  node never meets that condition — it is steered into the tunnel before the
  forwarding check. The way back does: after decapsulating a reverse-tunneled
  reply, the home agent forwards the inner packet out of the very interface
  the tunneled packet arrived on, and would send a useless Redirect to the
  mobile node's home address on every reply. A real stack attributes
  decapsulated packets to the tunnel interface instead; the flag stands in for
  that difference.

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
SSID and the mobility state (at home / away / route-optimized), and the green
label on the right is its current IP address. Both update as the
simulation runs. Note that the foreign network's router is a plain
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
listings. The first tunneled ping that reaches the mobile node
triggers the return-routability procedure with the correspondent node, the
Binding Update installs a binding there, and from the next ping onward the
traffic takes the direct path (~20 ms) — until the return home tears
everything down again.

Results
-------

This section follows one run of each configuration in time order. Until the
mobile node has its care-of address, at 19.9 s, the three configurations are
the same simulation. After that point, we follow the run without Mobile IPv6
first, because it shows the problem, and then the two runs with Mobile IPv6.

The pings are named after their ICMPv6 sequence number: the correspondent node
sends ``ping0`` at t = 1 s and one more every 0.5 s, so ``ping<N>`` leaves at
1 + 0.5·N s. All times come from the default random seed. Where a value
depends on the seed, we also give its range over ten runs with different
seeds.

Before the move
~~~~~~~~~~~~~~~

After the start, the mobile node associates with ``apHome``, forms its home
address by stateless address autoconfiguration (SLAAC), and checks it with
duplicate address detection (DAD). The first reply, to ``ping9`` at 5.5 s,
comes this late only because the home agent cannot resolve the home address
while the mobile node is still checking it.

From then on, the pings take the home path: correspondent node, ``backbone``,
``homeAgent``, ``apHome``, mobile node. The round-trip time (RTT) stays at
about 14 ms (median 14.07 ms), as The Model predicts. No Mobile IPv6 message is
sent while the node is at home. The one higher point, 15.98 ms for ``ping19``
at 10.5 s, is not an 802.11 retransmission. It is a probe of Neighbor
Unreachability Detection (NUD), the Neighbor Discovery check that a neighbor is
still reachable, which the mobile node queues just ahead of its reply.

Leaving home
~~~~~~~~~~~~

At 15 s the mobile node starts to move to the foreign network. The last reply
at home arrives at 17.014 s (``ping32``). The node hears the last ``apHome``
beacon at 17.407 s, but it stays associated: it declares the access point lost
only after several missed beacons, at 17.757 s. Meanwhile the home agent still
sends the pings to ``apHome``, which transmits each of ``ping33`` to ``ping37``
seven times and then drops it.

The node then scans both wireless channels. Channel 1 is empty, and channel 2
has ``apForeign``. The scan ends at 18.407 s, and the node is associated with
``apForeign`` at 18.408 s.

The association makes the node send a Router Solicitation (RS) at once.
``foreignRouter`` answers with a Router Advertisement (RA) after a random delay
of 0.457 s: the standard requires a random delay of up to 0.5 s before a
solicited Router Advertisement. The Router Advertisement reaches the node at
18.867 s with the prefix ``2001:db8:0:3::/64``. This is not the home prefix, so
the node has moved. It marks its link-local address and its home address
tentative, and starts duplicate address detection (DAD) with one Neighbor
Solicitation for its link-local address.

The check takes 1 s plus a random 0.057 s. The Mobile IPv6 standard prefers to
skip this random part for a care-of address when something else already
spreads the nodes out in time, as the randomly delayed Router Advertisement
does here; both choices are allowed. As the implementation notes say, INET
probes only the link-local address. Duplicate address detection completes at
19.924 s. In the same event, the node assigns its care-of address,
``2001:db8:0:3:8aa:ff:fe00:d``, and its home address becomes usable again.

Here is this part of the run as a sequence chart, taken from the
``WithoutMipv6`` run. Each horizontal line is one node, each arrow is one hop
of one packet, and the time axis is linear, so the waits appear at their true
length:

.. figure:: media/seqchart-p1-movement.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P1 -- WithoutMipv6, 18.40-19.93 s, LINEAR timeline;
   RS, the delayed RA, the one DAD NS, then silence until "DAD completed".
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p1-movement.txt)

The Router Solicitation goes up to ``foreignRouter`` at once, and the Router
Advertisement comes back almost half a second later. Then comes the single
Neighbor Solicitation of duplicate address detection (DAD), and after it a
second of silence, in which the node may not use its new addresses yet. At
19.924 s all three runs have a working care-of address. This is where they
part.

Without Mobile IPv6
~~~~~~~~~~~~~~~~~~~

Without Mobile IPv6, nothing happens at 19.924 s. The node has a new, working
address, but nobody knows it. The correspondent node keeps sending to the home
address, and the pings keep going to the home link, where nobody answers.

Here is the round-trip time of every ping in the ``WithoutMipv6`` run. The
shaded band is the time the node spends away from home. The three
configurations are plotted separately on identical axes, because at home their
points coincide and would hide one another:

.. figure:: media/pingrtt-without.png
   :align: center

..
   PLACEHOLDER: RTT chart, WithoutMipv6 (being re-captured: pinned axes, off-scale points marked).
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/pingrtt-without.txt)

No reply arrives for 35.0 s, from 17.014 s until the node is home again. The
home agent holds the last few pings until the node answers it again, at
52.007 s, so their replies arrive late: the markers on the top edge, at about
52 s, stand for round-trip times of 523.5 to 2015.8 ms. Over ten seeds, the gap
without replies is 34.5–37.0 s. This is the problem that Mobile IPv6 solves.

The home registration
~~~~~~~~~~~~~~~~~~~~~

We now go back to 19.9 s, the moment the care-of address is ready, and follow
the runs with Mobile IPv6. The ``BidirectionalTunneling`` and
``RouteOptimization`` runs are the same until 20.04 s.

The video below shows the handover in the ``RouteOptimization`` configuration,
from the last reply at home to the direct path. The colored lines trace the
path each ping takes, and the label above the mobile node shows its status:

.. video:: media/handover.mp4
   :align: center

..
   PLACEHOLDER: the file is the OLD capture; wave 2 re-captures it (RouteOptimization,
   window 16.5-22.5 s, label pacing 1.5-2 s).
   VIDEO RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/handover.txt)

The pings first take the home path, then stop while the node moves. After the
registration they take the detour through the home agent for a moment, and
then the direct path through ``foreignRouter``. The status label changes from
"at home" to "away (via home agent)", and then to "away (route-optimized, 1
CN)".

In the same event in which duplicate address detection (DAD) completes, at
19.924 s, the mobile node sends a Binding Update (BU) to its home agent. The
source address is the care-of address. The Binding Update carries sequence
number 1, a lifetime of 3600 s, and the A (acknowledge) and H (home
registration) flags. The node expects the Binding Acknowledgement (BA) within
1.5 s; otherwise it would send the Binding Update again at 21.424 s.

The home agent receives the Binding Update at 19.953 s. It creates a binding
cache entry and the tunnel to the care-of address at once, but it holds the
Binding Acknowledgement (BA) for exactly 1 s. This is the stand-in for
duplicate address detection described in the implementation notes. The
standard asks the home agent to check the home address on the home link before
it acknowledges a first registration, and that check also takes about a
second. A standard home agent would start tunneling only after the check;
INET's home agent starts at once. The same pings are lost either way.

Here is the home agent at 20.5 s, in the middle of the hold. Its interfaces now
include ``ip6tun0``, the tunnel to the care-of address:

.. figure:: media/homeagent-tunnel.png
   :align: center

..
   PLACEHOLDER: canvas screenshot of the homeAgent interior at ~20.5 s, BidirectionalTunneling,
   showing ip6tun0 next to the Ethernet interfaces.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/homeagent-tunnel.txt)

At the same moment, its binding cache holds one entry:

.. figure:: media/bindingcache.png
   :align: center

..
   PLACEHOLDER: binding cache inspector at ~20.5 s (being re-captured: sequence number 1).
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/bindingcache.txt)

The entry maps the home address to the care-of address, with sequence number 1.
The lifetime of 3600 s is INET's default for a home registration
(``maxHaBindingLifeTime``), not a value from the standard. The tunnel
interface exists only while the binding does.

At 20.0 s the correspondent node sends ``ping38``. The home agent intercepts it
and sends it through the tunnel, and it reaches the mobile node at 20.038 s.
The node answers, but it drops its own reply. The reply's source is the home
address, and until the Binding Acknowledgement (BA) arrives, the node has no
reverse tunnel to carry it. ``ping39`` reaches the node at 20.520 s, 0.447 s
before the binding becomes active, and its reply is dropped the same way. The
home agent is ready, but the mobile node is not. The standard has no rule that
makes the node discard these packets; this is the INET behavior listed in the
implementation notes.

At 20.953 s, 1 s after the Binding Update arrived, the home agent sends the
Binding Acknowledgement with sequence number 1. It reaches the mobile node at
20.967 s. The node marks the binding active and creates its reverse tunnel,
from the care-of address to the home agent. One Binding Update, one Binding
Acknowledgement: the retransmission planned for 21.424 s is not needed,
because the 1.5 s timeout leaves room for the home agent's 1 s wait. The
standard chose the value for this reason.

``ping40`` leaves the correspondent node at 21.0 s. It goes to the home agent,
through the tunnel to the mobile node, and its reply comes back through the
reverse tunnel. The home agent unwraps the reply at 21.034 s and forwards it to
the correspondent node, where it arrives at 21.040 s. The round trip takes
40.36 ms.

The outage, from the last reply at home to this first reply abroad, is 4.03 s.
It is the same in both Mobile IPv6 configurations. Over ten seeds it is
4.0–7.5 s, with a median of 4.5 s; the random parts are the delay of duplicate
address detection, the delay of the Router Advertisement and, in some runs,
the rate limit on Router Advertisements. This run is at the low end: none of
the ten runs had a shorter outage.

Here is the registration as a sequence chart, from the
``BidirectionalTunneling`` run. The bracket marks the 1 s hold, and the red
stubs mark the two dropped replies. The time axis is not linear here: busy
stretches get more room than idle ones, so the bracket's label gives the true
length of the hold:

.. figure:: media/seqchart-p2-registration.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P2 -- BidirectionalTunneling, 19.92-21.05 s, NONLINEAR timeline;
   time-label bracket "BA held 1.000 s" on homeAgent (#11152 -> #11762);
   "reply dropped" stubs at #11322 (ping38) and #11603 (ping39).
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p2-registration.txt)

The Binding Update travels down from ``mobileNode`` through ``apForeign``,
``foreignRouter`` and ``backbone`` to ``homeAgent``. The next arrows belong to
``ping38`` and ``ping39``. They bend at the ``homeAgent`` line and then go out
to the foreign network, but no reply arrow leaves ``mobileNode``. After the
bracket, the Binding Acknowledgement travels to the mobile node, and the round
trip of ``ping40`` follows, through the home agent in both directions.

Return routability and the direct path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the ``RouteOptimization`` configuration, the arrival of ``ping38`` at
20.038 s also starts return routability. The standard names a tunneled packet
from a correspondent node as one reason to start it. The node sends the Home
Test Init (HoTI) and the Care-of Test Init (CoTI) at the same time. The Home
Test Init has the home address as its source, so the node drops it, like the
ping replies. The Care-of Test Init goes directly to the correspondent node,
which answers with the Care-of Test (CoT). The Care-of Test is back at the
mobile node at 20.057 s.

Here is that moment as a sequence chart; the red stub marks the dropped Home
Test Init:

.. figure:: media/seqchart-p3-careoftest.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P3 -- RouteOptimization, 20.030-20.065 s;
   ping38 arriving, CoTI/CoT direct; "HoTI dropped" stub at #11324.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p3-careoftest.txt)

The Care-of Test Init and the Care-of Test run straight between the two nodes,
without touching ``homeAgent``. The care-of half of the test is finished a
second before the home half can leave the mobile node.

The node sends the Home Test Init (HoTI) again at 21.038 s, 1 s after the first
copy. Now the binding is active, so the message goes through the reverse
tunnel: it reaches the home agent at 21.052 s and the correspondent node at
21.058 s. The correspondent node sends the Home Test (HoT) to the home address.
The home agent intercepts it and tunnels it to the care-of address, and it
reaches the mobile node at 21.077 s. The node now holds both tokens, so return
routability is complete.

In the same event, the node sends a Binding Update (BU) to the correspondent
node, with sequence number 1 and a lifetime of 420 s. The correspondent node
creates its binding cache entry at 21.088 s and answers with a Binding
Acknowledgement (BA), which reaches the node at 21.097 s. The node's status
becomes "away (route-optimized, 1 CN)"; it was "away (via home agent)" for
1.17 s. Asking for this acknowledgement is the mobile node's choice (INET
always asks), but a correspondent node that is asked must answer. The standard
lets this Binding Update go only after both tests are complete and the home
agent has acknowledged the home registration. Here the home agent's Binding
Acknowledgement arrived at 20.967 s, well before.

Here is the second half of return routability and the registration at the
correspondent node:

.. figure:: media/seqchart-p4-hometest.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P4 -- RouteOptimization, 21.03-21.10 s; HoTI through the
   tunnel, HoT via homeAgent, BU to the CN and its BA; label "sent again, 1 s after the
   dropped copy" at #12082.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p4-hometest.txt)

The Home Test Init and the Home Test both bend at the ``homeAgent`` line: the
home half of the test travels through the tunnel, as it must. The Binding
Update to the correspondent node and its acknowledgement then run directly.

From ``ping41`` at 21.5 s on, the pings take the direct path: correspondent
node, ``backbone``, ``foreignRouter``, mobile node. The request carries a
type 2 routing header with the home address, and the reply carries a Home
Address destination option. The round trip takes 20.18 ms. Exactly one reply
in this run came through the tunnel, the reply to ``ping40``; the same is true
in each of the ten seeds. Here is the direct path as a sequence chart:

.. figure:: media/seqchart-p5-direct.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P5 -- RouteOptimization, 21.49-22.05 s; ping41 and ping42
   direct, crossing the homeAgent band without touching it.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p5-direct.txt)

An arrow that bends at a line stops at that node; an arrow that only crosses a
line passes its position on the chart. The ping arrows now run between
``correspondentNode`` and ``mobileNode`` and cross the ``homeAgent`` line
without bending: the home agent is out of the path.

Away from home
~~~~~~~~~~~~~~

While the node is away, neither Mobile IPv6 configuration sends any Mobile
IPv6 message. The binding lasts 3600 s at the home agent and 420 s at the
correspondent node, both far longer than the 30 s stay. Here are the
round-trip times of the two Mobile IPv6 runs, on the same axes as before:

.. figure:: media/pingrtt-bidirectional.png
   :align: center

..
   PLACEHOLDER: RTT chart, BidirectionalTunneling (being re-captured).
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/pingrtt-bidirectional.txt)

.. figure:: media/pingrtt-routeopt.png
   :align: center

..
   PLACEHOLDER: RTT chart, RouteOptimization (being re-captured).
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/pingrtt-routeopt.txt)

With bidirectional tunneling, the pings stay on a plateau of 40.34 ms (median;
40.26–40.40 ms) for the whole stay. With route optimization, one reply at
40 ms is followed by a plateau of 20.16 ms (median; 20.08–20.22 ms). Both
plateaus match the path arithmetic in The Model. No answered ping in any
configuration needed an 802.11 retransmission.

The two forwarding modes also differ inside each packet. Here is a tunneled
ping request, captured on the home agent's backbone link and opened in Qtenv's
object inspector:

.. figure:: media/tunneled_packet.png
   :align: center

..
   PLACEHOLDER: object inspector, tunneled ping (BidirectionalTunneling), two IPv6 header chunks.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/tunneled_packet.txt)

The packet has two IPv6 headers. The outer header runs from the home agent
(``2001:db8:0:1:8aa:ff:fe00:1``) to the care-of address
(``2001:db8:0:3:8aa:ff:fe00:d``). The inner header is the correspondent node's
original packet, addressed to the home address. The outer header adds 40
bytes: the ping is 170 bytes on this link, against 130 bytes at home. The two
destination addresses have the same interface identifier (``8aa:ff:fe00:d``)
under different prefixes, so identity and location appear in one packet. (The
numbers in parentheses after protocol names, such as ``ipv6(40)``, are INET's
internal protocol identifiers, not byte counts.)

Here is a route-optimized ping request, recorded at the correspondent node
into a packet capture (PCAP) file and opened in Wireshark:

.. figure:: media/ropacket_wireshark.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: Wireshark detail pane, route-optimized ping (RouteOptimization), Address[1] = home address.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/ropacket_wireshark.txt)

There is one IPv6 header, addressed to the care-of address, and a type 2
routing header whose ``Address[1]`` field holds the home address. The routing
header adds 24 bytes instead of the tunnel's 40, and the packet takes no
detour. Wireshark shows 146 bytes where INET reports 154 bytes, because INET
counts the Ethernet preamble and start-of-frame delimiter, which a capture file
does not store.

Coming home
~~~~~~~~~~~

At 48 s the node starts back. The last reply while away arrives at 50.020 s
with route optimization (50.040 s with bidirectional tunneling). The following
pings still go to the care-of address, and ``apForeign`` drops each of them
after seven transmissions. The node loses the ``apForeign`` beacons at
50.721 s, scans, and is associated with ``apHome`` at 51.372 s. It sends a
Router Solicitation (RS) at once.

Now the node waits. The home agent must answer with a Router Advertisement
(RA), but it had multicast one on the home link at 50.614 s, and a router sends
at most one multicast Router Advertisement every 3 s. So the answer leaves the
home agent at 53.821 s, 2.45 s after the solicitation arrived, and reaches the
node at 53.822 s. In the ``BidirectionalTunneling`` run the last multicast
Router Advertisement was earlier, at 49.648 s, and the wait is 1.42 s. The 3 s
gap is the standard's default, which Mobile IPv6 allows a home agent to lower;
INET keeps it fixed.

The Router Advertisement carries the home prefix, so the node knows that it is
home. It removes its care-of address and its reverse tunnel, and it does not
run duplicate address detection (DAD). The standard forbids a node to probe its
own home address while its binding is still alive, to avoid a conflict with
the home agent, which still uses that address. In the same event, the node
sends the de-registration Binding Update (BU) to the home agent, with sequence
number 2 and lifetime 0. With route optimization, it sends one to the
correspondent node as well.

The home agent deletes the binding and the tunnel at 53.823 s and sends the
Binding Acknowledgement (BA), with sequence number 2 and lifetime 0, at once. A
de-registration needs no check, so there is no hold. The acknowledgement
reaches the node at 53.825 s. The standard asks the home agent to keep the
deleted entry, marked invalid, for about 10 s; INET removes it at once. The
node then announces its return with the unsolicited Neighbor Advertisement
described in the About section. Here it changes nothing, because the home
agent's neighbor entry for the node still holds the right MAC address. The
correspondent node deletes its binding at 53.830 s, and its Binding
Acknowledgement reaches the node at 53.837 s.

The first reply at home is the reply to ``ping106``, at 54.014 s, with a round
trip of 13.91 ms. With bidirectional tunneling, the first reply at home is the
reply to ``ping104``, at 53.014 s.

Here is the return in the ``RouteOptimization`` run as a sequence chart. The
bracket marks the wait for the Router Advertisement:

.. figure:: media/seqchart-p6-return.png
   :align: center
   :width: 100%

..
   PLACEHOLDER: sequence chart P6 -- RouteOptimization, 51.3-54.05 s, NONLINEAR timeline;
   time-label bracket "RA delay 2.45 s" on homeAgent (RS #28738 -> RA #30136), saying it is
   the 3 s rate limit; de-registration BUs and BAs, the unsolicited NA, ping106 round trip.
   FIGURE RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/seqchart-p6-return.txt)

The Router Solicitation reaches ``homeAgent`` right after the association, and
then nothing happens for the length of the bracket. After the Router
Advertisement, the two de-registration Binding Updates leave the mobile node in
the same instant, and both acknowledgements come back within milliseconds.
The last arrows are the round trip of ``ping106`` on the home path.

The video below shows the same return: the direct path, the walk home, the
de-registration, and the pings back on the home path. The status label returns
to "at home", and the address label to the home address:

.. video:: media/returnhome.mp4
   :align: center

..
   PLACEHOLDER: the file is the OLD capture; wave 2 re-captures it (RouteOptimization,
   window to be set from the corrected return times, which now end at 54.014 s).
   VIDEO RECIPE: to be supplied by the analyst (panel/g4-behavior/recipes/returnhome.txt)

So the return is not shorter in this run, although it skips duplicate address
detection and the home agent's hold. With route optimization it takes 3.99 s,
against 4.03 s on the way out; with bidirectional tunneling it takes 2.97 s.
The node saves both waits for duplicate address detection, and then spends the
time waiting for the rate-limited Router Advertisement instead. That wait
depends on when the home agent last multicast an advertisement, so it changes
from run to run. Over ten seeds, the return takes 1.5–4.5 s with route
optimization and 2.0–4.0 s with bidirectional tunneling, against 4.0–7.5 s on
the way out, where this run is at the minimum.

The plain host of the ``WithoutMipv6`` run gets its first reply earlier in this
run, at 52.016 s, because its home agent had sent no multicast Router
Advertisement in the previous 3 s. Which kind of host is answered first after
the return depends on the run; Mobile IPv6 gives no advantage here. Back home,
all three runs return to the 14 ms plateau.

What the handover costs
~~~~~~~~~~~~~~~~~~~~~~~

Where do the 4.03 s of the outbound outage go? The steps above give the terms,
and each term is either a protocol timer or a value of this scenario:

.. list-table::
   :header-rows: 1
   :widths: 50 15 35

   * - Term
     - Time
     - Kind
   * - Out of range, still associated with ``apHome``
     - 0.743 s
     - scenario value (802.11 beacon loss)
   * - Scan and association
     - 0.651 s
     - scenario value (scan settings, two channels)
   * - Wait for the Router Advertisement
     - 0.459 s
     - protocol timer (random, up to 0.5 s)
   * - Duplicate address detection on the new link
     - 1.057 s
     - protocol timer (1 s, plus a random part)
   * - Binding Update, the home agent's hold, Binding Acknowledgement
     - 1.043 s
     - protocol timer (1 s hold) plus 43 ms of network path
   * - Next ping and its round trip
     - 0.074 s
     - measurement granularity (0.5 s ping interval)
   * - **Total**
     - **4.026 s**
     -

The protocol timers (the wait for the Router Advertisement, duplicate address
detection, and the home agent's hold) take 2.52 s. The two terms tied to
duplicate address detection (DAD) alone take 2.10 s, more than half of the
outage. The scenario values, the time 802.11 takes to notice the lost access
point, to scan and to associate, take 1.39 s. The network paths take 43 ms,
and the rest comes from the 0.5 s spacing of the pings.

Neither Mobile IPv6 configuration changes these terms: bidirectional tunneling
and route optimization have the same outage, because route optimization starts
only after the first tunneled packet. Its gain is on the path afterward, 20 ms
instead of 40 ms. That gain comes from where the home agent sits in this
network, 5 ms off the backbone; with a home agent close to the correspondent
node, the gain shrinks. The share of duplicate address detection is what
optimizations such as Optimistic Duplicate Address Detection (RFC 4429)
target: they let a node use a new address while the check still runs.

The standard's default timers predict an outage of about 3.9 s without the
random part of duplicate address detection, and about 4.4 s with it. The
median over ten seeds, 4.5 s, lies close to the second value. A measured
802.11 testbed (Cabellos-Aparicio et al., 2005) reports a mean Mobile IPv6
handover of 2.1 s, 87 % of it in the IPv6 phase, so the same timers dominate
there. That testbed shows no wait at the home agent, and it starts its clock
at the scan, which accounts for most of the difference.

The two modes also cost differently per packet. The tunnel adds 40 bytes to
every packet, so it lowers the largest packet the node can send without
fragmentation. Route optimization adds only 24 bytes, but in IPv6 extension
headers, which some networks filter out.

In practice, the pattern of the ``BidirectionalTunneling`` run, an anchor that
keeps the node's address and a tunnel to wherever the node is, is what mobile
operator networks, Wi-Fi calling and enterprise Wi-Fi controllers use, under
other names and protocols. Route optimization did not spread. It needs support
in every correspondent node, firewalls and filters drop its headers, and it
reveals the node's location to every peer.

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
