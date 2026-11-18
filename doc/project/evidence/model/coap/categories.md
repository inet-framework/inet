# CoAP — test category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test-anatomy.md](../../../design/test-anatomy.md), [testing.md](../../../rule/testing.md), [coverage.md](coverage.md)

Step 9 artifact of the standards test workflow. Which test category each check belongs to, and why.

## The decision, per check

All 25 specification checks and the 2 lifecycle/statistics checks live in the protocol test suite under `tests/protocol/coap/`. Every CoAP behavior is observable as UDP datagrams on a link between endpoints or as scalar/packet signals emitted by the application modules during a live simulation.

| Check | Observation class | Category | Reason |
| --- | --- | --- | --- |
| [message-encoding-and-option-delta-framing](../../protocol/coap/checks/encoding.md#message-encoding-and-option-delta-framing) | encoding plus wire | protocol test | Verifies both dissected `CoapHeader` fields and serialized UDP payload octets on the wire across live exchanges. |
| [malformed-datagram-rejection-and-silent-discard](../../protocol/coap/checks/encoding.md#malformed-datagram-rejection-and-silent-discard) | wire | protocol test | Injects raw malformed UDP payloads into a live CoAP endpoint and observes `RST`, `4.00`/`4.02`, or silence on the link. |
| [non-confirmable-request-and-response](../../protocol/coap/checks/messaging.md#non-confirmable-request-and-response) | wire | protocol test | Two `NON` datagrams on a link with an absence guard on `ACK`/`RST`. |
| [response-matching-by-endpoint-and-token](../../protocol/coap/checks/messaging.md#response-matching-by-endpoint-and-token) | wire plus end-to-end | protocol test | Observes token correlation, foreign-endpoint rejection, and `RST` on unexpected `CON` responses. |
| [confirmable-retransmission-and-exponential-backoff](../../protocol/coap/checks/messaging.md#confirmable-retransmission-and-exponential-backoff) | wire | protocol test | Drops `CON` frames on a `PacketTap` and verifies deterministic `[2s, 3s]` initial backoff, doubling intervals, and `MAX_RETRANSMIT` cap. |
| [deferred-separate-response-exchange](../../protocol/coap/checks/messaging.md#deferred-separate-response-exchange) | wire | protocol test | Observes the four-way `CON` request -> Empty `ACK` -> separate `CON` response -> Empty `ACK` sequence on the link. |
| [duplicate-suppression-and-cached-acknowledgment](../../protocol/coap/checks/messaging.md#duplicate-suppression-and-cached-acknowledgment) | wire plus end-to-end | protocol test | Duplicates datagrams and drops `ACK`s on a `PacketTap` to verify at-most-once execution and cached `ACK` replay. |
| [empty-confirmable-ping-and-reset-handling](../../protocol/coap/checks/messaging.md#empty-confirmable-ping-and-reset-handling) | wire | protocol test | Injects Empty `CON`/`NON`/`ACK`/`RST` datagrams and observes `RST` or silence. |
| [congestion-admission-and-probing-rate-limit](../../protocol/coap/checks/messaging.md#congestion-admission-and-probing-rate-limit) | wire | protocol test | Observes queuing behind `NSTART=1` and inter-probe delay governed by `PROBING_RATE`. |
| [resource-methods-get-post-put-and-delete](../../protocol/coap/checks/resources.md#resource-methods-get-post-put-and-delete) | wire | protocol test | Exercises CRUD exchanges and status codes over UDP. |
| [critical-options-conditional-requests-and-content-negotiation](../../protocol/coap/checks/resources.md#critical-options-conditional-requests-and-content-negotiation) | wire | protocol test | Observes `2.03`, `4.02`, `4.06`, `4.12`, `5.05`, and `RST` responses on the link. |
| [core-link-format-resource-discovery](../../protocol/coap/checks/resources.md#core-link-format-resource-discovery) | wire | protocol test | Inspects `GET /.well-known/core` responses and query-filtered link-format payloads. |
| [observe-registration-and-initial-notification](../../protocol/coap/checks/observe.md#observe-registration-and-initial-notification) | wire | protocol test | Observes `Observe=0` registration responses and state-change notifications. |
| [notification-ordering-and-128-second-rule](../../protocol/coap/checks/observe.md#notification-ordering-and-128-second-rule) | wire plus end-to-end | protocol test | Injects reordered/wrapped notifications and checks acceptance via the client's notification signal. |
| [notification-freshness-and-re-registration](../../protocol/coap/checks/observe.md#notification-freshness-and-re-registration) | wire | protocol test | Observes re-registration `GET` inside `[Max-Age + 5s, Max-Age + 15s]`. |
| [confirmable-notification-supersession-and-retry-removal](../../protocol/coap/checks/observe.md#confirmable-notification-supersession-and-retry-removal) | wire | protocol test | Drops `CON` notifications on a `PacketTap` while updating the resource and observes superseded retries and observer removal. |
| [observation-cancellation-by-observe1-reset-and-error](../../protocol/coap/checks/observe.md#observation-cancellation-by-observe1-reset-and-error) | wire | protocol test | Observes explicit `Observe=1`, `RST` rejection, and `4.04` deletion termination. |
| [block1-atomic-request-upload](../../protocol/coap/checks/blockwise.md#block1-atomic-request-upload) | wire plus end-to-end | protocol test | Observes `Block1` `2.31 Continue` steps, mid-upload `GET` atomicity, and final `2.04` assembly. |
| [block2-response-download-and-etag-consistency](../../protocol/coap/checks/blockwise.md#block2-response-download-and-etag-consistency) | wire plus end-to-end | protocol test | Observes sequential `Block2` blocks and mid-transfer `ETag` mismatch restart. |
| [block-size-negotiation-for-block1-and-block2](../../protocol/coap/checks/blockwise.md#block-size-negotiation-for-block1-and-block2) | wire | protocol test | Observes `SZX` negotiation and recomputed `NUM = offset / blockSize`. |
| [blockwise-transfer-error-responses-400-408-413-and-503](../../protocol/coap/checks/blockwise.md#blockwise-transfer-error-responses-400-408-413-and-503) | wire | protocol test | Observes `4.00`, `4.08`, `4.13`, and `5.03` error responses on invalid/expired/oversized block transfers. |
| [combined-block1-upload-and-block2-response-transfer](../../protocol/coap/checks/blockwise.md#combined-block1-upload-and-block2-response-transfer) | wire plus end-to-end | protocol test | Observes chained `Block1` upload and `Block2` response retrieval in one exchange. |
| [request-tag-concurrent-block1-uploads](../../protocol/coap/checks/blockwise.md#request-tag-concurrent-block1-uploads) | wire plus end-to-end | protocol test | Interleaves two `Block1` uploads with distinct `Request-Tag` options (`38`) on the wire. |
| [observe-notification-with-block2-follow-up-retrieval](../../protocol/coap/checks/blockwise.md#observe-notification-with-block2-follow-up-retrieval) | wire plus end-to-end | protocol test | Verifies registration Token on notifications and distinct exchange Token without `Observe` on follow-up `Block2` requests. |
| [observe-and-block2-version-change-supersession-and-recovery](../../protocol/coap/checks/blockwise.md#observe-and-block2-version-change-supersession-and-recovery) | wire plus end-to-end | protocol test | Verifies supersession by newer notifications and jittered re-registration on `ETag` mismatch. |
