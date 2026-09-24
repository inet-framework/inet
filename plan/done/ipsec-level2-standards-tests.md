# IPsec level 2 — catalogs, feature map, checks and tests for RFC 4301, RFC 4302, RFC 4303

**Status:** done. Started and finished 2026-09-24 on `topic/standards-tests-ipsec-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-ipsec-level2`.

The fourth pass of wave 1, after RIP, ND, and IGMP with MLD. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md),
steps 2 to 9, at **level 2, Core**. The level 1 pass of wave 0 wrote the standards map and part 1
of the conformance document; this pass starts from them.

The user's decision of 2026-09-24 holds here too: the pass measures the model and repairs
nothing; the failures are repaired later.

Commit group: `ipsec-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.
The pass delivers every output of the section "What a pass delivers" of the guide. That section
is on `topic/standards-tests-igmp-mld-level2` and not yet on this branch; the steps below follow
it.

## What the pass takes over from the earlier passes

- The catalogs are drafted in parallel by agents with one brief, and a script checks every quote
  against its line reference; the drafts are then merged, reviewed and indexed.
- The feature map comes from a script that must place every entry, and a script checks that every
  entry is in exactly one check or in the closing list of `checks.md`.
- Tests come from a generator with shared mockups, and the ledger table from a generator.
- An exploration test (never committed) shows the cause of a failure before it counts as a finding.
- A known-failing observation goes last, or into a test of its own; every observation must be able
  to fail in its scenario; a check that is stricter than the standard is a test error.
- `notes.md` gets each lesson when it occurs.

## Steps

1. [x] **Plan** — this file.
2. [x] **Step 2, the standards map** — `protocol/ipsec/standards.md`: target level 2, and the
   sections that level 2 needs beyond the level 1 list.
3. [x] **Step 3, catalogs** — `standard/rfc4301/catalog.md`, `standard/rfc4302/catalog.md`,
   `standard/rfc4303/catalog.md`; every quote checked against its lines. RFC 4301 370 entries,
   RFC 4302 190, RFC 4303 237: 797 in all.
4. [x] **Step 4, feature map** — `protocol/ipsec/features.md` (`IPSEC-F-*`). With step 3 in one
   commit. 29 features: 21 mandatory, 7 optional, 1 unstated; every entry is placed.
5. [x] **Step 5, checks** — `protocol/ipsec/checks.md` and `protocol/ipsec/checks/*.md`, with
   the closing list of the statements without a check. 23 checks in 7 files; 325 statements are
   in a check, and the other 472 are in the closing list, in 15 groups of what a check would
   need.
6. [x] **Step 6, tests** — `tests/protocol/ipsec/Rfc430[123]*.test` and the helper header
   `IpsecChecks.h`. 31 tests for the 23 checks. First full run: 15 PASS, 6 FAIL declared expected
   (missing features), 10 FAIL.
7. [x] **Steps 7 to 9, and the ledger** — `model/ipsec/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`, from one fresh run. Run at `829ba07bae`: 31 tests, 15 PASS,
   16 FAIL, 6 of them declared expected. Twelve gaps: six defects, one untestable claim, five
   unimplemented features. Level 2 is reached for the normal path; 6 statements are owed. The
   matrix holds 8 `confirmed`, 9 `partial`, 2 `defect`, 1 `declined`, 3 `unverified` and 6 `out
   of claim`.
8. [x] **Notes** — `model/ipsec/notes.md`: the model quirks, the scenario and tooling traps, and
   the follow-ups with every gap by number (1 to 12).
9. [x] Gates, then move this plan to `plan/done/`. The four gates pass on the 8 commits over
   `origin/master` (links, seals, commits, classification).

Working scripts: `audit/ipsec-level2/` in `inet-master` (outside git), with a `README.md` that
says what each one does.

## Decisions and facts found on the way

- **The in-scope sections grow for level 2.** The level 1 list left out the SA and its two modes
  (RFC 4301 §4 to §4.3, with "A host implementation of IPsec MUST support both transport and
  tunnel mode", `rfc4301.txt:873-875`), the introduction to the databases (§4.4), the location
  of each header (§3.1 of RFC 4302 and RFC 4303) with the algorithms (§3.2), and the conformance
  sections (RFC 4301 §10, RFC 4302 §5, RFC 4303 §5). `standards.md` now names them, with the
  target level 2.
- **The RIP, ND, IGMP and MLD passes left their target level at 1.** Their `standards.md` still
  said "Level 1 — Survey" and "none yet" for the catalogs. Corrected on their branches:
  `3396c80ea1` (RIP), `03f8e313c5` (ND), `8dba80c00a` (IGMP and MLD).
- **The catalogs are drafted by seven agents** with one brief (scratchpad
  `ipsec/ipsec-catalog-brief.md`): RFC 4301 in four ranges, RFC 4302 in one, RFC 4303 in two.
- **The merge normalizes the drafts.** The agents recorded a keyword that the text writes in lower
  case in two ways, as `description` or as the keyword itself; the merge writes `must (lower
  case)` for both, as the other catalogs do. It also writes each lead on one line with its
  keywords in lower case, and puts a Strength line that a draft wrapped back on one line — a
  wrapped line hid RFC4302-SPI-7 from a listing and would hide it from a generator.
- **AH is optional, ESP mandatory.** RFC 4301 says "MUST support ESP" and "MAY support AH"
  (RFC4301-OVW-5), and RFC 4302 makes all of AH a must for an implementation that offers it
  (RFC4302-CONF-1). The two AH features are `optional`; each rule inside them is a must once AH
  is there.
- **Four features depend on key management**, which the in-scope set leaves to level 5:
  SA creation with the PFP flags and lifetimes, named SPD entries, the anti-replay window's
  notification, and the negotiation of ESN and TFC padding. Their statements stay in the catalog
  and in the map; the checks decide what a statically keyed implementation can show.
- **Two mandatory features have no core check at level 2.** The named SPD entries need a key
  management protocol, and the anti-replay service needs a packet that arrives twice. A sender
  with two SAs that share an SPI would make such a packet, but that is a configuration the
  standard forbids, so the replay stays with level 3, as the standards map planned.
- **A statically keyed host has a defined answer for key management.** Where a rule depends on
  key management, a check uses the case that RFC 4301 gives for a host without it: a PROTECT
  entry without an SA discards the packet (RFC4301-OUT-10), and an SA at the end of its lifetime
  ends, because no replacement comes.
- **The observer reads an encrypted ESP payload with the keys of the SA.** The SAs are keyed by
  hand, so the checks can read the trailer and the inner header; where a check needs plaintext
  on the link, it uses NULL encryption (padding contents, TFC padding).
- **The closing list comes from a generator** (scratchpad `ipsec/gen-ipsec-closing.py`) that
  refuses a statement in no group or in two, and the placement checker confirms that every entry
  is in exactly one of a check and the closing list.
- **The checks name HMAC-SHA-256-128, not HMAC-SHA-1-96.** The exploration showed that the
  model's `HMAC_SHA1` makes a 20-octet ICV, where HMAC-SHA-1-96 of RFC 2404 makes 12 — a matter
  of the algorithm documents, level 5. HMAC-SHA-256-128 (16 octets) is the integrity algorithm
  that RFC 8221 makes mandatory, and the model's `HMAC_SHA2_256_128` makes 16 octets. The checks
  and the tests changed together, so that each test tells the story of its check.
- **The check of a both-NULL SA accepts a refusal at configuration.** The model stops at
  initialization with "Cannot set authenticationAlg=NONE if espMode=INTEGRITY", on stderr. The
  check now names both forms of the refusal, and the test expects the error (`%exitcode: 1`,
  `%contains-regex: stderr`).
- **Test tooling facts found on the way** (they go into `notes.md`):
  - a pattern with an assertion matches only the first event that its filter picks, so a counting
    guard with an assertion counts one; the length of the delivered datagrams is a guard of its own;
  - the tester does not subscribe to the `packetReceived` signal of an application, so a datagram
    is "delivered" when UDP emits `packetSentToUpper` with its port in the `L4PortInd` tag;
  - `**.ipv4.hasIpsec` also gives the router IPsec, which then asks for an SPD; only the hosts get
    it (`*.host*`);
  - the MTU of an Ethernet interface is a parameter of its MAC, `eth[1].mac.mtu`;
  - the exploration dump (`mkexplore.py`) showed the chunk layout of every packet kind before a
    test read it: the AH payload is an `EncryptedChunk` although AH does not encrypt, the ICV is a
    `ByteCountChunk` at the end, an IPv6 fragment has an `Ipv6FragmentHeader` chunk, and a
    fragment carries a `SliceChunk` of the protected packet.
- **The model applies IPsec before fragmentation**, on IPv4 and IPv6: only the first fragment
  holds the AH or ESP header, and B reassembles before IPsec. An IPv4 router fragments a protected
  packet on its way, and B still delivers it.
- **The IPv6 layer keeps no path MTU.** R sends Packet Too Big, Icmpv6 passes it up as an
  indication (`Icmpv6.cc:124`), and A goes on with packets of 1410 octets. Neither IPsec nor IPv6
  has code for it, so the test declares the failure expected: a missing feature.
- **AH across a router passes, but the model cannot fail it.** The receiver never verifies the
  ICV (the TODO at `IPsec.cc:832`), so a changed TTL cannot make it reject a packet. The ledger
  records the pass and says that only level 3, an ICV that fails, gives the verdict weight.
- **The claim of a feature in the matrix follows the stated refusals.** The model claims all
  three RFCs, so each feature is claimed by its document, except the six that a stated refusal of
  part 1 names (tunnel mode, multicast, anti-replay, DSCP selection, SA creation with its lifetime,
  named SPD entries): those are `out of claim`. Dummy packets and the path MTU have no stated
  refusal, so the table makes them `defect`; `results.md` classes their failures as unimplemented
  features, because no code exists.
- **Level 2 is reached for the normal path**, as the IGMP pass read the criterion: 19 of the 21
  mandatory features have core checks that ran, and the other two, named SPD entries and
  anti-replay, need key management or a replayed packet.
