# MPLS level 2 — catalogs, feature map, checks and tests for RFC 3031, RFC 3032, RFC 3443, RFC 5462

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-mpls-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-mpls-level2`.

The fifth and last pass of wave 1, after RIP, ND, IGMP with MLD, and IPsec. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md),
steps 2 to 9, at **level 2, Core**. The level 1 pass of wave 0 wrote the standards map and part 1
of the conformance document; this pass starts from them.

The user's decision of 2026-09-24 holds here too: the pass measures the model and repairs
nothing; the failures are repaired later.

Commit group: `mpls-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.
The pass delivers every output of the section "What a pass delivers" of the guide (on
`topic/standards-tests-igmp-mld-level2`, not yet on this branch), `notes.md` included.

## What the pass takes over from the earlier passes

- Catalogs drafted by agents with one brief, every quote checked, drafts merged with the lead
  and strength normalization of the IPsec merge.
- A feature map and a closing list from generators that refuse an unplaced entry.
- Tests from a generator with shared mockups; the exploration dump before a failure counts.
- A count with an assertion matches once; a delivered datagram is seen at `<host>.udp`
  (`packetSentToUpper`); IPsec-like per-node parameters go to the right nodes only.
- `notes.md` gets each lesson when it occurs; `standards.md` gets the target level of the pass.

## Steps

1. [x] **Plan** — this file.
2. [x] **Step 2, the standards map** — `protocol/mpls/standards.md`: target level 2, and the
   sections that level 2 needs beyond the level 1 list.
3. [x] **Step 3, catalogs** — `standard/rfc3031/catalog.md`, `standard/rfc3032/catalog.md`,
   `standard/rfc3443/catalog.md`, `standard/rfc5462/catalog.md`; every quote checked.
4. [x] **Step 4, feature map** — `protocol/mpls/features.md` (`MPLS-F-*`).
5. [x] **Step 5, checks** — `protocol/mpls/checks.md` and `protocol/mpls/checks/*.md`, with the
   closing list.
6. [ ] **Step 6, tests** — `tests/protocol/mpls/Rfc30*.test`, `Rfc3443*.test`, and the helper
   header `MplsChecks.h`.
7. [ ] **Steps 7 to 9, and the ledger** — `model/mpls/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`, from one fresh run.
8. [ ] **Notes** — `model/mpls/notes.md`, with every gap by number in the follow-ups.
9. [ ] Gates, then move this plan to `plan/done/`.

Working scripts: `audit/mpls-level2/` in `inet-master` (outside git), with a `README.md`.

## Decisions and facts found on the way

- **The in-scope sections grow for level 2.** RFC 3031 adds the labeled packet (§3.3), the LSP
  with its ingress and egress (§3.15) and the Implicit NULL label (§4.1.5), which §3.16 needs.
  RFC 3032 adds fragmentation and path MTU (§3, nine MUST lines), and the encapsulation on PPP
  links (§4) and on LAN media (§5), because a labeled packet crosses a link in every test. RFC
  3443 takes §2 and §3 whole; RFC 5462 takes §2.1 and §3.
- **The catalogs are drafted by three agents** (scratchpad `mpls/mpls-catalog-brief.md`): RFC
  3031, RFC 3032, and RFC 3443 with RFC 5462.
- **The model's own static ingress binding serves the tests.** `RsvpClassifier` binds a
  destination to a label of the LIB without RSVP signaling (`<fecentry>` with a `<label>`,
  `RsvpClassifier::readItemFromXML`), and `LibTable` reads a static LIB, as the example
  `examples/mpls/testte_tunnel` does. The tests need no module of their own for the FTN.
- **The mockups use PPP links, because MPLS over Ethernet does not work in the model.** On an
  Ethernet interface, the ARP module gets a labeled packet and throws "Unknown message received"
  (the FIXME of `Mpls.cc:210-214`). With `GlobalArp`, the labeled packet reaches the MAC without
  an Ethernet header, and "Cannot convert chunk from type inet::MplsHeader to type
  inet::EthernetMacHeader" stops the run. On PPP links, the path A — R1 — R2 — R3 — B works:
  R1 pushes 100, R2 swaps to 200, R3 pops. The PPP protocol field of a labeled packet is 0281
  hex. The Ethernet encapsulation of RFC 3032 §5 becomes a check of its own.
- **Facts of the exploration run that the checks must expect.** The MPLS TTL is 0 in every
  entry: `Mpls::pushLabel` and `swapLabel` never set it. The IPv4 TTL stays 32 from A to B,
  because the ingress labels the packet before the IPv4 forwarding step, and the egress pops it
  and sends it to the link with no decrement. The model has no MTU code: a labeled packet of 1032
  octets crossed a link whose MTU is 500. An incoming label without a LIB entry is discarded
  (`Mpls.cc:241-246`). The pop of the last label gives the packet to IPv4, whatever its protocol.
- **The feature map has 14 features** (11 mandatory, 3 optional) and places all 157 entries:
  RFC 3031 with 49, RFC 3032 with 81, RFC 3443 with 24, RFC 5462 with 3. RFC 3031 and RFC 3443
  have few keywords, so three features are `mandatory` by the "only path" rule: the label stack
  encoding, the label forwarding and the LAN encapsulation. ICMP is `optional`: its must holds
  only for an LSR that sends an ICMP message. Penultimate hop popping stays `mandatory`: its
  condition, an LSR that can pop at all, holds for every LSR that ends an LSP.
- **15 checks in six files** (`encoding`, `forwarding`, `reserved-labels`, `ttl`,
  `fragmentation`, `links`) place 99 entries; the closing list places the other 58 in 14 groups.
  Every mandatory feature has a core check except the discard of a label without a binding,
  which the standards map puts at level 3. The mockups use one path, A — R1 — R2 — R3 — B, on PPP
  links, and one Ethernet variant for RFC 3032 §5. A rule makes the bindings by hand, as a label
  distribution protocol would, for the request for penultimate hop popping and for the Explicit
  NULL and Implicit NULL labels. A second rule fixes the Uniform Model and RFC 1812 IPv4
  forwarding in the LSRs, so that the TTL on each link is known.
- **IPv6 and the Router Alert label go to the closing list.** A labeled IPv6 datagram needs an
  IPv6 mockup; the Router Alert label needs local software in the LSR and a rule for what that
  software does, which RFC 3032 does not give. The ledger decides whether each is owed.
