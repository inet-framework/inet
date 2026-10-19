# TXOP admission: publication checks

All six source commits build in debug mode and pass their selected tests. The final source also compiles in release mode. This record supplements the [implementation evidence](ieee80211-txop-admission-remaining-execution.md) for the user's publication request on 2026-10-19.

The branch starts at `d841dc77db7d0b141bf31e5e215a0de9bb281957`, the open upstream PR #1364 head. The checked upstream master is `0f1458fbdc1379ad93ad6d7a04975e5607b26197`. The publication remains a draft because that dependency and the separate peer-permission integration remain open. Fingerprint baselines remain unchanged.

## Commit boundaries

Each source commit has its own build and direct test result. The validation procedure restored that exact commit's tracked source and test definitions before execution. It then restored the saved final working files. The unrelated `AGENTS.md` edit remained unchanged.

| Commit | Purpose | Direct evidence |
| --- | --- | --- |
| `a8557bc651` | C1a adds bounded fragment partitions. | Debug build and the bounded-fragmentation unit fixture pass. |
| `9c4bf543d8` | C1b adds reversible fragments and group ownership. | Debug build and the fragment-preparation module fixture pass. |
| `04b2e32c65` | C2 adds allocation-scoped retry records. | Debug build and the MSDU recovery module fixture pass. |
| `d98c98fcfb` | C3 prepares common admission without Basic or modern activation. | Debug build and six Compressed and A-MPDU control fixtures pass. |
| `c419a07251` | C4 activates modern Normal Ack admission. | Debug build and eight admission, protection, cancellation, collision, and management fixtures pass. |
| `a6c85b80ba` | C5 activates Basic admission under the accepted R1 rule. | Debug build and five Basic, fragment-retirement, Compressed, and profile fixtures pass. |

C1 uses two commits because the policy operation can stand alone before service and ownership changes. C3 changes 416 source lines. Its message explains why cost, validation, commitment, submission, and Duration consumers need the same prepared representation. C4 and C5 contain their activation tests. C6 contains the plan, release note, and evidence records.

The [verification metadata](../reproductions/ieee80211-txop-admission-remaining/publication-20261019/verification.json) records each commit SHA, exact selector, mode, and exit status. The raw publication logs remain under `/tmp/inet-txop-publication/`. The earlier implementation artifact folder retains the detailed behavioral evidence.

The retained console captures preserve their original bytes and hashes. A directory-specific Git attribute excludes only those `.txt` captures from whitespace checks. Authored source and prose retain their normal checks.

## Final source checks

Both final builds return zero:

```sh
make MODE=debug -j8
make MODE=release -j8
```

The debug library SHA-256 is `1de36100640807565d8456334072201ff3f412a3e5fbfe28e0376a5e0fc57bf2`. It matches the implementation campaign's library exactly. The release library SHA-256 is `70436a0ffc1aa69d1a74059609e6cb1cd02035fa959fe12c95861dbebace6b2b`. Release verification covers compilation. Behavioral verification uses debug mode with assertions.

The final repeated campaign passes 31 module fixtures, four unit fixtures, and the legacy fragmentation protocol fixture. It includes 96 Normal Ack runs and 36 Basic runs. The Onoe and AARF campaigns each pass sixteen additional runs. The [execution record](ieee80211-txop-admission-remaining-execution.md#verification-commands-and-results) gives their commands. Each selected fingerprint row repeats its earlier difference. The [causality report](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/c5-fingerprint-causality.md) explains those results.

## Mechanical checks and their limits

The source-seal check passes against upstream master. The six-source-commit series check passes against the dependency head. Its one size note concerns C3's 416 source lines and the explanation in that commit. The final documentation commit receives the same series checks before the push.

The project-wide architecture, naming, and interface checks return failures. Each complete output is identical to the corresponding check at the unchanged dependency head. The interface check reports the same fifteen classes. The naming check reports the same twenty-three violations. These comparisons establish no new mechanical finding in this topic. They do not make the global checks pass.

Clang-tidy covers the seven changed C++ implementation files through the debug compilation flags. It reports warnings in unchanged code and no warning on an added line. This evidence does not establish a clean project-wide C++ gate. No new architecture or naming exception enters the ledgers.

The PR description contains three native Mermaid diagrams. Mermaid parses all three. The description check reports no note. This validates syntax and description structure; it does not establish a visual rendering result.

## Open integration work

The separate Basic peer-permission activation still owns rejection between two known HT peers and the joint fallback case. This publication does not change agreement negotiation. The plan remains under `plan/pending/` until that combined requirement has its evidence. Baseline approval remains a separate action.
