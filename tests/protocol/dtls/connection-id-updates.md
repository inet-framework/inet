# Recoverable DTLS 1.3 CID updates

## Scope and requirements

C7 implements [RFC 9147 §9](https://www.rfc-editor.org/rfc/rfc9147.html#section-9) and its independent recovery requirements in [§5.8.4](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.8.4). Enable `connectionIdEnabled` at both endpoints. The initial negotiation remains described in [connection-ids.md](connection-ids.md). NewConnectionId and RequestConnectionId are protected handshake types 10 and 9; their bodies have registered serializers. An ACK retires a transmission, while a NewConnectionId response fulfills a request. Neither substitutes for the other.

## Setup and operations

The protocol cases use two `StandardHost` nodes, real UDP-backed `Dtls` engines, a `PacketTap` on their Ethernet link, 1 ms link delay, seed 0 and declared checksums/FCS. `DtlsTestDriver` calls the public interface and records outcomes; the fixture does not implement protocol recovery. Tap mutations represent modeled authenticated peers because protection is abstract.

```mermaid
flowchart LR
    C[Client driver + Dtls + UDP] --- T[PacketTap: loss, delay, duplicates, malformed bodies]
    T --- S[Server UDP + Dtls + driver]
```

`requestConnectionIds(handle, count)` admits counts 1–255. `provideConnectionIds(handle, count, immediate)` supplies locally allocated IDs; count zero is a valid spare response. `activateSpareConnectionId(handle)` consumes the oldest retained peer spare. Each operation returns synchronous admission, rather than a promise of delivery. Invalid, closing and retired handles refuse the operation. Nonnegotiated or directionally empty IDs prohibit the corresponding wire operation; detected peer violations use alert 10.

The receive index holds at most `maxReceiveConnectionIds` entries per association, configured from 1 to 4, default 4. Spare issuance reserves one unused slot for an immediate replacement. A request receives as many spares as fit, including zero. Immediate replacement takes effect at the receiver before its ACK; the issuer retires its old index entries when that update is acknowledged. Old IDs then fail lookup. Own IDs remain eight bytes and use the engine's lifetime allocation counter. Peer IDs can have other wire lengths; an application operation whose reserved header width changes is skipped with one `sendError`, after any active service finishes. Retained handshake or ACK output that no longer fits fails the association before UDP handoff.

Current/spare peer choices are bounded by the same configured count. Duplicate entries consume no extra peer slots; excess spares are discarded. An immediate list must contain at least one entry, and replaces the old choices. The implementation keeps the latest processed body and sequence for each CID message type to validate fresh-numbered duplicates. It does not retain an unbounded history of retired peer IDs; the issuer is responsible for nonreuse.

## Recovery and failure policy

Ticket, KeyUpdate, CID-update and CID-request flights have separate timers and ACK matching. CID input reassembly uses separate recovery owners while sharing the association's retained-byte bound. A complete CID body remains reserved through logical input service. Identical duplicates arriving during or after service are acknowledged without paying logical handshake cost again. Conflicting content uses alert 47; invalid framing uses alert 50. The initial retry is one second, doubles to 60 seconds, and a fresh protected record number carries each retry.

Only one unacknowledged outgoing update or request is admitted. Request admission also waits for fulfillment. Request deadline expiry stops retry and queued request output, emits `connectionIdRequestTimeout`, and retains the ACK/fulfillment distinction for late responses. Ordinary application datagrams continue. A later response can release admission once the request ACK also arrives. An entirely lost request can leave a gap in ordered post-handshake delivery; this policy does not promise further post-handshake progress without late delivery. Unacknowledged CID replacements and incomplete incoming CID messages fail the owning association at their deadline. A crossing peer request can occupy one deferred response slot while a CID update awaits ACK; another fresh request while that slot is occupied uses alert 52.

Old send counters are retained only while an active flight can retransmit in that epoch. A second KeyUpdate is refused while such a flight still needs an epoch older than the current one; the finite model retains one previous read epoch. Expired request flights retain late-ACK identity without keeping an unused send counter. Close, stop and crash cancel all owners and remove their receive-index entries.

## Direct checks

| Case | Stimulus and required observation |
| --- | --- |
| `Rfc9147ConnectionIdUpdate` | Lose the first request, update, request ACK and update ACK. Require retries, busy refusals, distinct initial/spare/immediate wire IDs, KeyUpdate progress, exactly-once data, and rejection of the retired initial ID. |
| `...Fragments` | Set the UDP limit to 48 bytes, lose one update fragment, require bounded fragments and a reassembled usable spare set. |
| `...InputCost` | Deliver fresh-numbered copies during and after 30 ms logical input service. Require one logical charge and ACKs for both copies. |
| `...Overlap` | Delay a ticket and lose its ACK while starting KeyUpdate and an immediate CID update. Require ticket/CID retransmission and subsequent new-epoch application delivery. |
| `...Unfulfilled`, `...RequestTimeout` | Suppress the response or the request. Require one expiry observation, no further retries, refusal of another request, and continuing data delivery. |
| `...LateFulfillment` | Restore response delivery after expiry. Require a later request to be admitted after fulfillment. |
| `...ZeroResponse` | Limit the server to one receive ID. Require empty spare replies to fulfill requests without manufacturing a usable spare. |
| `...Timeout` | Lose every replacement ACK. Require one owning-association failure and refusal of operations on its retired handle. |
| `...MalformedVector`, `...InvalidUsage`, `...Conflict` | Corrupt an inner length, usage or fresh-numbered processed duplicate. Require alerts 50, 47 and 47 respectively. |
| `...Unnegotiated` | Turn a protected KeyUpdate into a CID request without negotiation. Require alert 10. |
| `...Excessive` | Inject two further fresh requests while the first response is pending. Require the third to exceed the single deferred slot with alert 52. |

The `...` names expand to `Rfc9147ConnectionIdUpdate`. `DtlsConnectionIdUpdateSerializer_1` independently checks exact request/update octets, empty spare encoding, round trips and every truncated prefix. Existing allocator/lifecycle, record, reassembly, ticket and KeyUpdate checks remain controls.

Executed commands and counts belong to the [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md). These checks establish the finite model's behavior, not cryptographic security or interoperability. Migration and path validation are later stages. Release-mode execution, exact-parent trajectory comparisons and fingerprints remain unrun here. Mermaid rendering is unverified.
