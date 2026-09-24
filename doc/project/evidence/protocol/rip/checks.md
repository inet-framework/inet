# RIP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc2453/catalog.md](../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../standard/rfc2080/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
group under [`checks/`](checks); the section names are the anchors that the coverage ledger
links to.

## Common mockups

Five networks serve every check. Each exists twice: once for RIP version 2 over IPv4, and
once for RIPng over IPv6, with the same names. Every link is an Ethernet link, every network
has the cost 1, and every timer has the value of the standard: a periodic update every 30
seconds, a timeout of 180 seconds, a garbage-collection time of 120 seconds.

| Network | IPv4 | IPv6 |
| --- | --- | --- |
| netA, the network of host A | 10.0.1.0/24 | 2001:db8:1::/64 |
| netB, the network of host B | 10.0.2.0/24 | 2001:db8:2::/64 |
| netC, the network of host C | 10.0.3.0/24 | 2001:db8:3::/64 |
| L1, between R1 and R2 | 10.0.12.0/24, R1 .1, R2 .2 | 2001:db8:12::/64 |
| L2, between R2 and R3 | 10.0.23.0/24, R2 .2, R3 .3 | 2001:db8:23::/64 |
| L3, between R1 and R3 | 10.0.13.0/24, R1 .1, R3 .3 | 2001:db8:13::/64 |

On IPv6 every router interface also has its link-local address, and a host takes its global
address from its router. No router has a static route to a remote network: every route that
is not directly connected comes from RIP.

**The pair.** Two routers on one link, each with a stub network and a host on it.

```
host A --netA-- R1 --L1-- R2 --netB-- host B
```

**The chain.** Three routers in a line.

```
host A --netA-- R1 --L1-- R2 --L2-- R3 --netC-- host C
```

**The triangle.** Three routers, each linked to the other two, with netA at R1 and netC at
R3.

```
host A --netA-- R1 --L1-- R2
                 \        |
                 L3       L2
                   \      |
                    R3 --netC-- host C
```

