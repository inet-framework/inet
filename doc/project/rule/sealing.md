# Sealing rules

> **Kind:** rule · **Status:** current · **Seal:** whole · **Owns:** `SR-*` · **Stands on:** [architecture.md](architecture.md), [naming.md](naming.md), [documentation.md](documentation.md)

A **seal** says: *this was audited, it complied, and it is now frozen against AI change.* Sealing is
the terminal state of the enforcement pipeline, not a shortcut around it. A change is checked
mechanically by the gates in [enforcement/](../enforcement/README.md), semantically by the
[agent-review checklist](../enforcement/checklist/general.md), and its known deviations are tracked
in the two ledgers. Sealing records the result: *this has passed the gate and is now frozen, so later
broad tasks cannot silently churn it.*

This document is the **policy**. What is sealed right now lives in three places, and §*SR-STATE-WHERE*
says which.

## The rule that overrides every other instruction

**A sealed path and a closed unit MUST NOT be modified by an AI in any way — no edits, no
reformatting, no "while I'm here" cleanups, no incidental change as part of a larger task — unless
the user gives explicit permission for that specific path or unit in the current conversation.**

This overrides every other instruction, including a broad task that would otherwise touch it. If a
change you are asked to make needs a sealed path, STOP, say that the path is sealed, and ask for
permission before you proceed.

## Index

Every sealing rule in document order.

**What a seal covers**

