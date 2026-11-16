# CoAP over DTLS Observe and Blockwise Session Binding

This document explains the production changes in commit C4 and the protocol tests verifying RFC 7641 (Observe), RFC 7959 (Blockwise), and RFC 9175 Section 3 (`Request-Tag`) over DTLS 1.3.

## Production Implementation

- **Client Session Teardown ([CoapClient.h](../../../src/inet/applications/coap/CoapClient.h), [CoapClient.cc](../../../src/inet/applications/coap/CoapClient.cc)):**
  - `onDtlsSessionClosing(id, reason)` drains `queuedBlockTransfers` matching `id` and invokes `onRequestFailed()` before active requests fail so `failLogicalTransfer()` never launches queued transfers onto a dying association.
  - `onDtlsSessionClosed(id, reason)` terminates any remaining `observationsByPeerToken` entries bound to `id` via `terminateObservation()`, clearing `observationsByCacheKey` and emitting `activeObservationsSignal`.
- **Server Session Teardown ([CoapServer.cc](../../../src/inet/applications/coap/CoapServer.cc)):** `onDtlsSessionClosed(id, reason)` purges `pendingSeparateResponses`, removes `observers` matching `id` via `removeObserverAndEmit()`, and erases `incomingBlock1` and `retainedBlock2` contexts bound to `id`.

## Test Verification

- **Observe Over DTLS ([Rfc7641DtlsObserve.test](Rfc7641DtlsObserve.test)):** Verifies subscription, multiple resource updates crossing DTLS `KeyUpdate` (`rekeyAfterRecords = 2`), notification supersession when the client's `CoapAck` is dropped and a newer state update supersedes it, and post-close termination (`onObserveTerminated` on client, `activeObservers == 0` on server, and zero notifications on subsequent mutations).
- **Blockwise & Request-Tag Over DTLS ([Rfc7959DtlsBlockwise.test](Rfc7959DtlsBlockwise.test)):** Verifies `3 KiB` `Block1 PUT` and `Block2 GET` with `preferredBlockSize = 1024B`, two concurrent `Request-Tag` `Block1` uploads over one DTLS association without block splicing, `Observe + Block2` notifications, and `4.08 Request Entity Incomplete` rejection when session 2 sends an orphaned `Block1 (NUM=1)` after session 1 closed.
