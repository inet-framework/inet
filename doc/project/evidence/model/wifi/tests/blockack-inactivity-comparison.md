# Inactivity deadline: parent-to-commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-20 · **Seal:** none · **Owns:** — · **Stands on:** [comparison method](../pr1343-comparison-method.md), [test guide](blockack-inactivity.md)

The finite-inactivity run fails before the lifecycle fix and passes after that fix. The zero-timeout control passes on both revisions. This record covers only `WifiBlockAckInactivity.test`. The BAR fixture has its own [comparison record](blockack-bar-without-agreement-comparison.md).

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
| 0: finite timeout | FAIL: recipient agreement remains at its absolute deadline | PASS: agreement disappears at the deadline; timeout DELBA and its ACK complete |
| 1: zero timeout | PASS: agreement remains active without DELBA | PASS: the same assertions succeed |

One file contains two simulation runs. The parent failure reaches the exact-deadline assertion. The after revision passes both runs. Ordinary PASS expectations remain unchanged in both revisions.

## Absolute inactivity deadline

Before this commit, HCF treats an absolute expiration time as a relative delay. After this commit, HCF schedules the earliest absolute deadline and removes the expired agreement before timer recalculation.

The request and recipient policy both select 20 ms. This equality avoids a dependency on advisory-timeout negotiation. A read-only observer obtains the stored deadline. It requires the agreement one nanosecond before that deadline and no agreement at or one nanosecond after it.

The production expiry event enters the event queue before the boundary observations. A concurrent assertion requires recipient-role DELBA with reason code 39 for the correct peer and TID. Transmission cannot start before the captured deadline. The controlled peer must receive and acknowledge DELBA.

The zero-timeout control requires an infinite deadline and an active agreement at 1.025 s. It prohibits DELBA until 1.031 s. Neither fixture changes the agreement state or the production timer.

## Reproduce and inspect

Copy these six inputs from the fix into its parent worktree:

- `tests/protocol/wifi/common/WifiBlockAckInactivity.test`.
- `tests/protocol/wifi/BlockAckLifecycleTestSupport.h`.
- `tests/protocol/wifi/BlockAckRequestProbe.h`.
- `tests/protocol/wifi/ned/BlockAckLifecycleHcf.ned`.
- `tests/protocol/wifi/ned/BlockAckRequestProbe.ned`.
- `tests/protocol/wifi/ini/_blockackLifecycle.ini`.

Run this selector on both debug builds:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAckInactivity\.test$' --no-concurrent
```

The parent output is `06-942a7e2780-parent-output/WifiBlockAckInactivity.out`. The after log is `06-942a7e2780-protocol.log`. That after selector also repeats other fixtures. Only the inactivity fixture verdict belongs to this comparison.

## Limits

The 20 ms timeout is not an integer number of time units. The controlled request bypasses ordinary channel access and a complete originator procedure. The test covers one recipient agreement without DATA refresh. It does not establish originator expiration, concurrent deadlines, or refresh behavior. The guide explains a compliant timeout value and the Basic profile limits.
