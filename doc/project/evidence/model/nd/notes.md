# Neighbor Discovery — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what Neighbor Discovery
added, in one pass: level 2 on 2026-09-24, over RFC 4861, RFC 4862, RFC 5942 and RFC 6980. The
gaps themselves are in [`results.md`](results.md#the-model-gaps).

## Model quirks

### The host stores what the router advertises, and uses little of it

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

`Ipv6RoutingTable` gives `advCurHopLimit = default(30)` and `advLinkMtu = default(1280)`
([Ipv6RoutingTable.ned:101-102](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.ned#L101)),
where RFC 4861 gives 64 and zero (gap 2). AdvReachableTime and AdvRetransTimer default to 3600
and 1, "seconds" in their comments, in fields of milliseconds
([Ipv6InterfaceData.h:35-36](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L35)),
where the standard gives zero. These two have no parameter at all, which RFC4861-RCFG-1 asks for.
Gaps 2 and 3 hide each other: after a repair of gap 3 alone, every host would send at most 1280
octets.

### The advertisement timer of the router has one time for two jobs

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

`sendRedirect` sets a length of 40 octets and adds no option
([Ipv6NeighbourDiscovery.cc:2379](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc#L2379)),
and the model has no class for the Redirected Header option (gap 8). A host of the model follows
the Redirect anyway. A repair needs the option class and a case in the serializer first.

### The solicited-node groups are a match, not a membership

A node accepts a packet to a solicited-node address because the address matches one of its own
([Ipv6InterfaceData.cc:375](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc#L375)),
and it never joins the group, so MLD never reports it (gap 9). The MLD pass found the same cause
from the other side. A switch that snoops MLD would drop every solicitation of address
resolution and of Duplicate Address Detection.

### The addresses of the configurator are never tested, and an expired address stays

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

1. **The advertisements of the router, and the host during address tests** (gaps 5, 6 and 7):
   move `nextScheduledRATime` forward with each answer, cap the first periodic advertisement,
   and keep the advertisements that come during Duplicate Address Detection. Together they leave
   hosts without a default router. Then run `Rfc4861RouterLifetimeZero`, `Rfc4861RaSolicited`
   and `Rfc4861RaUnsolicited` again: RFC4861-RA-17, ADV-19 and the intervals of ADV-22 and
   ADV-36 get their first verdicts.
2. **Use what the router advertises** (gaps 1, 3 and 4): the stored CurHopLimit when a packet
   leaves, the stored LinkMTU for fragmentation, and the advertised Retrans Timer for address
   resolution, in milliseconds, also in Duplicate Address Detection.
3. **The router defaults** (gap 2) in the same change as gap 3: 64 and zero. Give
   AdvReachableTime and AdvRetransTimer a parameter with a default of zero, and remove
   `NdRouterVariables` from the tests.
4. **Check the expiry of an address when it is used** (gap 11), at the FIXME of
   `getPreferredAddress`.
5. **Join the solicited-node groups** (gap 9) in the interface data. MLD then reports them without
   a change of its own, and MLD gap 5 closes with it.
6. **The options of the Redirect** (gap 8): a class for the Redirected Header option, its case in
   the serializer, then both options in `sendRedirect`.
7. **Test the addresses of the configurator** (gap 10): assign them tentative, so that Duplicate
   Address Detection runs.
8. **Level 3 and the 88 owed statements**: the validation of every received message, the
   receiver halves of the Reserved fields and of the options, and the fragmented ND messages of
   RFC 6980. See [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes), and
   the closing list of
   [`checks.md`](../../protocol/nd/checks.md#statements-this-pass-wrote-no-check-for) for the level
   2 statements that this pass left.
