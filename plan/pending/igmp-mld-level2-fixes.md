# IGMP and MLD level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-igmp-mld-level2-fixes`, on
top of `topic/standards-tests-igmp-mld-level2` at `8dba80c00a`. Worktree:
`/home/levy/workspace/inet-standards-tests-igmp-mld-level2-fixes`.

The level 2 twin pass of 2026-09-24 measured the models and repaired nothing. This branch repairs
the twelve defects of IGMP and the sixteen defects of MLD of
[`igmp/results.md`](../../doc/project/evidence/model/igmp/results.md#the-model-gaps) and
[`mld/results.md`](../../doc/project/evidence/model/mld/results.md#the-model-gaps), one gap of one
protocol for each commit, and adds the two missing features that the user put in scope on
2026-09-25: the IGMPv2 querier mode of an IGMPv3 router (IGMP gap 13) and the MLDv1 mode of an
MLDv2 router (MLD gap 17).

Commit group: `igmp-mld-model-repairs`. Each repair commit carries the source change, the tests
whose gap paragraph changes, the WHATSNEW entry, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

As on the ND repair branch: the IGMP and MLD suites in debug; the whole protocol suite (base: 371
tests, 285 PASS, 44 FAIL declared expected, the 42 IGMP and MLD failures) and all module tests
after each gap; the CI fingerprint set in release (base: 1752 tests equal, one expected error);
the whole statistical set in the GitHub-job copy for each commit (base: 929 tests, 890 PASS, 10
SKIP, 27 FAIL of 802.11 EDCA, 2 expected ERROR), with the moved results regenerated at the commit
that moves them.

## Steps

The twin gaps come next to each other, IGMP first.

1. [x] **Plan** — this file.
2. [x] **IGMP gap 1, MLD gap 1** — the Queries carry the QRV and the QQIC.
3. [x] **IGMP gap 2** — an IGMP message has the precedence of Internetwork Control.
4. [x] **MLD gap 2** — an MLD message leaves from a link-local address. A test repair comes first:
   the first Query waits for the link-local address.
5. [x] **MLD gap 3** — an MLD message carries the Router Alert option. A repair of the Payload
   Length of `Ipv6::encapsulate` comes first.
6. [x] **MLD gap 4** — the ICMPv6 checksum covers the pseudo-header.
7. [x] **MLD gap 5** — a node reports its solicited-node addresses: the change of ND gap 9
   (`affa7e64fa` on `topic/standards-tests-nd-level2-fixes`), applied again here. Two commits
   come first: `Mldv1` ignores an MLDv2 Report, and `Rfc2710DoneInV1Mode` waits for the Report
   for G.
8. [x] **IGMP gap 3, MLD gap 6** — the default intervals of RFC 9776 and RFC 9777.
9. [x] **IGMP gap 4, MLD gap 7** — the Older Version Querier Present Interval.
10. [x] **IGMP gap 5, MLD gap 8** — a second change merges the pending records.
11. [x] **IGMP gap 6, MLD gap 9** — the older-version mode of a host has a report delay timer,
    repeats its unsolicited Report and suppresses it. A test repair comes before MLD gap 9:
    `Rfc2710ReportSuppression` counts the Reports for G.
12. [x] **IGMP gap 7, MLD gap 10** — a change of the compatibility mode cancels the pending timers.
13. [x] **IGMP gap 8, MLD gap 11** — an answer to a Query leaves out empty records.
14. [x] **IGMP gap 9, MLD gap 12** — the S flag comes from the lowered timer.
15. [x] **IGMP gap 10, MLD gap 13** — a Report does not cancel the retransmissions of the Queries.
16. [x] **IGMP gap 11, MLD gap 14** — a source-specific Query lowers the Source Timers. Gaps 10
    and 11 are one commit for each protocol, after two commits for the source-map loops and the
    BLOCK row of EXCLUDE mode.
17. [x] **IGMP gap 12, MLD gap 15** — the forwarding asks for a listener of the source.
18. [x] **MLD gap 16** — an MLDv1 node reads an MLDv2 Query. A test repair comes first:
    `Rfc9777RouterV1Listener` waits for the Report of C for G.
19. [x] **IGMP gap 13** — the IGMPv2 querier mode of an IGMPv3 router (missing feature), with the
    IGMPv1 mode and a new check and test for it.
20. [x] **MLD gap 17** — the MLDv1 mode of an MLDv2 router (missing feature).
21. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` of both protocols follow it; the statistics branch.
22. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **IGMP gap 1, MLD gap 1.** The QRV is 0 when the Robustness Variable is above 7 (RFC 9776
  section 4.1.6). The assertion of `Igmpv3::codeTime` allowed codes outside `0x10..0x1f`; it is
  exact now. MLD has its own 8-bit QQIC coder, `Mldv2::codeQqic`, because the Maximum Response
  Code of MLDv2 has 16 bits and another format.
- **MLD gap 2.** A node has no tested link-local address for about 2 seconds after the start.
  RFC 3810 section 5.2.13 sends a Report from the unspecified address during that time, but
  `Ipv6::fragmentPostRouting` replaces an unspecified source with the preferred address. So a
  Report or a Done waits in the module (`heldMessages`) and leaves when the address is valid, and
  a Query is not sent; the querier starts then. `Ipv6InterfaceData` emitted no signal when an
  address became tentative or valid, so `permanentlyAssign` and `tentativelyAssign` call
  `changed1(F_IP_ADDRESS)` now. Two tests expected the first Query within 1 s of the start; a
  separate `tests:` commit gives them 3 s and words the check from the moment that R is up.
  Statistics: 5 results move (ipv6/mld and the 3 IPv6 PIM examples); the statistics branch of
  the same name holds them.
- **The Payload Length.** `Ipv6::encapsulate` set the Payload Length before it inserted the
  extension headers of `Ipv6ExtHeaderReq`, so the receiver cut off the end of the message. No
  module used the tag before MLD gap 3, so the repair is a separate commit that moves nothing.
- **MLD gap 3.** The option is a class, `Ipv6RouterAlertOption`, with a PadN option of no data
  after it: 8 octets, as a real node sends it. A multicast router delivered every ICMPv6
  multicast datagram locally, to catch the MLD messages; with the Hop-by-Hop header the Next
  Header is 0, and `Rfc2710DoneAtV1Router` failed. The router now delivers a multicast datagram
  that carries the Router Alert option, which is what RFC 2711 asks. The rule for ICMPv6 went
  away: its only other effect was that a host with `multicastForwarding` on passed the Router
  Solicitations of other hosts to Neighbor Discovery (inet/ipsec Multicast6, `tplx` only).
- **MLD gap 4.** The pseudo-header needs the final source address, which only
  `Ipv6::fragmentPostRouting` knows, after the source selection and the deferral for Duplicate
  Address Detection. So `Icmpv6::insertChecksum` leaves a computed checksum at 0, and the Ipv6
  module computes it there, like the post-routing hook of UDP, but without a hook: every ICMPv6
  sender is inside the IPv6 layer. The pseudo-header takes the final destination of a Routing
  header (RFC 8200 section 8.1) and the home address of a Home Address option (RFC 6275 section
  11.3.1); without the second, the ping replies of MIPv6 route optimization failed the check. An
  ICMPv6 error about a local datagram does not pass `Ipv6`, so `Icmpv6::sendErrorMessage`
  computes it with the loopback addresses of its `L3AddressInd`. The fingerprint tool runs the
  `~tND` rows with `checksumMode="computed"`, so every IPv6 example of that set moves in its
  packet data; each moved row was run again in debug with no checksum drop.
- **MLD gap 5.** A host now reports its solicited-node groups at about 2 s, before the first
  MLDv1 Query puts it in MLDv1 mode. `Mldv1::processMldMessage` read every message as an
  `MldMessage` and stopped the run on the MLDv2 Report; it ignores the type now (RFC 4443
  section 2.4 (b)), in a commit of its own. `Rfc2710DoneInV1Mode` took any MLDv1 Report of A as
  the Report for G, which its check names; a `tests:` commit filters on G. The ND branch
  changed the pinned seed of `MLD_host_basic` and `MLD_host_groupstates` to 0; on this branch
  host1 still wins the report race at seed 1, so only the list of seeds in their notes
  changes.
- **IGMP gap 3, MLD gap 6.** Under RFC 3376 and RFC 3810 the Group Membership Interval and the
  Older (Version) Host Present Interval were equal, and the router timer for an older host used
  `groupMembershipInterval`. RFC 9776 section 8.13 and RFC 9777 section 9.13 keep the second at
  [Robustness Variable] × [Query Interval] + [Query Response Interval], so each module has a
  new parameter for it (`olderHostPresentInterval`, `olderVersionHostPresentInterval`) with that
  default. The first version of the IGMP commit missed this and was amended. RFC9776-TIMER-25
  and its MLD twin stay owed: no test measures the value.
- **The MLDv1 router of the checks.** A comparison of verdicts misses a test that fails at an
  earlier step than before: after MLD gap 2, five tests with an MLDv1 router failed at a Query
  premise, before their documented observation. The failure reasons of all failing tests are now
  compared with `results.md` after each gap. The repair gives the MLDv1 router of the mockups no
  boot delay and no Duplicate Address Detection, so its Queries come at the times that the
  checks compute; the earlier repair of the MLDv2 querier checks (3 s for the first Query) stays.
  `Rfc9777ListenerV1Mode` now fails at observation 5, at the held and retransmitted MLDv2
  Reports of the solicited-node groups: the gap 10 repair must also drop the held Reports of the
  old mode.
- **Statistics are cumulative.** Each run compares with the files of the statistics branch, so
  the results of one commit must be regenerated before the next commit runs.
  `stat-seq.sh` runs, regenerates and makes a WIP commit per step, in order.
- **IGMP gap 5, MLD gap 8.** The retransmission state of RFC 9776 section 5.1: a count for the
  Filter-Mode-Change record and a count for each source; every State-Change Report that leaves
  decrements them all, and Table 4 builds each Report from the current state. The log lines that
  the module tests read stay.
- **IGMP gap 6, MLD gap 9.** The host state machine of RFC 2236 section 6 and RFC 2710 section 5
  with its own timer, `olderVersionReportTimer`, and flag, `lastReporter`; the return to version 3
  stops them. `IGMPv3_interop_host` sent its IGMPv1 Query before the answer to its IGMPv2 Query
  of 10 s could leave; its IGMPv2 Query now has 1 s.
- **IGMP gap 7, MLD gap 10.** `cancelHostTimers` at every change of the compatibility mode. MLD
  also deletes the held Reports of the other version (MLD gap 2); without that,
  `Rfc9777ListenerV1Mode` saw Version 2 Reports of the solicited-node groups in MLDv1 mode.
- **IGMP gap 8, MLD gap 11.** An entry in INCLUDE mode with no sources has no reception state.
- **IGMP gaps 10 and 11, MLD gaps 13 and 14.** One mechanism, RFC 9776 section 6.6.3: without
  the lowering, the new retransmission state puts every source into the Query with the S flag;
  with the lowering alone, the old retransmission queries no source. So one commit for each
  protocol. The lowered timers now end in EXCLUDE mode, and the module tests found three old
  defects of the source-map loops (erase and increment, and a loop that tested the next entry),
  and a dead loop of the BLOCK row in EXCLUDE mode; two commits before repair them.
  `IGMPv3_router2` and `IGMPv3_router3` now expect the sequence of RFC 9776.
- **IGMP gap 12, MLD gap 15.** `Ipv4InterfaceData::hasMulticastListener(group, source)` returned
  the negation of the answer; no caller used it until now.
- **IGMP gap 13.** RFC 9776 has both an automatic rule (section 6.6.2, an older General Query
  lowers the version) and a configuration option (section 7.3.1); `routerVersion` gives both, per
  interface. The IGMPv1 mode needed a check of its own; the level 2 pass had put its statements
  to level 5.
- **MLD gap 17.** RFC 9777 section 8.3.1 has only the configuration option, and a warning.
- **Open finding.** `Icmpv6` emits `packetDropped` when a checksum is wrong, but `Icmpv6.ned`
  does not declare the signal, so a debug run stops there. The drop path is not more reachable
  than before; not repaired on this branch.

