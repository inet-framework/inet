# Neighbor Discovery — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what Neighbor Discovery
added, in one pass: level 2 on 2026-09-24, over RFC 4861, RFC 4862, RFC 5942 and RFC 6980, and
the repairs of the eleven gaps on 2026-09-25. The gaps themselves are in
[`results.md`](results.md#the-model-gaps).

## Model quirks

### The host stores what the router advertises, and uses little of it

**Fixed on** 2026-09-25 (gaps 1, 3 and 4): a datagram takes the CurHopLimit of its outgoing
interface, which `fragmentPostRouting` writes after routing; fragmentation uses a smaller
LinkMTU; address resolution, the probes and Duplicate Address Detection wait the RetransTimer,
a time that an advertisement sets from milliseconds. What follows describes the model before
the repair.

A host copies Cur Hop Limit, the MTU option and Retrans Timer out of each advertisement, and
then sends with other values:

- the hop limit of a packet comes from the constant `IPv6_DEFAULT_ADVCURHOPLIMIT`, 30
  ([Ipv6.cc:964](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L964),
  [Ipv6InterfaceData.h:31](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L31)),
  and no code reads the stored value (gap 1);
- fragmentation uses the MTU of the interface
  ([Ipv6.cc:1059](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1059)), and no code reads
  the stored LinkMTU (gap 3);
- address resolution waits the node constant RETRANS_TIMER
  ([Ipv6NeighbourDiscovery.cc:695](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L695)),
  not the advertised value (gap 4).

Duplicate Address Detection reads the advertised Retrans Timer, but as seconds
([Ipv6NeighbourDiscovery.cc:851](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L851)):
a router that advertised 1000 milliseconds would make its hosts wait 1000 s. No test of this
pass shows it. A check that sets a router variable and looks only at the advertisement passes;
look at what the host does next.

### The router variables have wrong defaults, and two of them have no parameter

**Fixed on** 2026-09-25 for three defaults (gaps 2 and 4): AdvCurHopLimit 64, AdvLinkMTU 0 with
no MTU option, AdvRetransTimer 0. AdvReachableTime still defaults to 3600 in a field of
milliseconds, and a host still reads that field as seconds; AdvReachableTime and
AdvRetransTimer still have no parameter. What follows describes the model before the repair.

`Ipv6RoutingTable` gives `advCurHopLimit = default(30)` and `advLinkMtu = default(1280)`
([Ipv6RoutingTable.ned:101-102](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.ned#L101)),
where RFC 4861 gives 64 and zero (gap 2). AdvReachableTime and AdvRetransTimer default to 3600
and 1, "seconds" in their comments, in fields of milliseconds
([Ipv6InterfaceData.h:35-36](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L35)),
where the standard gives zero. These two have no parameter at all, which RFC4861-RCFG-1 asks for.
Gaps 2 and 3 hide each other: after a repair of gap 3 alone, every host would send at most 1280
octets.

### The advertisement timer of the router has one time for two jobs

**Fixed on** 2026-09-25 (gaps 5 and 6): an answer sets the time of the next advertisement again,
a multicast answer restarts the timer, and the timers before the first three advertisements
are at most 16 s. The capped interval is exactly 16 s, so the timer now starts at the random
boot time of the router: with all routers at t = 0, twenty GPSR routers sent at the same
instant, and the radio medium stopped the run. What follows describes the model before the
repair.

`processRsPacket` answers a solicitation only when the answer comes before
`nextScheduledRATime`
([Ipv6NeighbourDiscovery.cc:1210](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L1210)),
and sets that time to the answer. `sendSolicitedRa`
([Ipv6NeighbourDiscovery.cc:1723](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L1723))
does not move it forward, so after the first answer the time stays in the past, and the router
answers no other solicitation (gap 5). `createRaTimer` draws the first periodic advertisement
from the whole interval, 198 to 600 s
([Ipv6NeighbourDiscovery.cc:1639](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L1639)),
and only later timers get the cap of the first advertisements (gap 6). Together they give a host
that comes up late no advertisement for up to ten minutes.

### A host drops every advertisement while it tests an address

**Fixed on** 2026-09-25 (gap 7): the Default Router List and the parameters of the link follow
every valid advertisement; the prefixes of an advertisement during a detection still wait,
because a new address would start a second detection. What follows describes the model before
the repair.

`processRaPacket` drops an advertisement during Duplicate Address Detection
([Ipv6NeighbourDiscovery.cc:1380](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L1380)),
next to the TODO "improve this procedure in order to allow reinitiating DAD". The advertisement
of one router starts the test of a new address, and the advertisement of a second router, 45 ms
later, is lost (gap 7). With gaps 5 and 6 the second router sends no other one in time. The
host also stops its solicitations after any valid advertisement
([Ipv6NeighbourDiscovery.cc:1388](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L1388)),
also one with Router Lifetime zero; no check of this pass isolates that. A host that comes up late
sends its solicitation without the Source Link-Layer Address option, which RFC4861-RS-12 asks
for; a trace showed it, and no check observes it yet.

### The Redirect is the fixed part only

**Fixed on** 2026-09-25 (gap 8): the Redirect carries the Target Link-Layer Address option when
the router knows the target, and the new `Ipv6NdRedirectedHeader` option with as much of the
invoking packet as fits into 1280 octets. A router builds the Redirect before it resolves the
next hop, so the first Redirect to an unresolved target has no Target Link-Layer Address
option, which the text allows. What follows describes the model before the repair.

`sendRedirect` sets a length of 40 octets and adds no option
([Ipv6NeighbourDiscovery.cc:2379](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L2379)),
and the model has no class for the Redirected Header option (gap 8). A host of the model follows
the Redirect anyway. A repair needs the option class and a case in the serializer first.

### The solicited-node groups are a match, not a membership

**Fixed on** 2026-09-25 (gap 9): an interface joins the solicited-node group of each unicast
address, one membership for all addresses of the group, because the join and leave signals
drive MLDv1 on every call. What follows describes the model before the repair.

A node accepts a packet to a solicited-node address because the address matches one of its own
([Ipv6InterfaceData.cc:375](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc#L375)),
and it never joins the group, so MLD never reports it (gap 9). The MLD pass found the same cause
from the other side. A switch that snoops MLD would drop every solicitation of address
resolution and of Duplicate Address Detection.

### The addresses of the configurator are never tested, and an expired address stays

**Fixed on** 2026-09-25 (gaps 10 and 11): the configurator assigns a tentative address, Neighbor
Discovery tests every tentative address when the node has booted, and a timer removes an address
when its valid lifetime ends. What follows describes the model before the repair.

The configurator assigns its addresses as not tentative
([Ipv6NetworkConfigurator.cc:172](../../../../../src/inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.cc#L172)),
so a router never tests its global address (gap 10). A host removes an expired address only when
an address is added or removed
([Ipv6InterfaceData.cc:446](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc#L446)),
and `getPreferredAddress` returns the cached one, next to "FIXME TODO check expiry time!"
([Ipv6InterfaceData.h:559](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L559))
(gap 11). The prefix itself expires correctly.

### MLD is off, and when it is on, the router reports from its global address

`Ipv6NetworkLayer` has `hasMld = false`
([Ipv6NetworkLayer.ned:72](../../../../../src/inet/networklayer/ipv6/Ipv6NetworkLayer.ned#L72)).
With MLD on, the router reports ff02::2 from its global address, where MLD asks for a link-local
source; that rule belongs to MLD, and the MLD pass records it.

## Scenario quirks

### The configurator hides Neighbor Discovery

With its defaults, `Ipv6NetworkConfigurator` gives every host with one interface a default route
([Ipv6NetworkConfigurator.cc:928-960](../../../../../src/inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.cc#L928)),
and on-link routes towards the routers, also with `assignAddressesToHosts = false`. The first run
measured the configurator: a host reached its router without an advertisement, and gaps 5, 6 and
7 did not show. The mockups now set `assignAddressesToHosts = false` and `addStaticRoutes = false`,
and give each router its routes in `<route>` elements: an on-link route has an interface and no
gateway, and a remote route has the link-local address of the next router as its gateway. A host
has no route until an advertisement gives it one.

### The mockups

- `Router6` and `StandardHost6` on an `EthernetSwitch`, with a fixed MAC address on every
  interface (`0A-00-00-00-00-XX`), so the link-local addresses are known, fe80::800:ff:fe00:XX,
  and so are the addresses that SLAAC forms, for example 2001:db8:1::800:ff:fe00:a.
- `**.ipv6.hasMld = true` only in the two checks of the multicast groups.
- A node that comes up late: `**.hasStatus = true`, `initialStatus = "down"` and
  `<startup module='r2'/>` in the `ScenarioManager`.

### Start the second router late when gap 7 would hide the rule

With all nodes up at once, host A dropped the advertisement of R1, because the one of R2 had
started the test of an address, and the Redirect checks never reached their stimulus. The
Redirect checks now start R2 and host C at 5 s. `Rfc4861RouterLifetimeZero` keeps the start at
the same instant and fails on gap 7: one check shows the gap, the others reach their own rules.

The order of the two answers was random, and the repair of gap 6 changed the draws, so the test
passed with gap 7 still in the code. A delay of 250 ms on the link of R1 now makes the order
structural: the answer of R1 always comes after the answer of R2, and during the detection that
it starts.

### A repair moves the random draws, and a deadline at the exact bound fails

- A repair that draws more random numbers changes every later draw. Gap 6 changed the order of
  two answers in `Rfc4861RouterLifetimeZero`, and gap 9 swapped the roles of the two hosts of the
  MLDv1 module tests, which pin a seed for that; they now pin seed 0. After such a repair, look
  at each test whose premise is an order of random events.
- The deadline of a step fires before an event of the same instant. A capped advertising
  interval is exactly 16 s, and `within(16.0)` failed on it; the check says "at most 16 s", so
  the test now waits 17 s and asserts the bound.
- A check whose premise the model does not set up fails for the wrong reason: the Redirect test
  assumed that R1 knows R2, but a router builds the Redirect before it resolves the next hop. The
  test now makes R1 ping R2 first.

### A router variable that the model cannot set

AdvReachableTime and AdvRetransTimer have no parameter. `NdRouterVariables` of
[NdChecks.h](../../../../../tests/protocol/nd/NdChecks.h) is a small management module that sets
them in the interface data at `INITSTAGE_LAST`. Its NED needs
`@class(::inet::protocoltest::NdRouterVariables)`, because `opp_test` puts the NED of a test into
a namespace of its own. The missing parameter is a finding, not a test error.

### A check stricter than the standard, and checks that share a scenario

- The first fields check demanded the Source Link-Layer Address option in every advertisement.
  RFC4861-RA-24 lets a router leave it out, so the check and its test now accept its absence.
  Read the `may` entries next to each observation before a failure becomes a gap.
- Two scenarios serve two checks each, so that one rule does not hide another: a prefix with the
  L flag clear (on-link determination, then the Redirect to an on-link destination), and a prefix
  with a valid lifetime of 30 s and a router that stops (the prefix, then the address).
- The check of RFC 6980 needs a Redirect that would exceed the MTU of the link: an echo request
  of 1448 octets (`packetSize = 1400B`) makes one, if the router does not cut the redirected
  packet.
- Where one failing field hid other verdicts, one check became several tests: the Router
  Advertisement fields (fields, MTU option, Cur Hop Limit) and the Redirect fields (header with
  the Target Link-Layer Address option, Redirected Header option).

### Two readings of the text, and three flaws of RFC 4861

The checks fix two readings: "about every RetransTimer" is 1 to 1.5 times the timer, and the cap
of the first unsolicited advertisements holds for the intervals after the first and the second
one. The catalog names three flaws of RFC 4861 itself: RetransTimer in seconds in §7.2.6, the
default of MinRtrAdvInterval, and the name CurHopLimit in §6.2.3. Three features are `optional`,
because the `must` of each holds only once a node does the optional thing: unsolicited
advertisements, the processing of a Redirect, and the change of the role of a router.

## Tooling quirks

### Reading an ND message out of a frame

The icmpv6 dissector reaches every ND message, so a filter can select one by `icmpv6.type`. But
an address field is an object, the options are a list of objects, and many checks compare a field
with an address that exists only at run time. [NdChecks.h](../../../../../tests/protocol/nd/NdChecks.h)
reads the chunks instead:

- `ndOf`, `raOf`, `nsOf`, `naOf`, `rsOf`, `redirectOf`, and `isRa` and its siblings;
- `optionOf`, `hasOption` and `linkLayerOption` for the options;
- `findChunks`, because an ICMPv6 error carries the header of the packet that caused it after its
  own;
- `ndBytesOf` and `ndOptionBytes`, the octets as the serializer writes them, for the Reserved
  fields and the Redirected Header option, which the model's classes do not hold;
- `require`, an assertion that throws a sentence.

### The exploration test and the generators

`mkexplore.py` in `audit/nd-level2/` of `inet-master` (outside git) makes `ZzExplore.test`, a copy
of a test that prints every ICMPv6 message of the named MACs. It found the configurator trap and
the cause of every gap. Never commit it. `gen-nd-tests.py` holds the five mockups, the MAC
addresses and the routes; `gen-nd-specs.py` writes the 37 tests; `gen-nd-ledger.py` writes the
tables of the ledger; `check-nd-placement.py` finds a catalog entry that is in no check and not in
the closing list. The `README.md` there lists them all.

## Follow-ups, in the order I would do them

The eleven gaps are repaired, on `topic/standards-tests-nd-level2-fixes`; the follow-ups that
repaired them are gone from this list.

1. **The Reachable Time in milliseconds**, the same repair as gap 4: a host sets BaseReachableTime
   from the field in milliseconds, and AdvReachableTime defaults to zero. Give AdvReachableTime
   and AdvRetransTimer a parameter (RFC4861-RCFG-1), and remove `NdRouterVariables` from the
   tests.
2. **The Source Link-Layer Address option of a host that comes up late** (RFC4861-RS-12): a
   trace of `Rfc4861RaSolicited` showed a solicitation of host B without it; write the check
   first.
3. **Level 3 and the 88 owed statements**: the validation of every received message, the
   receiver halves of the Reserved fields and of the options, and the fragmented ND messages of
   RFC 6980. See [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes), and
   the closing list of
   [`checks.md`](../../protocol/nd/checks.md#statements-this-pass-wrote-no-check-for) for the level
   2 statements that this pass left.
