# Association reset: parent-to-commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-20 · **Seal:** none · **Owns:** — · **Stands on:** [comparison method](../pr1343-comparison-method.md), [test guide](blockack-same-ap-reassociation.md)

The reassociation fixture fails before the association reset fix and passes after that fix. Equal peer parameters leave four old agreement roles active before the fix. After the fix, all four disappear and fresh setup succeeds.

## Compared revisions

The selected commit has the subject `ieee80211: fix: clear old peer state before association replacement`. It adds [WifiBlockAckSameApReassociation.test](../../../../../../tests/protocol/wifi/common/WifiBlockAckSameApReassociation.test). The before revision is that commit's first parent.

| Role | Executed source revision | Production tree (`src/`) |
| --- | --- | --- |
| Before: parent of the association reset fix | `d58fe2b72590a195fa69f4c128fddcbb2a7269d8` | `d6fb238177e86a61b28a45ab040d5d9c5a3cf3b8` |
| After: association reset fix | `b207e821d89879452760f6f73d173601f7c9f28f` | `92b9a2b8864c87d43c034bace4c37c502e17bcf1` |

These hashes pin the recorded executions. The current fix also contains source citations for association reset and comments that identify INET cleanup policy. Those comments change the production tree hash but preserve C++ tokens. The four listed fixture inputs retain their executed contents. The results below describe the recorded revisions; they do not claim a repeated parent comparison after the comment correction.

## Results

| Observation | Before the commit | After the commit |
| --- | --- | --- |
| Roles before replacement | Four active roles from real ADDBA exchanges | Four active roles from real ADDBA exchanges |
| Peer rates and advertised HT capabilities | Equal before and after replacement | Equal before and after replacement |
| Roles after replacement | Four remain; the reset assertion fails | Zero remain |
| Fresh setup and application delivery | The fixture stops at the earlier reset failure | Fresh ADDBA succeeds in both directions; both destinations receive new traffic |
| Final roles | The fixture stops before this assertion | Four fresh roles exist |
| Fixture verdict | FAIL | PASS |

One file contains one simulation run. The parent fails at the post-reassociation role assertion. The after revision completes every assertion. The same test controls and successful expectations apply to both revisions.

## Cause and assertion boundary

Before this commit, equal capability and rate values can hide association replacement from MAC cleanup. After this commit, management supplies the old peer explicitly before association replacement. The MAC forwards that command to HCF. HCF clears both agreement roles independently of a capability change.

The fixture starts bidirectional UDP traffic through the AP. Real ADDBA creates originator and recipient roles at each endpoint. The agent requests same-AP reassociation without another authentication exchange. The fixture never calls the reset operation or supplies agreement state.

Inactivity timeouts remain zero, so expiration cannot clear the roles for this assertion. The observer checks the old roles one millisecond after station confirmation. This interval permits the AP to receive the response ACK and commit replacement. A later traffic batch must create fresh agreements and deliver UDP in both directions.

## Reproduce and inspect

Copy these four inputs from the fix into its parent worktree:

- `tests/protocol/wifi/common/WifiBlockAckSameApReassociation.test`.
- `tests/protocol/wifi/BlockAckLifecycleTestSupport.h`.
- `tests/protocol/wifi/ned/BlockAckLifecycleHcf.ned`.
- `tests/protocol/wifi/ini/_blockackLifecycle.ini`.

Run this selector on both debug builds:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAckSameApReassociation\.test$' --no-concurrent
```

The parent output is `07-b2a043d55c-parent-output/WifiBlockAckSameApReassociation.out`. The after log is `07-b2a043d55c-protocol.log`. That after selector repeats all five fixtures. Only the reassociation verdict belongs to this comparison.

## Limits

The fixture checks one successful same-AP replacement with equal peer parameters. It does not check protected management procedures, arbitrary roaming, or MAC stop/start. The stations use the Basic compatibility profile. The guide explains the standards justification and its limits.
