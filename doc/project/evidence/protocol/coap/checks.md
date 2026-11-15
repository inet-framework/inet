# CoAP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc7252/catalog.md](../../standard/rfc7252/catalog.md), [rfc6690/catalog.md](../../standard/rfc6690/catalog.md), [rfc7641/catalog.md](../../standard/rfc7641/catalog.md), [rfc7959/catalog.md](../../standard/rfc7959/catalog.md), [rfc9175/catalog.md](../../standard/rfc9175/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specifications only and name no simulation model and no code. This file holds the common mockups, the scenario constants, the rules every check obeys, and the index. The checks themselves live in five files under [`checks/`](checks); the section names are the anchors that the coverage ledger links to.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Message encoding and option delta framing](checks/encoding.md#message-encoding-and-option-delta-framing) | `checks/encoding.md` | RFC7252-FMT-1, FMT-3, FMT-5, RFC7959-OPT-1 |
| [Malformed datagram rejection and silent discard](checks/encoding.md#malformed-datagram-rejection-and-silent-discard) | `checks/encoding.md` | RFC7252-FMT-2, FMT-4, FMT-5, FMT-6, FMT-7, FMT-8, ERR-1, RFC7959-OPT-1 |
| [Non-confirmable request and response](checks/messaging.md#non-confirmable-request-and-response) | `checks/messaging.md` | RFC7252-NON-1 |
| [Response matching by endpoint and token](checks/messaging.md#response-matching-by-endpoint-and-token) | `checks/messaging.md` | RFC7252-MATCH-1 |
| [Confirmable retransmission and exponential backoff](checks/messaging.md#confirmable-retransmission-and-exponential-backoff) | `checks/messaging.md` | RFC7252-CON-1, CON-2, PIGGY-1 |
| [Deferred separate response exchange](checks/messaging.md#deferred-separate-response-exchange) | `checks/messaging.md` | RFC7252-SEP-1, CON-2 |
| [Duplicate suppression and cached acknowledgment](checks/messaging.md#duplicate-suppression-and-cached-acknowledgment) | `checks/messaging.md` | RFC7252-DUP-1 |
| [Empty Confirmable ping and Reset handling](checks/messaging.md#empty-confirmable-ping-and-reset-handling) | `checks/messaging.md` | RFC7252-PING-1, CON-2 |
| [Congestion admission and probing rate limit](checks/messaging.md#congestion-admission-and-probing-rate-limit) | `checks/messaging.md` | RFC7252-CC-1 |
| [Resource methods GET, POST, PUT, and DELETE](checks/resources.md#resource-methods-get-post-put-and-delete) | `checks/resources.md` | RFC7252-METH-1, URI-1 |
| [Critical options, conditional requests, and content negotiation](checks/resources.md#critical-options-conditional-requests-and-content-negotiation) | `checks/resources.md` | RFC7252-URI-1, OPT-1, COND-1, PROXY-1 |
| [CoRE Link Format resource discovery](checks/resources.md#core-link-format-resource-discovery) | `checks/resources.md` | RFC6690-CORE-1, CORE-2, CORE-3 |
| [Observe registration and initial notification](checks/observe.md#observe-registration-and-initial-notification) | `checks/observe.md` | RFC7641-REG-1, REG-2, NOTIFY-1 |
| [Notification ordering and 128-second rule](checks/observe.md#notification-ordering-and-128-second-rule) | `checks/observe.md` | RFC7641-ORDER-1 |
| [Notification freshness and re-registration](checks/observe.md#notification-freshness-and-re-registration) | `checks/observe.md` | RFC7641-FRESH-1 |
| [Confirmable notification supersession and retry removal](checks/observe.md#confirmable-notification-supersession-and-retry-removal) | `checks/observe.md` | RFC7641-SUPER-1 |
| [Observation cancellation by Observe=1, Reset, and error](checks/observe.md#observation-cancellation-by-observe1-reset-and-error) | `checks/observe.md` | RFC7641-CANCEL-1 |
| [Block1 atomic request upload](checks/blockwise.md#block1-atomic-request-upload) | `checks/blockwise.md` | RFC7959-B1-1 |
| [Block2 response download and ETag consistency](checks/blockwise.md#block2-response-download-and-etag-consistency) | `checks/blockwise.md` | RFC7959-B2-1, ERR-1 |
| [Block size negotiation for Block1 and Block2](checks/blockwise.md#block-size-negotiation-for-block1-and-block2) | `checks/blockwise.md` | RFC7959-NEG-1 |
| [Blockwise transfer error responses 4.00, 4.08, 4.13, and 5.03](checks/blockwise.md#blockwise-transfer-error-responses-400-408-413-and-503) | `checks/blockwise.md` | RFC7959-OPT-1, ERR-1 |
| [Combined Block1 upload and Block2 response transfer](checks/blockwise.md#combined-block1-upload-and-block2-response-transfer) | `checks/blockwise.md` | RFC7959-COMB-1 |
| [Request-Tag concurrent Block1 uploads](checks/blockwise.md#request-tag-concurrent-block1-uploads) | `checks/blockwise.md` | RFC9175-RTAG-1 |
| [Observe notification with Block2 follow-up retrieval](checks/blockwise.md#observe-notification-with-block2-follow-up-retrieval) | `checks/blockwise.md` | RFC7959-OBS-1 |
| [Observe and Block2 version change supersession and recovery](checks/blockwise.md#observe-and-block2-version-change-supersession-and-recovery) | `checks/blockwise.md` | RFC7959-OBS-1 |

Twenty-five specification checks cover all 41 statements in the five catalogs. Two additional implementation checks (`CoapLifecycle.test` and `CoapStatistics.test`) verify node lifecycle recovery and emitted signal/statistic counts.

## Common mockups

Two mockups serve the checks.

### The plain pair mockup with a relay

One client endpoint `C` (`host1`) and one server endpoint `S` (`host2`) on a link with a transparent relay `T` (`tap`) between them.

```
   client C  ------  relay T  ------  server S
```

- On normal-path checks, `T` passes every datagram unchanged.
- On level 3 edge checks, `T` drops, delays, duplicates, or mutates selected UDP datagrams in flight, or a test peer injects crafted CoAP datagrams with specific wire octets.

### The multi-peer switch mockup

One client or server endpoint `H` (`host1`) connected through a relay `T` (`tap`) and an Ethernet switch to multiple peer endpoints `P[0..k]` (`peer[*]`), used when a check needs a foreign source endpoint, multiple concurrent observers, or independent per-destination congestion state.

## Rules every check obeys

- **One observation confirms the stimulus.** Every check records the initial request or injected datagram on the wire before judging the peer's response or silence.
- **Default constants.** Unless a check states otherwise, the CoAP server listens on UDP port `5683`, `ACK_TIMEOUT = 2s`, `ACK_RANDOM_FACTOR = 1.5`, `MAX_RETRANSMIT = 4`, `NSTART = 1`, `DEFAULT_LEISURE = 5s`, `PROBING_RATE = 1 byte/s`, `EXCHANGE_LIFETIME = 247s`, and `NON_LIFETIME = 145s`.
- **Wire-level verification.** Header fields, option deltas/lengths, and payload markers are checked both as dissected fields and as serialized wire octets.
