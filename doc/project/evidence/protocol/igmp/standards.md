# IGMP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Internet Group Management Protocol, records which document governs each contested
clause, and pins the set that the next pass tests against. IGMP has three versions in one
line of descent — each document restates and extends its predecessor rather than describing
a separate protocol — so this map has one base document, not several.
[`mld/standards.md`](../mld/standards.md) maps the parallel line of descent for IPv6; the two
address families reached Internet Standard on the same day in 2025, and the notes below say
which sections of each correspond.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 9776, the general exchange (§4 message formats, §5 host state diagram and report generation, §6.1 State-Change Report retransmission, §6.3-§6.4 router forwarding and reception rules, §7 host and router compatibility with older versions); RFC 2236 §6-§7 (the version 2 exchange the compatibility mode falls back to) | the normal exchange: an unsolicited Report on join, a General Query answered within the Max Response Time, a State-Change Report on a filter-mode change, and the version 2 compatibility fallback |
| level 3, Edge | RFC 9776, line 482: "Unrecognized message types MUST be silently ignored" (RFC 2236 states the same rule as a lower-case "should", `rfc2236.txt:123`) | a crafted message with an out-of-range type exercises the rule |
| level 4, Dynamics | RFC 9776 §8 in full: the Group Membership Interval, the Other Querier Present Interval, the Older Version Querier/Host Present Interval, the startup and last-member query counts and intervals — several of which changed formula from RFC 2236/RFC 3376 to RFC 9776 (see the override table) | the timers and the retransmission counts that decide how fast the router notices a departed member |
| level 5, Complete | RFC 1112 (version 1 hosts and routers); RFC 4604 (SSM-aware behavior; RFC 9776 §6.4 and §8.1.2 already carry its rules) | a version 1 host or router, and a router or host that restricts itself to the SSM address range |

What a pass actually reached is not recorded here. It is in
[`model/igmp/coverage.md`](../../model/igmp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 9776 | Internet Group Management Protocol, Version 3 | March 2025 | Internet Standard | `base`; `obsoletes` RFC 3376; `updates` RFC 2236 | [`standards/RFC/rfc9776.txt`](../../../../../../standards/RFC/rfc9776.txt), 2026-09-23 |
| RFC 2236 | Internet Group Management Protocol, Version 2 | November 1997 | Proposed Standard | `companion`; `updates` RFC 1112; `updated by` RFC 3376 and RFC 9776; not obsoleted | [`standards/RFC/rfc2236.txt`](../../../../../../standards/RFC/rfc2236.txt), 2026-09-23 |
| RFC 1112 | Host extensions for IP multicasting | August 1989 | Internet Standard | version 1; `updated by` RFC 2236; **not** marked `obsoleted by` anything in the register | no |
| RFC 3376 | Internet Group Management Protocol, Version 3 | October 2002 | Proposed Standard | obsoleted by RFC 9776; `updates` RFC 2236; `updated by` RFC 4604 | no |
| RFC 4604 | Using IGMPv3 and MLDv2 for Source-Specific Multicast | August 2006 | Proposed Standard | `updates` RFC 3376 and RFC 3810; not obsoleted; not shown by the register as updating RFC 9776 or RFC 9777, even though both fold its content in by reference | no |

Source of the texts:

- `rfc9776.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc9776.txt) —
  <https://www.rfc-editor.org/rfc/rfc9776.txt>, downloaded 2026-09-23.
- `rfc2236.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc2236.txt) —
  <https://www.rfc-editor.org/rfc/rfc2236.txt>, downloaded 2026-09-23.

`grep -c '\bMUST\b' rfc9776.txt` counts 48 lines, `\bSHOULD\b` counts 20, `\bMAY\b` counts 4.
`rfc2236.txt` counts 25 `MUST` lines, 10 `SHOULD` and 6 `MAY`; both documents formally adopt
the RFC 2119 keywords (`rfc2236.txt:44-46`). RFC 1112 predates RFC 2119 by eight years and is
not downloaded; the survey lead that called it "obsoleted by RFC 2236" is not what the register
says — see the note below.

Four relationship notes the register alone does not give:

- **RFC 1112 is not marked obsoleted.** Its own record shows `obsoleted_by: []` and
  `updated_by: [RFC2236]` only. IGMPv1 was superseded in practice, not withdrawn on paper;
  RFC 2236 restates and extends it rather than replacing a text the register still treats as
  live. This corrects a lead in the survey family, which called RFC 1112 "obsoleted by
  RFC 2236... from knowledge, not checked."
- **RFC 2236 is not obsoleted either**, by RFC 3376 or by RFC 9776. Both later documents
  `update` it, and RFC 9776 keeps citing it by number for the version 2 compatibility path
  (`rfc9776.txt:445-451` and the compatibility sections). RFC 2236 is a live document a router
  running IGMPv3 still has to read.
