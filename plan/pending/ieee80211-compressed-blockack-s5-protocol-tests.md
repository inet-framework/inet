# S5 protocol verification by commit

This plan assigns one reviewer document to each S5 commit. S5-P1 through S5-P10 identify the unchanged commit order. Master supplies merged S4 through PR #1344. The [implementation status](ieee80211-compressed-blockack-replacement.md) defines the supported production scope.

Protocol comparisons use identical fixtures against each applicable commit and its parent. The parent must compile the fixture. Its specific failure must match the documented bug or missing capability. The child must pass the same input. Helper API absence at compilation is not protocol evidence.

## Commit owners and documents

| Stable identifier | Commit responsibility | Verification and reviewer document |
| --- | --- | --- |
| S5-P1 | Detect SIFS from local endpoints in Rx. | [Local interval and physical completion checks.](../../tests/protocol/wifi/s5-physical-timing.md) |
| S5-P2 | include the HT signal extension at 2.4 GHz. | [One physical HT exchange; parent duration assertion fails and child passes.](../../tests/protocol/wifi/s5-ht-signal-extension.md) |
| S5-P3 | preserve advertised RIFS permission. | [Eight management cases; the parent loses permission and the child passes.](../../tests/protocol/wifi/s5-rifs-permission.md) |
| S5-P4 | release temporary Block Ack bitmap allocations. | [Allocation ownership; a protocol exchange cannot establish leak freedom.](../../tests/protocol/wifi/s5-bitmap-ownership.md) |
| S5-P5 | serialize Basic and Compressed Block Ack Types. | [Reserved BAR Types fail in the parent; three child cases pass.](../../tests/protocol/wifi/s5-wire-types.md) |
| S5-P6 | prepare QoS DATA without queue extraction. | [Owned preparation; the protocol caller enters in S5-P9.](../../tests/protocol/wifi/s5-reversible-preparation.md) |
| S5-P7 | retain explicit Compressed receipt state. | [Receipt representation; the protocol caller enters in S5-P9.](../../tests/protocol/wifi/s5-receipt-state.md) |
| S5-P8 | select legal Compressed control rates. | [Rate helpers; the protocol caller enters in S5-P9.](../../tests/protocol/wifi/s5-control-rates.md) |
| S5-P9 | complete explicit immediate Compressed Block Ack. | [Explicit exchange cannot initialize in the parent; child feedback passes.](../../tests/protocol/wifi/s5-compressed-exchange.md) |
| S5-P10 | default HCF to immediate Compressed Block Ack. | [Explicit profiles pass in both; the NED default passes only in the child.](../../tests/protocol/wifi/s5-default-profile.md) |

## Execution and evidence boundaries

All protocol cases use seed zero. Each comparison builds a fresh debug library in one retained worktree before execution. The fixture-specific document gives the exact selector, scenario, source-tree identities, observed failure, child result, and coverage limits.

The isolated worktree is `/home/user/omnetpp_ws/inet-s5-protocol-verification-20261027`. The local record directory is `/tmp/inet-s5-protocol-design-20261027/`. Its `comparison-results.json` contains the source checkpoints, commands, fixture hashes, and normalized outcomes. Its `evidence/` directory retains simulation output before the runner clears generated work directories.

A parent initialization error can demonstrate an absent feature. A post-run script error can expose a protocol FAIL hidden by another run's PASS. The documents distinguish those outcomes from compilation errors and on-air assertion failures. No expected-failure declaration changes in the committed fixtures.

Shared timing, preparation, receipt, and rate documents retain direct-test references without a claimed parent protocol verdict. The bitmap ownership document also records the absent leak-detector campaign. These exclusions do not establish full correctness or remove the later integration obligation.

## Standards and diagrams

The applicable revision is IEEE Std 802.11-2024. The reviewer documents identify the normative clauses, tables, corpus identifiers, and physical PDF pages used for their assertions. Corpus lint reports unresolved headings and references elsewhere. The selected requirement nodes resolve uniquely. The VHT RIFS requirement needs direct PDF inspection at physical page 2886 because its corpus node omits that sentence.

All nine Mermaid diagrams render successfully to SVG and PNG. Visual inspection checks the exchange, duration, and default-selection diagrams. The documents keep protocol observations separate from ownership diagrams and helper checks.
