..
   All per-config sequence charts are now captured via the OMNeT++ IDE
   MCP (omnetpp-ide-mcp skill). dora_sequence_chart.png covers BasicDHCP,
   lease_renewal.png the LeaseRenewal T1 cycle, and so on. The two
   network topology screenshots (network.png, roaming_network.png) and
   the interface-tables overlay (interface_tables.png) were captured
   via the omnetpp-mcp-sim skill.

Dynamic Host Configuration Protocol (DHCP)
===========================================

Goals
-----

Before a host can communicate over IP, it needs an IP address, subnet mask,
and default gateway. Assigning these parameters manually on every host does
not scale well: administrators must track which addresses are already in use
to avoid conflicts, update configurations when hosts move between subnets,
and reclaim addresses when hosts are decommissioned. In networks where hosts
join and leave frequently, this becomes impractical.

The Dynamic Host Configuration Protocol (DHCP), defined in :rfc:`2131`,
automates this process. The server manages a pool of addresses
and leases them to clients for a limited time, reclaiming them when the
lease expires.

This showcase demonstrates how to set up DHCP-based address assignment in
INET using the :ned:`DhcpServer` and :ned:`DhcpClient` application modules.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/general/dhcp <https://github.com/inet-framework/inet/tree/master/showcases/general/dhcp>`__

About DHCP
----------

DHCP is an application-layer protocol that operates over UDP (server on
port 67, client on port 68). Because a client has no IP address when it
first joins the network, DHCP messages are sent as link-layer broadcasts.
This means DHCP operates within a single LAN segment — a router does not
forward DHCP broadcasts unless a DHCP relay agent is configured.

The DHCP server is configured with a pool of IP addresses that it can
hand out. It derives the available range from its own interface address and
subnet mask, and maintains a table of which addresses are currently in use
and by which client (identified by MAC address).

A DHCP address assignment is called a *lease* — the server does not give
the address to the client permanently, but grants it for a limited duration
(the *lease time*). In a real network this ensures that addresses are eventually returned to
the pool if a client leaves without explicitly releasing them.

The initial address acquisition uses a four-message exchange known as
**DORA**:

1. **Discover** — The client broadcasts a DHCPDISCOVER message to locate
   available servers.
2. **Offer** — The server responds with a DHCPOFFER containing an available
   IP address, the subnet mask, the default gateway, the lease duration,
   the T1 and T2 timers (their purpose is explained below), and a
   *server identifier* the client uses to direct its DHCPREQUEST at the
   chosen server.
3. **Request** — The client broadcasts a DHCPREQUEST indicating which offer
   it accepts. (Broadcasting rather than unicasting informs any other servers
   that their offers were not chosen.)
4. **Acknowledge** — The server confirms the lease with a DHCPACK.
   If the request can't be granted (e.g. the address is no longer
   available, or the client asked for something the server can't
   honour) the reply is an explicit refusal called a DHCPNAK
   ("negative acknowledgement"); the client throws its current lease
   state away and starts a fresh DORA.

After receiving the DHCPACK, the client enters the **BOUND** state and
configures its interface with the leased address.

Two additional message types extend the protocol. A client that no
longer needs its address sends a **DHCPRELEASE** so the server can
return the address to the pool immediately, rather than waiting for
the lease to expire. A client that, after the DHCPACK, detects the
offered address is already in use on the network (typically via an ARP
probe per RFC 5227) sends a **DHCPDECLINE**; the server then
quarantines that address for a while and the client restarts the DORA
exchange. INET's coverage of these two messages is described in
the *DHCP in INET* section below.

Before the lease expires, the client must extend it. This happens in two
stages, providing a fallback in case the original server becomes
unreachable:

- **Renewing** (at time T1, typically half the lease time) — The client
  sends a unicast DHCPREQUEST directly to the server that granted the
  lease. Under normal conditions, the server replies with a DHCPACK and
  the lease is extended. This is the common case.

- **Rebinding** (at time T2, typically 87.5% of the lease time) — If the
  client received no response during the renewing phase (e.g. the original
  server is down), it falls back to *broadcasting* a DHCPREQUEST so that
  any available DHCP server on the LAN can respond and extend the lease.

If neither succeeds and the lease expires, the client loses its address and
must start over with a new Discover.

A client that restarts *without having released its lease* —
typically after a crash or power cut, where there was no chance to
send DHCPRELEASE — can take a shortcut known as **INIT-REBOOT**:
instead of a full DORA exchange it broadcasts a DHCPREQUEST naming
the previously held address. The server confirms with a DHCPACK if
the binding is still valid, or rejects it with a DHCPNAK, in which
case the client falls back to a full DORA.

This showcase includes eight configurations that illustrate different
aspects of the protocol:

- **BasicDHCP** — The standard four-message DORA exchange where clients
  obtain addresses from a server.
- **LeaseRenewal** — A short lease time triggers the renewal mechanism
  during the simulation.
