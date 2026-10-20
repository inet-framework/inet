# BAR without an agreement: parent-to-commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-20 · **Seal:** none · **Owns:** — · **Stands on:** [comparison method](../pr1343-comparison-method.md), [test guide](blockack-bar-without-agreement.md)

The orphan-BAR run fails before the lifecycle fix and passes after that fix. The established-agreement control passes on both revisions. This record covers only `WifiBlockAckBarWithoutAgreement.test`. The inactivity fixture has its own [comparison record](blockack-inactivity-comparison.md).

## Compared revisions

The selected commit has the subject `ieee80211: fix: preserve Block Ack setup and teardown boundaries`. It adds [WifiBlockAckBarWithoutAgreement.test](../../../../../../tests/protocol/wifi/common/WifiBlockAckBarWithoutAgreement.test) and [WifiBlockAckInactivity.test](../../../../../../tests/protocol/wifi/common/WifiBlockAckInactivity.test). Either fixture identifies the same fix in the comparison method.

| Role | Executed source revision | Production tree (`src/`) |
| --- | --- | --- |
| Before: parent of the lifecycle fix | `6c90fe0b9b85f199d867ab08bd42097b3437530e` | `3d134699aca48ba038d9dec3a8b947c32b754c2d` |
| After: lifecycle fix | `d58fe2b72590a195fa69f4c128fddcbb2a7269d8` | `d6fb238177e86a61b28a45ab040d5d9c5a3cf3b8` |

These hashes pin the recorded executions. A documentation rewrite can change current commit IDs without a production or fixture change. Both compared input sets retain their executed contents.

## Results

| Run | Before the commit | After the commit |
| --- | --- | --- |
| 0: no agreement | FAIL: no required DELBA within 20 ms | PASS: recipient-role DELBA has the correct peer and TID |
| 1: established agreement | PASS: Basic Block Ack reaches the controlled peer; no DELBA appears | PASS: the same assertions succeed |

One file contains two simulation runs. The parent failure reaches the orphan-BAR deadline assertion. The after revision passes both runs. Ordinary PASS expectations remain unchanged in both revisions.

## BAR recovery

Before this commit, the recipient does not start the required recovery when a valid BAR arrives without an agreement. After this commit, HCF requests recipient-role DELBA without a recipient agreement allocation. The test requires the correct receiver, TID, and Initiator value.

The stimulus sends a crafted Basic BAR through production Tx and the radio. Run 0 selects TID 5 without an agreement. Run 1 establishes TID 6 through real ADDBA before the BAR. Its controlled peer consumes the Basic Block Ack after production Rx validation. It takes ownership before frame deletion. The recipient response paths remain unchanged by the fixture.

## Reproduce and inspect

Copy these four inputs from the fix into its parent worktree:

- `tests/protocol/wifi/common/WifiBlockAckBarWithoutAgreement.test`.
- `tests/protocol/wifi/BlockAckLifecycleTestSupport.h`.
- `tests/protocol/wifi/ned/BlockAckLifecycleHcf.ned`.
- `tests/protocol/wifi/ini/_blockackLifecycle.ini`.

Run this selector on both debug builds:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAckBarWithoutAgreement\.test$' --no-concurrent
```

The parent output is `06-942a7e2780-parent-output/WifiBlockAckBarWithoutAgreement.out`. The after log is `06-942a7e2780-protocol.log`. That after selector also repeats other fixtures. Only the BAR fixture verdict belongs to this comparison.

## Limits

The fixture supplies the absent agreement directly. It does not reproduce earlier agreement teardown and lost DELBA attempts. Its crafted sender bypasses ordinary EDCA channel access. Basic BAR uses a reserved current-standard type value. The guide explains the legitimate recovery condition and these fixture limits.
