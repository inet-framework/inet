# Standards tests, wave 0 — debt, fresh run records, and level 1 for the Mode B protocols

**Status:** done. Started and finished 2026-09-23 on `topic/standards-tests-wave0`, from `master` at
`28536bd0a5`. Worktree: `/home/levy/workspace/inet-standards-tests-wave0`.

Source: the survey `audit/sweep/standards-tests.md` in `inet-master` (local, not in git), §6
"Wave 0". The user chose two extensions on 2026-09-23: the level-1 sweep covers the 17 Mode B
protocols, and the guide changes so that a run record survives a rebase.

## What changed since the survey

The survey named two stale run records (TCP, DHCP). A check of every run record shows that the
debt is systemic: **all 17 run records in `evidence/model/` name a commit that is not on
`master`**. The branches were rebased when they landed. The objects still exist, and the `src/`
tree of each old commit is identical to a range of commits on `master`:

| Old commit | Used by | Same `src/` tree as `master` commits |
| --- | --- | --- |
| `0868c36c88` | ipv4, ipv6, quic, udp | `512d6c1b15` to `cae555ebc8` |
| `223ba89ce5` | arp | the same range |
| `4e20c74e82`, `4acb050ab1` | dhcp | the same range |
| `e0ac3b7307` | tcp | `83396aa655` to `ab1447501a` |

The survey also misread one fact. The "obsolete-citation sweep" of TCP pass 4 (`0dd31d7948`)
counted the old citations; it never changed a source file ("No source file changed"). The
pass-log sentence "moved the model's own references off RFC 793" is the error, not the code.

## The fresh run

Run on 2026-09-23 18:26 +0200 in the worktree, before any commit of this plan:

- INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- OMNeT++: 6.4.0, commit `cf58891643` (`omnetpp-6.x`; only IDE project files under `ui/` differ)
- Build: debug. The worktree linked the objects of `inet-master`, deleted the 412 objects with
  whole-second times (copied objects that make cannot judge), and rebuilt them: 408 compiled
  files, no undefined `inet::` symbol.
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/<p>$'`, once per suite

| Suite | Tests | PASS | FAIL (expected) | FAIL (unexpected) | The ledger said |
| --- | ---: | ---: | ---: | ---: | --- |
| arp | 16 | 16 | 0 | 0 | 13 PASS, 3 unexpected (row notes: repaired) |
| udp | 13 | 11 | 2 | 0 | 10 PASS, 1 expected, 2 unexpected |
| ipv4 | 22 | 20 | 2 | 0 | 16 PASS, 2 expected, 4 unexpected |
| dhcp | 26 | 21 | 5 | 0 | 13 PASS, 5 expected, 8 unexpected |
| tcp | 27 | 25 | 2 | 0 | 22 PASS, 2 expected, 3 unexpected |
| quic | 11 | 10 | 1 | 0 | 8 PASS, 1 expected, 2 unexpected |
| ipv6 | 27 | 27 | 0 | 0 | 19 PASS, 8 unexpected |

The declared failures: udp `Rfc1122ChecksumDefault`, `Rfc1122MulticastSourceAddress`; ipv4
`Rfc1122InvalidSourceAddress`, `Rfc1122VersionDiscard`; dhcp `Rfc2131BroadcastBitClear`,
`Rfc2131InformWithoutLease`, `Rfc2131ReleaseOnShutdown`, `Rfc6842ClientIdentifierEchoed`,
`Rfc6842ForeignClientIdentifier`; tcp `Rfc9293ChecksumDefault`, `Rfc9293Push`; quic
`Rfc9000VersionNegotiation`.

## Steps

Commit group: `standards-tests-wave0`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and at the end `check-commits.sh master..HEAD` and
`check-classification.sh master..HEAD`.

1. [x] **The guide: a run record survives a rebase.** Done in `97f4559eee`. The run-record table and script of
   `guide/derive-tests-from-a-standard.md` step 7 gain the `src/` and `tests/protocol/` tree
   hashes. The guide also says what a level-1 pass records instead of a run: the commit and
   the trees that the claims were read from.
