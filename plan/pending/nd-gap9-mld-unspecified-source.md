# ND gap 9, second half — an MLD Report before the address is tested

**Status:** started on 2026-11-24 on the local branch `topic/standards-tests-nd-gap9-fixes`, on master `bfb149475b`. Worktree: `/home/levy/workspace/inet-standards-tests-nd-gap9-fixes`. No pull request, and no push until the user asks for it.

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

1. [ ] **Plan** — this file.
2. [ ] **The check** — the MLD check of a node that reports before it has a link-local address (RFC9777-REP-34, REP-37, REP-39), from the standard text alone.
3. [ ] **IPv6** — a local MLD Report or Done keeps the unspecified source. Moves nothing alone.
4. [ ] **The receivers** — MLDv1 and MLDv2 discard a Report or a Done from the unspecified address. Moves nothing alone.
5. [ ] **MLDv1** — a Report or a Done from the unspecified address before the link-local address is valid, and new Reports after it. Test: `Rfc4862GroupsBeforeDad`.
6. [ ] **MLDv2** — the same for MLDv2 Reports. The new MLD tests of step 2.
7. [ ] **The documents** — fresh runs; the ND and MLD `results.md`, `coverage.md`, `conformance.md` follow them.
8. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way
