# Immediate Compressed Block Ack verification scope

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [implementation status](ieee80211-compressed-blockack-replacement.md), [admission contract](ieee80211-compressed-blockack-txop-admission.md)

The series retains ten functional S5 commits and two document commits on base `7fda2e3987c94177d9ac619c26beb59353571d94`. Master supplies merged stage S4 through PR #1344. The local-interval reconstruction requires debug and release builds at every replacement boundary. Each boundary also requires its focused tests. The execution record is `/tmp/inet-local-interframe-20261028/execution.md`.

The original publication at `d8f98b8bf3f4477f5780564c983662006c11c27f` used final-head verification only. Its intermediate commits received structural checks without builds or runtime tests. Those results remain historical evidence for that original head.

## Production scope

HCF defaults to immediate Compressed Block Ack for eligible HT links. Individual MPDUs and A-MSDU use explicit BAR feedback. Separate receipt state preserves positive bits after delivery. Rx detects SIFS or permitted RIFS from local observations under the coverage-class-zero limit. Reversible DATA preparation permits complete exchange admission before source removal.

Master supplies S4 agreement lifecycle, borrowed preview, BAR-only contention, window operations, DATA Retry, and completion DELBA. The S5 series uses those contracts. It adds no S4 commits or recorded fingerprint expectations.

## Verification contract for the reconstruction

Both debug and release builds must compile each replacement tree. Debug tests retain runtime assertions. The checks use the active checkout and its installed `opp_run_*` interfaces. The test commands require explicit filters. A missing runner capability must remain an explicit evidence gap.

The focused module selection covers prepared QoS DATA, Compressed admission, migration fallback, input validation, originator feedback, and local ACK completion. Basic scheduling and profile-selection fixtures provide compatibility controls. Protocol fixtures own physical interval boundaries and their delivery consequences. The unit selection covers wire fields, receipt records, control rates, HT modes, HT capabilities, and shared window operations.

The implementation also supplies n/ac protocol fixtures for Basic and Compressed operation. A fixture inventory does not establish an executed protocol result. The PR description must state whether protocol, serializer, and fingerprint campaigns ran on the published head.

The local mechanical checks cover the commit series, source protection, architecture, names, interfaces, whitespace, and document links. Findings require comparison with pinned base `7fda2e3987c94177d9ac619c26beb59353571d94`. An inherited finding remains a failed gate; it is not a new S5 defect. Publication gates remain outside this local reconstruction.

## Evidence limits

No result from another source tree establishes a current pass. The original publication logs reside under `/tmp/inet-s5-master-20261027/` for its original head. The local reconstruction uses `/tmp/inet-local-interframe-20261028/`. These local files can be absent elsewhere. Each record must name its source tree and evidence limits.

A-MPDU, implicit BAR, delayed Compressed Block Ack, Multi-TID Block Ack, and complete VHT transfer conformance remain outside the supported scope. General Basic admission and admission outside active Compressed agreements retain their limits. Complete NAV conformance, MAC stop/start recovery, and general performance remain unverified.

Basic compatibility retains the IEEE Std 802.11e-2005 format without a complete conformance claim. The selected deterministic cases cannot establish all configurations, seeds, PHY models, or external extensions. No baseline update is authorized or required by this verification contract.
