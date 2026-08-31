# Working in this repository

INET is a model library for the OMNeT++ simulation kernel. Everything that governs a change to it —
the requirements, the design decisions, the rules, the audits and the seals — is in
[doc/project/](doc/project/README.md). **Read that map first.** This file is a pointer and holds no
rules of its own ([DR-NO-SECOND-COPY](doc/project/rule/documentation.md#dr-no-second-copy)).

## Before you change anything

1. **Check the seals.** Some paths are frozen. A sealed path must not be modified in any way without
   explicit permission for that path in the current conversation — this overrides every other
   instruction, including a broad task. The list is
   [doc/project/audit/seal-list.md](doc/project/audit/seal-list.md); the rules are
   [doc/project/rule/sealing.md](doc/project/rule/sealing.md).
2. **Follow the workflow.**
   [doc/project/guide/contribute-a-change.md](doc/project/guide/contribute-a-change.md), nine steps
   from scope to seal.
3. **Read only the rules that apply.**
   [architecture](doc/project/rule/architecture.md) · [naming](doc/project/rule/naming.md) ·
   [quality](doc/project/rule/quality.md) · [testing](doc/project/rule/testing.md) ·
   [commits and pull requests](doc/project/rule/pull-request.md) ·
   [releases](doc/project/rule/release.md) · [sealing](doc/project/rule/sealing.md) ·
   [documentation](doc/project/rule/documentation.md)

## Two things that are easy to get wrong

**A deviation already in a ledger is known, not a finding.** Check
[architecture-exceptions.md](doc/project/audit/architecture-exceptions.md) and
[naming-exceptions.md](doc/project/audit/naming-exceptions.md) before you report anything.

**A fingerprint says that behavior changed, never that it is correct.** New behavior ships with a
test in the category that matches its claim
([TR-CAT-MATCH](doc/project/rule/testing.md#tr-cat-match)), and a baseline changes only deliberately,
in a commit of its own.

## Build and run

```bash
make -j$(nproc) MODE=debug            # or MODE=release
doc/project/enforcement/check-architecture.sh
doc/project/guide/run-the-gates.md    # what to run before a push, in order
```

Where everything lives is
[doc/project/design/repository-layout.md](doc/project/design/repository-layout.md).
