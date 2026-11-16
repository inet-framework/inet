# DTLS 1.2 connection IDs, heartbeat and path budgets

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [extension profile](../../../doc/project/evidence/standard/rfc9147/extensions.md), [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md)

This is C15 of the [plan](../../../plan/pending/dtls-extensions.md). The [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md) records commands and actual outcomes. The same association, socket, bounded probe slot and path-budget state serve both selected versions. There is no legacy ACK, KeyUpdate or dynamic CID-update exchange.

## Records and negotiation

RFC 9146 adds outer content type 25 and a peer-chosen CID before the two-byte length. Its length is negotiated, not encoded in each header. Protected records retain the full epoch and 48-bit number, eight-byte explicit nonce and modeled sixteen-byte tag. An encrypted inner content type adds one byte. Non-CID overhead is 37 bytes; CID overhead is 38 plus the outgoing CID length. The entire inner plaintext is at most 16384 bytes, giving 16383 application bytes with no padding.

Local receive IDs are eight bytes or empty. Outgoing IDs support lengths 1–255. A nonempty negotiated receive ID requires CID-format protected records. Every full and resumed handshake negotiates fresh IDs; close and lifecycle cleanup remove their indexes. Heartbeat permission follows the Hello offers. Path validation requires a fresh correlated response before changing the destination. Protection remains a retained-content simulation abstraction.

`Dtls12CidSerializer_1.test` checks independent header octets and length boundaries. `Dtls12CidProtection_1.test` checks nonce, inner limits, empty/tagged content and unchanged input on failure. Production tests below use real UDP/IP and actual retained protocol chunks, declared checksums/FCS, seed zero and directional processing costs.

| Cases | Setup, stimulus and required observation |
| --- | --- |
| `Rfc9146ConnectionId`, `Certificate`, `Resumption` | Full PSK, certificate and abbreviated exchanges select 1.2. Check actual type-25 fields and application delivery; resumption allocates fresh IDs. |
| `ConnectionIdEmptyClient`, `EmptyServer` | One endpoint offers empty receive CID. Require directional 1154/1163-byte application budgets and delivery at each boundary. |
| `ConnectionIdPeerLength` | Edit ServerHello to advertise three bytes. Require the client's protected Finished to use a 16-byte header and that CID. This proves outgoing foreign-length encoding only: the unchanged peer still owns its eight-byte index. |
| `ConnectionIdTags`, `InnerLimit` | Preserve region tags on actual output/delivery; allow 16383 application bytes with IP fragmentation when discovery is off, refuse 16384 and retain later usable data. |
| `ConnectionIdUnknown`, `Missing`, `InnerType`, `WrongVersion` | Mutate real records to an unknown/missing CID, invalid inner type or live 1.3 association's CID. Require drop and surviving valid traffic. |
| `ConnectionIdApiBounds`, `Lifecycle` | Refuse 1.3-only APIs; stop/crash and restart the server, requiring distinct IDs and usable new associations. |
| `ConnectionIdExhaustion` | Set only the server's test allocator counter to zero before the real cookie exchange. Forbid malformed classification, require RESOURCE_LIMIT and empty admission/index state with no accepted callback. Restore the injected counter and require the client's genuine retry to establish once and deliver data. |
| `ConnectionIdClose`, `Fatal`, `Renegotiation` | Observe protected closure, fatal decode alert, or warning no_renegotiation after a captured ClientHello. The warning preserves data. |

Grouped names retain the `Rfc9146` prefix and `.test` suffix. Each fixture gives its exact schedule and assertions.

## Paths and routing

```mermaid
flowchart LR
    C[Client eth0 / eth1] <-->|two selectable links| R[Router]
    R <-->|controlled egress MTU| S[Server]
```

`DtlsPathMtuNetwork` has two client interfaces, three router interfaces and one server interface. IPv4 uses 10.0.1/24 and 10.0.2/24 for the client links, and 10.0.3/24 for the server link. GlobalArp supplies resolution. Explicit host routes select the initial gateway; the combined migration fixture changes both interface and gateway to the second link. IPv6 uses 2001:db8:1::/64, :2::/64 and :3::/64, with DAD disabled and an explicit client /128 route to the server. The combined fixture updates its route before requesting the move.

`Rfc9146Migration` translates UDP ports at a tap to model IPv4 NAT. `MigrationOldRecord`, `Competing`, `NoPermission` and `FreshResponse` check replay freshness, candidate isolation, missing probe permission and a delayed response below the accepted-record high-water mark. A rejected path must not poison replay admission. `MigrationRouted` and `Ipv6` use real source/interface selection; `Rebind` and `RebindTimeout` exercise socket replacement and its failure policy. Successful validation preserves the handle. Static legacy CID reuse requires no spare-CID request.

## Heartbeat and MTU

`Rfc6347Heartbeat` and `HeartbeatCid` require exact 1200-byte requests and smaller 64/73-byte responses with the selected overhead. `Permissions`, `WrongResponse`, `MalformedLength` and `Timeout` check admission, token matching, strict lengths and bounded retry. All retain one shared probe slot.

`Rfc6347PathMtu` and `NoCid` grow from a 512-byte UDP base. `Ipv4Hint` uses real router feedback with a 96-byte quote, enough for IP/UDP, CID header, nonce and token prefix. `Ipv6` uses actual PTB and subtracts 48 consumed IPv6/UDP bytes: MTU 1280 yields a confirmed 1232-byte UDP budget, or 1186 application bytes with an eight-byte CID. Discovery permits no source fragment. `ShortQuote` and `StaleQuote` ignore insufficient/obsolete evidence. `Blackhole`, `Isolation` and `Lifecycle` cover suppressed feedback, progressing peers and cleanup.

`PathMtuMigration` confirms the old path, lowers router MTUs to 620, validates the move, rejects queued oversized output and delivers fitting traffic. Both directions reset to base and independently confirm 546 application bytes. `PathMtuMigrationIpv6` lowers relevant egress MTUs from 2000 to 1280. Explicit interface selection in this checkout's IPv6 model emits Ethernet broadcasts; ICMPv6 suppresses feedback for those frames. This fixture requires both probe timeouts and conservative confirmation of 1152 UDP bytes (1106 application bytes), then usable reverse data. The separate fixed-path case owns the successful IPv6 PTB claim.

These checks do not prove cryptographic authentication of a changed path or ICMP, real-endpoint interoperability, arbitrary CID receive lengths, IPv6 NAT or hardware performance. Mermaid rendering has not been checked.

## C15b resource-admission correction

The final integration audit found that legacy initialization threw when it could not allocate a server receive ID. The queued reassembly guard classified this as malformed input and retained prevalidation. The same fixture fails on the original library with a forbidden MALFORMED_RECORD at 0.26607968 s, then passes after moving allocation to the server admission owner. The corrected owner removes prevalidation and reports RESOURCE_LIMIT before inserting an association or invoking accepted. Normal allocation, numbering and wire output keep the same sequence of operations in the same event.

The fixture bypasses only allocator exhaustion/rearming, not the Hello exchange, cookie validation, input service, retry or data path. Rearming the injected counter is test fault recovery; production namespaces still never rewind. Parent/child fixture hashes, matching builds, raw outputs and complete integration outcomes are recorded in the execution ledger.
