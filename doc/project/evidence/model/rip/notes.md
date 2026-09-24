# RIP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what RIP added, in one pass:
level 2 on 2026-09-24, RIP version 2 and RIPng together, one twin test for each check. The gaps
themselves are in [`results.md`](results.md#the-model-gaps).

## Model quirks

### One module plays both protocols, and RIPng inherits the fields of version 2

`Rip` runs RIP version 2 or RIPng by its `mode` parameter, with one message class for both.
`RipPacket` has `version = 2` by default
([RipPacket.msg:60](../../../../../src/inet/routing/rip/RipPacket.msg#L60)), and no code sets it,
so every RIPng message says version 2 (gap 1). The RIPng entries also carry address family 2, a
field that RIPng does not have; nothing reads it, because RIPng has no serializer. The serializer
of version 2 names RFC 1058
([RipPacketSerializer.cc:16](../../../../../src/inet/routing/rip/RipPacketSerializer.cc#L16)).

### A RIPng answer to a unicast address has no interface

`Rip::sendPacket` attaches the outgoing interface, and for RIPng the link-local source address,
only when the destination is multicast
([Rip.cc:982-992](../../../../../src/inet/routing/rip/Rip.cc#L982)). The answer to a table request
goes to the link-local address of the requester, which needs its interface. IPv6 sends it out of
the first interface, where neighbor discovery fails, and drops it (gap 2). RIP version 2 has no
such problem, because an IPv4 address routes.

### The IPv6 routing table forgets the network of a link that comes back

When the carrier of an interface goes down, `Ipv6RoutingTable` deletes its routes, and it adds
nothing back when the carrier returns
([Ipv6RoutingTable.cc:182-190](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.cc#L182)).
RIP imports an interface route only on the `routeAdded` signal
([Rip.cc:357-371](../../../../../src/inet/routing/rip/Rip.cc#L357)), so a RIPng router never
advertises that network again (gap 3). `Ipv4RoutingTable::updateNetmaskRoutes` restores the
routes, which is why the IPv4 twin passes. The repair belongs to the IPv6 routing table, and it
can change other IPv6 suites.

### The timers of a route are one function, called when the router sends

`Rip::checkExpiredRoutes` does the expiry and the garbage collection of every route. It runs only
from `Rip::sendRoutes` ([Rip.cc:537, 551](../../../../../src/inet/routing/rip/Rip.cc#L537)), and
no timer of its own fires. Three gaps come from this one function:

- an expiry shows at the next update of the router, up to 30 s late (gap 4);
- it purges only learned routes, of type `RIP_ROUTE_RTE`
  ([Rip.cc:905-918](../../../../../src/inet/routing/rip/Rip.cc#L905)), so a network that the
  router lost itself stays at metric 16 for good (gap 5);
- it purges a route at the time of its last update plus 180 s plus 120 s, and
  `Rip::processResponse` refreshes that time with every entry of the next hop, metric 16 too
  ([Rip.cc:688-689](../../../../../src/inet/routing/rip/Rip.cc#L688)), so a repeated withdrawal
  keeps the route (gap 6).

After a timeout the same sum is right by accident, so the checks of the garbage collection after
an expiry pass. Together, gaps 5 and 6 keep every withdrawn network in the updates of the whole
domain. Repair the three gaps in one change.

### The periodic update takes the clock precaution, not the random offset

The update timer runs every 30 s exactly ([Rip.cc:136](../../../../../src/inet/routing/rip/Rip.cc#L136)),
and the start of each router is random (`startupTime`, uniform from 0 s to the update interval).
RFC 2453 §3.8 and RFC 2080 §2.3 require one of two precautions against synchronization: a clock
that load does not change, or a random offset each time the timer is set. The model takes the
first. The ledger keeps the random offset as `later`, a statistical test; that test must accept
a model without the offset, or it is stricter than the standard.

### Three malformed inputs stop the run

`Rip` throws on a message with an unknown command
([Rip.cc:159](../../../../../src/inet/routing/rip/Rip.cc#L159)), on a request whose entry of
address family 0 is not the whole-table form
([Rip.cc:503](../../../../../src/inet/routing/rip/Rip.cc#L503)), and on a request entry with an
unknown address family ([Rip.cc:515](../../../../../src/inet/routing/rip/Rip.cc#L515)). RFC 2453
§3.9.1 processes every request that is not the whole-table form entry by entry, so the second
throw breaks a rule; for the other two, the in-scope set has no rule. It is the pattern of IPv4,
TCP and QUIC: a crafted input reaches a `throw`, and the run stops. The level 3 checks meet it
first.

### No dissector reaches the RIP header

The UDP port table has no entry for port 520 or 521
([ProtocolGroup.cc:157-164](../../../../../src/inet/common/ProtocolGroup.cc#L157)), so
`RipProtocolDissector` is never called on a frame. A filter expression over `rip.*` cannot
match, and a capture shows the message as an unknown payload.

## Scenario quirks

### A route change comes from the lifecycle and the links

RIP has a lifecycle, so the scenarios use it:

- `**.hasStatus = true` on every node, then `<crash module='R1'/>` in the `ScenarioManager`
  stops the updates of R1: the stimulus of an expiry.
- `*.R3.status.initialStatus = "down"` and `<startup module='R3'/>` add a router, and with it a
  shorter path, later in the run.
- A lost network is a broken link. `<disconnect>` breaks one direction of a link, so break both
  gates, for example `hostA` `ethg$o[0]` and `R1` `ethg$o[0]`. `<connect>` with both inout gates
  and `channel-type='inet.node.ethernet.Eth100M'` brings the link back.

### Fix the start of each router when a check measures a timer

The default start of each router is random. In the first run the two routers of the expiry check
started 1.3 s apart. The late expiry of gap 4 then fell 1.3 s after the true instant, inside the
6 s window, and the check passed. The check now sets `*.R1.rip.startupTime = 1s` and
`*.R2.rip.startupTime = 16s`: the expiry falls between two updates of R2, and the check fails
15 s late. The note of the check says why. See also
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks).

### The start-up exchange looks like periodic updates

A router that starts asks its neighbors for their tables, and gets their answers. Each route that
it learns then causes a triggered update, and these updates can hold the marker of the check. The
first procedures observed from 0 s and read the start-up messages as periodic updates. The
transport and periodic checks now start to observe at 40 s, after the start-up exchange. The
marker rule is in [`checks.md`](../../protocol/rip/checks.md#rules-every-check-obeys).

### The IPv6 mockups

- `RipRouter` and `StandardHost` with `hasIpv4 = false` and `hasIpv6 = true`, and
  `**.rip.mode = "RIPng"`.
- `**.ipv6.configurator.networkConfiguratorModule = "configurator"`.
- On the configurator, `addRemoteRoutes = false` and `addDefaultRoutes = false`. Do not use
  `addStaticRoutes = false`: it also removes the on-link routes that RIPng imports.

### A mask is a prefix length

The route entry of the model holds a prefix length where RFC 2453 has a subnet mask. The helpers
take the prefix length, and each test names the mask it stands for: 24 for 255.255.255.0.

## Tooling quirks

### The tests read the message out of the frame

Because no dissector reaches RIP, [RipChecks.h](../../../../../tests/protocol/rip/RipChecks.h)
walks the chunks of the frame (`findChunk`, `ripOf`, `udpOf`, `ipv4Of`, `ipv6Of`), and reads the
entries of a message (`findEntry`, `metricOf`). `findEntry` skips the RIPng next hop entries,
with metric 0xFF. `isUpdate` and the periodic form with the marker follow the rules of
`checks.md`.

### Assertions that say why

`require(holds, "what the message held")` of `RipChecks.h` throws a sentence when an assertion
fails, and the tester prints "the predicate raised:" and the sentence. `entriesOf` gives the
entries of a message for that sentence.

### Two test errors of the first runs

- The rate tests anchored their windows at a first step that could match at 20 s, before the
  change they measured. A window starts at the previous match; see
  [`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks).
- The RIPng request test read the zero prefix of a whole-table request as `::`, and the model
  holds the unspecified address. Both are 16 zero octets on the wire, and the test accepts both.

### The generated tests and ledger

`gen-rip-tests.py` in `audit/rip-level2/` of `inet-master` (outside git) holds five network
templates, IPv4 and IPv6, and writes the tests from `rip-specs-1.py` and `rip-specs-2.py`.
`gen-rip-ledger.py` writes the statement table of the ledger from a mapping of all 168
statements, and refuses to run when a statement has no row. The `README.md` there lists the
scripts.

### Put a field that the model gets wrong last

Gap 1 fails the version of every RIPng message. The RIPng checks read the version last, so the
observations before it keep their verdicts. The expiry check lost its garbage-collection half to
the late expiry of gap 4, so the garbage collection became a check of its own.

## Follow-ups, in the order I would do them

1. **Set the version of RIPng to 1** (gap 1): one line where the message is made. Then run
   `Rfc2080UpdateFields` and `Rfc2080LostNetwork` again.
2. **Send a RIPng answer on the interface of the request** (gap 2): attach the interface and
   the link-local source address in `Rip::sendPacket` also for a link-local unicast
   destination. Then run `Rfc2080TableRequest` again.
3. **Give the route timers their own events** (gaps 4, 5 and 6, in one change): an expiry timer
   per route, a purge of the interface routes that the router lost, and no refresh of the last
   update time by a metric of 16. Then run both `RouteExpiry`, both
   `LostNetworkGarbageCollection` and both `WithdrawnRouteGarbageCollection` tests again, and
   the two `ExpiryGarbageCollection` tests, which pass now by accident.
4. **Restore the on-link routes of IPv6 when the carrier returns** (gap 3), in
   `Ipv6RoutingTable`, as `Ipv4RoutingTable::updateNetmaskRoutes` does. Then run
   `Rfc2080GarbageCollection` again: its observation 4, RFC2080-TIMER-7, gets its first verdict.
   Run the IPv6 and ND suites too.
5. **Add ports 520 and 521 to the UDP port table**, so that the dissector reaches RIP and a
   filter expression can read it.
6. **Level 3**: the validation of a response, the specific request, the route tag, the next hop
   of a received entry, and the three malformed inputs that stop the run. See
   [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes) for the 21 owed
   statements, and the closing list of
   [`checks.md`](../../protocol/rip/checks.md#statements-this-pass-wrote-no-check-for) for the
   level 2 statements that this pass left.
7. **Level 4**: the random timer of the triggered update, and a check of the precaution against
   synchronization that accepts either precaution.
