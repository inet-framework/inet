# Basic transfer completion: before and after the change

> **Kind:** measurement · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [guide](blockack-transfer-completion.md)

Both completion cases fail before S4 and pass on its final production source. The queue-exhaustion case misses BAR. The one-position-window case observes DATA where BAR is required.

## Exact source and fixture inputs

The parent is `02f24a3a4d519511cf24731e25d926068b11d24d`, with source tree `2a26acc1bd1d8d5ccac42c509cbfced219554175`. The final S4 source tree is `20aade533739a1877a87391909da7dd2a91c79c7`.

The recorded controls use `WifiBlockAckTransferCompletion.test` with SHA-256 `e46b8804822747bc1558a802d0d7054aadf04b445370992bd5b5714b9864b0fa`. The shared topology and lifecycle inputs remain equal. No fixture supplies agreement state or injected feedback.

The current fixture also rejects the program's FAIL line. This output guard changes no runtime scenario or protocol assertion. It prevents one successful run from hiding the other run's failure.

## Verification of the output guard

The corrected fixture has SHA-256 `7d526bcf8c213ebe7fdb4e4e5182393d546ef60e79feb4e044c97c3372d08156`. Both normal runs pass through the integrated debug protocol runner. Separate diagnostic INI files delay traffic beyond the protocol deadline in either run or both runs. Each simulation batch returns status 0, even when a protocol verdict is FAIL.

The checks compare the same captured output against the previous fixture and the corrected fixture:

| Run 0 | Run 1 | Previous aggregate | Corrected aggregate |
| --- | --- | --- | --- |
| PASS | PASS | PASS | PASS |
| FAIL | PASS | PASS | FAIL |
| PASS | FAIL | PASS | FAIL |
| FAIL | FAIL | FAIL | FAIL |

These diagnostic failures are intentional controls, not failures of the normal completion scenarios. `opp_test check` returns 0 for PASS and 2 for FAIL, with zero harness errors. The guard rejects both mixed-result orders while it preserves the valid two-PASS result.

The debug build log is `/tmp/pr1344-r1-debug.log`. The normal protocol log is `/tmp/pr1344-r1-protocol.log`. The verdict matrix is `/tmp/pr1344-r1-verdict-matrix.json`. Detailed checker logs use `/tmp/pr1344-r1-<case>-<before|after>.log`. These local artifacts can be absent in another checkout.

## Observed results

| Run | Before S4 | After S4 |
| --- | --- | --- |
| 0: two packets, window 64 | FAIL: BAR deadline at step 3 expires. | PASS through matched feedback, DELBA, and ACK. |
| 1: four packets, window one | FAIL: the next DATA-or-BAR event fails the BAR predicate. | PASS through matched feedback, DELBA, and ACK. |

Both parent failures occur after real association, successful ADDBA, and the first Block Ack DATA. They reach the intended protocol boundary. They do not arise from a build or initialization error.

After S4, the policy selects BAR below threshold five. HCF permits BAR-only contention. The frame sequence receives matching feedback. Completion DELBA follows when no DATA remains. Zero inactivity timeout excludes inactivity teardown as an explanation.

## Commands and evidence

Build debug artifacts with `make MODE=debug -j12`. From each checkout root, run:

```sh
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiBlockAckTransferCompletion\.test$'
```

The parent command uses `/tmp/pr1344-controls-20261022`. Its combined selection also includes the Retry fixture and two existing lifecycle controls. Its log is `/tmp/pr1344-control-base-protocol.log`. Preserved outputs are under `/tmp/pr1344-parent-protocol-artifacts/WifiBlockAckTransferCompletion/`.

The integrated run uses `/home/user/omnetpp_ws/inet-compressed-blockack-s4`. Its log is `/tmp/pr1344-correction-final-protocol-regression.log`. The completion fixture passes both runs. That wider selection also exposes two pre-existing lifecycle fixture assumptions; its aggregate failure is not a completion-test failure.

General configuration runs 0 and 1 use seed 0 and a 1.1-second limit. The program stops when it reaches its verdict. The parent runner returns 1. The isolated final completion selection returns 0.

## Limits

These controls establish the two stated exchange sequences. The independent Retry correction does not create BAR-only access or completion DELBA. The exact reconstructed parent comparison receives its own verification record. No general performance, NAV, admission, or additional-seed claim follows from these results.
