# History

> **Kind:** history · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** every document in this folder set

How the current state came to be. Every other document in `doc/project/` describes what **is**; this
one says how and why we got there ([DR-WHAT-IS](../rule/documentation.md#dr-what-is)). A reader who
wants the current state is not made to read the history, and a reader who wants the history finds it
in one place.

A step is a plan in `plan/done/`, dated by the day it landed.

## The document set

The set began as `doc/architecture/`: nine documents that grew from an architecture note into
requirements, naming rules, commit rules, seal rules, two exception ledgers, three checklists and two
kinds of audit report. By the end only two of its documents were about architecture, and one document
(`architectural-requirements.md`) held four kinds of text that changed for four different reasons.

- 2026-08-31 — [The project documentation structure](../../../plan/done/project-documentation-structure.md).
  `doc/architecture/` becomes `doc/project/`, arranged as a chain from requirement to design to rule
  to code, with `enforcement/`, `audit/`, `evidence/`, `guide/` and `history/` beside it. Every
  document gains a header that states its kind, its seal unit and what it stands on. Every rule
  family gains a prefix, and the four that had no document — quality, testing, release,
  documentation — get one.

Three facts that the implementation found, and that the plan did not predict:

**An identifier heading could not be cited.** `### AR-ORG-DOMAINS — Layered, domain-partitioned
source tree…` anchors as `#ar-org-domains--layered-domain-partitioned-source-tree-with-acyclic-dependencies`,
which nobody writes by hand and which breaks whenever the statement is reworded. Every identifier
heading now holds the identifier alone, and an Index table restores the overview
([DR-ID-HEADING](../rule/documentation.md#dr-id-heading)). 101 headings converted.

**The requirements could not be linked to at all.** They were bullet items, so no citation could
reach one, and no requirement could carry a seal flag. They are now headings.

**The clang-tidy configuration had never run.** The copy under `doc/architecture/enforcement/` said
in its own header that it had to be moved to the repository root to take effect, and it never was;
`_scripts/clang-tidy/run.sh` held a second, different check list behind a hand-run `--fix`. They are
now one file at the root.

## The rules

The rules predate this folder. They were written into `architectural-requirements.md`,
`naming-conventions.md` and `pull-requests.md` between 2026-07 and 2026-08, and the ledgers were
seeded by a repository-wide scan on 2026-07-20 that produced `NS-01`…`NS-05`, `NV-01`…`NV-17`,
`AS-01` and `AV-ORG-01`…`AV-VIS-01`.

## The seals

`common/packet/` was sealed on 2026-07-20, as the first and so far only sealed path. The audit that
earned it is [common-packet.md](../audit/report/subsystem/common-packet.md).

The 2026-08-31 restructure found that this seal does not satisfy
[SR-AUDIT-FIRST](../rule/sealing.md#sr-audit-first): it was recorded over two `AV-ORG` clusters that
are still `Open (decide)` rather than sanctioned. The finding is recorded in
[audit/seal-list.md](../audit/seal-list.md) rather than quietly repaired, because the registry is
where a reader checks.

## Adding a step

One section per subject, in the order of the chain that [README.md](../README.md) draws. Each entry
is a date, a link to the plan in `plan/done/`, and one paragraph on what changed and why. Facts the
implementation discovered belong here too — they are the most useful part, because they are what a
later reader cannot reconstruct from the documents themselves.
