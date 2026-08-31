# Audit a subsystem

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [audit/README.md](../audit/README.md), [rule/sealing.md](../rule/sealing.md)

How to audit a directory tree, write the report, and take it to a seal. The states a path passes
through are in [audit/README.md](../audit/README.md); the rules are `SR-*` in
[rule/sealing.md](../rule/sealing.md).

**Audit the moment a path is named as the next thing to seal** — before you invite review, and before
you offer to seal ([SR-AUDIT-FIRST](../rule/sealing.md#sr-audit-first)).

## 1. Fix the scope and the commit

Name the exact directory, whether it is recursive, and the commit you are auditing. A folder audit
covers the whole subtree: every file under it must comply, or carry a recorded exception, before the
folder can seal as a unit.

```bash
git rev-parse HEAD          # the commit the report will name
find src/inet/<path> -type f | wc -l
```

## 2. Run the mechanical gates

```bash
doc/project/enforcement/check-architecture.sh src/inet/<path>
doc/project/enforcement/check-cpp.sh src/inet/<path>
doc/project/enforcement/check-naming.sh src/inet/<path>
```

Record the exact commands and their output. The report must let someone repeat the audit.

## 3. Read for what a gate cannot see

The gates cover dependency direction and C++ identifiers. Read the subtree against the rest yourself,
or with the [agent-review checklist](../enforcement/checklist/general.md):

- [rule/architecture.md](../rule/architecture.md) — the semantic rules: composition, contracts,
  observation neutrality, extension without core edits.
- [rule/naming.md](../rule/naming.md) — the NED, `.msg` and semantic names a linter cannot see.
- [rule/quality.md](../rule/quality.md) — comments, error idioms, state ownership.
- the domain document, if the path is inside its scope.

## 4. Sort every finding

Each finding is one of three things, and the sorting is the judgment:

| The finding is | Do |
| --- | --- |
| a real departure to repair | an `AV-*` or `NV-*` row, with a suggested repair and `Status = Open` |
| a deliberate, permanent exception | an `AS-*` or `NS-*` row with the reason, **and** the allowlist entry in the gate |
| already in a ledger | nothing — it is known, not a finding |

**A finding that is not in a ledger is lost.** Add the rows in the same change as the report.

## 5. Write the report

`audit/report/subsystem/<slug>.md`, where the slug is the path with slashes turned to hyphens and
`src/inet/` stripped — `common/packet` becomes `common-packet.md`. It holds the scope, the date, the
commit, the commands, the rules checked, one row per finding with its ledger identifier, and a verdict
line. A re-audit rewrites the file; git holds the older text.

[common-packet.md](../audit/report/subsystem/common-packet.md) is the worked example.

## 6. Report before you offer

**Present the audit result first.** Do not offer to seal, and never say "seal as-is", before the
report exists. A path seals once it complies, or once a specific non-compliance is explicitly
accepted by the user and recorded as an `AS-*` or `NS-*` row.

**Do not seal over an unsanctioned violation.** An open `AV-*` or `NV-*` under the path means the
path is not ready. Repair it, or get it sanctioned.

## 7. The user seals

Only the user moves a path into `sealed`. When they do, add the row to
[audit/seal-list.md](../audit/seal-list.md) — trailing `/` for a recursive folder, bare path for one
file — **in the same commit that records the compliant state**
([SR-RECORD-IN-COMMIT](../rule/sealing.md#sr-record-in-commit)). The row cites the report
([SR-CITE-THE-AUDIT](../rule/sealing.md#sr-cite-the-audit)).

Then run the gate:

```bash
doc/project/enforcement/check-seals.sh
```