- **CleanShutdown** — A client is shut down cleanly, sending a
  DHCPRELEASE so the server immediately frees the lease; the restarted
  client then performs a fresh DORA.
- **ClientCrash** — A client is crashed and restarted mid-simulation,
  demonstrating INIT-REBOOT (request previous IP without a full DORA).
- **LeaseExpiration** — A client crashes without sending DHCPRELEASE
  and the server reclaims the address back into the pool once the
  lease expires, so a different client can acquire it.
- **ServerReboot** — The DHCP server is shut down and restarted, losing
  its lease database. When clients attempt to renew, the server rejects
  the request and clients must re-acquire addresses.
- **LossyDORA** — The server is initially down and the client's
  retransmits drive recovery once it comes back up, illustrating the
  exponential-backoff retransmission strategy from RFC 2131.
- **Roaming** — A mobile wireless client moves between two access points,
  each served by a separate DHCP server on a different subnet.

The Model
---------

DHCP in INET
~~~~~~~~~~~~

INET provides two application modules for DHCP.

:ned:`DhcpServer` listens on a network interface, manages the address
pool, and responds to client requests. Key parameters:

- :par:`interface` — which interface to serve DHCP on; if omitted, the
  only non-loopback interface is used.
- :par:`numReservedAddresses` — number of addresses to skip counting
  from the network address. With the server on 192.168.1.1 in
  192.168.1.0/24 and ``numReservedAddresses=10``, the pool starts at
  192.168.1.10.
- :par:`maxNumClients` — maximum number of concurrent leases.
- :par:`gateway` — default gateway announced to clients; if left
  empty, defaults to the server's own interface address.
- :par:`leaseTime` — how long the server grants a lease for.
- :par:`offerHoldTime` — how long an offered-but-not-yet-acknowledged
  address is reserved (default 120 s).
- :par:`declineHoldTime` — how long a DHCPDECLINEd address is
  quarantined before being offered again (default 60 s).
- :par:`startTime` — when the DhcpServer application begins listening
  (default 0 s).

.. note::

   The server reclaims leases on its own once the lease expires — the
   address simply becomes available again for the next client.

:ned:`DhcpClient` runs the DHCP client on a host interface. Key
parameters:

- :par:`interface` — which NIC to configure via DHCP; if omitted, the
  only non-loopback interface is used.
- :par:`startTime` — when the client begins the DORA exchange.
- :par:`initialRetransmitDelay` (default 4 s) — initial wait between
  retransmits of DHCPDISCOVER / DHCPREQUEST.
- :par:`maxRetransmitDelay` (default 64 s) — cap on the
  exponentially-doubled retransmit delay; ±1 s jitter is added on
  each attempt.
- :par:`maxRetransmitCount` (default 4) — after this many unanswered
  retransmits the client restarts the DORA exchange.
- :par:`minRenewRetransmitInterval` (default 60 s) — floor on the
  retransmit interval during renewal and rebinding, which otherwise
  follows a "half of the remaining interval" rule.
- :par:`declineOfferedIp` — test hook that forces the client to
  decline a specific offered address. Until INET ships an RFC-5227
  ARP probe the client never produces a DHCPDECLINE on its own, so
  this parameter is the only way to exercise the DECLINE round-trip.

On a graceful shutdown while holding a lease the client emits a
DHCPRELEASE; a crash does not.

The T1 and T2 renewal timers are not client-side defaults — the server
sends them in every DHCPOFFER and DHCPACK message (T1 = 0.5 × lease
time, T2 = 0.875 × lease time). The values that take effect are the
ones received in the DHCPACK; T1/T2 carried in an unaccepted DHCPOFFER
are ignored.

The server NAKs a DHCPREQUEST under three conditions:

- The client's DORA REQUEST asks for a different address than the one
  the server just offered. In a normal DORA the REQUEST's
  *requested-IP* option echoes back the address from the DHCPOFFER —
  effectively the client saying "yes, I accept that one." A mismatch
  is a sign of a misbehaving or out-of-sync client; the server
  refuses.
- An INIT-REBOOT REQUEST names an address that the server has on file
  but on a different subnet (the client has moved to another network
  since it last held this lease).
- A RENEWING or REBINDING REQUEST names an IP the server has no record
  of — typically because the server has been restarted and lost its
  lease database (the ServerReboot scenario below).

