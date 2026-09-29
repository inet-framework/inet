# ND — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/nd/standards.md), [features.md](../../protocol/nd/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow. Part 1 records what the model intends: which
standards it claims to implement, mapped onto the standards map. Part 2 crosses the claims
with the feature support of the ledger, feature by feature.

Run record of the ledger state that part 2 comes from:

- Date: 2026-09-29 17:29 +0200
- INET: branch `master`, commit `24675c3a37`, tree clean
- Trees: src `8b4f86968e`, tests/protocol `1f1d62beca`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/nd$'`

Part 1 was first read by the level 1 pass, at src `16dc528e10`, commit `e360ca980e` of the wave 0
branch. The ND code is the same in `5c4f41c600`: the two trees differ only in seven files of
IEEE 802.11, and the tree of the run, `8b4f86968e`, differs from it in one line of
`src/inet/common/InitStages.cc`. The line numbers of part 1 therefore hold for all three.

## Part 1 — the claims

Every place in the model that names a document of the ND family. Source:
`src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.{cc,h,ned}`,
`Ipv6NeighbourCache.h`, `Ipv6NdMessage.msg`, `doc/src/users-guide/ch-ipv6.rst`, `WHATSNEW`.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2461 | [`Ipv6NeighbourDiscovery.h:37`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h) | "Implements RFC 2461 Neighbor Discovery for Ipv6." The doxygen comment directly above the module class — the first place a reader of the header looks. |
| RFC 2461 | [`Ipv6NeighbourCache.h:21`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourCache.h) | "Ipv6 Neighbour Cache (RFC 2461 Neighbor Discovery for Ipv6)." The class doc of the cache the module uses. |
| none | [`Ipv6NeighbourDiscovery.ned:12-13`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.ned) | "Implements IPv6 Neighbor Discovery." The module documentation comment, which the module reference publishes, names **no document at all**. |
| RFC 4861, RFC 4862, RFC 4429 | [`Ipv6NeighbourDiscovery.ned:29-37`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.ned) | Six parameter comments: `minIntervalBetweenRAs`/`maxIntervalBetweenRAs` (RFC 4861: MinRtrAdvInterval/MaxRtrAdvInterval), `dupAddrDetectTransmits` (RFC 4862 §5.1), `optimisticDad` (RFC 4429, named explicitly), `sendGratuitousNa` (RFC 4861 §7.2.6), `retransTimer`/`baseReachableTime` (RFC 4861 §10). |
| RFC 2461 | [`Ipv6NeighbourDiscovery.h:213,307,326,347`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h) | Four method-level doc comments citing RFC 2461 §6.3.5, §6.3.7, §6.3.4, §6.2.6. |
| RFC 4862 | [`Ipv6NeighbourDiscovery.h:274`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h) | "Called when DAD detects a duplicate address (RFC 4862, Section 5.4.5)." |
| RFC 2462 | [`Ipv6NeighbourDiscovery.h:282,285`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h) | A comment block that quotes "RFC 2462-Ipv6 Stateless Address Autoconfiguration: Section 1" before the autoconfiguration methods. |
| RFC 2461 | [`Ipv6NeighbourDiscovery.cc`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc), 24 lines: `256,453,634,654,726,746,949,1007,1125,1151,1262,1270,1391,1421,1688,1737,1819,1850,1920,1932,1978,2063,2185,2472` | Inline comments naming an RFC 2461 section above the code that implements it — validation of RA/NS/NA, router-advertisement sending, redirect processing, the conceptual sending algorithm. |
| RFC 4861 | [`Ipv6NeighbourDiscovery.cc:955,1067,1209,1358,1949,2384,2406`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc) | Inline comments naming an RFC 4861 section: router-solicitation retry (§6.3.7), RA rate limiting (§6.2.6), NCE creation on receiving an NS with SLLAO (§7.2.3), building and processing a Redirect (§8.2, §8.3). |
| RFC 4862 | [`Ipv6NeighbourDiscovery.cc:815,850,966,1607,1901,2565`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc) | Inline comments naming an RFC 4862 section: disabling DAD when `DupAddrDetectTransmits` is zero (§5.4), the join delay (§5.4.2), invalidating an autoconfigured address (§5.5.3), the DAD-in-progress duplicate check (§5.4.3). |
| RFC 2462 | [`Ipv6NeighbourDiscovery.cc:832,1896`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc) | Two inline comments naming RFC 2462 §5.4.2 and §5.4.3, the same subject as the current RFC 4862 citations two lines away in the same function family. |
| RFC 7527 (not claimed) | [`Ipv6NeighbourDiscovery.cc:1901-1904`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc) | "In a real stack, looped-back self-generated NS would need to be filtered (e.g. via RFC 7527 Nonce option). In simulation, the MAC layer never delivers a frame back to the sender... treat as duplicate." A reasoned non-claim: the comment names the document and gives the reason the model does not need it. |
| RFC 3775 (a different protocol, MIPv6) | [`Ipv6NeighbourDiscovery.cc:1284,2499`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc); [`Ipv6NdMessage.msg:33-34,64,90,101,156`](../../../../../src/inet/networklayer/icmpv6/Ipv6NdMessage.msg); [`Ipv6NeighbourCache.h:64`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourCache.h) | The H-bit, Advertisement Interval and Home Agent Information options that MIPv6 adds to a Router Advertisement. Out of the ND family; belongs to the mipv6 protocol's own pass. |
| `draft-ietf-ipv6-2461bis-*` (pre-RFC 4861 drafts) | [`Ipv6NeighbourDiscovery.cc:246-247,564,2279`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc) | Three comments citing IETF working drafts that RFC 4861 superseded on publication. No register entry; historical curiosities, not actionable. |
| RFC 2461 | [`Ipv6NdMessage.msg:18,19,20,22,26,126,155,192,209`](../../../../../src/inet/networklayer/icmpv6/Ipv6NdMessage.msg) | Option-length constants and message type comments. |
| RFC 4861 | [`Ipv6NdMessage.msg:63,142,178`](../../../../../src/inet/networklayer/icmpv6/Ipv6NdMessage.msg) | Line 63 cites both "RFC 2461 / RFC 4861" together for the Prefix Information option; lines 142 and 178 cite RFC 4861 alone for the NS and NA layouts. |
| RFC 4861, RFC 4862 | [`ch-ipv6.rst:37-38`](../../../../../doc/src/users-guide/ch-ipv6.rst), repeated at [`ch-ipv6.rst:127-128`](../../../../../doc/src/users-guide/ch-ipv6.rst) | "implements Neighbor Discovery (RFC 4861) and stateless address autoconfiguration (RFC 4862)." The user's guide — the document most readers reach first — names the current pair correctly, even though the class header does not. |
| RFC 4429 | [`ch-ipv6.rst:158`](../../../../../doc/src/users-guide/ch-ipv6.rst) | "optimistic DAD (`optimisticDad`, RFC 4429)." |
| RFC 4861 | [`WHATSNEW:406-410`](../../../../../WHATSNEW), [`WHATSNEW:511-514`](../../../../../WHATSNEW), [`WHATSNEW:5664`](../../../../../WHATSNEW) | Three release-note entries: the default Router Advertisement interval "follow[s] RFC 4861"; Redirect sending and processing "was implemented (RFC 4861)"; Default Router Selection "as specified in RFC 4861 6.3.6". |
| RFC 4862 | [`WHATSNEW:512`](../../../../../WHATSNEW) | DAD for autoconfigured global addresses "in accordance with RFC 4862." |

