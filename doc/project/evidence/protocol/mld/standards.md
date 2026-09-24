# MLD — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around Multicast Listener Discovery, records which document governs each contested clause,
and pins the set that the next pass tests against. MLD is the IPv6 line of descent that
parallels IGMP's IPv4 line: version 1 and version 2 describe the same state machines RFC 9776
and RFC 9777 describe for IGMP, over the two address families. Both pairs reached Internet
Standard on the same day, March 2025. [`igmp/standards.md`](../igmp/standards.md) is the
parallel map; the correspondence table below lines up the two families section by section.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 9777, the general exchange (§5 message formats, §6 the listener state machine and report generation, §6.1 State-Change Report retransmission, §7 router forwarding and reception rules, §8 interoperation with MLDv1); RFC 2710 §5-§7 (the version 1 exchange the compatibility mode falls back to) | the normal exchange: an unsolicited Report on join, a General Query answered within the Maximum Response Delay, a State-Change Report on a filter-mode change, and the version 1 compatibility fallback |
| level 3, Edge | RFC 9777's unrecognized-message-type rule (see the override table; RFC 2710 has no equivalent explicit statement) | a crafted message with an out-of-range type exercises the rule |
| level 4, Dynamics | RFC 9777 §9 in full: the Multicast Address Listening Interval, the Other Querier Present Interval, the Older Version Querier/Host Present Interval, the startup and last-listener query counts and intervals — one of which changed formula from RFC 2710/RFC 3810 to RFC 9777 (see the override table) | the timers and the retransmission counts that decide how fast the router notices a departed listener |
| level 5, Complete | RFC 4604 (SSM-aware behavior for MLDv2); querier election and MLD snooping | a router or host that restricts itself to the SSM address range; a link with more than one querier |

