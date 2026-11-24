# ND gap 9, second half — an MLD Report before the address is tested

**Status:** repaired on 2026-11-24, the documents follow. Started on 2026-11-24 on the local branch `topic/standards-tests-nd-gap9-fixes`, on master `bfb149475b`. Worktree: `/home/levy/workspace/inet-standards-tests-nd-gap9-fixes`. No pull request, and no push until the user asks for it.

`Rfc4862GroupsBeforeDad` is the last failing test of the ND repairs (#1392). The user decided on 2026-10-28 to leave this half of gap 9 for a later change, and on 2026-11-24 asked for it in a local branch. This plan repairs it in MLDv1 and MLDv2, and gives the MLD statements of the same case their check.

## The defect

RFC 4862 section 5.4.2: before an interface sends the Neighbor Solicitation of Duplicate Address Detection, it joins the solicited-node group of the tentative address, and the MLD Report for that group informs MLD-snooping switches. RFC 3590 section 4 (MLDv1) and RFC 9777 section 5.2.14 (MLDv2) send a Report, and in MLDv1 a Done, from the unspecified address while the interface has no valid link-local address.

The model holds such a message until the link-local address is tested, so the Report leaves after the solicitation:

- `Mldv1::sendToIPv6` and `Mldv2::sendToIPv6` put a Report or a Done into `heldMessages` while `hasValidLinkLocalAddress` is false, and `sendHeldMessages` sends them from the link-local address when `interfaceIpv6ConfigChangedSignal` comes.
- The comment there gives the reason: `Ipv6::fragmentPostRouting` gives a local datagram from the unspecified address the preferred address of its interface. The only exception is a datagram to a solicited-node multicast address, which is the Neighbor Solicitation of DAD. With a tentative preferred address, the datagram waits in `pendingDadQueue`.

The default MLD module of a node is `Mldv1` (`Ipv6NetworkLayer.ned`), so `Rfc4862GroupsBeforeDad` runs MLDv1. The MLD checks run MLDv2.

## The statements

- RFC 3590 section 4: a Report or a Done leaves from the unspecified address if no valid link-local address is available; once one is available, a node SHOULD send new Reports for all addresses that it joined on the interface; a router MUST silently discard a Report or a Done from the unspecified address. Section 6: hosts and routers silently discard every MLD message from the unspecified address. The MLD standards map lists RFC 3590 as a companion at level 5, with no catalog.
- RFC 9777 section 5.2.14, catalog entries RFC9777-REP-34 (the second half: the unspecified address before a valid link-local address), REP-37 (a router discards a Report from the unspecified address), REP-38 (the node keeps the new listening state), REP-39 (SHOULD: new Reports once a valid link-local address is available). REP-37 to REP-39 are `owed` in `coverage.md`, for "a later level 2 pass: a node that reports before it has a link-local address".
- ND: RFC4862-DAD-11, DAD-16, RFC4861-AR-4, AR-6, the statements of `Rfc4862GroupsBeforeDad`.

## The design

1. **IPv6.** `fragmentPostRouting` keeps the unspecified source of a local MLD Report or Done (ICMPv6 types 131, 132 and 143), next to the exception for the solicited-node destination. MLD gives the source of every Report and Done: its link-local address when it is valid, else nothing. So the IPv6 layer sees an MLD message from the unspecified address only when MLD means it. A rule by the type of the `L3AddressReq` source (explicit IPv6 `::` against no address) is not used: a UDP socket bound to `::` gives an explicit `::` too.
2. **The receivers.** An MLDv1 or MLDv2 node silently discards a Report or a Done from the unspecified address (RFC 3590 sections 4 and 6, RFC9777-REP-37), both as a router and as a host (no Report suppression by such a Report). A Query from the unspecified address is not in this change.
3. **The senders.** A Report or a Done leaves at once, from the link-local address when it is valid, else from the unspecified address; the held messages go. An interface that sent from the unspecified address sends, when its link-local address becomes valid, a new Report for each group that it listens to: MLDv1 one Report for each group, MLDv2 one Current-State Report of all groups, the Report of a General Query (RFC 3590 section 4, RFC9777-REP-39).

The Router Solicitation of ND has the same limit: RFC 4861 section 6.3.7 allows the unspecified address, but `fragmentPostRouting` gives it the preferred address. No test needs it, so it stays as it is, as a finding.

## How each step is measured

As for the ND repairs on 2026-11-24: the copy of the CI job in the image of `ubuntu-26.04` (`~/workspace/ghci-standards-fixes`, release) and its debug copy (`~/workspace/ghci-standards-fixes-dbg`). For each commit that can change a result: a clean build, the complete fingerprint test and the statistical tests that move at the tip, against the values of the commit before (`tools/series.py`); the protocol suites ND, MLD and IGMP and the module tests in debug. The base is master, where all fingerprint, statistical and chart tests pass.

## Steps

The series follows PR-SPLIT-PREPARE: the commits before a change of results move nothing, and each change of results is a small commit (10 source lines each).

1. [x] **Plan** — this file.
2. [x] **The check** — `doc: add: MLD check of a start of listening before the link-local address is tested` (RFC9777-REP-34, REP-37, REP-39), from the standard text alone.
3. [x] **IPv6, the source** — `ipv6: change: a local MLD Report or Done keeps the unspecified source address`. Moves nothing.
4. [x] **IPv6, the forwarding** — `ipv6: change: a router does not forward a multicast datagram from the unspecified address` (RFC 4291 section 2.5.2). Moves nothing.
5. [x] **The MLD module tests** — `tests: fix: the MLD module tests inject messages from a link-local address`. Moves nothing.
6. [x] **The receivers** — `icmpv6: change: an MLD node discards a Report or Done from the unspecified address`. Moves nothing.
7. [x] **The new Reports** — `icmpv6: change: an MLD node reports its groups again when its link-local address becomes valid`. Moves nothing: the set of interfaces that sent from the unspecified address stays empty.
8. [x] **MLDv1** — `icmpv6: fix: an MLDv1 Report leaves before the link-local address is tested`, with `Rfc4862GroupsBeforeDad` and the release note. Moves 3 fingerprint configurations and 3 statistical results: `ipv6/mld` MldDemo, `pim` dm_ipv6 and sm_ipv6.
9. [x] **Two MLD checks** — `tests: fix: two MLD checks allow a Report before the link-local address is valid`. Moves nothing.
10. [x] **Two MLDv2 module tests** — `tests: fix: two MLDv2 module tests join after the link-local address`. Moves nothing.
11. [x] **MLDv2** — `icmpv6: fix: an MLDv2 Report leaves before the link-local address is tested`, with `Rfc9777ReportBeforeLinkLocal`. Moves 2 fingerprint configurations and 2 statistical results: `ipv6/mld` MldV2Ssm and `pim` ssm_ipv6.
12. [x] **The clean-up** — `icmpv6: change: MLD holds no message until the link-local address is valid`. Moves nothing.
13. [ ] **The documents** — fresh runs; the ND and MLD `results.md`, `coverage.md`, `conformance.md` follow them.
14. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **The measurement.** `tools/series.py` replayed the 12 commits on master `bfb149475b` (`logs/series/gap9`), with a clean release build for each commit that can change a result, the complete fingerprint test and the 7 statistical tests of the MLD examples. The commits of steps 3 to 7, 9, 10 and 12 move nothing; step 8 and step 11 move what the list says, and no chart moves. In all 5 statistical results the applications send and receive the same packets; the nodes send more Ethernet frames (MldDemo 38 to 45, dm_ipv6 493 to 522, sm_ipv6 464 to 491, MldV2Ssm 48 to 52, ssm_ipv6 232 to 240). A debug run of each commit (the ND and MLD suites and the 16 MLD and PIM module tests) shows `Rfc4862GroupsBeforeDad` FAIL up to step 7 and PASS from step 8 on, and all other tests PASS at each commit. `Rfc9777ReportBeforeLinkLocal`, run at the parent of step 11, fails at its first guard (RFC9777-REP-34).
- **The forwarding (step 4).** With step 8 alone, `IPv6_pimdm_flood_and_prune` stopped at an assertion of `PimDm::unroutableMulticastPacketArrived`: an MLDv1 Report goes to the address of its group, and a router tried to forward one from the unspecified address. RFC 4291 section 2.5.2 forbids that forwarding, so the IPv6 layer stops it before the route lookup.
- **The module tests (steps 5 and 10).** `TestMld` injected every message without a source, so from the unspecified address; it now gives `fe80::99`. `MLDv2_router_basic` and `MLDv2_ssm` joined at the start, before Neighbor Discovery assigns the link-local address of a host (`hostBootupTime`, 0.4 to 1 s); the receivers now join at 1.5 s, so that the router still sees the State-Change Report that the tests check.
- **Two MLD checks (step 9).** "Router Alert on every message" asked a link-local source for every message, and "Listener in MLDv1 mode" forbade every Version 2 Report from the start. RFC 9777 section 5 allows a Report from the unspecified address before the link-local address is valid, and COMPL-9 holds from the first MLDv1 Query that the host receives (section 8.2.1). The held messages had hidden both.
- **The parameter of `cancelHostTimers`** came with the held messages in #1388; the clean-up removes it.
- **Not changed**: a Query from the unspecified address (RFC 3590 section 4, RFC 9777 section 5.1.14) is not discarded on receipt; the Router Solicitation of ND still gets the preferred address in `fragmentPostRouting`, though RFC 4861 section 6.3.7 allows the unspecified address. No test needs either.