Mapped onto the standards map, [`standards.md`](../../protocol/nd/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 4861 | yes, `base` | **yes**, in the NED parameters, about a third of the `.cc` section citations, both message-format comments, the user's guide, and three WHATSNEW entries | The current claim is real and specific — it names sections. It sits beside a larger, older claim to RFC 2461 in the same files. |
| RFC 4862 | yes, `base` | **yes**, in the NED parameters, six `.cc` section citations, one `.h` comment, the user's guide, and one WHATSNEW entry | Same pattern: current and specific, alongside an older claim to RFC 2462. |
| RFC 2461 | no, obsoleted | **yes**, and this is the larger claim by line count | **The headline obsolete claim.** The module's own class doc (`.h:37`) and the cache's class doc (`Ipv6NeighbourCache.h:21`) — the two places a reader of the header files looks first — both name RFC 2461. Of roughly 33 section-numbered citations across `Ipv6NeighbourDiscovery.cc`, 24 name RFC 2461 and only 7 name RFC 4861; `Ipv6NdMessage.msg` has 9 RFC 2461 citations against 3 RFC 4861 ones. The obsolete document is cited about three times as often as the current one in the source, even though the NED file and the user's guide — what a module browser and a reader of the manual actually see — are current. |
| RFC 2462 | no, obsoleted | named, alongside the current claim | Two `.h` lines and two `.cc` lines name RFC 2462 for the same DAD subject that six other `.cc` lines and one `.h` line name RFC 4862 for. Smaller than the RFC 2461 gap, but the same shape. |
| RFC 4429 | no, level 5 | **yes**, by name, in the NED parameter and in the user's guide | A claimed document outside the in-scope set — a scope gap for a level 5 pass, not a verdict now. The claim is honest: `optimisticDad` defaults to `false`, and the comment states the RFC 4862 behavior is what runs by default. |
| RFC 5942 | yes, override | no | Nothing in the model names it. The on-link override is unclaimed either way — see the facts below. |
| RFC 6980 | yes, override | no | Nothing in the model names it or the fragmentation prohibition. |
| RFC 7527 | no, level 5 | **declined, with a reason** | The one mention (`cc:1901-1904`) explains why the mechanism is not needed in a simulation: the MAC layer never loops a frame back to its sender, so the condition RFC 7527 guards against cannot occur. A reasoned non-claim, not a gap. |
| RFC 3775 | no, a different protocol (MIPv6) | named, but out of family | Nine citations across three files carry MIPv6 option flags on top of an ND message. They belong to the mipv6 protocol's own claims and its own pass, not to ND. |
| RFC 7048, RFC 7559, RFC 8028, RFC 8319, RFC 8425, RFC 9131, RFC 9685, RFC 9762, RFC 9926 | no, level 4/5/administrative | no | Nothing in the model names any of these nine documents. |
| RFC 4443 | yes, companion of the ipv6 pass | no, not from ND | The ipv6 pass's own claims document, not repeated here. |

**The finding of the survey.** The model claims the current base pair, RFC 4861 and
RFC 4862, in the places that matter most to a reader: the NED parameter documentation and
the user's guide. But the class-level documentation comments of both the module and its
cache — what a C++ API browser shows first — still name RFC 2461, and the bulk of the
inline source comments, roughly three obsolete citations for every current one, agree with
the class doc rather than the NED file. The two vintages sit side by side in the same
functions without a note that one superseded the other. RFC 4429 is claimed by name outside
the in-scope set, honestly, as an option that defaults off. RFC 7527 is the one document the
model discusses and declines with a stated reason. Nine of the twelve documents that update
RFC 4861 are never mentioned at all, which is expected — none of them changes a clause this
pass brought into scope.

For the level 2 pass, five facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. Two methods are declared with a body-stub comment, "TODO Not implemented yet!"
   (`Ipv6NeighbourDiscovery.h:233,238`: `processArTimeout`, `dropQueuedPacketsAwaitingAr`,
   the address-resolution timeout and the queued-packet cleanup). A bare TODO is a claim; a
   check that reaches this path and fails is a defect, not a declared expected failure.
2. Anycast target handling (`cc:2002`, "TODO: anycast target address handling is not
   implemented") and multiple global addresses configured from different advertised prefixes
   (`cc:2513-2514`, "not supported with this code") are both named refusals with a reason —
   declarable, at whatever level first exercises a second prefix or an anycast target.
3. The RFC 7527 mention (`cc:1901-1904`) is a declarable non-claim for the same reason: the
   comment names the document and gives the reason it does not apply to a simulated MAC layer.
4. Four `// TODO improve this code` comments remain without a reason
   (`cc:921,1382,1447,2581`, DAD and RA edge-case refinement) — bare TODOs, claims by the
   guide's table, so a check that reaches one of these paths and fails is a defect.
5. The on-link definition of RFC 4861 §2.1 is unclaimed in either its original or its
   RFC 5942-amended form: nothing in the model builds or consults a Prefix List /
   Destination Cache "on-link" test the way the two documents describe it. A level 2 pass
   that adds RFC 5942 to the catalog should read the model's actual on-link logic before
   assuming either version is implemented.

## Part 2 — the conformance matrix

A feature is `claimed` when the claims of part 1 cover its governing source document. The
model claims RFC 4861 and RFC 4862 as wholes, and every feature of the map comes from one of the
two; the two features that RFC 5942 and RFC 6980 change, ND-F-ON-LINK-DETERMINATION and
ND-F-NO-FRAGMENTATION, rest on RFC 4861 first. So every feature of the map is claimed. The
support comes from [`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [ND-F-MESSAGE-FORMAT](../../protocol/nd/features.md#nd-f-message-format) | mandatory | yes | supported | `confirmed` |
| [ND-F-OPTIONS](../../protocol/nd/features.md#nd-f-options) | mandatory | yes | supported | `confirmed` |
| [ND-F-MULTICAST-GROUPS](../../protocol/nd/features.md#nd-f-multicast-groups) | mandatory | yes | partial | `partial` — RFC4861-AR-4: no solicited-node group is joined, [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) |
| [ND-F-ROUTER-SOLICITATION](../../protocol/nd/features.md#nd-f-router-solicitation) | optional | yes | partial | `partial` — the random delay of RFC4861-HOST-47 is level 4 |
| [ND-F-ROUTER-ADVERTISEMENT](../../protocol/nd/features.md#nd-f-router-advertisement) | mandatory | yes | partial | `partial` — RFC4861-ADV-7, ADV-11: two defaults, [gap 2](results.md#gap-2-defect--two-defaults-of-the-router-are-wrong); ADV-29: only the first solicitation is answered, [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| [ND-F-ADVERTISEMENT-TIMING](../../protocol/nd/features.md#nd-f-advertisement-timing) | mandatory | yes | partial | `partial` — RFC4861-ADV-23, ADV-32 fail, and ADV-22, ADV-36 are not reached, [gap 5](results.md#gap-5-defect--the-router-answers-only-its-first-solicitation) and [gap 6](results.md#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) |
| [ND-F-ROUTER-DISCOVERY](../../protocol/nd/features.md#nd-f-router-discovery) | mandatory | yes | partial | `partial` — RFC4861-RA-17 is not reached: [gap 7](results.md#gap-7-defect--a-host-drops-every-advertisement-during-duplicate-address-detection); HOST-12 passes with one router only |
| [ND-F-PARAMETER-DISCOVERY](../../protocol/nd/features.md#nd-f-parameter-discovery) | optional | yes | not supported | `declined` by the table of the guide, but the run shows no choice: the host copies each value and then does not use it, [gap 1](results.md#gap-1-defect--the-hop-limit-of-a-host-is-a-constant), [gap 3](results.md#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu), [gap 4](results.md#gap-4-defect--address-resolution-waits-a-constant-not-retranstimer) |
| [ND-F-ON-LINK-DETERMINATION](../../protocol/nd/features.md#nd-f-on-link-determination) | mandatory | yes | partial | `partial` — no core check fails; RFC4861-HOST-31, HOST-32 are owed, RFC5942-ONLINK-9, ONLINK-10 are level 3 |
| [ND-F-ADDRESS-RESOLUTION](../../protocol/nd/features.md#nd-f-address-resolution) | mandatory | yes | partial | `partial` — no core check fails; the state of RFC4861-AR-45 is level 4 |
| [ND-F-RESOLUTION-FAILURE](../../protocol/nd/features.md#nd-f-resolution-failure) | mandatory | yes | supported | `confirmed` |
| [ND-F-UNSOLICITED-ADVERTISEMENT](../../protocol/nd/features.md#nd-f-unsolicited-advertisement) | optional | yes | untested | `unverified` — owed at level 2: a node whose link-layer address changes |
| [ND-F-ANYCAST-AND-PROXY](../../protocol/nd/features.md#nd-f-anycast-and-proxy) | optional | yes | untested | `unverified` — the model declines anycast targets with a TODO, `Ipv6NeighbourDiscovery.cc:2002`; the proxy is owed |
| [ND-F-REDIRECT](../../protocol/nd/features.md#nd-f-redirect) | optional | yes | supported | `confirmed` — the core rules hold; the supporting options fail, [gap 8](results.md#gap-8-defect--the-redirect-carries-no-option) |
| [ND-F-REDIRECT-PROCESSING](../../protocol/nd/features.md#nd-f-redirect-processing) | optional | yes | partial | `partial` — no core check fails; the cache state of RFC4861-RDH-3, RDH-7 is level 4 |
| [ND-F-MESSAGE-VALIDATION](../../protocol/nd/features.md#nd-f-message-validation) | mandatory | yes | untested | `unverified` — level 3 |
| [ND-F-ROUTER-ROLE-CHANGE](../../protocol/nd/features.md#nd-f-router-role-change) | optional | yes | untested | `unverified` — owed at level 2 |
| [ND-F-ROUTER-CONSISTENCY](../../protocol/nd/features.md#nd-f-router-consistency) | optional | yes | untested | `unverified` — owed at level 2 |
| [ND-F-LINK-LOCAL-ADDRESS](../../protocol/nd/features.md#nd-f-link-local-address) | mandatory | yes | partial | `partial` — no core check fails; the infinite lifetime of RFC4862-LL-7 is level 4 |
| [ND-F-DUPLICATE-ADDRESS-DETECTION](../../protocol/nd/features.md#nd-f-duplicate-address-detection) | mandatory | yes | partial | `partial` — RFC4862-DAD-11: the solicited-node group is not joined, [gap 9](results.md#gap-9-defect--a-node-does-not-join-its-solicited-node-groups); DAD-20, DAD-21 are owed; the supporting DAD-1 fails, [gap 10](results.md#gap-10-defect--a-router-does-not-test-its-configured-address) |
| [ND-F-STATELESS-AUTOCONFIGURATION](../../protocol/nd/features.md#nd-f-stateless-autoconfiguration) | mandatory | yes | partial | `partial` — no core check fails; the two-hour rule, RFC4862-GLOB-12, GLOB-13, GLOB-15, is owed |
| [ND-F-ADDRESS-LIFETIME](../../protocol/nd/features.md#nd-f-address-lifetime) | mandatory | yes | not supported | `defect` — RFC4862-GLOB-22, GLOB-23: an expired address stays in use, [gap 11](results.md#gap-11-defect--an-expired-address-stays-in-use) |
| [ND-F-NO-FRAGMENTATION](../../protocol/nd/features.md#nd-f-no-fragmentation) | mandatory | yes | partial | `partial` — the sender half holds; the receiver half, RFC6980-FRAG-2 to FRAG-6, is level 3 |

Four features are `confirmed`, twelve `partial`, one `declined`, one `defect` and five
`unverified`.

### How to read the matrix

- **The one `defect`**, ND-F-ADDRESS-LIFETIME, is a mandatory feature whose two checked core
  statements both fail: the model forms an address with the lifetimes of the prefix, and
  never retires it. Its other core statements, the deprecation of RFC4862-GLOB-16 and GLOB-18,
  are owed.
- **The one `declined`**, ND-F-PARAMETER-DISCOVERY, is the verdict that the table gives an
  optional feature that is not supported. The word is wrong for what the run found: the model
  claims the feature in its code, and the three gaps show a value that is stored and never
  used. Read it as three statement-level defects of [`results.md`](results.md#the-model-gaps).
- **The `partial` verdicts** fall into two groups. Five hold a failing core check: the
  multicast groups, the advertisement and its timing, router discovery and Duplicate Address
  Detection, which gaps 2, 5, 6, 7 and 9 explain. The other seven fail no core check, and are
  partial only because some core statement needs a later level or an owed check.
- **The `unverified` verdicts** are the level 3 work and the owed checks, not verdicts on the
  model.

## Headlines for the next pass

1. **ND-F-ADDRESS-LIFETIME is a `defect`**, and ND-F-MESSAGE-VALIDATION is `unverified` and
   mandatory. The first needs a repair of gap 11; the second needs level 3.
2. **The eleven gaps** of [`results.md`](results.md#the-model-gaps) are repairs, not tests: none
   is declared, so the suite stays red until they are repaired. After a repair of gaps 5 to 7,
   RFC4861-RA-17 and the intervals of ADV-22 and ADV-36 reach their observations for the first
   time.
3. **The obsolete claims of level 1 stand**: the class documentation of the module and of the
   cache names RFC 2461, and the inline comments name it more often than RFC 4861.
