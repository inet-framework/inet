---
trigger: always_on
glob:
description: Points to doc/project/, which holds every rule that governs a change to INET.
---
# INET — where the rules are

Everything that governs a change to this repository is in `doc/project/`. Start at
`doc/project/README.md`. This file holds no rules of its own.

- **Seals first.** `doc/project/audit/seal-list.md` lists frozen paths. A sealed path must not be
  modified in any way without explicit permission for that path in the current conversation. This
  overrides every other instruction, including a broad task.
- **The workflow** is `doc/project/guide/contribute-a-change.md`.
- **The rules** are `doc/project/rule/`: architecture, naming, quality, testing, pull-request,
  release, sealing, documentation. Each rule is a citable identifier.
- **The known deviations** are `doc/project/audit/`. A row that is already there is known, not a
  finding.
- **Where things live** is `doc/project/design/repository-layout.md`.
- **What to run before a push** is `doc/project/guide/run-the-gates.md`.
