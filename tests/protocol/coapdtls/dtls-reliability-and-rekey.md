# CoAP over DTLS Confirmable Reliability, Ping, and KeyUpdate

This document explains the production change in commit C3 and the protocol test [Rfc7252DtlsReliabilityAndRekey.test](Rfc7252DtlsReliabilityAndRekey.test) covering RFC 7252 Sections 4.2, 4.3, 4.8 and RFC 9175 Section 4.2 over DTLS 1.3.

## Production Implementation

- **DTLS Session Resolution in `sendConfirmablePing()` ([CoapEndpointBase.cc](../../../src/inet/applications/coap/CoapEndpointBase.cc)):** Resolves `peer.sessionId` through `resolveDtlsPeerSession()` when `isDtlsMode()` is active (or verifies an explicit session ID is still open and non-closing) before submitting Empty `CON` ping messages to the outbound queue.
- **Unchanged Backoff & Token Monotonicity:** Exponential backoff retries reuse `pumpOutboundQueue()` and `processConfirmableRetransmissions()` cleanly on the resolved `AssociationId`, and `allocateToken()` generates monotonic 8-byte tokens without reuse across DTLS epoch rotations.

## Test Verification

- **Dropped DTLS Application Request & ACK + Duplicate Suppression:** Drops the first client `CoapRequest` record and first server `CoapResponse` record at `tap`; verifies client CON retry backoff (`retransmission == 2`), single server `PUT` execution (`nonDupPutRequests == 1`), `duplicateReceived == 1`, and cached ACK replay inside a new DTLS record.
- **Malformed CoAP & Empty CON Ping Inside Valid DTLS Records:** Uses `CoapDtlsTestPeer` to inject malformed `CON` (`TKL = 9`), malformed `NON` (`TKL = 9`), and Empty `CON` (`0.00`), verifying `RST` for CON / Empty and silent discard for malformed `NON`, plus `RelTestClient::sendConfirmablePing()` receiving peer `RST`.
- **DTLS 1.3 KeyUpdate Across CoAP Requests:** Sets `rekeyAfterRecords = 2` on client and server and issues 5 sequential `CON GET` requests; verifies `DtlsCiphertextHeader` `epochBits` advances `3 -> 0 -> 1` (epochs `3 -> 4 -> 5`), all 5 complete (`failedCount == 0`) with 5 unique 8-byte tokens.
