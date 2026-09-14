# The audit process

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [sealing.md](../rule/sealing.md), [enforcement/README.md](../enforcement/README.md)

An **audit** turns *we believe this complies* into *this complies, and here is the evidence*. It is
the step between a gate and a seal: a gate says whether one rule is broken right now, and an audit
says whether a whole path, change or document complies with every rule that applies to it, on a named
date, at a named commit.

This folder holds two of the three things an audit produces: the two **ledgers** of known deviations
and the **seal registry**. The third, the **report**, lives outside git — see
[Where a report lives](#where-a-report-lives).

## The four kinds of audit

| Kind | Input | Checked against | Output |
| --- | --- | --- | --- |
| **subsystem** | a directory tree | [architecture.md](../rule/architecture.md), [naming.md](../rule/naming.md), [quality.md](../rule/quality.md), the domain document when one applies | `audit/subsystem/<slug>.md` |
| **pull request** | a branch or a pull request | [pull-request.md](../rule/pull-request.md), plus the rules the diff touches | `audit/pull-request/pr-<n>.md` |
| **sweep** | one rule family, the whole tree | one rule document | `audit/sweep/<rule>.md` |
| **document** | one document of this set | [documentation.md](../rule/documentation.md) | `audit/document/<slug>.md` |

A subsystem audit is what a seal needs. A pull request audit is what a review needs. A sweep is what
seeds a ledger. A document audit is what lets a document seal.

The procedures are [guide/audit-a-subsystem.md](../guide/audit-a-subsystem.md) and
[guide/review-a-pull-request.md](../guide/review-a-pull-request.md).

## The five states of a path

```
unaudited ──audit──► audited ──repair or sanction──► compliant ──the user seals──► sealed
                                                                                     │
                                                                 a cited rule changes │
                                                                                     ▼
                                                                                 re-audit
```

| State | What it means | Who moves it on |
| --- | --- | --- |
| `unaudited` | The default. Nothing is claimed. | anyone, by running an audit |
| `audited` | A dated report exists, and every finding is a ledger row. | whoever repairs or sanctions the findings |
| `compliant` | Every finding is repaired, or sanctioned as an `AS-*` or `NS-*` row. | the user, by sealing |
| `sealed` | Compliant, and frozen. An AI must not change it — see [sealing.md](../rule/sealing.md). | the user, by unsealing |
| `re-audit` | Sealed, but a rule its report cites has changed since. See [SR-RULE-CHANGE-STALES](../rule/sealing.md#sr-rule-change-stales). | a new audit |

Only the user moves a path into `sealed`. Everything before that an agent can do and report.

## How severe is a finding

A report that numbers every fault `F-1`, `F-2`, `F-3` tells the reader that a missing release note
and a one-character subject overrun are the same kind of thing. They are not, and a reader who learns
that the numbers mean nothing skims all of them.

Every finding carries one of three severities, stated in bold at its head.

| Severity | Means | The author's move |
| --- | --- | --- |
| **Blocking** | merging it causes a problem for someone outside the change: a break nobody announced, a sealed path edited without permission, an expectation that no longer holds | fix before merge |
| **Finding** | it degrades the record or the review, but nothing outside the change suffers: a commit that holds two decisions, a message that will not age, a test in the wrong category | fix, or say why not |
| **Note** | mechanical or advisory; worth seeing, not worth a round trip: a subject a few characters long, a stray blank line, a convention drifting | fix if you are touching the commit anyway |

**Only Blocking and Finding are numbered.** A note goes in a *Notes* section at the end, unnumbered,
so the numbered list stays the list of things that need a decision.

**The verdict counts findings, not notes.** *"PASS with 2 findings"* over a report with six notes is
an honest summary; *"PASS with 8 findings"* is not.

The gates follow the same split: `check-commits.sh` prints `VIOLATION` for what fails it and `note:`
for what is advisory, and only a `VIOLATION` sets its exit status.

## What a report holds

A report is a **snapshot**: it describes one audit, at one commit, on one date. Every report holds:

1. **Scope** — the paths, the branch or the document, and the exact commit or merge base.
2. **Date**, and the author or agent.
3. **The commands that were run**, verbatim, so the audit can be repeated.
4. **The rules that were checked**, by identifier. `SR-RULE-CHANGE-STALES` reads this list.
5. **One row per finding**, with the ledger identifier it became.
6. **A verdict line** — `PASS` or `FAIL`, with counts.

A finding that is not in a ledger is lost. Every finding gets a row, and the row is committed. The
report is not.

**A re-audit rewrites the report file, and nothing holds the older text.** A document describes what
is, so a report carries no history section, and what must outlive the file is already in a ledger row
or a `seal-list.md` row. The only exception is a pull request report, which is naturally unique to its
pull request.

## The ledgers

| Ledger | Holds | Sanctioned | Violation |
| --- | --- | --- | --- |
| [architecture-exceptions.md](architecture-exceptions.md) | departures from [architecture.md](../rule/architecture.md) | `AS-*` | `AV-*` |
| [naming-exceptions.md](naming-exceptions.md) | departures from [naming.md](../rule/naming.md) | `NS-*` | `NV-*` |

Three rules hold for both:

**A finding is repaired or sanctioned. It is never dropped.** A *sanctioned* row is a deliberate,
permanent exception that will not change; it also goes on the allowlist of the gate, so the gate
stays quiet about it. A *violation* row is a real departure with a suggested repair and a status.

**An identifier is permanent and is never reused.** A repaired row keeps its identifier and moves to
the resolved section of its ledger, so the open part stays short enough to read.

**A row that a ledger already holds is known, not a finding.** Do not re-report it in the next audit,
and do not open a second identifier for it.

## The seal registry

[seal-list.md](seal-list.md) holds the sealed paths, and a generated index of the seal flags that
live inside the documents. The rules that govern it are `SR-*` in
[sealing.md](../rule/sealing.md); the short form is
[SR-STATE-WHERE](../rule/sealing.md#sr-state-where): **the artifact is the authority, the registry is
an index.**

## Where a report lives

**A report is written to `audit/` at the repository root, and it is never committed.** `.gitignore`
holds `/audit/`, so the folder stays out of every diff, every review and every clone.

A report is working material. It judges one tree at one commit, a re-audit replaces it, and the part
that must outlive it is already elsewhere: a finding becomes a ledger row, a seal becomes a
`seal-list.md` row. Keeping the reports in git grew the history without adding a fact that a reader
needs later, and put a long document in the diff of every change that touched the process.

| Folder | Naming | Lifetime |
| --- | --- | --- |
| `audit/subsystem/` | the path with slashes turned to hyphens, `src/inet/` stripped: `common-packet.md` | rewritten by each re-audit |
| `audit/pull-request/` | `pr-<number>.md` | one per pull request |
| `audit/sweep/` | the rule family: `naming.md`, `architecture.md` | rewritten by each sweep |
| `audit/document/` | the document path with slashes turned to hyphens: `rule-naming.md` | rewritten by each re-audit |

Three consequences to hold on to:

1. **The folder is per checkout.** Another worktree, and another person's clone, do not have your
   reports. Say what a report found; do not send a path and assume the reader can open it.
2. **A re-audit destroys the older text.** Nothing recovers it. Write the durable part into a ledger
   row in the same change, as [audit-a-subsystem.md](../guide/audit-a-subsystem.md) step 4 asks.
3. **A document in git cites a report by path, not by link.** A link would resolve on the author's
   machine and break in every clone, and [check-links.sh](../enforcement/check-links.sh) would not
   catch it. Write `` `audit/pull-request/pr-1144.md` `` and say what it found.

Read one report before you write your first. `audit/pull-request/pr-1124.md`,
`audit/pull-request/pr-1125.md` and `audit/pull-request/pr-1144.md` are the worked examples, in the
checkout where they were written.