2. [x] **TCP ledger.** Done in `28a483eb3d`; see the decisions below for the level. Rebuild `model/tcp/coverage.md` for the in-scope set of pass 4: the 39
   rows of RFC 6298 and RFC 5681, the 10 features of level 4, the achieved-level text and table,
   and the fresh run. Correct the pass-log sentence about the sweep. Correct the summary
   sentence of `conformance.md`. Give `results.md` the trees of its old commit.
3. [x] **The six other ledgers** (arp `887770ae37`, quic `50d4d8c59a`, ipv4 `ee39dc807b`,
   udp `8a46bf4945`, ipv6 `179975ae33`, dhcp `b98d771aa3`) (arp, dhcp, ipv4, ipv6, quic, udp), one commit each: the fresh
   run record, each row verdict from the fresh run, the feature support by the rule of step 7,
   the conformance matrix from the new support, and the trees of the old commit in each snapshot
   document. Fix the inconsistent ARP summary line on the way.
4. [x] **Small debt**, one commit, `828c49cb46`, which also turned the 16 links of
   `model/wifi/results.md` into `audit/` into inline paths: the UDP `features.md` prose count; the stale "one tap
   carries one rule" in `model/ipv4/notes.md`; the Mobile IPv6 entry of `AUTHORING.md`, whose
   two tests were removed on 2026-09-10.
5. [x] **Level 1 for the Mode B protocols**, one commit per protocol: RFC texts in
   `evidence/standard/`, `protocol/<proto>/standards.md`, `model/<proto>/conformance.md` (part 1
   only) and `model/<proto>/coverage.md` (the achieved level and the pass log). Folder names
   from the survey, §2.3:

   | `<proto>` | Base documents |
   | --- | --- |
   | `rip` | RFC 2453, RFC 2080 |
   | `nd` | RFC 4861, RFC 4862 |
   | `igmp` | RFC 9776, RFC 2236 |
   | `mld` | RFC 9777, RFC 2710 |
   | `ipsec` | RFC 4301, RFC 4302, RFC 4303 |
   | `mpls` | RFC 3031, RFC 3032 |
   | `mipv6` | RFC 6275 |
   | `pmipv6` | RFC 5213 (and RFC 6275 from `mipv6`) |
   | `ipv6tunnel` | RFC 2473 |
   | `diffserv` | RFC 2474, RFC 2475, RFC 2597, RFC 3246, RFC 2697, RFC 2698 |
   | `ospfv2` | RFC 2328 |
   | `ospfv3` | RFC 5340 (and RFC 2328 from `ospfv2`) |
   | `bgp` | RFC 4271, RFC 4760, RFC 5492 |
   | `pim` | RFC 7761, RFC 3973 |
   | `aodv` | RFC 3561 |
   | `sctp` | RFC 9260 |
   | `rtp` | RFC 3550, RFC 3551 |

   The level-2 in-scope set of each map comes from the register, and the target level is 1.
   `rip` goes first and is the template for the others: done in `e360ca980e`. Six agents
   write the other 16; each protocol gets its own commit after review.
6. [x] Gates, then move this plan to `plan/done/`. All four gates pass on `master..HEAD`:
   225 files and 0 broken links, seals in step, commits and classification clean.

## Decisions and facts found on the way

- The fresh run replaces the run record of every ledger. A `results.md` stays the record of its
  pass; it keeps its old commit and gains the trees of that commit, so a reader can find the
  same code on `master`.
- **TCP was not at level 4.** Pass 4 recorded "4, reached" from tables that still held RFC
  9293 only. With RFC 6298 and RFC 5681 in the in-scope set, nine mandatory statements have no
  check (RFC6298-UPD-1, SAMP-1, GRAN-1; RFC5681-USE-1, CA-2, FR-4, SSTH-1, FR-5, ACK-1), two
  of them named by a test that does not assert them, and TIME-WAIT has no catalog entry. The
  ledger now says level 2 reached, level 3 partial, level 4 partial, with 18 `owed` rows.
- **Claims that the TCP conformance document got wrong:** `Tcp.ned` names RFC 793 and RFC 2581,
  not RFC 9293 and RFC 5681; the lost-SYN quote in `TcpBaseAlg.cc:138-140` has no RFC number;
  `TcpBaseAlg.cc:400` cites RFC 5681 for the idle restart, which makes RFC5681-IDLE-1 claimed.
