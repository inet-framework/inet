# CoAP — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/coap/checks.md), [features.md](../../protocol/coap/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow. This initial skeleton lists the 27 planned protocol tests across commits `C2`–`C8` and is updated with the executed run record and verdicts in `C9`.

## Planned test suite

| Test | Commit | Check |
| --- | --- | --- |
| `Rfc7252MessageEncoding.test` | C2 (extended in C5, C6) | [message-encoding-and-option-delta-framing](../../protocol/coap/checks/encoding.md#message-encoding-and-option-delta-framing) |
| `Rfc7252MalformedDatagrams.test` | C2 (extended in C3, C5, C6) | [malformed-datagram-rejection-and-silent-discard](../../protocol/coap/checks/encoding.md#malformed-datagram-rejection-and-silent-discard) |
| `Rfc7252NonConfirmable.test` | C2 | [non-confirmable-request-and-response](../../protocol/coap/checks/messaging.md#non-confirmable-request-and-response) |
| `Rfc7252ResponseMatching.test` | C2 (extended in C3) | [response-matching-by-endpoint-and-token](../../protocol/coap/checks/messaging.md#response-matching-by-endpoint-and-token) |
| `Rfc7252ConfirmableRetry.test` | C3 | [confirmable-retransmission-and-exponential-backoff](../../protocol/coap/checks/messaging.md#confirmable-retransmission-and-exponential-backoff) |
| `Rfc7252SeparateResponse.test` | C3 | [deferred-separate-response-exchange](../../protocol/coap/checks/messaging.md#deferred-separate-response-exchange) |
| `Rfc7252DuplicateSuppression.test` | C3 (extended in C4) | [duplicate-suppression-and-cached-acknowledgment](../../protocol/coap/checks/messaging.md#duplicate-suppression-and-cached-acknowledgment) |
| `Rfc7252ResetAndEmpty.test` | C3 | [empty-confirmable-ping-and-reset-handling](../../protocol/coap/checks/messaging.md#empty-confirmable-ping-and-reset-handling) |
| `Rfc7252CongestionAdmission.test` | C3 | [congestion-admission-and-probing-rate-limit](../../protocol/coap/checks/messaging.md#congestion-admission-and-probing-rate-limit) |
| `Rfc7252ResourceMethods.test` | C4 | [resource-methods-get-post-put-and-delete](../../protocol/coap/checks/resources.md#resource-methods-get-post-put-and-delete) |
| `Rfc7252OptionsAndConditions.test` | C4 | [critical-options-conditional-requests-and-content-negotiation](../../protocol/coap/checks/resources.md#critical-options-conditional-requests-and-content-negotiation) |
| `Rfc6690ResourceDiscovery.test` | C4 (extended in C5) | [core-link-format-resource-discovery](../../protocol/coap/checks/resources.md#core-link-format-resource-discovery) |
| `Rfc7641Registration.test` | C5 | [observe-registration-and-initial-notification](../../protocol/coap/checks/observe.md#observe-registration-and-initial-notification) |
| `Rfc7641NotificationOrdering.test` | C5 | [notification-ordering-and-128-second-rule](../../protocol/coap/checks/observe.md#notification-ordering-and-128-second-rule) |
| `Rfc7641Freshness.test` | C5 | [notification-freshness-and-re-registration](../../protocol/coap/checks/observe.md#notification-freshness-and-re-registration) |
| `Rfc7641NotificationReliability.test` | C5 | [confirmable-notification-supersession-and-retry-removal](../../protocol/coap/checks/observe.md#confirmable-notification-supersession-and-retry-removal) |
| `Rfc7641Cancellation.test` | C5 | [observation-cancellation-by-observe1-reset-and-error](../../protocol/coap/checks/observe.md#observation-cancellation-by-observe1-reset-and-error) |
| `Rfc7959Block1Upload.test` | C6 | [block1-atomic-request-upload](../../protocol/coap/checks/blockwise.md#block1-atomic-request-upload) |
| `Rfc7959Block2Download.test` | C6 | [block2-response-download-and-etag-consistency](../../protocol/coap/checks/blockwise.md#block2-response-download-and-etag-consistency) |
| `Rfc7959BlockNegotiation.test` | C6 | [block-size-negotiation-for-block1-and-block2](../../protocol/coap/checks/blockwise.md#block-size-negotiation-for-block1-and-block2) |
| `Rfc7959TransferFailures.test` | C6 | [blockwise-transfer-error-responses-400-408-413-and-503](../../protocol/coap/checks/blockwise.md#blockwise-transfer-error-responses-400-408-413-and-503) |
| `Rfc7959CombinedRequestResponse.test` | C6 | [combined-block1-upload-and-block2-response-transfer](../../protocol/coap/checks/blockwise.md#combined-block1-upload-and-block2-response-transfer) |
| `Rfc9175RequestTag.test` | C6 | [request-tag-concurrent-block1-uploads](../../protocol/coap/checks/blockwise.md#request-tag-concurrent-block1-uploads) |
| `Rfc7959ObserveBlock2.test` | C7 | [observe-notification-with-block2-follow-up-retrieval](../../protocol/coap/checks/blockwise.md#observe-notification-with-block2-follow-up-retrieval) |
| `Rfc7959ObserveVersionChange.test` | C7 | [observe-and-block2-version-change-supersession-and-recovery](../../protocol/coap/checks/blockwise.md#observe-and-block2-version-change-supersession-and-recovery) |
| `CoapLifecycle.test` | C8 | INET node stop/start/crash lifecycle recovery |
| `CoapStatistics.test` | C8 | Emitted CoAP exchange, Observe, and blockwise signals and statistics |