| Rule | Statement |
| --- | --- |
| [SR-DEFAULT-OPEN](#sr-default-open) | Everything is unsealed by default |
| [SR-SCOPE](#sr-scope) | A seal covers a path, a whole document, or one unit inside a document |
| [SR-STATE-WHERE](#sr-state-where) | The artifact is the authority; the registry is an index |
| [SR-FOLDER-RECURSIVE](#sr-folder-recursive) | A folder seal is recursive and covers files added later |
| [SR-SEAL-GENERATED](#sr-seal-generated) | A generated file follows the seal of its source |
| [SR-COVERS-TEXT](#sr-covers-text) | The seal covers the text and the identifier, not the position |

**The flags inside a document**

| Rule | Statement |
| --- | --- |
| [SR-FLAG-VALUES](#sr-flag-values) | Two flags, three readings |
| [SR-FLAG-PLACEMENT](#sr-flag-placement) | A flag never goes in a heading |
| [SR-FLAG-COVERAGE](#sr-flag-coverage) | A document declares its seal unit, and `complete` means every unit carries a flag |
| [SR-PROMOTE](#sr-promote) | A document whose every unit is closed is promoted to a whole seal |

**Getting to a seal, and back**

| Rule | Statement |
| --- | --- |
| [SR-AUDIT-FIRST](#sr-audit-first) | Nothing seals before it is audited and complies |
| [SR-CITE-THE-AUDIT](#sr-cite-the-audit) | Every seal cites the audit report that earned it |
| [SR-RECORD-IN-COMMIT](#sr-record-in-commit) | A seal is recorded in the same commit that records the compliant state |
| [SR-RULE-CHANGE-STALES](#sr-rule-change-stales) | A rule change marks every seal that cites it for re-audit |
| [SR-VIOLATION-IN-SEALED](#sr-violation-in-sealed) | A violation found in a sealed path is reported, not repaired |
| [SR-UNSEAL](#sr-unseal) | Unsealing is a deliberate act, with a reason in the commit |

## What a seal covers

### SR-DEFAULT-OPEN

**Everything is unsealed by default.**

Nothing is sealed unless it matches an entry in [audit/seal-list.md](../audit/seal-list.md) or
carries a `🔒` flag. Absence *is* the unsealed state; there are no rows to maintain for the things that
are not sealed. The sealed list is an allowlist of the small, settled part of the tree that has been
driven all the way to a reviewed, compliant, frozen state, so that scarce review attention moves
forward instead of re-touching settled ground.

*Enforced at T4 — agent review, and the loud rule at the head of this document.*

### SR-SCOPE

**A seal covers one of three things: a path, a whole document, or one unit inside a document.**

| Scope | Examples |
| --- | --- |
| a path | `common/packet/` (a folder), `common/INETDefs.h` (a file). Paths are relative to `src/inet/`. |
| a whole document | `rule/sealing.md`, `design/decisions.md` |
| a unit in a document | one `R-*` requirement, one section of an anatomy, one `NR-*` rule, one sanctioned ledger row |

The third scope is what lets a document settle one requirement at a time instead of waiting until
every word of it is final.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh).*

### SR-STATE-WHERE

**The artifact is the authority. The registry is an index.**

Each scope keeps its state where a reader already looks:

| Scope | Where the state lives |
| --- | --- |
| a path | [audit/seal-list.md](../audit/seal-list.md), because a `.cc` file cannot carry a flag |
| a whole document | the `Seal` field of its header |
| a unit in a document | an inline flag on the unit |

`seal-list.md` therefore owns the paths, and holds a **generated** index of the document flags with
their counts. Nobody keeps two lists by hand, and the index cannot drift, because
`check-seals.sh --write` rewrites it from the tree.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) compares the index against the tree.*

### SR-FOLDER-RECURSIVE

**A folder entry seals every file under it, at any depth, including files added later.**

A folder entry is written with a trailing `/`. A file is sealed if, and only if, the list holds that
exact file *or* holds any ancestor directory of it as a folder entry.

Two consequences are worth spelling out. **Adding a new file under a sealed folder is itself a
modification of the sealed subtree**, and needs permission like any edit. And sealing a subsystem as
a folder is the way to freeze it as a unit and keep new files in it frozen by default — which is the
point.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) resolves ancestors.*

### SR-SEAL-GENERATED

**A generated file follows the seal of the source it is generated from.**

A folder seal covers the `*_m.h` and `*_m.cc` pair because they sit beside their `.msg`. A seal on a
lone `.msg` covers its generated pair by intent. Never hand-edit a generated pair in any case, sealed
or not — see [NR-GEN](naming.md#nr-gen).

*Enforced at T4 — agent review.*

### SR-COVERS-TEXT

**The seal covers the text of the unit and its identifier. It does not cover its position.**

A regroup that moves a closed unit without changing one word of it is allowed, and it lands as a
mechanical commit under [PR-SPLIT-MECHANICAL](pull-request.md#pr-split-mechanical). This is what
makes stable identifiers worth having: a document can be reordered while every closed unit and every
citation to it survives.

*Enforced at T4 — agent review: did the words change, or only the order?*

## The flags inside a document

### SR-FLAG-VALUES

**Two flags, three readings.**

| Flag | Reading |
| --- | --- |
| no flag | **Open, not yet considered.** The default. Nothing is claimed. |
| `⬜` | **Open on purpose.** The unit was looked at and left open: still under discussion, or waiting for a decision. |
| `🔒` | **Closed.** The unit was audited, it complies, and the user froze it. |

The default keeps the cost at zero, so a document carries flags only on the units that need them.
`⬜` is not maintenance work; it is a statement. Use it to mark the frontier — the units being worked
on now — so a reader can tell *we left this open* from *we never got here*.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) rejects any other glyph.*

### SR-FLAG-PLACEMENT

**A flag never goes in a heading.**

A heading is the citation anchor. `[NR-NED-GATE](naming.md#nr-ned-gate)` must not break because a
rule closed. The flag goes on the body instead:

| Unit | Placement |
| --- | --- |
| a section or a rule with a heading | the flag opens the first line of the body under the heading |
| a bullet item | the flag opens the item |
| a table row | a first column named `Seal` |
| the whole document | the `Seal` field of the header, as `whole; closed` |

```markdown
### NR-NED-GATE

`🔒` **A gate is `<stem>In` or `<stem>Out`, and the stem names the peer it faces.**
```

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh).*

### SR-FLAG-COVERAGE

**A document declares the unit it seals by, in the `Seal` field of its header.**

| Value | The unit that carries a flag |
| --- | --- |
| `none` | the document carries no flags |
| `whole` | the document, as one unit |
| `by section` | each `##` or `###` section |
| `by requirement`, `by rule`, `by decision` | each item that owns an identifier |
| `by row` | each row of a table |

A `by <unit>` value may add `, complete`. Then **every** unit must carry a flag, and the gate fails
when one does not. Use `complete` where the frontier itself is information — the requirements are the
case — and leave it off everywhere else.

A flag in a document that declares `none`, or on a unit kind the header did not declare, is an error.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh).*

### SR-PROMOTE

**When every unit of a document is closed, promote the document to a whole seal.**

The header becomes `**Seal:** whole; closed` and the inline flags come out, in the same commit. A
document does not carry both forms: the promotion is what keeps a settled document quiet instead of
speckled with a flag on every heading.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) reports a `by <unit>` document whose units are all closed.*

## Getting to a seal, and back

### SR-AUDIT-FIRST

**Nothing seals before it has been audited and complies.**

Do not offer to seal, and never say "seal as-is", until the audit has been reported. **Audit the
moment you introduce a path as the next thing to seal** — before you invite review and before you
offer to seal. For a folder, the audit covers the whole subtree.

The audit checks against [architecture.md](architecture.md), [naming.md](naming.md),
[quality.md](quality.md), the domain document when one applies, and the gates that apply. Present the
result first. A path seals once it complies, **or** once a specific non-compliance is explicitly
accepted by the user — recorded as an `AS-*` row in
[audit/architecture-exceptions.md](../audit/architecture-exceptions.md) or an `NS-*` row in
[audit/naming-exceptions.md](../audit/naming-exceptions.md). **Do not seal over an unsanctioned
violation** (`AV-*` or `NV-*`): repair it, or get it sanctioned first.

[guide/audit-a-subsystem.md](../guide/audit-a-subsystem.md) is the procedure.

*Enforced at T5 — the user seals, and only after the report.*

### SR-CITE-THE-AUDIT

**Every seal names the audit report that earned it.**

A path row carries a link to its report under [audit/report/](../audit/README.md); a closed unit
carries one in the same row or in the seal index. The report names the rules it checked and the
commit it examined. Without that citation a seal is a claim with no evidence, and nobody can tell
whether it is still true.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) rejects a seal row with no report.*

### SR-RECORD-IN-COMMIT

**Record a seal in the same commit that records the compliant state.**

Add the path to [audit/seal-list.md](../audit/seal-list.md), with a trailing `/` for a recursive
folder and no trailing slash for a single file. Keep the list grouped and ordered as that document
describes, and do not silently drop an entry.

*Enforced at T4 — agent review of the diff.*

### SR-RULE-CHANGE-STALES

**When a rule changes in a way that changes what compliance means, every seal whose report cites that
rule is marked for re-audit.**

A seal is a statement about a moment: *this complied, against these rules, on this date*. A rule that
moves afterwards can make the statement false without anyone touching the code. The reports name the
rules they checked, so the affected rows can be found and marked `re-audit`.

The judgment of *material* is the author's: a reworded sentence that means the same thing stales
nothing; a rule that gains a clause, changes a tier, or narrows an exception stales every seal that
cites it. When in doubt, mark it — a re-audit is cheap next to a seal that is quietly wrong.

Without this rule the seal list slowly becomes a list of old opinions, which is worse than no list,
because it is trusted.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) can find the rows; deciding *material* is T5.*

### SR-VIOLATION-IN-SEALED

**A violation found in a sealed path is reported, not repaired.**

The seal is a claim that the path complied *when it was sealed*. It is not a guarantee that the claim
is still perfect. When you find a violation in a sealed path:

1. **Report it** — the file, the location, and the rule it breaks.
2. **Ask permission before you repair it.** The seal holds until the user grants permission for that
   path. Do not touch it meanwhile, and do not fold the repair into an unrelated task.
3. Once permission is given and the repair lands, the path stays sealed: it was re-audited and
   re-complied in that same commit. If the user instead accepts the deviation, record it as an `AS-*`
   or `NS-*` row and leave the seal.

*Enforced at T4 — agent review.*

### SR-UNSEAL

**Removing an entry unseals it, and that is a deliberate act with a reason in the commit.**

Unsealing is not a shortcut around the rule at the head of this document. Ask for it explicitly, and
record why. To edit one file inside a recursively sealed folder without unsealing the whole subtree,
get permission for that one file instead of deleting the folder entry.

*Enforced at T5 — the user unseals.*

## What sealing does not replace

Sealing composes with the gates and the ledgers; it does not stand in for either. The gates run on
every change and catch a regression. The ledgers track the known deviations. Sealing marks the parts
of the tree that have been driven all the way to a reviewed, compliant, frozen state.