A REQUEST that names a different *server identifier* (the client picked
another server's offer) does not cause a NAK; the server simply
releases its own pending offer for that client.

On receiving a DHCPDECLINE the server marks the slot as DECLINED for
``declineHoldTime`` rather than freeing it. A later DHCPDISCOVER from
the same client is not re-offered the DECLINED address — it is given
a different address from the pool.

Both modules are added to a :ned:`StandardHost` as applications
(``app[*]``). The server needs a statically configured IP address on its
interface (assigned by :ned:`Ipv4NetworkConfigurator`); client interfaces
are left unconfigured so they receive their addresses via DHCP.

The Network
~~~~~~~~~~~

The network consists of a DHCP server (``dhcpServer``), an Ethernet switch
(``switch``), and three DHCP clients (``client[0..2]``), all connected via
100 Mbps Ethernet links. An :ned:`IntegratedCanvasVisualizer` displays the
acquired addresses on the canvas.

The network topology:

.. figure:: media/network.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas
   config:   BasicDHCP                 # ../omnetpp.ini
   seed:     default
   shows:    the network topology — dhcpServer, switch, client[0..2] on
             100 Mbps Ethernet
   anchor:   initial state (t=0, before run). No timing — the self-check is
             structural: if the module set/links differ, the NED changed.
   capture:  get_canvas_image, area=all_elements; was 728×478
   stamp:    captured 2026-06, INET 4.6

The :ned:`Ipv4NetworkConfigurator` assigns a static IP address only to the
server — client interfaces are left unconfigured so they obtain addresses
via DHCP:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [General]
   :end-at: netmask

Configurations and Results
--------------------------

BasicDHCP
~~~~~~~~~

This configuration sets up a straightforward DHCP scenario. The server is
configured with :par:`numReservedAddresses` = 10, :par:`maxNumClients` = 50,
:par:`gateway` = "192.168.1.1", and :par:`leaseTime` = 3600s. Each client
starts its DHCP process at a random time within the first 2 seconds.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config BasicDHCP]
   :end-before: [Config LeaseRenewal]

All three clients obtain addresses starting from 192.168.1.10.
``numReservedAddresses=10`` reserves the first 10 addresses, counting
from the network address: .0 is the network address (not assignable to
a host) and .1–.9 are reserved, which keeps the static server address
.1 inside that range. The pool therefore starts at .10 and, capped by
``maxNumClients=50``, spans 50 addresses (.10–.59). The interface table
visualizer displays the acquired address and prefix length next to each
host.

The following sequence chart shows the DORA exchange (time is non-linear):

.. figure:: media/dora_sequence_chart.png
   :align: center

The sequence chart shows the message *flow*; the *contents* of one of
those messages — the server's DHCPOFFER — are shown below in Qtenv's
**packet inspector**. As the offer travels down the protocol stack each
layer wraps it, so on the wire it is a complete Ethernet frame. The
inspector's ``content`` field shows that frame as a ``SequenceChunk``
whose ``chunks`` are the protocol layers, one nested inside the next:

.. figure:: media/dhcp_offer_frame.png
   :align: center