What a pass actually reached is not recorded here. It is in
[`model/mld/coverage.md`](../../model/mld/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 9777 | Multicast Listener Discovery Version 2 (MLDv2) for IPv6 | March 2025 | Internet Standard (STD 101) | `base`; `obsoletes` RFC 3810; `updates` RFC 2710 | [`standards/RFC/rfc9777.txt`](../../../../../../standards/RFC/rfc9777.txt), 2026-09-23 |
| RFC 2710 | Multicast Listener Discovery (MLD) for IPv6 | October 1999 | Proposed Standard | `companion`; `updated by` RFC 3590, RFC 3810 and RFC 9777; not obsoleted | [`standards/RFC/rfc2710.txt`](../../../../../../standards/RFC/rfc2710.txt), 2026-09-23 |
| RFC 3810 | Multicast Listener Discovery Version 2 (MLDv2) for IPv6 | June 2004 | Proposed Standard | obsoleted by RFC 9777; `updates` RFC 2710; `updated by` RFC 4604 | no |
| RFC 3590 | Source Address Selection for the Multicast Listener Discovery (MLD) Protocol | September 2003 | Proposed Standard | `updates` RFC 2710; not obsoleted; not named by either RFC 3810 or RFC 9777 (checked by grep, neither downloaded text mentions "3590") | no |
| RFC 4604 | Using IGMPv3 and MLDv2 for Source-Specific Multicast | August 2006 | Proposed Standard | `updates` RFC 3376 and RFC 3810 (shared with [`igmp/standards.md`](../igmp/standards.md#document-list)); not shown by the register as updating RFC 9776 or RFC 9777 | no |

Source of the texts:

- `rfc9777.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc9777.txt) —
  <https://www.rfc-editor.org/rfc/rfc9777.txt>, downloaded 2026-09-23.
- `rfc2710.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2710.txt) —
  <https://www.rfc-editor.org/rfc/rfc2710.txt>, downloaded 2026-09-23.
- RFC 4443 (ICMPv6) is not downloaded again for this protocol either; every MLD message is an
  ICMPv6 message (types 130/131/132/143), and it belongs to the ipv6 pass, already downloaded as
  [`standards/RFC/rfc4443.txt`](../../../../../../standards/RFC/rfc4443.txt) — see
  [`ipv6/standards.md`](../ipv6/standards.md#document-list).

`grep -c '\bMUST\b' rfc9777.txt` counts 44 lines, `\bSHOULD\b` counts 15, `\bMAY\b` counts 4.
`rfc2710.txt` counts 17 `MUST` lines, 5 `SHOULD` and 3 `MAY`; both documents formally adopt
the RFC 2119 keywords.

### Where IGMP and MLD correspond

The two documents share one editor and one structure. Section numbers differ by exactly one,
starting at §2, because MLD carries an extra "Protocol Overview" section IGMP folds into its
introduction:

| Subject | RFC 9776 (IGMP) | RFC 9777 (MLD) |
| --- | --- | --- |
| Service interface | §2 | §3 |
| Reception state maintained by the node | §3 | §4 |
| Message formats | §4 | §5 |
| Protocol for the host/listener | §5 | §6 |
| Protocol for the router | §6 | §7 |
| Interoperation with the older version | §7 | §8 |
| Timers and defaults | §8 | §9 |

RFC 2236 and RFC 2710 correspond less exactly, because RFC 2710 was written after RFC 2236
and restructures around a message-format section RFC 2236 does not have:

| Subject | RFC 2236 (IGMPv2) | RFC 2710 (MLDv1) |
| --- | --- | --- |
| Host state diagram | §6 | §5 (there: "Node State Transition Diagram") |
| Router state diagram | §7 | §6 (there: "Router State Transition Diagram") |
| Timers and defaults | §8 | §7 |

Four relationship notes the register alone does not give, in the same shape as the IGMP ones:

- **RFC 2710 is not marked obsoleted**, by RFC 3810 or by RFC 9777. Both later documents
  `update` it, and MLDv1 report/query/done procedures stay governed by RFC 2710's own text
  where RFC 9777 does not restate them.
- **RFC 9777 is the 2025 Internet Standard revision of RFC 3810**, published the same month as
  RFC 9776. Its "Summary of Changes" appendix has two parts: §B.1 against MLDv1
  (`rfc9777.txt:2966-3030`) and §B.2 against RFC 3810 (`rfc9777.txt:3031-3046`). §B.2 names
  "Erratum 6725" for the Group Membership Interval Timer change — the same erratum number
  RFC 9776's Appendix C names for IGMP's identical formula fix (`rfc9776.txt:2523-2545`),
  confirming the two changes are one correction applied to both documents, not independent
  drift. §B.2's other three items (Erratum 4773, a Resv-field definition; Erratum 5977, which
  addresses require an MLD message; a Reserved-to-Flags field rename for a future IANA
  registry) touch no clause this pass brought into scope.
- **RFC 9777 did not carry every RFC 9776-style fix over.** RFC 9776 changed IGMP's Older
  Version Querier Present Interval formula and downgraded it to a `SHOULD`
  (`rfc9776.txt:2110-2122`); RFC 9777's equivalent clause, §9.12, is unchanged from RFC 3810
  and stays a `MUST` (`rfc9777.txt:2622-2631`). The two documents are siblings, not mirrors —
  the pass that keeps their maps parallel still has to check each clause on its own.
- **RFC 3590 is a narrow, orphaned companion.** It fixes the source address a node chooses for
  an MLD message it sends while its own address is still tentative (during SLAAC). Neither
  RFC 3810 nor RFC 9777 names it, so the later text does not carry it forward.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Multicast Address Listening Interval, the timeout after which a router decides a group has no more listeners | RFC 2710 §7.4, `rfc2710.txt:923-929`: "MUST be ((the Robustness Variable) times (the Query Interval)) plus (**one** Query Response Interval)" — RFC 3810 §9.4 states the identical formula (not downloaded; same clause number, same wording) | RFC 9777 §9.4, `rfc9777.txt:2551-2557`: "MUST be ([Robustness Variable] times [Query Interval]) plus **2 times** [Query Response Interval]" | RFC 9777 | **yes**, level 4 |
| Older Version Querier Present Interval | RFC 3810 §8.2.1 (not downloaded; same clause as below) | RFC 9777 §9.12, `rfc9777.txt:2622-2631`: "MUST be ([Robustness Variable] times [Query Interval] in the last Query received) plus ([Query Response Interval])" — **unchanged** from RFC 3810; the register shows no formula or keyword change here, unlike IGMP's equivalent clause | RFC 9777 (no substantive change from RFC 3810) | not a conflict; background |
| Unrecognized message types | RFC 2710 states no rule for an unrecognized MLD type at all (`grep -i unrecognized rfc2710.txt` finds nothing) | RFC 9777, `rfc9777.txt:765`: "Unrecognized message types MUST be silently ignored." — the rule enters the family only with MLDv2, not as an upgrade of an older statement the way IGMP's did | RFC 9777 | **yes**, level 3 |
| Router Alert on every message | RFC 2710 §3, `rfc2710.txt:83-85`: "All MLD messages described in this document **are sent with** a link-local IPv6 Source Address, an IPv6 Hop Limit of 1, and an IPv6 Router Alert option... in a Hop-by-Hop Options header" — descriptive, no RFC 2119 keyword | RFC 9777 §5, `rfc9777.txt:738-741`: "All MLDv2 messages described in this document **MUST** be sent with a link-local IPv6 Source Address, an IPv6 Hop Limit of 1, and an IPv6 Router Alert option [RFC2711] in a Hop-by-Hop Options header" — upgraded to a formal MUST, exactly the same shape of change IGMP's Router Alert clause underwent (`igmp/standards.md`) | RFC 9777 | yes, level 2 — a base-document requirement on every message; see the facts in [`conformance.md`](../../model/mld/conformance.md) |
| SSM-aware router and host behavior | RFC 3810 has no SSM-aware concept | RFC 4604 (a companion to both RFC 3810 and RFC 9777 by subject, though the register does not show it updating RFC 9777 directly) | RFC 9777, with RFC 4604 as the document that introduced the rule | no; level 5 |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 9777 | March 2025, Internet Standard (STD 101); §5 to §9 | none yet; level 2 writes `standard/rfc9777/catalog.md` |
| RFC 2710 | October 1999, Proposed Standard; §5 to §7 | none yet; level 2 writes `standard/rfc2710/catalog.md` |

RFC 4443 stays a background companion, exactly as in the ipv6 pass: every MLD message is an
ICMPv6 message, and the checks tolerate its general rules without re-deriving them.

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 3810 | Obsoleted by RFC 9777. |
| RFC 3590 | Level 5 / edge case. A narrow source-address-selection fix for the DAD-tentative case; not referenced by either RFC 3810 or RFC 9777. |
| RFC 4604 | Level 5. SSM-aware behavior. |
| RFC 4443 | Owned by the ipv6 pass; referenced as a companion, not re-scoped here. |