- **RFC 9776 is a 2025 errata revision of RFC 3376, not a new design.** Its own
  Appendix C (`rfc9776.txt:2523-2545`) lists six changes: two are formula corrections to timer
  definitions (Group Membership Interval, Older Version Querier Present Interval — both in the
  override table below), one is a Router Filter Mode clarification, one is a Querier-election
  clarification, and two are metadata-only fixes to the Obsoletes/Updates relationship with
  RFC 2236. It is "backward compatible with [RFC 3376]" by the abstract's own words, and it
  absorbs RFC 4604's SSM-aware behavior into its own text (`rfc9776.txt:205-209,834-837,
  1497-1501`), citing RFC 4604 by name at each point rather than silently rewriting it.
- **RFC 4604 keeps citing the obsoleted RFC 3376 and RFC 3810 in its own header, not their
  successors.** The register shows no `updated_by` for RFC 4604 and no `updates` relation from
  RFC 4604 to RFC 9776 or RFC 9777. RFC 9776 resolves this by absorption (the previous note);
  a reader who follows the register alone, without reading RFC 9776's text, would not learn
  that RFC 4604 is now folded in.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Group Membership Interval, the timeout after which a router decides a group has no more members | RFC 2236 §8.4, `rfc2236.txt:979-984`: "MUST be ((the Robustness Variable) times (the Query Interval)) plus (**one** Query Response Interval)" — RFC 3376 §8.4 states the identical formula (not downloaded; same clause number) | RFC 9776 §8.4, `rfc9776.txt:2045-2052`: "MUST be ([Robustness Variable] times [Query Interval]) plus (**2 *** [Query Response Interval])" | RFC 9776 | **yes**, level 4 |
| Older Version Querier Present Interval, the timeout for a v3 host to fall back to v3 after an older Query | RFC 2236/RFC 3376 §8.12 (not downloaded in the RFC 2236 text under this number; RFC 3376 §8.12 states "MUST be ((the Robustness Variable) times (the Query Interval in the last Query received)) plus (one Query Response Interval)", by clause) | RFC 9776 §8.12, `rfc9776.txt:2110-2122`: downgraded to "SHOULD be [Robustness Variable] times [Query Interval] plus (10 times the Max Response Time in the last received Query Message)" — a different formula and a lower keyword | RFC 9776 | **yes**, level 4 |
| Unrecognized message types | RFC 2236, `rfc2236.txt:123`: "Unrecognized message types **should** be silently ignored" — lower case, a description rather than the formal keyword, even though the document adopts RFC 2119 (`rfc2236.txt:44-46`) | RFC 9776, `rfc9776.txt:482`: "Unrecognized message types **MUST** be silently ignored" | RFC 9776 | **yes**, level 3 |
| The Router Filter Mode / forwarding-suggestion table | RFC 3376 §6.3 (not downloaded; same table shape by clause) | RFC 9776 §6.3, `rfc9776.txt:1447-1493` (Table 7); Appendix C names this a clarification (Erratum 5562), not a new rule | RFC 9776 | yes, background — no numeric or keyword change found between the two tables |
| SSM-aware router and host behavior | RFC 3376 has no SSM-aware concept | RFC 4604, folded into RFC 9776 §6.4 and §8.1.2, `rfc9776.txt:834-837,1497-1501`: SSM-aware routers and hosts SHOULD ignore EXCLUDE-mode records and older-version messages in the SSM address range | RFC 9776, with RFC 4604 as the document that introduced the rule | no; level 5 |
| Router Alert on every message | RFC 2236, `rfc2236.txt:1272`: "The IGMPv2 spec requires the presence of the IP Router Alert option" | RFC 9776, `rfc9776.txt:447-450`: "Every IGMP message described in this document is sent with... an IP Router Alert option [RFC2113] in its IP header" | RFC 9776; unchanged in substance since RFC 2236/RFC 1112 | yes, level 2 — a base-document requirement on every message, not a version conflict; see the facts in [`conformance.md`](../../model/igmp/conformance.md) |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 9776 | March 2025, Internet Standard; §4 to §8 | none yet; level 2 writes `standard/rfc9776/catalog.md` |
| RFC 2236 | November 1997, Proposed Standard; §6 to §7, kept alive by RFC 9776's own compatibility sections | none yet; level 2 writes `standard/rfc2236/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1112 | Level 5. The compatibility text of RFC 2236 and RFC 9776 restates the version 1 behavior, so the current pair covers it by reference; a version 1 host or router of its own is level 5. |
| RFC 3376 | Obsoleted by RFC 9776. |
| RFC 4604 | Level 5. SSM-aware behavior; RFC 9776 already carries its text by reference. |
| RFC 3488 (RGMP) | Not a relative of IGMP in the register. RGMP reuses the IGMP wire format for its own Hello message, but it is a separate protocol: a shared format, not a shared protocol family. |