**The pair with many routes.** The pair, where R1 also holds static routes to remote
destinations through host A: 30 networks `10.100.k.0/24` for k from 1 to 30, and two routes
with the same network number and different masks, `10.200.0.0/16` and `10.200.0.0/24`. The
IPv6 twin holds 80 prefixes `2001:db8:100:k::/64` for k from 1 to 80, more than one message
of an Ethernet MTU can carry (see the size arithmetic of
[Messages limited by the MTU](checks/response-contents.md#messages-limited-by-the-mtu-ripng)).

**The pair with three stubs.** The pair, where R1 has three stub networks netA1, netA2 and
netA3 (IPv4 10.0.4.0/24, 10.0.5.0/24, 10.0.6.0/24; IPv6 2001:db8:4::/64 to 2001:db8:6::/64),
each with one host.

## Rules every check obeys

- **An observation names the link and the sender.** "On L1, from R1" means the messages that
  R1 sends onto L1, as they leave R1. The RIP header and the route entries are read from the
  message; the ports, the addresses and the hop limit from the UDP and IP headers around it.
- **An update, a periodic update, a triggered update.** An *update* is a response with
  command 2 that goes to the multicast group (224.0.0.9 or FF02::9). Each check names a
  *marker*: a network that the router is attached to and that the check leaves alone. A
  *periodic update* of the router is an update that holds the entry of the marker, because
  every periodic update holds the whole table that split horizon lets through. A *triggered
  update* is an update that does not hold it. A triggered update may hold more than the
  changed routes, so a check that must tell the two apart keeps its route changes away from
  the instants of the periodic updates it measures. The answer to a request goes to a
  unicast address, so it is neither.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the route was learned, the link went down, the router restarted
  — before the observation that checks the rule. A configuration that voids the stimulus
  then fails the check instead of passing it.
- **A time window follows the standard.** Where the standard gives a random interval, the
  window is the whole interval of the standard and never a value drawn from it: 25 to 35
  seconds between periodic updates in RIP version 2 and 15 to 45 seconds in RIPng, 1 to 5
  seconds for a triggered update. A check never demands a value from inside such an interval.
- **Split horizon has two lawful answers.** Where a route must not come back onto the
  network it was learned from, the observation accepts its absence and metric 16, the two
  forms RFC 2453 §3.4.3 permits, and fails only a finite metric.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Update transport and addressing (RIP version 2)](checks/update-message.md#update-transport-and-addressing-rip-version-2) | `checks/update-message.md` | RFC2453-MSG-1, MSG-2, MSG-3, MCAST-1, OUT-2 |
| [Update message fields (RIP version 2)](checks/update-message.md#update-message-fields-rip-version-2) | `checks/update-message.md` | RFC2453-MSG-9, MSG-10, MSG-12, GEN-2, MET-3, MET-4, MASK-1, NH-2 |
| [Update transport and addressing (RIPng)](checks/update-message.md#update-transport-and-addressing-ripng) | `checks/update-message.md` | RFC2080-MSG-1, MSG-2, MSG-3, OUT-3, GEN-1, RESP-5 |
| [Update message fields (RIPng)](checks/update-message.md#update-message-fields-ripng) | `checks/update-message.md` | RFC2080-MSG-8, GEN-3, MET-3, MET-4, MSG-10, GEN-5 |
| [Periodic update interval (RIP version 2)](checks/periodic-update.md#periodic-update-interval-rip-version-2) | `checks/periodic-update.md` | RFC2453-TIMER-1, TIMER-2 |
| [Periodic update interval (RIPng)](checks/periodic-update.md#periodic-update-interval-ripng) | `checks/periodic-update.md` | RFC2080-TIMER-1, OUT-2, TIMER-2 |
| [Metric through a chain (RIP version 2)](checks/route-learning.md#metric-through-a-chain-rip-version-2) | `checks/route-learning.md` | RFC2453-MET-5, RESP-6, TABLE-3; covers RFC2453-NH-1 |
| [Metric through a chain (RIPng)](checks/route-learning.md#metric-through-a-chain-ripng) | `checks/route-learning.md` | RFC2080-MET-5, RESP-8, TABLE-2 |
| [Shorter path kept (RIP version 2)](checks/route-learning.md#shorter-path-kept-rip-version-2) | `checks/route-learning.md` | RFC2453-RESP-9, RESP-12 |
| [Shorter path kept (RIPng)](checks/route-learning.md#shorter-path-kept-ripng) | `checks/route-learning.md` | RFC2080-RESP-11, RESP-14 |
| [Split horizon on the link of a learned route (RIP version 2)](checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | `checks/split-horizon.md` | RFC2453-SH-1, SH-2; covers RFC2453-GEN-5 |
| [Split horizon on the link of a learned route (RIPng)](checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | `checks/split-horizon.md` | RFC2080-SH-1, SH-2; covers RFC2080-GEN-7 |
| [Table request of a restarted router (RIP version 2)](checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `checks/table-request.md` | RFC2453-REQ-1, REQ-4, REQ-8, OUT-1, MSG-4, REQ-6, GEN-1 |
| [Table request of a restarted router (RIPng)](checks/table-request.md#table-request-of-a-restarted-router-ripng) | `checks/table-request.md` | RFC2080-REQ-1, REQ-4, REQ-8, OUT-1, MSG-4, REQ-6; covers RFC2080-GEN-1 |
| [Triggered update for a lost network (RIP version 2)](checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `checks/triggered-update.md` | RFC2453-TRIG-1, TRIG-5, TRIG-9, GEN-5, TRIG-6, TIMER-5, RESP-9 |
| [Triggered update for a lost network (RIPng)](checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `checks/triggered-update.md` | RFC2080-TRIG-4, TRIG-8, GEN-7, TRIG-5, OUT-2, TIMER-5, RESP-11 |
| [Rate of triggered updates (RIP version 2)](checks/triggered-update.md#rate-of-triggered-updates-rip-version-2) | `checks/triggered-update.md` | RFC2453-TRIG-2, TRIG-3 |
| [Rate of triggered updates (RIPng)](checks/triggered-update.md#rate-of-triggered-updates-ripng) | `checks/triggered-update.md` | RFC2080-TRIG-1, TRIG-2 |
| [Route expiry after a silent neighbor (RIP version 2)](checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2) | `checks/route-expiry.md` | RFC2453-TIMER-4, TIMER-5, TIMER-6, RESP-7 |
| [Route expiry after a silent neighbor (RIPng)](checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng) | `checks/route-expiry.md` | RFC2080-TIMER-4, TIMER-5, TIMER-6, RESP-9 |
| [Garbage collection ended by a new route (RIP version 2)](checks/route-expiry.md#garbage-collection-ended-by-a-new-route-rip-version-2) | `checks/route-expiry.md` | RFC2453-TIMER-7 |
| [Garbage collection ended by a new route (RIPng)](checks/route-expiry.md#garbage-collection-ended-by-a-new-route-ripng) | `checks/route-expiry.md` | RFC2080-TIMER-7 |
| [More than 25 routes (RIP version 2)](checks/response-contents.md#more-than-25-routes-rip-version-2) | `checks/response-contents.md` | RFC2453-GEN-3, MSG-7, TABLE-2 |
| [Messages limited by the MTU (RIPng)](checks/response-contents.md#messages-limited-by-the-mtu-ripng) | `checks/response-contents.md` | RFC2080-GEN-4, MSG-11 |

Twenty-four checks, and all of them are level 2: they observe a normal exchange, and the
stimuli are a router that starts or stops and a link that goes down or comes back, which a
scenario of the network sets up without touching a message.

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature, and the checks above give one to
every mandatory feature whose core statements a normal exchange can reach. The statements
below have no check in this pass. This section says **what a check would need**. It says
nothing about the simulation model: whether the absence of a check is acceptable is a
judgment about what the model claims, and that judgment lives in the coverage ledger.

**A crafted message** — a response or a request that no router of the mockups sends. This is
the toolset of level 3:

- the validation of a response: RFC2453-RESP-2, RESP-3, RESP-4, RESP-5 and RFC2080-RESP-2,
  RESP-3, RESP-4, RESP-6, RESP-7 need a response from a wrong port, from a stranger, from the
  router itself, with a wrong hop limit, or with invalid entries;
- the specific query: RFC2453-REQ-2, REQ-3, REQ-5, REQ-7 and RFC2080-REQ-2, REQ-3, REQ-5,
  REQ-7 need a querier that sends a request with named entries, or with none, from a port
  that is not the RIP port; RFC2453-MSG-5 and RFC2080-MSG-5 need the same querier;
- the route tag: RFC2453-TAG-1 and RFC2080-TAG-1 need a route that arrives with a tag other
  than zero;
- the next hop of a received entry: RFC2453-NH-3, NH-4 and RFC2080-NH-4, NH-6, NH-7 need a
  response that names a next hop other than its sender;
- the next hop entry of RIPng, RFC2080-NH-1 and NH-5, needs a response that holds one;
- the query of version 1, RFC2453-QRY-1, needs a request with version 1.

**A version 1 router in the network**, the interworking of level 5: RFC2453-ADDR-2, ADDR-3,
ADDR-4, MASK-2.

**A distribution**, a statistical test of level 4: the randomness of the offset of the periodic
update (the random half of RFC2453-TIMER-2 and RFC2080-TIMER-2) and of the triggered-update
timer (the random half of RFC2453-TRIG-3 and RFC2080-TRIG-2). The checks above test the
bounds of these intervals, not their distribution.

**The bit layout**, a serializer test: RFC2453-MSG-6, MSG-8, MSG-11 and RFC2080-MSG-6, MSG-7,
MSG-9, NH-2, NH-3.

**A second message path or a larger mockup**, work for a later level 2 pass:

- a triggered update suppressed because a periodic one is due, RFC2453-TRIG-4 and
  RFC2080-TRIG-3, needs a change timed within a few seconds of a periodic update;
- the clearing of the change flags, RFC2453-TRIG-8 and RFC2080-TRIG-7, needs two changes some
  seconds apart;
- the switch at half the timeout, RFC2453-RESP-11 and RFC2080-RESP-13, needs two equal paths
  and a next hop that goes silent;
- three routers on one broadcast network, RFC2453-SH-4;
- a configured cost other than 1, RFC2453-MET-2 and RFC2080-MET-2, and the per-interface
  control of split horizon, RFC2080-SH-3;
- a default route, RFC2453-ADDR-6, ADDR-7 and RFC2080-ADDR-1, ADDR-2, ADDR-3, needs a router
  that originates one;
- a host route, RFC2453-ADDR-1 and ADDR-5, needs a static route to one address;
- an interface with two link-local addresses, RFC2080-GEN-2;
- a network without broadcast, RFC2453-MCAST-3;
- a neighbor that keeps sending a withdrawn route with metric 16, RFC2453-RESP-10 and
  RFC2080-RESP-12 (the deletion starts only once);
- a route imported with a configured tag, RFC2453-TAG-2 and RFC2080-TAG-2;
- a second network behind the receiver of a multicast, to see that the multicast is not
  forwarded, RFC2453-MCAST-2.

**Another suite.** RFC2453-ADDR-8, the most specific match, is the forwarding rule of IPv4.

**No check of their own, because a check above shows them** — the ledger gives these
`covered` and names the check:

- the content of an entry and the range of a metric, RFC2453-GEN-4, MET-1 and RFC2080-GEN-6,
  MET-1: every update message check reads them;
- the same processing for every cause of a response, RFC2453-RESP-1 and RFC2080-RESP-1, and
  the content of a route, RFC2453-TABLE-1 and RFC2080-TABLE-1: every check that learns a route
  shows them;
- the refresh of the timeout by the next hop, RFC2453-RESP-8 and RFC2080-RESP-10, and the two
  timers of a route, RFC2453-TIMER-3 and RFC2080-TIMER-3: the checks of route expiry show them;
- multicast on every interface, RFC2453-MCAST-4: the transport check observes both networks
  of R1.

**Permissions**, which no observation can fail: RFC2453-SH-3 (the variants of split horizon),
RFC2453-TRIG-7 and RFC2080-TRIG-6 (a route or an update that need not be sent). They widen
what the checks above accept.