From outermost to innermost: an ``EthernetPhyHeader`` and
``EthernetMacHeader`` (a broadcast ``dest``, since the client has no
address yet, from the server's MAC), an ``Ipv4Header`` (from the server's
192.168.1.1 to the 255.255.255.255 broadcast), a ``UdpHeader`` (source
port 67 to destination port 68), the ``DhcpMessage`` itself (``op`` = 2,
BOOTREPLY, ``yiaddr`` = 192.168.1.10), and an ``EthernetFcs`` trailer.
Each chunk can be expanded further: drilling into the ``DhcpMessage``
reveals the DHCP fields and ``options`` — the subnet mask, the gateway
(``router`` option), the lease time (3600 s), the T1/T2 renewal/rebinding
timers (1800 s / 3150 s) and the server identifier (192.168.1.1) that the
offer hands the client.

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector (packet content / chunks)
   config:   BasicDHCP                 # ../omnetpp.ini
   seed:     default
   shows:    the server's first DHCPOFFER as a complete Ethernet frame in
             Qtenv's object inspector — the `content` (SequenceChunk)
             `chunks[6]`: EthernetPhyHeader, EthernetMacHeader, Ipv4Header,
             UdpHeader, DhcpMessage (yiaddr=192.168.1.10), EthernetFcs, each
             chunk at its summary line. Cropped to the chunks band — the
             packet/chunk metadata rows above are left out.
   anchor:   server's first DHCPOFFER, caught as the EthernetSignal in flight
             to switch.eth[0].mac at ~t=1.0977s; the frame's IPv4 src is the
             server's 192.168.1.1 and the DhcpMessage's yiaddr is 192.168.1.10.
             If those move, the pool/topology changed.
   capture:  the full frame only exists in flight (inside the EthernetSignal),
             and this Qtenv build exposes no logged-packet buffer, so it is
             caught live. Needs execute_cpp, so launch under opp_sandbox with
             CPLUS_INCLUDE_PATH set to the Qt6 include dirs (so execute_cpp can
             #include <QTreeView>). Then:
             1. set_stop_condition (before_event) firing when the next event is
                an inet::physicallayer::EthernetSignal named "DHCPOFFER";
                run_simulation stops with the signal still in the FES.
             2. execute_cpp: find that signal in the FES, take its
                getEncapsulatedPacket() (the inet::Packet whole frame), dup() it
                and re-parent the copy onto dhcpServer (cSoftOwner::take, reached
                via a derived-class pointer-to-member) for an inspectable path.
             3. open_inspector(type=object) on the copy, then execute_cpp drives
                its QTreeView: collapseAll(), expand `content` then its `chunks`
                node (chunk children left collapsed at their summaries).
             4. get_inspector_screenshot (was 1080 wide, to fit the chunk
                summaries incl. DhcpMessage's yiaddr), then PIL-crop to just the
                `chunks[6]` band (its header through EthernetFcs), dropping the
                packet/chunk metadata rows above (was ~1036x123).
   stamp:    captured 2026-06, INET 4.6, OMNeT++ 6.4.0aipre2

The interface table visualizer displays the acquired addresses:

.. figure:: media/interface_tables.png
   :align: center

On the server side, the same three bindings are recorded in its lease
database. The ``leased`` map, exposed in the server's module inspector,
maps each leased IP to the client MAC that holds it:

.. figure:: media/lease_table.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   BasicDHCP                 # ../omnetpp.ini
   seed:     default
   shows:    the server's `leased` map — three IP→client-MAC bindings
             (.10/.11/.12) after all clients are BOUND
   target:   dhcpServer → module inspector → `leased` field
   anchor:   after all 3 clients BOUND (~t=2–3s; same anchor as basicdhcp.mp4).
             Fewer/more bindings ⇒ client count or timing changed.
   capture:  open_inspector(dhcpServer) → get_inspector_screenshot, cropped;
             was 720×170
   stamp:    captured 2026-06, INET 4.6

The animation below shows the three DORA exchanges running back-to-back
through the switch, the per-client "Got IP …" bubbles fired when each
lease binds, and the interface-table labels updating from
``<unspec>`` to ``192.168.1.10/24`` / ``.11/24`` / ``.12/24``:

.. video:: media/basicdhcp.mp4
   :align: center

..
   VIDEO RECIPE (redo via the "video-recording" skill)
   config:   BasicDHCP                  # ../omnetpp.ini
   seed:     default
   shows:    three back-to-back DORA exchanges through the switch, the
             per-client "Got IP …" bubbles, and the interface-table labels
             flipping <unspec> → 192.168.1.10/.11/.12
   anchors:  all 3 clients complete DORA and reach BOUND within the first
             ~2–3s (client startTime=uniform(0s,2s)); last observed bind
             ~t=2.1s. Mechanism must hold; exact times are seed-dependent —
             if the binds drift far from this, an RNG-affecting .ini edit
             changed timing → re-tune and rewrite this recipe.
   window:   record sim-time 0s → ~3s (record from start; only an
             interfaceTableVisualizer is configured — no channel visualizer,
             so no fade-out wait)
   anim:     playback_speed=1           # set_animation_parameters, normal profile
   capture:  fps=2, crop_area=with_padding   # re-read crop_rect; canvas was 732×482
   encode:   ffmpeg -r 30 -vcodec libx264 -pix_fmt yuv420p (pad to even dims)
   post:     none
   stamp:    recorded 2026-06, INET 4.6

LeaseRenewal
~~~~~~~~~~~~

This configuration extends ``BasicDHCP`` and reduces the lease time to 60
seconds. Since the T1 timer fires at half the lease time (30s), clients
will attempt to renew their leases during the 200-second simulation. This
allows observation of the BOUND → RENEWING transition and the subsequent
unicast DHCPREQUEST/DHCPACK renewal exchange.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config LeaseRenewal]
   :end-before: [Config CleanShutdown]

During the simulation, the T1 timer fires at t≈31 s for each client
(half of the 60 s lease, measured from each client's own bind around
t≈1 s — clients start at ``uniform(0s, 2s)``),
triggering a unicast DHCPREQUEST to the server. The server responds with
a DHCPACK, extending the lease. This renewal cycle repeats throughout
the simulation.

Sequence chart of one client's first renewal cycle at T1, captured
against the wired path client → switch → dhcpServer. Renewal is the
client's first unicast to the server — the DORA exchange was all
broadcast, so the server's MAC was never cached. The renewal is
therefore preceded by an ARP exchange to resolve it, then carries the
DHCPREQUEST and DHCPACK:

.. figure:: media/lease_renewal.png
   :align: center

CleanShutdown
~~~~~~~~~~~~~

This configuration demonstrates the **DHCPRELEASE** path. A
:ned:`ScenarioManager` ``<shutdown>`` event gracefully takes
``client[0]`` down at t=30s, and a ``<startup>`` event brings it back
at t=60s. On its way out the client unicasts a DHCPRELEASE to the
granting server, forgets its address, and exits. The server immediately
returns the address to the pool without waiting for the lease to
expire. On restart the client has no surviving lease, so it performs a
fresh DORA from INIT.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config CleanShutdown]
   :end-before: [Config ClientCrash]

