# DATA Retry: before and after the correction

> **Kind:** measurement · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [guide](blockack-data-retry.md)

Both feedback cases fail at the Retry assertion on the parent. Both pass when the control adds only HCF's Retry branch. The final S4 source also passes both cases.

## Exact inputs and source controls

| Control | Production source |
| --- | --- |
| Parent | Commit `02f24a3a4d519511cf24731e25d926068b11d24d`; source tree `2a26acc1bd1d8d5ccac42c509cbfced219554175`. |
| Isolated correction | The same parent plus HCF's eleven-line Retry branch; source tree `5a58b7601444e9872312112e6f6e8ffc86207f67`. |
| Integrated S4 | Source tree `20aade533739a1877a87391909da7dd2a91c79c7`. |

All controls use the same `WifiBlockAckDataRetry.test` input. Its SHA-256 is `271696667cf84e675683c6e146d66fd0bd57fed144cf3221caefd7e18e2c14bf`. The fixture and shared lifecycle support build on both sides. No expected-failure declaration applies.

## Observed results

| Run | Parent | Isolated correction | Integrated S4 |
| --- | --- | --- | --- |
| 0: lost Block Ack | FAIL at the Retry predicate. | PASS. | PASS. |
| 1: cleared bitmap | FAIL at the Retry predicate. | PASS. | PASS. |

The parent reaches the retransmission of the captured sequence and TID. It fails because Retry remains clear. These failures are runtime assertion results, not setup or compilation errors. The isolated source changes only that transmitted header branch.

## Commands and artifacts

Build each source with `make MODE=debug -j12` before the test. From each checkout root, run:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiBlockAckDataRetry\.test$'
```

The parent run uses `/tmp/pr1344-controls-20261022`. Its combined control command also selects the completion and two existing lifecycle fixtures. The parent log is `/tmp/pr1344-control-base-protocol.log`. Preserved outputs are under `/tmp/pr1344-parent-protocol-artifacts/WifiBlockAckDataRetry/`.

The isolated run uses the same retained worktree after the source patch. Its log is `/tmp/pr1344-control-retry-protocol.log`. The integrated log is `/tmp/pr1344-correction-final-protocol-regression.log`. Both integrated Retry cases pass. The combined parent runner returns 1 for the two new failing fixtures.

All runs use General configuration, runs 0 and 1, seed 0, and a 1.1-second limit. The protocol program stops each run when it reaches its verdict. No additional seed or loss campaign follows from these results.

## Fingerprint scope

The isolated correction completes seven existing fingerprint cases. Five `~tND` values differ. Every selected event and packet-length fingerprint remains equal to its parent baseline. The detailed calculation is `/tmp/pr1344-retry-fingerprints.out`.

The user approved the five intermediate values on 2026-10-22. They apply only before transfer completion enters the series. The final S4 source retains its seven previously accepted baseline rows. These measurements do not establish aggregate retransmission behavior or complete Basic conformance.
