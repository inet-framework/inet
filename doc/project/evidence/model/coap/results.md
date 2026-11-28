# CoAP — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/coap/checks.md), [features.md](../../protocol/coap/features.md), [coverage.md](coverage.md)

Executed protocol conformance suite and example verification record across commits `C2`–`C9` on branch `feat/coap`.

## Executed protocol test suite

| Test | Commit | Check | Outcome |
| --- | --- | --- | --- |
| `Rfc7252MessageEncoding.test` | C2 (extended C5, C6) | [message-encoding-and-option-delta-framing](../../protocol/coap/checks/encoding.md#message-encoding-and-option-delta-framing) | PASS |
| `Rfc7252MalformedDatagrams.test` | C2 (extended C3, C5, C6) | [malformed-datagram-rejection-and-silent-discard](../../protocol/coap/checks/encoding.md#malformed-datagram-rejection-and-silent-discard) | PASS |
| `Rfc7252NonConfirmable.test` | C2 | [non-confirmable-request-and-response](../../protocol/coap/checks/messaging.md#non-confirmable-request-and-response) | PASS |
| `Rfc7252ResponseMatching.test` | C2 (extended C3) | [response-matching-by-endpoint-and-token](../../protocol/coap/checks/messaging.md#response-matching-by-endpoint-and-token) | PASS |
| `Rfc7252ConfirmableRetry.test` | C3 | [confirmable-retransmission-and-exponential-backoff](../../protocol/coap/checks/messaging.md#confirmable-retransmission-and-exponential-backoff) | PASS |
| `Rfc7252SeparateResponse.test` | C3 | [deferred-separate-response-exchange](../../protocol/coap/checks/messaging.md#deferred-separate-response-exchange) | PASS |
| `Rfc7252DuplicateSuppression.test` | C3 (extended C4) | [duplicate-suppression-and-cached-acknowledgment](../../protocol/coap/checks/messaging.md#duplicate-suppression-and-cached-acknowledgment) | PASS |
| `Rfc7252ResetAndEmpty.test` | C3 | [empty-confirmable-ping-and-reset-handling](../../protocol/coap/checks/messaging.md#empty-confirmable-ping-and-reset-handling) | PASS |
| `Rfc7252CongestionAdmission.test` | C3 | [congestion-admission-and-probing-rate-limit](../../protocol/coap/checks/messaging.md#congestion-admission-and-probing-rate-limit) | PASS |
| `Rfc7252ResourceMethods.test` | C4 | [resource-methods-get-post-put-and-delete](../../protocol/coap/checks/resources.md#resource-methods-get-post-put-and-delete) | PASS |
| `Rfc7252OptionsAndConditions.test` | C4 | [critical-options-conditional-requests-and-content-negotiation](../../protocol/coap/checks/resources.md#critical-options-conditional-requests-and-content-negotiation) | PASS |
| `Rfc6690ResourceDiscovery.test` | C4 (extended C5) | [core-link-format-resource-discovery](../../protocol/coap/checks/resources.md#core-link-format-resource-discovery) | PASS |
| `Rfc7641ObserveRegistration.test` | C5 | [observe-registration-and-initial-notification](../../protocol/coap/checks/observe.md#observe-registration-and-initial-notification) | PASS |
| `Rfc7641ObserveNotifications.test` | C5 | [confirmable-notification-supersession-and-retry-removal](../../protocol/coap/checks/observe.md#confirmable-notification-supersession-and-retry-removal) | PASS |
| `Rfc7641ObserveReordering.test` | C5 | [notification-ordering-and-128-second-rule](../../protocol/coap/checks/observe.md#notification-ordering-and-128-second-rule) | PASS |
| `Rfc7641ObserveCancellation.test` | C5 | [observation-cancellation-by-observe1-reset-and-error](../../protocol/coap/checks/observe.md#observation-cancellation-by-observe1-reset-and-error) | PASS |
| `Rfc7641ObserveFreshness.test` | C5 | [notification-freshness-and-re-registration](../../protocol/coap/checks/observe.md#notification-freshness-and-re-registration) | PASS |
| `Rfc7959Block1Upload.test` | C6 | [block1-atomic-request-upload](../../protocol/coap/checks/blockwise.md#block1-atomic-request-upload), [combined-block1-upload-and-block2-response-transfer](../../protocol/coap/checks/blockwise.md#combined-block1-upload-and-block2-response-transfer) | PASS |
| `Rfc7959Block2Download.test` | C6 | [block2-response-download-and-etag-consistency](../../protocol/coap/checks/blockwise.md#block2-response-download-and-etag-consistency) | PASS |
| `Rfc7959BlockNegotiation.test` | C6 | [block-size-negotiation-for-block1-and-block2](../../protocol/coap/checks/blockwise.md#block-size-negotiation-for-block1-and-block2) | PASS |
| `Rfc7959BlockErrorsAndGc.test` | C6 | [blockwise-transfer-error-responses-400-408-413-and-503](../../protocol/coap/checks/blockwise.md#blockwise-transfer-error-responses-400-408-413-and-503) | PASS |
| `Rfc9175RequestTag.test` | C6 | [request-tag-concurrent-block1-uploads](../../protocol/coap/checks/blockwise.md#request-tag-concurrent-block1-uploads) | PASS |
| `Rfc7959ObserveBlock2.test` | C7 | [observe-notification-with-block2-follow-up-retrieval](../../protocol/coap/checks/blockwise.md#observe-notification-with-block2-follow-up-retrieval) | PASS |
| `Rfc7959ObserveVersionChange.test` | C7 | [observe-and-block2-version-change-supersession-and-recovery](../../protocol/coap/checks/blockwise.md#observe-and-block2-version-change-supersession-and-recovery) | PASS |
| `CoapLifecycle.test` | C8 | INET node stop/start/crash lifecycle recovery | PASS |
| `CoapStatistics.test` | C8 | Emitted CoAP exchange, Observe, and blockwise signals and statistics | PASS |

## Observed builds, tests, and example commands

- **Full protocol suite (`debug`):** `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coap$' -f '.*\.test$'` — `[28/28] ◉ Tested test (26 concurrent): 26 PASS in 2:38.173` (exit `0`).
- **Release compilation:** `make MODE=release -j8` — built `out/clang-release/src/libINET.so` cleanly (exit `0`).
- **Feature-disabled isolation build & restore:** `opp_featuretool disable Coap CoapExamples CoapTests && make makefiles && make MODE=debug -j8 && opp_featuretool enable Coap CoapExamples && make makefiles && make MODE=debug -j8` — clean compilation both with `-Xinet/applications/coap` and after restoring `Coap`/`CoapExamples` (exit `0`).
- **Example scenarios (`examples/coap/omnetpp.ini`, Cmdenv run `#0`, `seed-set = 0`):** `NonConfirmable`, `ConfirmableLoss` (`retransmission:count 1`, `requestCompleted:count 5`), `SeparateResponse`, `ResourceDiscovery`, `Observe` (`notificationReceived:count 4`), `Block1Upload`, `Block2Download`, `ObserveBlockwise` (`notificationReceived:count 2`), `Ipv6` — all exited with code `0`.
