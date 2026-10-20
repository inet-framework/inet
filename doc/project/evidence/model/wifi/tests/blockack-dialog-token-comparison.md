# Negotiation: parent-to-commit comparison

> **Kind:** report · **Status:** snapshot 2026-10-20 · **Seal:** none · **Owns:** — · **Stands on:** [comparison method](../pr1343-comparison-method.md), [test guide](blockack-dialog-token.md)

Two token checks fail before the negotiation fix and pass after that fix. The matching-token control passes on both revisions. This comparison isolates the negotiation commit; it does not compare the endpoints of the PR.

## Compared revisions

The selected commit has the subject `ieee80211: fix: validate Block Ack negotiation parameters`. It adds [WifiBlockAckDialogToken.test](../../../../../../tests/protocol/wifi/common/WifiBlockAckDialogToken.test). The before revision is that commit's first parent.

| Role | Executed source revision | Production tree (`src/`) |
| --- | --- | --- |
| Before: parent of the negotiation fix | `6347242ed74b1edde2cd1d9450c9d204af959255` | `fa0b333c8d2f8ae4974b7fa2a0128d01008fdee8` |
| After: negotiation fix | `77656fef090e0d92a3e9b8b52e641a8313a60a4a` | `e673f4653cb58bd6cf9c1d0660d56ad6994c8d98` |

These hashes pin the recorded executions. A documentation rewrite can change the current commit IDs. The compared production and fixture inputs remain identical. The comparison method explains how to select the current fix and parent.

## Results

| Run | Before the commit | After the commit |
| --- | --- | --- |
| 0: token echo | FAIL: response token 1 replaces request token 37 | PASS: response copies token 37; response ACK completes |
| 1: altered response token | FAIL: originator accepts token 2 for request token 1 | PASS: originator rejects the altered token; no Block Ack-policy DATA appears during the 5 ms guard |
| 2: matching-token control | PASS: setup succeeds and Block Ack-policy DATA follows | PASS: the same assertions succeed |

One file contains three simulation runs. The parent fails two protocol assertions and passes the control. The after revision passes all three runs. The same successful assertions and ordinary PASS expectations apply to both revisions.

## Cause and assertion boundary

Before this commit, the recipient response retains its default token. The originator acceptance policy also lacks the required token match. After this commit, the recipient copies the request token and the originator policy checks response identity.

Run 0 sends a crafted request through production Tx. The AP receives it through the radio and builds its response through the production handler. The controlled peer checks the token, status, peer, and TID. It also completes the response ACK transmission.

Runs 1 and 2 start setup through application traffic and production HCF. Run 1 changes only the response token before transmission. The fixture captures the actual request token; it does not assume a fixed value. Run 2 leaves the response unchanged and checks normal acceptance.

## Reproduce and inspect

Copy these six inputs from the fix into its parent worktree:

- `tests/protocol/wifi/common/WifiBlockAckDialogToken.test`.
- `tests/protocol/wifi/BlockAckLifecycleTestSupport.h`.
- `tests/protocol/wifi/BlockAckRequestProbe.h`.
- `tests/protocol/wifi/ned/BlockAckLifecycleHcf.ned`.
- `tests/protocol/wifi/ned/BlockAckRequestProbe.ned`.
- `tests/protocol/wifi/ini/_blockackLifecycle.ini`.

Run this selector on both debug builds:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiBlockAckDialogToken\.test$' --no-concurrent
```

The recorded parent output is `03-f72df99e76-parent-output/WifiBlockAckDialogToken.out`. The after log is `03-f72df99e76-protocol.log`. The after selector also repeats the earlier duplicate fixture. Only the dialog-token verdict belongs to this comparison.

## Limits

The wrong-token mutation is an artificial robustness check. It does not establish a natural late response between compliant peers. The crafted request bypasses ordinary channel access and a complete originator procedure. The guide explains these limits and the Basic compatibility profile.