The scenario script:

.. literalinclude:: ../scenario_clean_shutdown.xml
   :language: xml

The DHCPRELEASE is visible in the server-side pcap as a single
BOOTREQUEST with message type ``DHCPRELEASE``, and the server log
shows the address returning to the pool the moment it arrives. With
the pool sized generously (50 addresses) the client typically
reacquires the same IP, but the path to it goes through a full DORA,
not INIT-REBOOT.

Sequence chart of the t=30 s shutdown showing the ARP resolution that
precedes the unicast DHCPRELEASE, followed by the RELEASE itself
travelling client → switch → dhcpServer:

.. figure:: media/clean_shutdown.png
   :align: center

The server's lease table confirms the release: ``client[0]``'s address
``192.168.1.10`` flips to ``FREE`` the instant the DHCPRELEASE arrives,
while ``.11`` and ``.12`` stay ``LEASED``:

.. figure:: media/clean_shutdown_leases.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   CleanShutdown             # ../omnetpp.ini
   shows:    server `leased` map with .10 FREE (released), .11/.12 LEASED
   target:   dhcpServer.app[0].leased → object inspector
   anchor:   just after the t=30 s DHCPRELEASE (~t=30–60 s, before restart)
   capture:  open_inspector(...leased) → expand → get_inspector_screenshot;
             was 720×170
   stamp:    captured 2026-06, INET 4.6

ClientCrash
~~~~~~~~~~~

This configuration contrasts directly with ``CleanShutdown``: the
scenario is otherwise identical — ``client[0]`` goes down at t=30s and
back up at t=60s — but the :ned:`ScenarioManager` event is a ``<crash>``
rather than a graceful ``<shutdown>``. Because a crash skips the
graceful-shutdown path, the client does *not* emit a DHCPRELEASE, and
it still remembers its previously held address across the restart. On
startup the client therefore enters **INIT-REBOOT** and broadcasts a
DHCPREQUEST for its previously held address (skipping the Discover and
Offer steps); the server responds with a DHCPACK (or a DHCPNAK if the
address is no longer valid). The lease time is set to 120 seconds.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ClientCrash]
   :end-before: [Config LeaseExpiration]

The scenario script:

.. literalinclude:: ../scenario.xml
   :language: xml

At t=30s, ``client[0]`` is crashed and its interface is deconfigured.
At t=60s, the client restarts; because its lease object survived the
crash it enters INIT-REBOOT and broadcasts a DHCPREQUEST for the
previously held address. The server confirms with a DHCPACK and the
client receives the same IP address as before the crash. The other two
clients continue their normal lease renewals throughout — their unicast
DHCPREQUEST/DHCPACK pairs around t≈61 s, t≈121 s, and t≈181 s (T1 = 60 s
of the 120 s lease) are visible in the server pcap.

Sequence chart of the INIT-REBOOT exchange at t=60 s. The broadcast
DHCPREQUEST from ``client[0]`` reaches ``dhcpServer`` via ``switch`` and
the server confirms the old binding with a DHCPACK — no Discover/Offer:

.. figure:: media/client_crash.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-ide-mcp" skill)
   type:     seqchart
   config:   ClientCrash               # ../omnetpp.ini, record-eventlog=true
   seed:     default
   shows:    the INIT-REBOOT exchange — client[0]'s broadcast DHCPREQUEST
             reaching dhcpServer via switch, answered by a DHCPACK (no
             Discover/Offer)
   source:   results/ClientCrash-*.elog (produced by record-eventlog=true)
   axes:     client[0] · switch · dhcpServer   (this top-to-bottom order)
   anchor:   REQUEST→ACK at t≈60s (client crashes t=30s, restarts t=60s).
             If the exchange moves off ~60s the scenario/timing changed →
             re-capture and rewrite.
   capture:  Sequence Chart screenshot, NONLINEAR timeline, cropped to the
             exchange; was 963×578
   stamp:    captured 2026-06, INET 4.6

LeaseExpiration
~~~~~~~~~~~~~~~

This configuration exercises the server's automatic **lease
expiration**. The pool is sized to exactly one
address (``maxNumClients = 1``) and the lease time is shortened to 30
seconds. ``client[0]`` starts up at t=0, takes the only address, and is
then *crashed* at t=5s — so no DHCPRELEASE is sent and the server
believes the lease is still in use. At t=80s, well after the lease
duration has elapsed, ``client[1]`` is brought up; it must be able to
acquire ``192.168.1.10`` only if the server has independently reclaimed
the abandoned slot.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config LeaseExpiration]
   :end-before: [Config ServerReboot]

The scenario script:

.. literalinclude:: ../scenario_lease_expiration.xml
   :language: xml

