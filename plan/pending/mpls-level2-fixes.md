# MPLS level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-mpls-level2-fixes`, on top of
`topic/standards-tests-mpls-level2` at `24675c3a37`. Worktree:
`/home/levy/workspace/inet-standards-tests-mpls-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the
gaps of [`results.md`](../../doc/project/evidence/model/mpls/results.md#the-model-gaps), one gap for
each commit, as the user asked on 2026-09-25: the defects and the small or medium missing
features. A large missing feature gets a plan of its own and no code.

Commit group: `mpls-model-repairs`. Each repair commit carries the source change, the test whose
declaration or gap paragraph changes, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

- **Build**: the worktree starts with the release and debug build of `inet-master` at
  `7772a7e4ef` (the same src tree, `5c4f41c600`), copied with hard links and fresh times.
- **Protocol tests**: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mpls$'`; the
  repaired tests pass, and no other test of the suite changes its verdict.
- **Fingerprints**: the CI set (`-f tplx -f '~tNl' -f '~tND'`), release, on a scratch copy of
  `tests/fingerprint`. Baseline on `inet-master` at `7772a7e4ef`, after its 12 stale 802.11
  objects were rebuilt: 1752 tests, equal to the expected results, one expected error.
- **Statistics**: in the GitHub-job copy (`ghci-statistical`), baseline at `7772a7e4ef`, then the
  tip of the branch; each changed result is attributed to the commit that moves it.
- **Module tests**: none of the module suite uses an MPLS router, and no protocol test outside
  `tests/protocol/mpls` does; the repairs change `Mpls` and `LibTable` only.

## Steps

1. [x] **Plan** — this file.
2. [x] **Gap 1** — the TTL of each label stack entry: the first label copies the IPv4 TTL, each
   LSR forwards with one less, and a pushed label copies the TTL below it (the Uniform Model).
   Tests: `Rfc3032FirstLabelTtl`, `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`.
3. [x] **Gap 2** — the LSP counts its hops in the IPv4 TTL: the ingress decrements the IPv4 TTL
   before it labels a datagram, and a pop that empties the stack writes the outgoing TTL into the
   IPv4 header. Tests: `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3443TtlAfterTwoPops`,
   `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate`.
4. [x] **Gap 3** — an LSR does not forward a labeled packet whose outgoing TTL is zero. Test:
   `Rfc3032TtlExpiry`.
5. [x] **Gap 4** — an MPLS router on an Ethernet link: packets of other protocols go up, and a
   labeled packet gets its Ethernet header. Test: `Rfc3032EthernetEncapsulation`.
6. [x] **Gap 5** — the reserved labels 0 and 3. Tests: `Rfc3032ExplicitNull`,
   `Rfc3032ImplicitNull`; their declarations go.
7. [x] **Gap 6** — the MTU check and the fragmentation of a labeled datagram, and the ICMP
   message for one with the DF bit. Tests: the three `Rfc3032TooBig*`; their declarations go.
8. [x] **Gap 7, a plan only** — PPP LCP and the MPLS Control Protocol:
   `plan/pending/ppp-lcp-and-mplscp.md`.
9. [x] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
   `notes.md` ("Fixed on" entries) follow it. No statistics branch: nothing moves.
10. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **The baseline needed a rebuild of `inet-master`.** Its objects of `Edca`, `Edcaf` and `Hcf`
  were compiled before `c0314ff5aa` (built 2026-09-23, commit pulled 2026-09-24), and the first
  baseline run gave 40 errors "Cannot cast nullptr to type NonQosRecoveryProcedure". The 12
  objects that depend on the changed files were deleted in both modes and rebuilt. A second
  cause of errors was the scratch copy: tests with the working folder `.` need the ini files of
  `tests/fingerprint`, so the scratch copy holds the whole folder.
- **Each repair moved no fingerprint and no statistical result of `examples/mpls`**, except one
  that was caught: with the Implicit NULL rule of gap 5, the LIB still allocated the labels 1,
  2 and 3 to RSVP-TE, the label 3 was popped, and two fingerprints and three statistical results
  of `examples/mpls` moved. RFC 3032 §2.1 reserves 0 to 15, so the allocation starts at 16, in
  the same commit; then nothing moves. The MPLS fingerprint rows hold `tplx` and `~tNl`, that is
  no packet contents, so a TTL value alone cannot move them.
- **Gap 2: the ingress decrements the IPv4 TTL itself.** The alternative, to send every datagram
  from a link up to IPv4 and label it on the way down, makes an LSP depend on the IPv4 routing
  table, and changes what an ingress without a route does. `Mpls` decrements, and leaves a
  datagram with TTL 1 to IPv4, which sends the ICMP Time Exceeded.
- **Gap 4: a LIB entry has no next hop.** `Mpls` tags a packet for a link as IPv4 does, so the
  Ethernet layer encapsulates it, but it has no next hop to resolve a MAC address for. The frame
  goes to the broadcast address: exact on a point-to-point link. Two costs remain: on a shared
  LAN every station gets the frame, and `Icmp` sends no error about a datagram that arrived in a
  link-layer broadcast. The repair is a next hop in the NHLFE (RFC 3031 §3.10), filled by RSVP-TE,
  LDP and the XML of the LIB, and resolved by ARP; a follow-up of `notes.md`.
- **Gap 6: `Mpls` got the parameter `icmpModule`**, default `^.ipv4.icmp`, optional, for the ICMP
  message about a too-big datagram with the DF bit. The fragmentation copies the algorithm of
  `Ipv4::fragmentAndSend`, with the room for the label stack.
- **Gap 7 has its plan**, `plan/pending/ppp-lcp-and-mplscp.md`: LCP must come first, and with it
  IPCP, or IPv4 stops on every PPP link.
- **The whole branch moves no statistical result.** The full suite at `db2fd89622` (930 tasks,
  release, GitHub-job copy) gives 890 PASS and the same 27 unexpected failures as the baseline,
  all of 802.11 EDCA. So the branch needs no commit in the statistics repository.
- **The documents follow a fresh run** at `db2fd89622`: 20 PASS, 1 FAIL declared expected. The
  ledger and the matrix come from `audit/mpls-level2-fixes/` of `inet-master` (outside git),
  the scripts of the level 2 pass with the verdicts of this run: 7 features supported, 3 partial,
  4 untested; the matrix has 7 `confirmed`, 3 `partial`, 3 `unverified`, 1 `out of claim`, and
  no `defect`.
- **Large run copies go to disk.** The fingerprint runs write a 2.8 GB copy of
  `tests/fingerprint`; they now live in `/var/tmp/claude-standards-fixes/` and each copy is
  deleted when its summary is read, because `/tmp` is a RAM file system.
