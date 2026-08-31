# Enforcement

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [architecture.md](../rule/architecture.md), [naming.md](../rule/naming.md), [pull-request.md](../rule/pull-request.md), [sealing.md](../rule/sealing.md)

The machinery that checks the rules. This document holds two things: the **tier ladder**, which every
rule document cites, and the **gate inventory**, which lists the checks that really exist.

A rule that nothing checks is a hope. The goal of
[AR-QUAL-ENFORCED](../rule/architecture.md#ar-qual-enforced) is to move every rule as far up the
ladder as it can go.

## The tier ladder

Each rule is enforced at the strongest tier available to it. A rule states its own tier, on the last
line of its section. This document does not repeat those tiers, because two copies drift.

| Tier | What happens |
| --- | --- |
| **T1** | **Will not build.** The compiler or the NED toolchain rejects it. |
| **T2** | **Fails a CI test.** A fingerprint, unit, module, statistical or validation test breaks. |
| **T3** | **A deterministic check flags it.** A linter, `featuretool`, a build matrix, or a script in this folder reports it in CI. |
| **T4** | **An agent reviewer flags it.** An LLM reviewer, run as a CI gate, judges the diff against the rule. This tier catches the *semantic* violations that no static rule can express: visualization logic in a protocol, a zero-time message used as a procedure call, prose that duplicates a NED declaration. It is what makes the judgment rules enforceable instead of merely hoped for, and it scales in a way human review does not. |
| **T5** | **A human decides.** Genuine design judgment and final sign-off. Is a fidelity level worth adding? Does this read as one system? |

T4 needs its own discipline, and [checklist/general.md](checklist/general.md) states it: **precision
over recall**. The reviewer flags only a clear violation, asks a question when it is unsure, and
never re-flags a deviation that a ledger already records. A noisy gate gets ignored, and an ignored
gate enforces nothing.

## The gate inventory

One row per check that exists. The table lists **machinery, not intent**, so it cannot drift from the
rules: a row disappears when its check is deleted, and coverage is read from the side that can be
measured.

| Gate | Tier | Runs | Covers |
| --- | --- | --- | --- |
| the C++ compiler and the NED toolchain | T1 | every build | the contract, composition, chunk, tag, signal, socket, queueing and lifecycle APIs |
| [check-architecture.sh](check-architecture.sh) | T3 | by hand; not yet in CI | `AR-ORG-DOMAINS`, `AR-ORG-VIS-SPLIT` over the `#include` graph |
| [check-cpp.sh](check-cpp.sh) | T3 | by hand; not yet in CI | the C++ half of `NR-*` and the bug and modernization rules of `QR-*`, through the root [`.clang-tidy`](../../../.clang-tidy) |
| [check-naming.sh](check-naming.sh) | T3 | by hand; not yet in CI | the mechanical half of `NR-*`: file names, package names, generated pairs, icon names |
| [check-commits.sh](check-commits.sh) | T3 | by hand; not yet in CI | `PR-SPLIT-WHITESPACE`, `PR-SPLIT-MOVE`, `PR-SPLIT-BASELINE`, `PR-SERIES-ORDER`, `PR-SERIES-LINEAR`, `PR-MSG-SUBJECT`, `PR-MSG-FACTS` |
| [check-seals.sh](check-seals.sh) | T3 | by hand; not yet in CI | `SR-FLAG-PLACEMENT`, `SR-FLAG-COVERAGE`, and the generated index of [seal-list.md](../audit/seal-list.md) |
| [checklist/general.md](checklist/general.md) | T4 | by an agent, on every change | the semantic architecture rules |
| [checklist/ieee80211.md](checklist/ieee80211.md) | T4 | by an agent, on an 802.11 diff | `AR-WLAN-*` |
| the test suites | T2 | GitHub Actions, twelve workflows | `TR-*`, `AR-QUAL-FINGERPRINT`, `AR-QUAL-DETERMINISM` |
| `inet_featuretool` | T3 | the feature workflow | `AR-EXT-FEATURES` |

**Nothing in this folder runs in CI yet.** That is the honest state, and it is the first thing to
repair. `.github/workflows/` holds twelve test workflows and no rule gate. Every `T3` row above is a
script a person must remember to run, which is the weakest form of every rule it covers.

## How to run them

```bash
# from the repository root
doc/project/enforcement/check-architecture.sh              # the whole tree
doc/project/enforcement/check-architecture.sh src/inet/common/packet   # one subtree
doc/project/enforcement/check-cpp.sh src/inet/linklayer/ethernet
doc/project/enforcement/check-naming.sh
doc/project/enforcement/check-commits.sh origin/master..HEAD
doc/project/enforcement/check-seals.sh                     # check
doc/project/enforcement/check-seals.sh --write             # check and rewrite the index
```

[guide/run-the-gates.md](../guide/run-the-gates.md) says which of them to run before a push, and in
which order.

## What belongs in this folder

**Only what a gate runs.** Heavy tooling stays in `python/inet/` and `_scripts/`; this folder holds
the direct transcription of a numbered rule, plus the small script that reads it.

**No copies.** A tool file that must sit at the repository root sits at the root, and this folder
cites it. The clang-tidy configuration is the case that taught the rule: clang-tidy finds its
configuration by a walk up from the source file, so a copy staged under `doc/` was never read by
anything. It now lives at [`/.clang-tidy`](../../../.clang-tidy) and covers two rule families at
once — the C++ names of [naming.md](../rule/naming.md) and the bug and modernization checks of
[quality.md](../rule/quality.md).

## Quality-attribute coverage

Which quality attributes each rule group serves, in the ISO/IEC 25010 vocabulary. The table is a
review aid: a change that weakens a rule attacks the attributes on its row.

| Rule group | Quality attributes served |
|---|---|
| AR-ORG | Modularity, Analysability, Modifiability |
| AR-MOD | Modularity, Reusability, Modifiability; Fidelity = the performance against accuracy trade-off |
| AR-PKT | Compatibility and Interoperability, Correctness, Performance efficiency |
| AR-COM | Modularity, Extensibility, Interoperability |
| AR-LIFE | Reliability (recoverability), Testability |
| AR-QUEUE | Modularity, Reusability, Extensibility |
| AR-OBS | Analysability, Testability |
| AR-CFG | Configurability, Maintainability |
| AR-EXT | Extensibility, Modularity, Modifiability |
| AR-BUILD | Deployability, Portability, Reproducibility |
| AR-QUAL | Testability, Reliability, Analysability, and the governance dimension |
| NR, QR, DR | Analysability, Maintainability |
| TR | Testability, Reliability |
| PR, RR, SR | Maintainability, and the governance dimension |