The server log shows a "Lease 192.168.1.10 (...) expired, returning
address to the pool." line about a lease time after client[0]
acquired the address. When client[1] joins at t=80s, the slot is
already free again and the DORA completes normally with ``client[1]``
receiving the same 192.168.1.10 that ``client[0]`` previously held.

Sequence chart of ``client[1]``'s DORA at t=80 s, after the server has
reclaimed ``192.168.1.10`` from the crashed ``client[0]``:

.. figure:: media/lease_expiration.png
   :align: center

The server's lease table tells the rest of the story. With ``client[0]``
crashed and silent, its lease simply ages out: about 30 s after the bind
the entry flips to ``FREE``, even though no DHCPRELEASE was ever sent —

.. figure:: media/lease_expiration_reclaimed.png
   :align: center

— and once ``client[1]`` starts at t=80 s, that same ``192.168.1.10`` is
handed to it (note the new client MAC ending ``…07``):

.. figure:: media/lease_expiration_reassigned.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector (two anchors)
   config:   LeaseExpiration           # ../omnetpp.ini
   shows:    .10 FREE after the ~t=31 s expiry (reclaimed), then .10 LEASED
             to client[1] (...07) after its t=80 s DORA
   target:   dhcpServer.app[0].leased → object inspector
   anchor:   reclaimed ~t=31–80 s (single FREE entry); reassigned ~t=90 s
   capture:  open_inspector(...leased) → expand → get_inspector_screenshot;
             was 720×130
   stamp:    captured 2026-06, INET 4.6

ServerReboot
~~~~~~~~~~~~

This configuration demonstrates what happens when the DHCP server reboots
and loses its lease database. The server is shut down at t=30s and
restarted at t=40s via :ned:`ScenarioManager`. When the server comes back
up, it has no record of previously granted leases.

The lease time is set to 100 seconds, so the T1 renewal timer fires at
t≈51 s (50 s after each client's own bind around t≈1 s). When a client
sends a unicast DHCPREQUEST to renew its lease, the server does not
recognize it and responds with a DHCPNAK. The client then falls back
to the INIT state and performs a full DORA exchange to obtain a new
address.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ServerReboot]
   :end-before: [Config LossyDORA]

The scenario script:

.. literalinclude:: ../scenario_server_reboot.xml
   :language: xml

The server shuts down at t=30s and restarts at t=40s. When the clients'
T1 timers fire around t≈51 s, the server no longer recognizes their
leases. The sequence chart shows the server responding with a DHCPNAK,
followed by the client performing a complete DORA exchange to obtain a
new address.

Sequence chart of the failed renewal and recovery at t≈51 s: the
unicast DHCPREQUEST (preceded by ARP resolution) reaches the restarted
server which has no lease record and responds with DHCPNAK, after which
the client falls back to a full DORA exchange:

.. figure:: media/server_reboot.png
   :align: center

Right after the reboot the server's lease table is **empty** — every
binding from before the crash is gone, which is exactly why the renewing
clients are met with a DHCPNAK:

.. figure:: media/server_reboot_leases.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     inspector
   config:   ServerReboot              # ../omnetpp.ini
   shows:    server `leased` map empty (size 0) just after the t=40 s restart
   target:   dhcpServer.app[0].leased → object inspector
   anchor:   t≈40–50 s, after restart, before clients re-DORA and repopulate it
   capture:  open_inspector(...leased) → get_inspector_screenshot; was 720×90
   stamp:    captured 2026-06, INET 4.6

The text log of ``client[0]``'s renewal at t≈51 s shows the round-trip
in one place — the RENEWING DHCPREQUEST hits the empty database, the
server returns a DHCPNAK, and the client falls straight into a fresh
DORA that re-binds the same 192.168.1.10:

.. literalinclude:: media/serverreboot.log
   :language: text

..
   LOG RECIPE (redo via the "omnetpp-mcp-sim" skill)
   config:   ServerReboot              # ../omnetpp.ini
   seed:     default
   shows:    client[0]'s NAK-then-re-DORA at t≈51 s
   anchor:   T1 fires t≈51 s (50 s after bind ~t=1.1 s); messages on the
             RENEWING → SELECTING → BOUND path
   capture:  inet -u Cmdenv -c ServerReboot --cmdenv-express-mode=false
             --cmdenv-log-prefix='[%t %M] ' --cmdenv-event-banners=false |
             sed 's/\x1b\[[0-9;]*m//g' | grep '\.app\[0\]\]' |
             range 51.097–51.099 s, client[0]/dhcpServer only
   stamp:    captured 2026-06, INET 4.6

The animation below shows the whole reboot cycle end to end. The
``dhcpServer`` icon greys out at t=30 s (the ``<shutdown>`` event),
clients keep their bound addresses through the downtime, the server
returns at t=40 s, and around t≈51 s the renewal NAKs trigger the
fresh DORAs — the per-client "Got IP …" bubbles fire again as each
client re-binds the same address:

.. video:: media/serverreboot.mp4
   :align: center

