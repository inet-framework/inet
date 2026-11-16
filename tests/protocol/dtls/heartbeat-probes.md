# Correlated DTLS heartbeat probes

## Scope and protocol

C8 implements the internal probe primitive for later migration and PMTU consumers. It adds [RFC 6520 §§2–4](https://www.rfc-editor.org/rfc/rfc6520.html#section-2) negotiation, body serialization, directional permissions, payload correlation, bounded responses and retransmission through the production DTLS engine. In 1.3, extension 15 appears in ClientHello and EncryptedExtensions, as listed by the [IANA extension registry](https://www.iana.org/assignments/tls-extensiontype-values/). Heartbeat uses protected record content type 24, independently of handshake ACKs. DTLS 1.2 support belongs to C15.

`heartbeatEnabled` defaults to false. When enabled, `heartbeatAcceptRequests` selects advertised mode 1 or 2. Both endpoints must negotiate support, and an endpoint may initiate requests only when its peer advertised permission. Unknown modes produce alert 47; invalid extension lengths produce alert 50. Replies without a client offer produce alert 110. Requests received contrary to local permission are silently discarded. Unknown message types, malformed lengths and unmatched or unsolicited responses cannot complete a probe or reach the application.

Each association has one shared probe slot. The engine allocates an eight-byte correlation token from a lifetime counter, which does not rewind on close or restart. This is modeled correlation, not an unpredictable security token. A request's chosen UDP size is carried in padding; the response echoes only its actual payload and supplies fresh 16-byte padding. Both encodings enforce the 16 KiB body bound and minimum padding. Response output must fit the configured UDP budget. Different directions can have different CID overheads and budgets.

## Ownership, queues and expiry

The association owns its probe payload, queued state, retry timer and deadline. It allocates timers only when starting a probe. The initial retry interval is one second and doubles to 60 seconds. Every transmission uses a fresh protected record number; retries keep the correlation payload. `probeTimeout`, default 16 seconds and constrained to at least one second, starts at the first UDP handoff. No automatic keepalive or public application probe API is introduced.

Requests and responses pay the selected suite's record output cost. Input passes through the ordinary deprotection queue and cost; heartbeat does not pay logical handshake cost. An operation checks association, probe identity, epoch-update eligibility and reserved header width at service completion. An expired active retry finishes its sampled service before being skipped. It cannot send a packet or finish a newer probe occupying the same slot. Queue admission failure releases the slot and preserves ordinary application traffic. Close, stop and crash use the association's existing timer/queue cleanup; the direct new lifecycle case exercises close and reconnect.

`probeResult` reports the stable `IDtls::ProbeResult` values: success 1, timeout 2 and local output refusal 3. These are aggregate engine observations; later internal path consumers retain their association context. Probe expiry preserves the association. This deliberately differs from RFC 6520 §3's recommendation to terminate after exhausted heartbeat recovery: a failed candidate path or size probe must not destroy a usable old path. PHA suspension is a C10 obligation and has not been implemented or validated by C8.

## Reproduction and observations

The protocol tests use production `Dtls`, UDP, IP and Ethernet. `ProbeDtls` exposes only the protected production initiation method so the primitive can be tested before its C9/C11 consumers exist. Its composed test app uses the existing driver and dispatcher. The wrapper supplies no handshake, probe, record or retry implementation.

```mermaid
flowchart LR
    C[Client Dtls + UDP] ---|1 ms half-link| T[PacketTap]
    T ---|1 ms half-link| S[Server UDP + Dtls]
```

Runs pin seed 0 and declared checksums/FCS. Positive-cost cases use 5 ms output and 3 ms input record costs. The 100 Mbps link has two half-links in each direction, including serialization at the tap. The main case sends a 1,200-byte UDP payload and receives a 57-byte response despite the server's smaller 512-byte send budget. Its expected completion time includes 16 ms processing, 4 ms propagation and both directions' Ethernet serialization.

| Case | Setup, stimulus and required observation |
| --- | --- |
| `Rfc9147Heartbeat` | Enable CID and heartbeat, request 1,200 bytes, refuse a concurrent request, compare the real payload echo, require 16-byte response padding and exact processing/link time, then deliver ordinary data. |
| `...Permissions` | Four independent pairs cover either disabled endpoint and both asymmetric permission choices. All establish; exactly the two permitted probe directions complete. |
| `...Timeout` | Drop all responses. Require three fresh-numbered requests at the initial/1/2-second retry schedule before a four-second deadline, continued data and a newly available slot. |
| `...WrongResponse` | Change the first response token. Require no early completion and successful retry with the correct token. |
| `...Unsolicited` | Deliver a delayed, fresh-numbered copy after completion. Require both responses to arrive and only the original completion. |
| `...MalformedLength` | Replace a real request body with wire bytes claiming a 65,535-byte payload while retaining its actual short size. Require no echo for that record and successful later retry. |
| `...InvalidOffer`, `...InvalidSelection` | Corrupt mode in ClientHello or EncryptedExtensions. Require alert 47 and failed establishment. |
| `...Admission` | Limit the client's processing capacity to 512 bytes. Refuse a 1,200-byte reservation, then complete a small probe and ordinary data. |
| `...QueuedExpiry` | Drop the first response and let a retry start three seconds of service. Expire it after two seconds and admit a newer probe. Require two full long charges, no old retry handoff, and only the newer completion. |
| `...Isolation` | Two clients share one server engine. Suppress responses for one client while the other exchanges ordinary data before probe expiry. |
| `...Lifecycle` | Close after an actual response was sent and suppressed. Wait past the old deadline, reconnect and require a distinct token and one successful new completion. |
| `...Resumption` | With a 32-byte modeled certificate fitting the no-cookie amplification budget, negotiate SHA-384 certificate-origin resumption, CID and early data. Require separate early/ordinary deliveries and a successful post-resumption probe. |

The `...` names expand to `Rfc9147Heartbeat`. `DtlsHeartbeatSerializer_1` uses an independent octet oracle, a round trip, every truncated prefix and an overclaimed length. Existing handshake, early-data, ticket, CID, KeyUpdate, selection and lifecycle cases are regression controls. Executed selections, commands and counts belong to the [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md).

These are deterministic finite-model checks. They do not prove cryptographic security, live-peer interoperability or realistic processing performance. Migration, routed size discovery, PHA suspension, active-probe stop/crash scenarios, release execution, exact-parent comparisons and fingerprint comparisons remain outside this stage's executed evidence. Mermaid rendering is unverified.