- **ARP:** since `bdde132792` the DHCP client calls `Arp::sendArpProbe`; the claim of RFC 5227
  no longer rests on code without a caller. `sendArpGratuitous` still has none.
- **The refresh agents dropped the claim-scan record** of two conformance documents (ipv4, udp)
  when they put in the new run record. The record belongs to part 1, so it was restored.
- **A snapshot claim can go stale silently.** "Every source file that part 1 cites is identical
  to the file at the commit above" stopped being true for QUIC after `948c8b5cb4`; the
  sentence now names what changed.
- **DHCP:** the repair `bdde132792` gave the server the code of RFC2131-DECL-4, so the row moves
  from `no check` to `owed` (the debt is 14), and DHCP-F-DECLINE reads `partial`.
  DHCP-F-DUPLICATE-DETECTION reads `partial` too: the server's probe (OFF-7) has no code. The
  debt heading lost its count, so its anchor survives the next change of the count.
- **The spec-first zone leaked in three maps** (igmp, mld, nd): sentences about what "the model"
  does. They were removed before the commits; each map keeps only a constant pointer to
  `conformance.md`.

## Level 1: what each map found

| `<proto>` | In-scope set for level 2 | Headline of level 1 |
| --- | --- | --- |
| `rip` | RFC 2453, RFC 2080 | claims both; the serializer names RFC 1058 for the version 2 layout; authentication declined in words |
| `nd` | RFC 4861, RFC 4862, RFC 5942, RFC 6980 | class docs name RFC 2461; about three of four code citations name RFC 2461 or RFC 2462 |
| `igmp` | RFC 9776, RFC 2236 | claims RFC 3376, obsoleted by RFC 9776 (March 2025); the GMI formula and the unknown-type rule changed |
| `mld` | RFC 9777, RFC 2710 | claims RFC 3810, obsoleted by RFC 9777; the "Parity gaps" note of MLDv2 is stale; no Router Alert |
| `ipsec` | RFC 4301, RFC 4302, RFC 4303 | clean claim with five stated limits; ICV verification is a bare TODO, so a claim |
| `mpls` | RFC 3031, RFC 3032, RFC 3443, RFC 5462 | no document named anywhere; the TTL of every label entry is 0 on the wire |
| `mipv6` | RFC 6275 | nine places name RFC 3775; only the serializer names RFC 6275; nonce fixed at 0 |
| `pmipv6` | RFC 5213, RFC 4283 | clean claim of RFC 5213 |
| `ipv6tunnel` | RFC 2473 | clean claim in eight places |
| `diffserv` | RFC 2474, 2475, 2597, 3246, 2697, 2698, 3260 | the EF code point cites RFC 2598, obsoleted by RFC 3246, which the queue cites |
| `ospfv2` | RFC 2328, a one-area slice | RFC1583Compatible cites RFC 3101 and RFC 1583, both wrong; authentication and LSA checksum return true |
| `ospfv3` | RFC 5340 and the RFC 2328 clauses it keeps | clean claim; steps 4 and 5 of the route calculation commented out |
| `bgp` | RFC 4271, 4760, 5492, 6793, 8212 | claims RFC 4271 and RFC 4760, not the two updates that change the normal exchange; AS numbers are 16-bit |
| `pim` | RFC 7761, 3973, 3956, 4607 | the PIM-SM module docs name RFC 4601; the checksum comment names RFC 2460 |
| `aodv` | RFC 3561, RFC 5148 | clean claim; local repair has a parameter and a bare TODO, so it is a claim |
| `sctp` | RFC 9260, a clause slice | no place names RFC 9260; RFC 4960 is an enum value; one comment names RFC 2960 |
| `rtp` | RFC 3550, RFC 3551 | the README names RFC 1889 and RFC 1890; payload type 10 lives in files the build excludes |

## What the next wave inherits

- The TCP debt of 18 `owed` statements, nine of them mandatory, and the TIME-WAIT catalog entry.
- The DHCP debt of 14 `owed` statements.
- The level 2 facts at the end of each new `conformance.md`, which decide for each gap whether
  a failing check is a defect or a declared expected failure.