..
   VIDEO RECIPE (redo via the "video-recording" skill)
   config:   ServerReboot               # ../omnetpp.ini
   seed:     default
   shows:    server greying out at t=30 s, returning at t=40 s, the renewal
             NAKs at t≈51 s and the back-to-back re-DORAs that follow
   anchors:  shutdown t=30 s, startup t=40 s (ScenarioManager script);
             T1 fires ~51 s on each client (lease 100 s, bind ~t=1.1 s)
   window:   record sim-time 0s → ~55s (covers shutdown, restart and the
             three NAK→DORA cycles; no channel visualizer, no fade-out wait)
   anim:     playback_speed=1           # set_animation_parameters, normal profile
   capture:  fps=2, crop_area=with_padding   # canvas was 732×482
   encode:   ffmpeg -r 30 -vcodec libx264 -pix_fmt yuv420p (pad to even dims)
   post:     none
   stamp:    recorded 2026-06, INET 4.6

LossyDORA
~~~~~~~~~

This configuration exercises the client's **retransmission strategy**.
The DHCP server is initially DOWN and
:ned:`ScenarioManager` brings it up at t=10s. ``client[0]`` starts at
its usual ``uniform(0s, 2s)`` and sends the first DHCPDISCOVER while
nobody is listening. The client then retransmits the same DHCPDISCOVER
(same xid) at exponentially growing intervals — ~4 s, then ~8 s, then
~16 s, each with ±1 s of jitter — until a reply arrives. The remaining
two clients are disabled in this config so the trace stays focused on
the retransmits.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config LossyDORA]
   :end-before: [Config Roaming]

The scenario script:

.. literalinclude:: ../scenario_lossy_dora.xml
   :language: xml

A PcapRecorder on the always-up ``switch`` captures the full sequence
(a recorder on ``dhcpServer`` would only start once the server itself
came up at t=10s and so would miss the retransmits sent during
downtime). The capture shows:

- the initial DHCPDISCOVER at t≈1 s (no reply — server is down);
- a retransmit at t≈5 s (still no reply);
- a third attempt at t≈14 s that succeeds, immediately followed by the
  OFFER, REQUEST and ACK.

The bind therefore lands around t≈14 s, not "right after the server
comes up at t=10 s": each unanswered DISCOVER doubles the wait, and
once the next scheduled retransmit is ~8 s after t=5 s (±1 s of
jitter), the t=10 s server-up window is already missed. The takeaway
is that the client recovers in two retransmits rather than sitting on
one long ``maxRetransmitDelay``-sized wait — not that it recovers
instantly.

The same retransmit machinery applies to DHCPREQUEST in REQUESTING /
REBOOTING, and to lease renewals in RENEWING / REBINDING (where
the "half the remaining interval" rule replaces the doubled-delay
schedule).

The retransmits are not fresh DORAs — they are *the same* DHCPDISCOVER
re-sent until a reply arrives. The client log shows all three carrying
the same transaction ID (``xid = 398764591``); only the third one
finds the server up, and the DHCPOFFER comes back with that same xid:

.. literalinclude:: media/lossy_dora.log
   :language: text

..
   LOG RECIPE (redo via the "omnetpp-mcp-sim" skill)
   config:   LossyDORA                  # ../omnetpp.ini
   seed:     default
   shows:    same xid (398764591) across all three DHCPDISCOVERs at
             t≈1.097 s / 5.528 s / 14.216 s, and on the eventual DHCPOFFER
   anchor:   first DISCOVER at t≈1.1 s (client start), third one followed by
             OFFER within microseconds — server is up at t=10 s
   capture:  inet -u Cmdenv -c LossyDORA --cmdenv-express-mode=false
             --cmdenv-log-prefix='[%t %M] ' --cmdenv-event-banners=false |
             grep client[0].app[0] / dhcpServer.app[0]; trim packet-tail
             options for readability
   stamp:    captured 2026-06, INET 4.6

Sequence chart of the entire LossyDORA run, with the three retransmits
visible as three DHCPDISCOVER arrows fanning out to ``dhcpServer`` at
t≈1 s, t≈5 s, and t≈14 s; only the third one returns an OFFER (the
server is finally up), and the REQUEST/ACK pair completes immediately
after:

.. figure:: media/lossy_dora.png
   :align: center

Roaming
~~~~~~~

This configuration uses a different network (:ned:`DhcpRoaming`) to
demonstrate DHCP in a wireless roaming scenario. Two access points
(``ap1``, ``ap2``) are each connected to a dedicated DHCP server on a
separate subnet (192.168.1.0/24 and 192.168.2.0/24). The wired host
named ``server`` is reachable through both DHCP servers, which act as
gateways (``forwarding = true``). Static routes on ``server`` ensure
both subnets are reachable.

The network topology for the roaming scenario:

.. figure:: media/roaming_network.png
   :align: center

