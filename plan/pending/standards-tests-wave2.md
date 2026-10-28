# Standards tests, wave 2 — level 2 for the larger native-RFC protocols

**Status:** pending. Written 2026-10-28 from `master` at `a40c4ffcdc` (equal to `origin/master`). The user settled D1 to D4 on 2026-10-28. No pass has started.

Source: the survey `audit/sweep/standards-tests.md` in `inet-master` (local, not in git), §6 "Wave 2", the protocol cards of §5.2, and the family reports 01 to 05 in `audit/sweep/standards-tests/`. Waves 0 and 1 are done: [`standards-tests-wave0.md`](../done/standards-tests-wave0.md) and the five wave 1 plans in `plan/done/` (RIP, ND, IGMP with MLD, IPsec, MPLS, all landed and pushed on 2026-09-29).

Each pass follows [`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md), steps 2 to 9, at **level 2, Core**. Wave 0 wrote step 2 (the standards map) and part 1 of step 8 (the model claims) for every protocol of this wave. Each pass starts from these files and updates them.

This file is the plan of the wave. Each pass writes its own plan, `plan/pending/<proto>-level2-standards-tests.md`, when it starts, as the guide asks.

## The passes

| # | Pass | `<proto>` | In-scope set of the level 1 map | Size | Depends on |
| --- | --- | --- | --- | --- | --- |
| 6 | Mobile IPv6 and Proxy Mobile IPv6 | `mipv6`, `pmipv6` | RFC 6275 (no clause slice yet); RFC 5213 §5 to §8, RFC 4283 §3 | 169 + 92 pages | Neighbor Discovery (PR #1392, open); IPv6 tunnel (RFC 6275 tunnels by RFC 2473) |
| 7 | SCTP | `sctp` | RFC 9260 §3 to §6, §9 | about 150 pages | none |
| 8 | OSPFv2, then OSPFv3 | `ospfv2`, `ospfv3` | RFC 2328 slice; RFC 5340 slice, with the RFC 2328 catalog shared | 244 + 94 pages | none |
| 9 | PIM-SM and PIM-DM | `pim` | RFC 7761, RFC 3973, RFC 3956, RFC 4607, each with a slice | 137 + 61 pages, and two short ones | IGMP and MLD (PR #1388, open) |
| 10 | BGP-4 | `bgp` | RFC 4271 slice, RFC 4760, RFC 5492, RFC 6793 §3 and §4, RFC 8212 §3 | 104 pages, and four short ones | none |
| 11 | AODV | `aodv` | RFC 3561 §5, §6.1, §6.3 to §6.7, §6.11; RFC 5148 §5 | about 37 pages | none |
| 12 | DiffServ | `diffserv` | RFC 2474, 2475, 2597, 3246, 2697, 2698, 3260, each with a slice | seven short documents | none |
| 13 | RTP and RTCP | `rtp` | RFC 3550 §5, §6.1, §6.4 to §6.6; RFC 3551 §6 | about 100 pages | none |
| new | IPv6-in-IPv6 tunnel | `ipv6tunnel` | RFC 2473 | 36 pages | none |

The survey put no wave on the IPv6 tunnel, but wave 0 wrote its level 1 map, and its card calls level 2 "cheap". Decision D3 below decides if it joins this wave.

Out of this plan:

- The Mode A items that the survey puts beside wave 2: QUIC level 3 (the rest), RFC 9002 and RFC 8899; ARP level 4. Each one gets its own plan.
- The repairs of the gaps that the passes find. As in wave 1, a later fix branch on top of each pass branch repairs them.
- Level 3 and higher. No pass of this wave needs a tooling gap of survey §4.2 closed: T2 (wireless fault helper) is level 3, and the protocol-test library has a tap radio medium since `48054f3c7a`; T3 and T4 are level 4.

## Decisions for the user

- **D1 — order, and the two open pull requests.** MIPv6 and PMIPv6 use Neighbor Discovery: router advertisements, movement detection, Duplicate Address Detection of the care-of address. PIM uses IGMP and MLD for the local members, and RFC 4607 (source-specific multicast) needs the source lists of IGMPv3 and MLDv2. The repairs of these protocols are in PR #1392 and PR #1388, and both are open. A pass on `master` would record ND and IGMP defects as MIPv6 and PIM failures. Recommendation: do the seven independent passes first, and do PIM and MIPv6 with PMIPv6 after the two pull requests are merged. If they are still open then, base the two passes on `master`, and give each failure that an ND or IGMP gap causes its cause in `results.md`.
- **D2 — one pass at a time, or in parallel.** Wave 1 did one pass after the other, and each pass took the lessons of the one before it. The documents of wave 2 are two to five times larger. Recommendation: one pass at a time, in the order of the next section. The catalog of each pass is drafted by agents in parallel, as in wave 1.
- **D3 — the IPv6 tunnel.** Recommendation: add it to this wave, before MIPv6. Its level 1 map exists, its pass is small, and the MIPv6 pass then has an RFC 2473 catalog for its tunnel rules.
- **D4 — the BGP judgment of survey §7.6.** `src/inet/routing/bgpv4/__TODO` says that NOTIFICATION, the optional path attributes and the error handling of §6 are "not implemented". But some parts exist: the enum value `BGP_NOTIFICATION`, an empty `BgpSession::sendNotificationMessage()`, and a message class for each of MULTI_EXIT_DISC, LOCAL_PREF, ATOMIC_AGGREGATE and AGGREGATOR. Two readings:
  - (a) per behavior, as the guide says (the giveaway rule of step 7): where a class, a field or a function for the behavior exists, the model claims it, and a failure is a plain defect; only a behavior with no code at all is an unimplemented feature;
  - (b) the `__TODO` file is a stated limitation for each item that it names, so each failure of these items is declared expected.

  Recommendation: (a). The guide already says "if the parameter, the field or the function argument the behavior needs is already there and carries a placeholder value, the mechanism is claimed whatever the comment says".

## Order

The order of D1 to D3:

1. [ ] IPv6 tunnel — `ipv6tunnel`
2. [ ] SCTP — `sctp`
3. [ ] OSPFv2, then OSPFv3 — `ospfv2`, `ospfv3`, one pass, the IGMP and MLD pattern
4. [ ] BGP-4 — `bgp`
5. [ ] AODV — `aodv`
6. [ ] DiffServ — `diffserv`
7. [ ] RTP and RTCP — `rtp`
8. [ ] PIM-SM and PIM-DM — `pim`, after PR #1388
9. [ ] Mobile IPv6 and PMIPv6 — `mipv6`, `pmipv6`, one pass, after PR #1392 and the IPv6 tunnel

## What changed since the level 1 records

Wave 0 wrote the level 1 records at `28536bd0a5`. `master` is 956 commits newer. I examined each record against `master` on 2026-10-28.

1. **The models of BGP, MIPv6, PMIPv6, SCTP, AODV and the IPv6 tunnel did not change.** Their level 1 claims are still true, except the BGP map of item 4.
2. **Four commits changed the other models.** `5ac997f379` (interface signal listeners) touches OSPFv2, OSPFv3 and PIM. `8869bb5384` and `8e112293d5` are refactors in DiffServ. `8f1c13d673` removes a part of RTP (item 3). At step 8, each pass examines the `file:line` citations of part 1 of its `conformance.md` again.
3. **The RTP claim of level 1 is stale.** `8f1c13d673` ("rtp: remove: the never-compiled payload type 10 and audio modules") removed the payload type 10 modules and the README lines that claimed them. `model/rtp/conformance.md` lines 22, 32 and 39 cite these lines. The model now has payload type 32 only: MPEG video by RFC 2250, which the map puts at level 5, and whose wire format does not follow RFC 2250. The RTP pass writes part 1 again first. The in-scope set comes from the register and not from the claim, so the payload type 10 row of RFC 3551 can stay, as a feature that the model does not claim. At step 2 the pass decides how its checks get a payload. The rules of the header, SR, RR and SDES do not depend on the payload format.
4. **The BGP map excludes a mandatory attribute.** `protocol/bgp/standards.md` line 74 excludes "the optional path attributes of §5.1.3 to §5.1.6". In RFC 4271, §5.1.3 is NEXT_HOP, "a well-known mandatory attribute" (`rfc4271.txt:1446`). §5.1.5 LOCAL_PREF "SHALL be included in all UPDATE messages that a given BGP speaker sends to other internal peers" (`rfc4271.txt:1590-1592`), and §5.1.6 ATOMIC_AGGREGATE is well-known discretionary (`rfc4271.txt:1608`). The two optional attributes are MULTI_EXIT_DISC, §5.1.4 (`rfc4271.txt:1553`), and AGGREGATOR, §5.1.7 (`rfc4271.txt:1647`). The BGP pass corrects the in-scope set at step 2, and records the correction in its decision log.
5. **The MIPv6 map has no clause slice.** It names RFC 6275 as a whole, 169 pages. At step 2 the pass selects the clauses of level 2: the home registration, return routability, and the home agent and correspondent node rules. The pass puts the rest in the out-of-scope table, with a level for each part.
6. **The texts are local.** `../standards/RFC/` holds every document of each in-scope set. RFC 3775 and RFC 4960 are not local. They are the obsolete documents that MIPv6 and SCTP claim, and no catalog needs them.

## What each pass takes over from wave 1

- **The pass measures the model and repairs nothing** (the user's decision of 2026-09-24). A failure is kept. Only an unimplemented feature, or a defect with a named limitation that blocks its repair, is declared expected.
- **Every output of "What a pass delivers"**, `notes.md` included. Each gap of `results.md` appears by its number in a follow-up of `notes.md`.
- **The tools of wave 1.** Agents draft the catalogs with one brief. A script checks each quote against its line reference. The drafts are merged, reviewed and indexed. A script places each entry in the feature map, and a script makes sure that each entry is in one check or in the closing list. A generator writes the tests from shared mockups, and another one writes the ledger tables. An exploration test shows the cause of a failure before it counts as a finding, and nobody commits it. The wave 1 scripts are in `audit/<proto>-level2/` of `inet-master`, each folder with a `README.md`.
- **The lessons** in `evidence/model/ipv4/notes.md#tooling-quirks` and in the `notes.md` of RIP, ND, IGMP, MLD, IPsec and MPLS. Read them before step 5.
- **The target level** in `standards.md` changes to 2 at step 2. Four wave 1 passes forgot this step.
- **One branch and one worktree per pass**: `topic/standards-tests-<proto>-level2` in `/home/levy/workspace/inet-standards-tests-<proto>-level2`, from `origin/master` at the start of the pass. The twin passes use both names: `ospfv2-ospfv3`, `mipv6-pmipv6`. The branch stays local, with no pull request, until the user asks for a push.
- **A fast build of a new worktree**: hard-link `out/` from a built worktree at the same commit, as `evidence/model/dhcp/notes.md` describes. Obey the two traps of that recipe: give the times in epoch seconds, and touch again the files that changed after the build.
- **The run command**: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/<proto>$'` (opp_repl). INET has no Python test framework of its own since `9e18ebb715`.
- **The gates before the last commit**: `check-links.sh`, `check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`. Each commit has a `Change:` trailer with the group `<proto>-standards-tests`.

## Notes for each pass

These notes come from the survey cards. Each pass plan copies its note and adds to it.

- **IPv6 tunnel.** Level 2: the outer addresses, Next Header 41, the outer Hop Limit, decapsulation. Mobile IPv6 has its own copy of the tunnel logic (`Mipv6.cc:2104`). The pass does not test that copy.
- **SCTP.** The serializer writes the INIT parameters in a fixed order (`tests/serializer/REMAINING_GAPS.md:88-100`), so a check compares the parsed fields and not the raw bytes. Level 2: the four-way handshake, ordered delivery on each stream, and SACK. The 11 module tests show scenarios. The extensions are level 5.
- **OSPFv2 and OSPFv3.** Divide the RFC 2328 catalog by clause (§9, §10, §12, §13). One `ospf` dissector serves both versions, so filter on the chunk class. OSPF has no signals. Level 2: Hello, the DD exchange with the I, M and MS bits, the Full state, and a Router-LSA with the correct links. OSPFv3 in one area only. Level 1 found two defects (authentication and the LSA checksum check return `true`). Both are outside level 2: authentication is level 5, and a bad checksum needs a corrupted packet (level 3).
- **BGP-4.** No module test and no signals, and 13 example scenarios. Observe the messages in the TCP stream. Level 2: OPEN, UPDATE, KEEPALIVE, the FSM of §8 and the decision process of §9. Apply D4. Correct the in-scope set first (item 4 above).
- **AODV.** RFC 3561 is Experimental and RFC 5148 is Informational. The level 1 map says how this status bounds the keywords. Level 2 needs a wireless mockup but no fault: RREQ and RREP, RERR on a link break, the sequence numbers, the "destination only" flag. Local repair is declared "not implemented yet" (`Aodv.ned:39`).
- **DiffServ.** DiffServ has no PDU of its own. Most statements describe an algorithm, not an RFC 2119 requirement, so the "only path" rule of step 4 decides the feature levels. Level 2: the DSCP after a marker, the outcome of a classifier, the color of a meter. The six module tests show the parts.
- **RTP and RTCP.** Write part 1 again first (item 3 above). The maintainer note says "crashes frequently" (`transportlayer/rtp/__TODO`), and no test of any kind exists. A crash is a finding. Level 2: the header, SR, RR and SDES.
- **PIM-SM and PIM-DM.** Two modes and two base documents: keep them apart in the standards map. PIM has the best signals of the routing protocols: a sent and a received signal for each message type, and the neighbor-table signals. Seven module tests. `PimSm.ned:42-43` declares two omissions. The RP is static or embedded (RFC 3956). The model has no Bootstrap.
- **Mobile IPv6 and PMIPv6.** One RFC 6275 catalog serves both protocols. The cookbook of `tests/protocol/lib/AUTHORING.md` gives the filter expressions for the home registration and return routability. Level 1 found seven named gaps on an unmerged branch (`git show ba7c3656c1:MIPV6_IMPLEMENTATION_GAPS.md`). The model claims RFC 3775 in its documentation and RFC 6275 in its serializer. PMIPv6 covers one scenario: one LMA and two MAGs.

## Steps of the wave

1. [x] The user settles D1 to D4. Record each answer in the decision log below.
2. [x] Commit this plan on `master` (group `standards-tests-wave2`).
3. [ ] Make one built debug worktree at the base commit, as the source of the hard-linked builds.
4. [ ] Do the passes of the section "Order". Each pass writes its plan, does steps 2 to 9 and the notes, runs the gates, and moves its plan to `plan/done/`. Mark the pass done in "Order".
5. [ ] Land the pass branches on `master` with the wave 1 recipe: rebase, map the cited hashes to the new commits, and add a new run-record commit if `src/` changed. Push only when the user asks.
6. [ ] Write the state of wave 2 into survey §6 (local file), and move this plan to `plan/done/`.

## Decision log

- 2026-10-28: the plan is written.
- 2026-10-28, D1: the seven independent passes come first. PIM and MIPv6 with PMIPv6 come last, after PR #1388 and PR #1392 are merged. If a pull request is still open then, the pass is based on `master`, and `results.md` gives each failure that an ND or IGMP gap causes its cause.
- 2026-10-28, D2: one pass at a time. Agents draft the catalog of each pass in parallel.
- 2026-10-28, D3: the IPv6 tunnel joins the wave as the first pass.
- 2026-10-28, D4: per behavior. Where a class, a field or a function for a BGP behavior exists, the model claims the behavior, and a failure is a plain defect. Only a behavior with no code is an unimplemented feature.
