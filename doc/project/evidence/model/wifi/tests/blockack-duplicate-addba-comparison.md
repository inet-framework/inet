# Action dispatch: parent-to-commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-20 · **Seal:** none · **Owns:** — · **Stands on:** [comparison method](../pr1343-comparison-method.md), [test guide](blockack-duplicate-addba.md)

The duplicate-request fixture fails before the Action dispatch fix and passes after that fix. The recipient dispatches both request copies before the fix. After the fix, the recipient dispatches once and completes both request ACK transmissions.

## Compared revisions

The selected commit has the subject `ieee80211: fix: dispatch complete Block Ack Actions once`. It adds [WifiBlockAckDuplicateAddba.test](../../../../../../tests/protocol/wifi/common/WifiBlockAckDuplicateAddba.test). The comparison uses that commit and its first parent, as defined by the [comparison method](../pr1343-comparison-method.md).

| Role | Executed source revision | Production tree (`src/`) |
| --- | --- | --- |
| Before: parent of the dispatch fix | `54d538def3e4e3c4277afd71550887b5d03f0ea3` | `2cc249d913c5eb4e3ca8f1f90ff69765292d1ea5` |
| After: dispatch fix | `6347242ed74b1edde2cd1d9450c9d204af959255` | `fa0b333c8d2f8ae4974b7fa2a0128d01008fdee8` |

These hashes pin the recorded executions. The current history can use different commit IDs after a documentation rewrite. Both compared production inputs and the fixture inputs remain identical to those executions.

## Results

| Observation | Before the commit | After the commit |
| --- | --- | --- |
| Original request and retry | Two receptions | Two receptions |
| Request dispatch | Two dispatches; the duplicate assertion fails | One dispatch |
| Request ACK submission | Two submissions at the failed assertion | Two submissions |
| Physical ACK completion | The fixture stops at the earlier dispatch failure | Both selected request ACKs complete |
| Response and later DATA | The fixture stops before these assertions | Response arrives; Block Ack-policy DATA follows |
| Fixture verdict | FAIL | PASS |

One file contains one simulation run. The parent failure reaches the duplicate-dispatch assertion. It does not result from a test build or initialization error. The after run completes every assertion.

## Cause and assertion boundary

Before this commit, HCF dispatches the management header after the recipient data service rejects its duplicate. After this commit, HCF dispatches only complete frames that the service accepts. The retry still receives its immediate ACK.

The test MAC suppresses the first request ACK at the originator. The test HCF holds the response until the retry arrives. It releases the response one millisecond later. The observer counts only ACK packets submitted for ADDBA Requests, through their packet tree IDs. It excludes unrelated DATA ACKs.

## Reproduce and inspect

Copy these four inputs from the fix into its parent worktree:

- `tests/protocol/wifi/common/WifiBlockAckDuplicateAddba.test`.
- `tests/protocol/wifi/BlockAckLifecycleTestSupport.h`.
- `tests/protocol/wifi/ned/BlockAckLifecycleHcf.ned`.
- `tests/protocol/wifi/ini/_blockackLifecycle.ini`.

Run this selector on both debug builds:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAckDuplicateAddba\.test$' --no-concurrent
```

The recorded parent output is `02-25fb756da1-parent-output/WifiBlockAckDuplicateAddba.out`. The after log is `02-25fb756da1-protocol.log`. Both paths use the local evidence directory from the comparison method. The after selector contains only this fixture.

## Limits

The fixture supplies ACK loss and a controlled response delay. It checks one fixed seed and one retry order. It does not establish complete channel-access behavior or full standards conformance. The guide separates the legitimate retry condition from these test controls.