The wireless client uses :ned:`RectangleMobility` to move back and forth
between the two access points at 20 m/s. As it moves out of range of one
access point and into range of the other, it associates with the new AP.
The :ned:`DhcpClient` subscribes to the link-layer association signal; when
a new association is detected it unbinds the current lease and restarts the
DORA exchange, obtaining an address from the DHCP server on the new subnet.

Config header:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Roaming]
   :end-at: sim-time-limit = 200s

The configurator gives the two DHCP servers their static addresses on
``eth0`` and wires up the routing so the wired ``server`` host is
reachable through either subnet; the mobility module drives the client
back and forth across the two coverage areas. The DHCP-relevant
parameters:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # DHCP client
   :end-at: *.dhcpServer*.forwarding = true

As the client moves from one access point's coverage area to the other,
it disassociates from the old AP and associates with the new one. The
DHCP client detects the interface change and initiates a new DORA
exchange with the DHCP server on the new subnet, obtaining an address
from a different address range.

Sequence chart of the t≈17.6 s roam to ``ap2``: 802.11 association
traffic completes between ``client`` and ``ap2``, and the new DORA goes
out via ``ap2`` to ``dhcpServer2`` (192.168.2.x subnet), distinct from
the initial DORA which used ``ap1`` / ``dhcpServer1`` on 192.168.1.x:

.. figure:: media/roaming.png
   :align: center

The subnet handover is also visible on the canvas. Before the roam,
the client sits under ``ap1``'s coverage circle (left) with its
interface labelled ``wlan0 192.168.1.10/24``:

.. figure:: media/roaming_before.png
   :align: center

After the roam, the client has moved under ``ap2``'s coverage circle
(right) and the interface-table label has flipped to ``wlan0
192.168.2.10/24`` — confirming that the new DORA on the 192.168.2.x
subnet has bound:

.. figure:: media/roaming_after.png
   :align: center

..
   FIGURE RECIPE (redo via the "omnetpp-mcp-sim" skill)
   type:     canvas (two anchors)
   config:   Roaming                   # ../omnetpp.ini
   seed:     default
   shows:    roaming_before: client under ap1's coverage, label
             "wlan0 192.168.1.10/24" (before roam);
             roaming_after: client under ap2's coverage, label
             "wlan0 192.168.2.10/24" (after roam)
   anchor:   before: t=10 s (client at x=200 m, well inside ap1 range);
             after: t=25 s (client at x=500 m, inside ap2 range, after the
             t≈17.6 s roam DORA bound 192.168.2.10). The label refreshes on
             the next visualizer tick, so t≥20 s is safe.
   capture:  rebuild_network → run_simulation t=10s → get_canvas_image
             area=all_elements (before); run to t=25s → get_canvas_image
             (after); was 778×629 and 851×630
   stamp:    captured 2026-06, INET 4.6

The animation below shows the whole run from a fresh start. The client
DORAs on 192.168.1.x, then crosses the canvas left → right; at
t≈17.6 s it associates with ``ap2`` and a second DORA binds
192.168.2.10 — the interface-table label flips on the next visualizer
tick:

.. video:: media/roaming.mp4
   :align: center

..
   VIDEO RECIPE (redo via the "video-recording" skill)
   config:   Roaming                    # ../omnetpp.ini
   seed:     default
   shows:    initial DORA via ap1/dhcpServer1 (~t=0.6 s, label →
             192.168.1.10/24), 20 m/s rectilinear motion, roam DORA via
             ap2/dhcpServer2 at t≈17.6 s, label flip to 192.168.2.10/24
   anchors:  first bind ~t=0.6 s; roam association t≈17.6 s; second bind
             within microseconds of association. Mechanism must hold;
             timing is seed-dependent.
   window:   record sim-time 0s → ~22s
   anim:     playback_speed=1           # set_animation_parameters, normal profile
   capture:  fps=50, crop=670:580:870:140  # tight crop around the canvas
   encode:   ffmpeg -r 50 -vcodec libx264 -pix_fmt yuv420p (pad to even dims)
   post:     none
   stamp:    recorded 2026-06, INET 4.6

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`DhcpShowcase.ned <../DhcpShowcase.ned>`,
:download:`scenario.xml <../scenario.xml>`,
:download:`scenario_clean_shutdown.xml <../scenario_clean_shutdown.xml>`,
:download:`scenario_lease_expiration.xml <../scenario_lease_expiration.xml>`,
:download:`scenario_server_reboot.xml <../scenario_server_reboot.xml>`,
:download:`scenario_lossy_dora.xml <../scenario_lossy_dora.xml>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/general/dhcp`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/general/dhcp && inet'

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

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.

.. TODO: The DhcpServer does not expose a parameter for the DNS server address
   announced to clients (the field defaults to 0.0.0.0). Consider adding a
   ``dns`` parameter and documenting it in the parameter list above.
