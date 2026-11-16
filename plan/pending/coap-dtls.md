# Implementation plan: CoAP over DTLS

> **Kind:** design · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [write-an-implementation-plan.md](../../doc/project/guide/write-an-implementation-plan.md), [add-a-protocol.md](../../doc/project/guide/add-a-protocol.md), [planning.md](../../doc/project/rule/planning.md), [pull-request.md](../../doc/project/rule/pull-request.md#pr-series-plan), [testing.md](../../doc/project/rule/testing.md#tr-plan-protocol)

Checkout and revision: `/home/miguel/workspace/inet-coap-dtls` (`feat/coap-dtls` at `24af974956bee37b004ee4cc486a3f9456faed4d`, with preserved local `AGENTS.md` edit).
Plan revision: 2026-11-16, revision 1 (approved).
Approval: approved by user in conversation on 2026-11-16 ("Execute the plan"), covering commits `C1`–`C7`, session-scoped `AssociationId` binding across DTLS 1.3 `KeyUpdate`, and `Dtls.h` lifecycle stage alignment (`C2`). No sealed path is modified.

## 1. Problem and intended behavior

INET currently provides CoAP over unsecured UDP (`NoSec`, default port `5683`) via [CoapEndpointBase](../../src/inet/applications/coap/CoapEndpointBase.ned), [CoapClient](../../src/inet/applications/coap/CoapClient.ned), and [CoapServer](../../src/inet/applications/coap/CoapServer.ned), and separately provides a DTLS 1.3 transport engine via [Dtls](../../src/inet/transportlayer/dtls/Dtls.ned) and [IDtls](../../src/inet/transportlayer/contract/dtls/IDtls.h). Users cannot currently simulate `coaps://` (RFC 7252 §9.1, default UDP port `5684`) over INET's DTLS 1.3 model to study handshake latency, cookie retries, certificate overhead, DTLS record expansion, processing delays, or session isolation alongside CoAP `CON`/`NON` reliability, Observe (`RFC 7641`), Block1/Block2 (`RFC 7959`), and `Request-Tag` (`RFC 9175`).

Intended behavior:

1. A user places `CoapDtlsClient` and `CoapDtlsServer` (compound `IApp` modules) in ordinary INET host `app[]` slots (IPv4 or IPv6), configures CoAP parameters on `app[i].coap.*` and DTLS 1.3 parameters (`profile = "psk"` or `"certificate"`, `clientAuthentication`, `cookieRequired`, `udpPayloadLimit`, `protectOutDelay`, `rekeyAfterRecords`) on `app[i].dtls.*`.
2. When `CoapDtlsClient` issues a request (for example `GET coaps://server:5684/sensors/temp` or `/sensors/temp` with default `peerPort = 5684`), it opens or reuses a DTLS 1.3 association for `(peerAddress, peerPort)` via `IDtls`, holds CoAP message dispatch (without starting CoAP `CON` retransmission timers prematurely) until DTLS reports `established(id)`, and then transmits CoAP messages protected inside DTLS application records (`contentType = 23`, outer protocol `dtlsProtocol`).
3. On lossy links, lost DTLS handshake records are recovered by `Dtls`, whereas lost DTLS application records are recovered by CoAP `CON` exponential-backoff retransmissions inside the established DTLS association, with CoAP duplicate suppression (`MID`) and response matching (`Token`) strictly bound to that DTLS association (`AssociationId`).
4. When a DTLS association closes (`close_notify`, fatal alert, handshake timeout, or same-tuple replacement), both client and server tear down every CoAP exchange, pending request, duplicate tombstone, Observe relation, and Block1/Block2 context bound to that `AssociationId`, preventing cross-session leakage or splicing.
5. Ordinary UDP CoAP (`dtlsModule = ""`, `Coap` feature) and standalone DTLS (`Dtls` feature) remain unchanged and independently buildable with the other disabled.

## 2. Current implementation and evidence

Direct inspection of the checkout confirms both subsystems exist and exposes the exact integration boundaries:

| Component / file | Verified current behavior and constraint |
| --- | --- |
| [CoapEndpointBase.h](../../src/inet/applications/coap/CoapEndpointBase.h) / [CoapEndpointBase.cc](../../src/inet/applications/coap/CoapEndpointBase.cc) | Inherits `ApplicationBase` and `UdpSocket::ICallback`. Owns `UdpSocket socket`, binds `(localAddress, localPort)` in `bindSocket()`, sends with `socket.sendTo()` after tagging `PacketProtocolTag = &getCoapProtocol()`, and reads `L3AddressInd`/`L4PortInd` in `socketDataArrived()`. Keys all exchanges, pending requests, congestion state, and duplicates by `CoapPeerEndpoint{address, port}`. Generates 8-byte monotonic RNG-seeded tokens in `allocateToken()` preserved across stop/crash/restart. |
| [CoapClient.cc](../../src/inet/applications/coap/CoapClient.cc) / [CoapClient.ned](../../src/inet/applications/coap/CoapClient.ned) | `peerPort` defaults to `5683`. `parseCoapUri()` accepts only `"coap"` and throws `"Unsupported CoAP URI scheme"` for `"coaps"`. `CoapClient` maintains `observationsByPeerToken`, `observationsByCacheKey`, `activeBlockTransfers`, and `queuedBlockTransfers` keyed by `CoapPeerEndpoint`. |
| [CoapServer.cc](../../src/inet/applications/coap/CoapServer.cc) / [CoapServer.ned](../../src/inet/applications/coap/CoapServer.ned) | `localPort` defaults to `5683`. Maintains `pendingSeparateResponses`, `observers`, `incomingBlock1`, and `retainedBlock2` keyed by `CoapPeerEndpoint` / `CoapObserverKey` / `CoapBlockRequestKey`. |
| [CoapProtocol.cc](../../src/inet/applications/coap/CoapProtocol.cc) / [UdpProtocolDissector.cc](../../src/inet/transportlayer/udp/UdpProtocolDissector.cc) | `CoapProtocol.cc` registers `COAP_UDP_PORT` (`5683`) in `ProtocolGroup::getUdpProtocolGroup()`. Port `5684` is not registered, and MUST NOT be registered as cleartext CoAP because `UdpProtocolDissector` inspects `findProtocol(port)` for UDP payloads, whereas UDP port `5684` carries `dtlsProtocol` records. |
| [IDtls.h](../../src/inet/transportlayer/contract/dtls/IDtls.h) / [DtlsSocket.h](../../src/inet/transportlayer/contract/dtls/DtlsSocket.h) | Non-owning facade over `IDtls` (`listen`, `open`, `send`, `close`, `requestKeyUpdate`) and `IDtls::ICallback` (`accepted`, `established`, `dataArrived`, `sendError`, `error`, `closed`). `open()` returns nonzero `AssociationId` synchronously or `0` on admission refusal. `IDtls` does not expose record epoch numbers. |
| [Dtls.h](../../src/inet/transportlayer/dtls/Dtls.h) / [Dtls.cc](../../src/inet/transportlayer/dtls/Dtls.cc) | Implements `IDtls` and owns `UdpSocket udpSocket`. `send()` queues packets before `ESTABLISHED` (up to `maxPendingBytes`), rejects payloads with `bytes + 22 > udpPayloadLimit` (default `1200B`, so max inner payload is `1178B`) via `callback->sendError(id, packetId, ...)`, and calls `packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol)` at `Dtls.cc:910`. `dataArrived(id, incoming)` passes `Packet*` without `L3AddressInd`/`L4PortInd` or `PacketProtocolTag`. |
| [Dtls.h](../../src/inet/transportlayer/dtls/Dtls.h#L80-L82) / [ModuleOperations.h](../../src/inet/common/lifecycle/ModuleOperations.h) | `Dtls` currently overrides `isInitializeStage`, `isModuleStartStage`, and `isModuleStopStage` to `INITSTAGE_APPLICATION_LAYER` / `STAGE_APPLICATION_LAYER`, the same stage as `ApplicationBase` (`CoapEndpointBase`). Because `ModuleStopOperation` runs top-down and visits same-stage submodules in declaration order, `Dtls` would stop before `CoapEndpointBase` if `dtls` precedes `coap`, causing `Dtls::handleStopOperation()` to fire `error`/`closed` callbacks synchronously before `CoapClient::handleStopOperation()` can record `requestAborted`. |
| [DtlsApp.ned](../../src/inet/applications/dtlsapp/DtlsApp.ned) / [decisions.md](../../doc/project/design/decisions.md#d-dtls-composition) | Establishes INET's pattern (`D-DTLS-COMPOSITION`): a compound `IApp` module embeds `dtls: <default("Dtls")> like IDtls` and connects `socketIn`/`socketOut` to `dtls`, while an application sibling accesses `dtls` via `dtlsModule` (`IDtls` contract, `AR-COM-SOCKETS`). |
| [seal-list.md](../../doc/project/audit/seal-list.md) | Only `src/inet/common/packet/` is sealed. `src/inet/applications/coap/`, `src/inet/transportlayer/dtls/`, and `src/inet/transportlayer/contract/dtls/` are unsealed. |
| [coap/standards.md](../../doc/project/evidence/protocol/coap/standards.md) / [coap/conformance.md](../../doc/project/evidence/model/coap/conformance.md) | Explicitly mark `RFC 7252 §9` (`coaps://` over DTLS) and `RFC 9175 §4` as out of scope of the UDP `NoSec` profile. |

## 3. Scope and acceptance criteria

### Standards profile

This work extends the existing CoAP profile in [standards.md](../../doc/project/evidence/protocol/coap/standards.md) with `coaps://` over DTLS 1.3:

| Included in this series | Explicit follow-up / out of scope |
| --- | --- |
| `RFC 7252 §6.2, §12.6`: `coaps://` URI scheme, default UDP port `5684`, `Uri-Host` / `Uri-Port` handling | `RawPublicKey` DTLS mode (`RFC 7252 §9.1.3.2`) — INET `Dtls` supports `"psk"` and `"certificate"` only |
| `RFC 7252 §9.1`: Unicast-only DTLS protection; multicast rejected in DTLS mode | DTLS 1.2 (`RFC 6347`) and DTLS Connection IDs (`RFC 9146`) |
| `RFC 7252 §9.1.1, §9.1.2` (session-scoped): CoAP `CON`/`NON`/`ACK`/`RST`, duplicate suppression, `NSTART`/`PROBING_RATE`, and response matching bound to `IDtls::AssociationId` | Aborting CoAP exchanges/Observe across DTLS 1.3 `KeyUpdate` epochs (see Section 4) |
| `RFC 9147` (via INET `Dtls` abstraction): PSK and Certificate profiles, optional client auth, cookie validation, handshake recovery, record overhead, processing delays, and `KeyUpdate` | Real cryptographic AEAD/signatures, computed UDP/MAC checksums over protected records, and external wire interoperability (inherited from [dtls/scope.md](../../doc/project/evidence/model/dtls/scope.md)) |
| `RFC 9175 §4`: Unique non-reused 8-byte client Tokens across requests and `KeyUpdate` rotations | `RFC 9175 §2` (`Echo` option `252`), `RFC 8613` (OSCORE), `RFC 8323` (CoAP over TCP/TLS/WS) |
| `RFC 7641` Observe, `RFC 7959` Block1/Block2, `RFC 9175 §3` `Request-Tag`, and `RFC 6690` discovery over DTLS, with session-close cleanup | Cross-protocol CoAP↔CoAPS forward/reverse proxying |

### Acceptance criteria

1. Every planned protocol test under `tests/protocol/coapdtls/` for DTLS handshake/exchange, session isolation, `CON` retry/dedup/`KeyUpdate`, Observe/Blockwise over DTLS, and DTLS lifecycle/statistics compiles and passes in debug mode, with each behavior commit paired with its reviewer `.md` document.
2. Existing CoAP UDP protocol tests (`tests/protocol/coap/`), DTLS unit/module/protocol tests (`tests/unit/`, `tests/module/`, `tests/protocol/dtls/`), and the new `DtlsApplicationOrderLifecycle_1.test` module test compile and pass without regressions.
3. Both `MODE=debug` and `MODE=release` build cleanly; feature-matrix builds succeed with `Coap=ON, Dtls=OFF, CoapDtls=OFF`, `Coap=OFF, Dtls=ON, CoapDtls=OFF`, `Coap=ON, Dtls=ON, CoapDtls=OFF`, and all enabled.
4. `examples/coapdtls/` scenarios run deterministically; no sealed path (`src/inet/common/packet/`) is modified; architecture, naming, interface, PR-series, and whatsnew gates pass.

## 4. Proposed design and reasons

### Component reuse and additions

| Proposed addition | Responsibility | Required consumer | Reuse option | Reason for choice |
| --- | --- | --- | --- | --- |
| `dtlsModule` parameter + `#ifdef INET_WITH_DTLS` `IDtls::ICallback` / `DtlsSocket` adapter path in `CoapEndpointBase` | Select UDP vs DTLS transport, map `AssociationId <-> CoapPeerEndpoint`, gate outbound dispatch on `established()`, route `sendError`/`closed`/`error` | `CoapClient`, `CoapServer` | Separate `CoapDtlsClient`/`CoapDtlsServer` C++ subclasses or a new `ICoapTransport` module interface | `CoapClient` and `CoapServer` both inherit `CoapEndpointBase`; separate subclasses would duplicate all session/callback logic (`QR-DUP`), while a new C++ module interface adds an extra submodule and interface contract for one internal transport switch (`AR-EXT-REUSE`, `QR-DESIGN-MINIMAL`). `#ifdef INET_WITH_DTLS` keeps `Coap` buildable without `Dtls` while including only `DtlsSocket.h` from `transportlayer/contract/dtls/` (`AR-COM-SOCKETS`). |
| `uint64_t sessionId = 0` field in `CoapPeerEndpoint` | Isolate DTLS sessions (`AssociationId`) in duplicate keys, pending requests, exchanges, congestion state, Observe keys, and Block1/Block2 keys | `CoapEndpointBase`, `CoapClient`, `CoapServer` | Separate per-session shadow maps in `CoapClient`/`CoapServer` | `sessionId = 0` preserves UDP behavior unchanged while making every existing map/key in `CoapEndpointBase`, `CoapClient`, and `CoapServer` automatically session-scoped (`QR-STATE-OWNER`, `QR-DESIGN-MINIMAL`). |
| `CoapDtlsClient.ned` and `CoapDtlsServer.ned` in `inet.applications.coapdtls` (`CoapDtls` feature) | Compound `IApp` modules wiring `coap` (`CoapClient`/`CoapServer`) to `dtls` (`<default("Dtls")> like IDtls`) | Host `app[]` slots and `examples/coapdtls` | Putting `CoapDtls*.ned` directly in `inet.applications.coap` | Compound NED modules follow `D-DTLS-COMPOSITION` and `DtlsApp.ned`; isolating them in `inet.applications.coapdtls` under `CoapDtls` (`requires = "Coap Dtls"`) prevents NED load failure when `Dtls` is disabled (`AR-EXT-FEATURES`). |
| `CoapDtlsExamples` feature + `examples/coapdtls/` | Runnable IPv4/IPv6/PSK/Certificate/Lossy CoAP-over-DTLS scenarios | Simulation users | Adding DTLS configs to `examples/coap/` | Keeping `CoapDtlsExamples` separate preserves `CoapExamples` when `Dtls` is disabled (`AR-EXT-FEATURES`). |
| `Dtls` lifecycle stage shift (`STAGE_TRANSPORT_LAYER`, `INITSTAGE_TRANSPORT_LAYER`) | Ensure `Dtls` initializes/starts before `ApplicationBase` consumers and stops after them | `CoapEndpointBase`, `DtlsSocketIo` | Submodule-order workarounds or string-matching `"node crashed"` in callbacks | Aligns `Dtls` with INET's bottom-up start / top-down stop lifecycle architecture ([ModuleOperations.h](../../src/inet/common/lifecycle/ModuleOperations.h), `AR-LIFE-OPERATIONS`). |

### Transport, handshake gating, and packet tags

1. **Client session lookup and handshake gating:** In DTLS mode (`!dtlsModule.empty()`), `CoapClient` resolves `(destAddr, destPort)` through `activeClientDtlsByAddrPort`. If no open, non-closing session exists, `CoapEndpointBase` calls `dtlsSocket.open(destAddr, destPort)`. If `open()` returns `0`, admission fails immediately (`capacityRejectedSignal`, `requestFailedSignal`, return `0`). If `open()` returns `id != 0`, `CoapPeerEndpoint{destAddr, destPort, id}` is used immediately across `pendingRequests`, `outboundQueue`, `CoapClientObservation`, and `CoapClientBlockTransfer`. In `pumpOutboundQueue()`, any item whose `peer.sessionId` has `!established` places `item.peer` into `blockedPeers` without starting `CON` retransmission timers or blocking unrelated peers. When `established(id)` fires, `CoapEndpointBase` marks the session established, refreshes `expiresAt = simTime() + exchangeLifetime` for pending requests bound to `id`, calls `pumpOutboundQueue(simTime())`, and reschedules `deadlineTimer`.
2. **`PacketProtocolTag` boundary:** `Dtls::sendApplication()` (`Dtls.cc:910`) uses `addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol)`. Therefore, `CoapEndpointBase::sendCoapPacketToPeer()` sets `PacketProtocolTag` to `&getCoapProtocol()` for `packetSentSignal`/`packetSentToLowerSignal`, and then removes `PacketProtocolTag` right before `dtlsSocket.send(id, packet)` so `Dtls` tags the wire record as `&dtlsProtocol`. On `dataArrived(id, packet)`, `CoapEndpointBase` attaches `PacketProtocolTag = &getCoapProtocol()` before emitting receive signals and parsing.
3. **Send errors and payload limits:** Before calling `dtlsSocket.send()`, `CoapEndpointBase` records `packet->getId()` in `dtlsInFlightPackets` when `outboundId != 0` or `requestId != 0`. On `sendError(id, packetId, reason)` (for example when `bytes + 22 > udpPayloadLimit` or `Dtls` queues overflow), `CoapEndpointBase` removes the matching `activeExchanges`/`pendingRequests` entry, decrements `activeConfirmables`, invokes `onOutboundItemFailed`/`dispatchRequestFailed`, and pumps `outboundQueue`—making sure `pumpOutboundQueue()` and `processConfirmableRetransmissions()` never dereference stale iterators across `sendCoapPacketToPeer()`.
4. **Epoch vs session policy (`RFC 7252 §9.1.1/§9.1.2` vs `RFC 9147` + `RFC 9175 §4`):** `IDtls` models DTLS 1.3 without renegotiation: peer identity and cipher profile are invariant within an `AssociationId`, and only `KeyUpdate` advances traffic-key epochs. `IDtls` does not expose record epoch numbers on `send()` or `dataArrived()`, and `CoapEndpointBase::allocateToken()` already produces globally non-reused 8-byte tokens per `RFC 9175 §4.2`. Rather than extending `IDtls` with epoch-tagging machinery (`AR-ORG-CONTRACT-PURITY`, `QR-DESIGN-MINIMAL`), CoAP strictly binds all state to `AssociationId` and preserves exchanges, Observe relations, and Blockwise transfers across in-session `KeyUpdate` rotations (consistent with `RFC 9175 §4` and DTLS 1.3 key rotation). Distinct handshakes (including `SameTupleReplacement`) receive distinct `AssociationId`s and are strictly isolated.

### State lifetime and lifecycle operations

- **DTLS session termination (`error(id)` / `closed(id)`):** While `operationalState == State::OPERATING`, `CoapEndpointBase::onDtlsSessionTerminated(id, reason)` removes `id` from `dtlsSessions` and `activeClientDtlsByAddrPort`, invokes `onDtlsSessionClosing(id, reason)` first (so `CoapClient` drains and fails `queuedBlockTransfers` for `sessionId == id` before active transfers fail), then erases and fails all `outboundQueue` items, `activeExchanges`, `pendingRequests`, `peerCongestion`, and `duplicateEntries` with `sessionId == id` (which via `dispatchRequestFailed()` cleanly handles `activeBlockTransfers`, `pendingObserveRegistrations`, and registration-stage `Observe + Block2` sub-requests), and finally invokes `onDtlsSessionClosed(id, reason)` so `CoapClient` terminates any remaining established `observationsByPeerToken` and `CoapServer` erases `pendingSeparateResponses`, `observers`, `incomingBlock1`, and `retainedBlock2` for `sessionId == id`.
- **Module start/stop/crash ordering:** Moving `Dtls` to `INITSTAGE_TRANSPORT_LAYER` / `STAGE_TRANSPORT_LAYER` (C2) guarantees `Dtls` initializes and starts before `CoapEndpointBase` (`STAGE_APPLICATION_LAYER`) and stops after `CoapEndpointBase`. Declaring `coap` before `dtls` in `CoapDtlsClient.ned` and `CoapDtlsServer.ned` also ensures `coap` handles single-stage `ModuleCrashOperation` before `dtls`. During `handleStopOperation()`, `CoapEndpointBase` cancels `deadlineTimer`, aborts unfinished requests (`requestAborted`), runs `onEndpointStop()`, closes active DTLS associations (iterating a key snapshot), and finishes without waiting on `UdpSocket` (since `Dtls` owns `udpSocket` closure and `closeTimeout`; `CoapEndpointBase`'s `localAddress`/`localPort`/`timeToLive`/`dscp`/`tos` parameters are unused in DTLS mode). Any later `error`/`closed`/`sendError` callback arriving while `operationalState != State::OPERATING` only erases `id` from `dtlsSessions` without double-aborting or rescheduling timers. `tokenCounter`, `nextMessageId`, `reservedOutgoingMids`, and `nextEtagRevision` survive stop/crash/restart identically to UDP mode; in DTLS mode, `duplicateEntries` for closed sessions are cleared on session close/stop because `Dtls` never reuses `AssociationId` values across restarts.

## 5. Affected components and compatibility

- **Modified existing files:**
  - `src/inet/transportlayer/dtls/Dtls.h` (C2: lifecycle stages `INITSTAGE_TRANSPORT_LAYER` / `STAGE_TRANSPORT_LAYER`).
  - `src/inet/applications/coap/CoapProtocol.h` (C3: `COAP_DTLS_PORT = 5684`).
  - `src/inet/applications/coap/CoapEndpointBase.ned`, `.h`, `.cc` (C3, C4: `dtlsModule`, `CoapPeerEndpoint::sessionId`, `#ifdef INET_WITH_DTLS` `IDtls::ICallback` / `DtlsSocket` transport, `sendError` tracking, `onDtlsSessionClosed`).
  - `src/inet/applications/coap/CoapClient.ned`, `.h`, `.cc` (C3, C5: conditional `peerPort` default, `coaps://` URI parsing, multicast rejection in DTLS mode, `onDtlsSessionClosed` cleanup).
  - `src/inet/applications/coap/CoapServer.h`, `.cc` (C3, C5: `dtlsSocket.listen()` in `onEndpointStart()`, `onDtlsSessionClosed` cleanup).
  - `.oppfeatures` (C3: `CoapDtls`; C7: `CoapDtlsExamples`).
  - `doc/project/evidence/standard/rfc7252/catalog.md`, `rfc9175/catalog.md`, `doc/project/evidence/protocol/coap/*`, `doc/project/evidence/model/coap/*` (C1, C7).
  - `doc/src/users-guide/ch-apps.rst` (C7: document `CoapDtlsClient`/`CoapDtlsServer` and link `ch-dtls.rst`).
- **Added new files:**
  - `src/inet/applications/coapdtls/package.ned`, `CoapDtlsClient.ned`, `CoapDtlsServer.ned` (C3).
  - `tests/module/DtlsApplicationOrderLifecycle_1.test` (C2).
  - `tests/protocol/coapdtls/CoapDtlsTestPeer.h`, `CoapDtlsTestPeer.cc`, `CoapDtlsTestApp.ned`, plus protocol tests and reviewer `.md` documents in C3–C6 (isolated from `tests/protocol/coap/` so UDP `CoapTests` runs cleanly when `Dtls` is disabled).
  - `examples/coapdtls/package.ned`, `CoapDtlsNetwork.ned`, `omnetpp.ini`, `resources.xml` and `doc/whatsnew/compatible/coap-dtls.txt` (C7).
- **Compatibility:**
  - Zero wire or NED breaking change to existing `CoapClient`/`CoapServer` over UDP (`dtlsModule = ""` default).
  - Existing `DtlsApp` configurations remain compatible; shifting `Dtls` to `STAGE_TRANSPORT_LAYER` preserves all existing `Dtls` tests (`DtlsEngineLifecycle_1`, `DtlsRecoveryLifecycle_1`, `DtlsRestartLifecycle_1`) while fixing upper-layer stop ordering.
  - No sealed path (`src/inet/common/packet/`) is touched.

## 6. Future commits and dependencies

| Commit and proposed subject | Dependencies | Detailed entry |
| --- | --- | --- |
| `C1: docs: define the CoAP over DTLS profile and conformance checks` | none | [C1](#c1-docs-define-the-coap-over-dtls-profile-and-conformance-checks) |
| `C2: dtls: align engine lifecycle stages with transport layer` | `C1` | [C2](#c2-dtls-align-engine-lifecycle-stages-with-transport-layer) |
| `C3: coap: add DTLS transport binding, session isolation, and coaps:// URI support` | `C1`, `C2` | [C3](#c3-coap-add-dtls-transport-binding-session-isolation-and-coaps-uri-support) |
| `C4: coap: support CON ping, loss recovery, and DTLS KeyUpdate over DTLS` | `C3` | [C4](#c4-coap-support-con-ping-loss-recovery-and-dtls-keyupdate-over-dtls) |
| `C5: coap: bind Observe and Blockwise state to DTLS session lifetime` | `C3`, `C4` | [C5](#c5-coap-bind-observe-and-blockwise-state-to-dtls-session-lifetime) |
| `C6: coap: verify DTLS endpoint lifecycle and statistics accounting` | `C2`, `C5` | [C6](#c6-coap-verify-dtls-endpoint-lifecycle-and-statistics-accounting) |
| `C7: coap: add CoAP over DTLS examples, documentation, and conformance update` | `C6` | [C7](#c7-coap-add-coap-over-dtls-examples-documentation-and-conformance-update) |

---

### C1: docs: define the CoAP over DTLS profile and conformance checks

Dependencies: none.
Purpose and scope: Pin the specification statements, model fidelity boundaries, feature map, and protocol check definitions under `doc/project/evidence/` before writing runtime code.

Protocol test and reviewer-document fields do not apply because C1 changes only project documentation and adds no runtime code or protocol behavior.

- Extend [rfc7252/catalog.md](../../doc/project/evidence/standard/rfc7252/catalog.md) with `RFC7252-DTLS-1` (`coaps://` scheme and default port `5684`, §6.2/§12.6), `RFC7252-DTLS-2` (unicast-only DTLS protection; multicast disallowed, §9.1), and `RFC7252-DTLS-3` (session-scoped messaging, duplicate suppression, and response matching, §9.1.1–9.1.2).
- Extend [rfc9175/catalog.md](../../doc/project/evidence/standard/rfc9175/catalog.md) with `RFC9175-TOKEN-1` (§4 non-reuse of client Tokens across requests and key updates).
- Update [standards.md](../../doc/project/evidence/protocol/coap/standards.md), [features.md](../../doc/project/evidence/protocol/coap/features.md) (`COAP-F-DTLS`), [checks.md](../../doc/project/evidence/protocol/coap/checks.md), new `doc/project/evidence/protocol/coap/checks/dtls.md`, and [coverage.md](../../doc/project/evidence/model/coap/coverage.md) / [conformance.md](../../doc/project/evidence/model/coap/conformance.md) / [notes.md](../../doc/project/evidence/model/coap/notes.md) (recording `COAP-F-DTLS` as planned/pending until C3–C6 execute).

Verification: run `doc/project/enforcement/check-links.sh` and `git diff --check`.

---

### C2: dtls: align engine lifecycle stages with transport layer

Dependencies: C1.
Purpose and scope: Change `Dtls::isInitializeStage`, `Dtls::isModuleStartStage`, and `Dtls::isModuleStopStage` in [Dtls.h](../../src/inet/transportlayer/dtls/Dtls.h#L80-L82) from `*_APPLICATION_LAYER` to `INITSTAGE_TRANSPORT_LAYER`, `ModuleStartOperation::STAGE_TRANSPORT_LAYER`, and `ModuleStopOperation::STAGE_TRANSPORT_LAYER` so `Dtls` initializes and starts before `ApplicationBase` consumers and stops after them.

Protocol test and reviewer-document fields do not apply because C2 adjusts OMNeT++ lifecycle stage scheduling only and changes no wire protocol exchange.

- **Task: Write module lifecycle test.** Add `tests/module/DtlsApplicationOrderLifecycle_1.test` containing an `ApplicationBase` mock callback consumer declared *after* `Dtls` (testing stop order) and *before* `Dtls` (testing start order) in a host driven by `ScenarioManager` `<shutdown>` and `<startup>`. Assert that on `startup`, `Dtls` enters `OPERATING` before the app's `handleStartOperation` calls `listen()`, and on `shutdown`, the app's `handleStopOperation` runs before `Dtls` tears down its `UdpSocket` and active associations.
- **Verification:** Build debug (`make -j$(nproc) MODE=debug`), run proposed `opp_run_module_tests --load . -p inet -m debug -f 'Dtls.*Lifecycle_1\.test'`, and rerun existing `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/dtls$' -f 'Rfc9147.*\.test$'` to confirm no regression.

---

### C3: coap: add DTLS transport binding, session isolation, and coaps:// URI support

Dependencies: C1, C2.
Purpose and scope: Implement DTLS transport support in `CoapEndpointBase`, `CoapClient`, `CoapServer`, and `CoapProtocol`, add `CoapDtlsClient` and `CoapDtlsServer` under `src/inet/applications/coapdtls/`, and declare the `CoapDtls` feature in `.oppfeatures`.

**Current protocol bug or gap:** Per RFC 7252 §6.2, §9.1, §9.1.1, §9.1.2, and §12.6, CoAP over DTLS uses `coaps://` (default port `5684`), forbids multicast, protects CoAP datagrams inside DTLS records, and isolates messaging and response matching per DTLS session. Currently [CoapEndpointBase](../../src/inet/applications/coap/CoapEndpointBase.cc) only supports `UdpSocket`, [CoapClient::parseCoapUri](../../src/inet/applications/coap/CoapClient.cc#L122-L133) rejects `coaps://`, and `CoapPeerEndpoint` carries only `(address, port)` with no DTLS session identifier.

**Proposed correction:** Compared with parent C2 (which ensures `Dtls` starts before and stops after `CoapEndpointBase`):
1. Define `COAP_DTLS_PORT = 5684` in `CoapProtocol.h` without adding port `5684` to `ProtocolGroup::getUdpProtocolGroup()`.
2. Add `string dtlsModule = default("");` in `CoapEndpointBase.ned`, conditional `peerPort = default(dtlsModule != "" ? 5684 : 5683);` in `CoapClient.ned`, and `uint64_t sessionId = 0;` in `CoapPeerEndpoint`.
3. In `CoapEndpointBase` (under `#ifdef INET_WITH_DTLS`), implement `IDtls::ICallback`, `dtlsSessions`, `activeClientDtlsByAddrPort`, `dtlsInFlightPackets`, handshake gating in `pumpOutboundQueue`, `PacketProtocolTag` stripping before `dtlsSocket.send()` / restoration on `dataArrived()`, `sendError` failure handling, and session-scoped cleanup via `onDtlsSessionTerminated`/`onDtlsSessionClosed`.
4. In `CoapClient`, accept `coaps://` (and reject `coap://` and multicast) when `dtlsModule != ""`, and open/reuse the DTLS association in `resolveDtlsPeerSession()`. In `CoapServer`, call `listenOnDtls()` in `onEndpointStart()` and override `onDtlsSessionClosed()` to purge `pendingSeparateResponses` belonging to closed `sessionId`s.
5. Add `src/inet/applications/coapdtls/package.ned`, `CoapDtlsClient.ned` (`coap` before `dtls`), `CoapDtlsServer.ned` (`coap` before `dtls`, `dtls.localPort = default(5684)`), and feature `CoapDtls` (`requires = "Coap Dtls"`, `compileFlags = "-DINET_WITH_COAPDTLS"`) in `.oppfeatures`.

**Existing tests:** No CoAP-over-DTLS protocol test exists (`tests/protocol/coap/` covers UDP `NoSec` only; `tests/protocol/dtls/` covers raw DTLS 1.3 only).

**Task: Write or extend protocol tests.** Add `tests/protocol/coapdtls/CoapDtlsTestApp.ned` (`package inet.protocoltest.coapdtls;`, isolated from `tests/protocol/coap/` so `CoapTests` remains runnable when `Dtls` is disabled) and `tests/protocol/coapdtls/CoapDtlsTestPeer.h`/`.cc` (using real `Dtls` + `IDtls::ICallback` so tests can inject arbitrary CoAP bytes or open/replace sessions over real DTLS 1.3 associations). Add two `.test` files using [TapPairNetwork.ned](../../tests/protocol/lib/TapPairNetwork.ned) with `ned-package-exclusions = inet.protocoltest.coapdtls.work`:
- `tests/protocol/coapdtls/Rfc7252DtlsHandshakeAndExchange.test`:
  - *IPv4 PSK + `coaps://`:* `host1` (`CoapDtlsClient`, `handshakeOutDelay = 3s` on server `dtls`, `ackTimeout = 2s`) issues `CON` `GET coaps://host2:5684/sensors/temp` and `NON` `POST` to `host2` (`CoapDtlsServer`, `cookieRequired = true`). Assert wire packets on port `5684` dissect as `dtlsProtocol` (not cleartext CoAP), `CoapClient` emits `retransmission == 0` during the `3s` handshake wait, and both requests receive `2.05`/`2.04` (`requestCompleted == 2`).
  - *IPv6 Certificate + mutual auth:* `StandardHost6` + `Ipv6FlatNetworkConfigurator` with `dtls.profile = "certificate"` and server `dtls.clientAuthentication = true`; assert handshake succeeds and `GET`/`PUT` complete.
  - *Scheme & multicast rejection:* Invoke programmatic `submitCoapRequest()` calls inside `try`/`catch (const cRuntimeError&)` for `coap://` on DTLS client, `coaps://` on UDP client, and a multicast destination on DTLS client; assert each raises `cRuntimeError`.
- `tests/protocol/coapdtls/Rfc7252DtlsSessionIsolation.test`:
  - *Same-tuple replacement isolation:* `CoapDtlsTestPeer` on `host1` establishes DTLS session 1 to `CoapDtlsServer` (`responseDelay = 1s`), sends a `CON` `GET` (`MID = 42`), then before the `1s` response fires, establishes replacement DTLS session 2 on the same `(IP, port)` tuple (closing session 1). Assert `CoapDtlsServer` drops session 1's pending separate response (never sends it on session 2), and a new `CON` `GET` on session 2 with the same `MID = 42` is NOT suppressed as a duplicate and returns `2.05`.
  - *Handshake timeout & `sendError` oversize failure:* Configure `host1` against a blackholed peer (`handshakeTimeout = 1s`) and separately send a request exceeding `dtls.udpPayloadLimit` (`500B`); assert `requestFailed` fires once per failed request and `pendingRequests` drains to `0`.

**Task: Write the reviewer document.** Add `tests/protocol/coapdtls/dtls-transport-and-isolation.md` associated with `C3` and the two tests above. Include sequence diagrams showing: (a) client outbound gating during cookie-retried DTLS handshake vs CoAP `ackTimeout`, and (b) same-tuple DTLS replacement canceling pending session-1 server work and isolating `MID` duplicate state on session 2.

**Verification:** Build debug (`make -j$(nproc) MODE=debug`), run proposed `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coapdtls$' -f 'Rfc7252Dtls(HandshakeAndExchange|SessionIsolation)\.test$'`, and check `Coap=ON, Dtls=OFF, CoapDtls=OFF` compilation plus `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coap$' -f '.*\.test$'`.

---

### C4: coap: support CON ping, loss recovery, and DTLS KeyUpdate over DTLS

Dependencies: C3.
Purpose and scope: Add DTLS session resolution to `CoapEndpointBase::sendConfirmablePing()` and establish CoAP message-layer reliability (`CON` retry/backoff, duplicate suppression, malformed CoAP inside valid DTLS records, and `KeyUpdate` token uniqueness per `RFC 9175 §4.2`) over DTLS.

**Current protocol bug or gap:** After C3, basic request/response and session isolation work, but `CoapEndpointBase::sendConfirmablePing()` still passes an unresolved `CoapPeerEndpoint` (`sessionId = 0`), and lossy application datagrams (`TapPairNetwork` dropping DTLS application records), malformed CoAP payloads inside valid DTLS records (`CON` -> `RST`, `NON` -> silent discard), and in-session DTLS 1.3 `KeyUpdate` transitions lack protocol-level proof.

**Proposed correction:** Relative to C3, update `CoapEndpointBase::sendConfirmablePing()` to resolve or open the peer's DTLS session when `isDtlsMode()` is active (returning `false` if synchronous `open()` fails), leaving `pumpOutboundQueue()`, `processConfirmableRetransmissions()`, and `allocateToken()`'s monotonic 8-byte token generator unchanged across `KeyUpdate` queuing and epoch rotation.

**Existing tests:** None for lossy CoAP-over-DTLS messaging or `KeyUpdate`.

**Task: Write or extend protocol tests.** Add `tests/protocol/coapdtls/Rfc7252DtlsReliabilityAndRekey.test` on `TapPairNetwork`:
- *Application loss & duplicate suppression:* `CoapDtlsClient` sends `CON` `PUT` (`+1` increment or body replacement); `tap` drops the 1st DTLS application record (request) and later drops the server's 1st DTLS application ACK. Assert client retransmits with exponential backoff (`ACK_TIMEOUT`..), server logs `duplicateReceived == 1`, executes the mutation once, replays cached `ACK` in a fresh DTLS record sequence, and client completes (`requestCompleted == 1`).
- *Malformed inner CoAP & Empty `CON` ping:* `CoapDtlsTestPeer` sends a valid DTLS record carrying malformed CoAP `CON` (`TKL=9`) -> assert server answers `RST` inside DTLS and increments `malformedReceived`; malformed `NON` -> silent discard; Empty `CON` (`0.00`) -> `RST`.
- *DTLS 1.3 `KeyUpdate` during CoAP exchanges:* Set `*.dtls.rekeyAfterRecords = 2`; `CoapDtlsClient` issues 5 sequential `CON` `GET` requests over one association. Assert wire `DtlsCiphertextHeader` `epochBits` advances across key updates (`3 -> 0 -> 1`, corresponding to epochs `3 -> 4 -> 5`), all 5 requests complete (`requestCompleted == 5`) with 5 distinct 8-byte tokens, and zero requests time out.

**Task: Write the reviewer document.** Add `tests/protocol/coapdtls/dtls-reliability-and-rekey.md` associated with `C4` and `Rfc7252DtlsReliabilityAndRekey.test`, including sequence diagrams for (a) dropped DTLS application ACK causing CoAP `CON` retry inside a new DTLS record number, and (b) `KeyUpdate` queuing and resuming CoAP exchanges without token reuse.

**Verification:** Run proposed `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coapdtls$' -f 'Rfc7252DtlsReliabilityAndRekey\.test$'`.

---

### C5: coap: bind Observe and Blockwise state to DTLS session lifetime

Dependencies: C3, C4.
Purpose and scope: Bind `CoapClient` and `CoapServer` Observe (`RFC 7641`), Block1/Block2 (`RFC 7959`), `Request-Tag` (`RFC 9175 §3`), and `Observe + Block2` state to DTLS session lifetime (`onDtlsSessionClosed`) and verify all compound flows over DTLS.

**Current protocol bug or gap:** After C3–C4, `CoapEndpointBase` cleans up base exchanges and pending requests on `closed(id)`/`error(id)`, and `CoapServer` drops `pendingSeparateResponses`, but `CoapClient` (`observationsByPeerToken`, `observationsByCacheKey`, `queuedBlockTransfers`) and `CoapServer` (`observers`, `incomingBlock1`, `retainedBlock2`) still retain Observe and Blockwise entries until timeout/stop. If a DTLS association closes or is replaced, the server could attempt notifications onto a closed `AssociationId`, `failLogicalTransfer()` could launch a queued block transfer onto a closed session, or a new session could hit stale client Observe cache or server block state.

**Proposed correction:** Relative to C4, implement `onDtlsSessionClosing` / `onDtlsSessionClosed` in `CoapClient` and extend `onDtlsSessionClosed` in `CoapServer`:
- `CoapClient`: in `onDtlsSessionClosing(id, reason)` (before base `pendingRequests` failure), fail and drain `queuedBlockTransfers` with `peer.sessionId == id` so `failLogicalTransfer()` does not launch queued transfers onto the closed session; after base `dispatchRequestFailed()` completes (`pendingObserveRegistrations` and `pendingObserveBlock2SubRequests`), in `onDtlsSessionClosed(id, reason)` terminate any remaining established `observationsByPeerToken` with `key.peer.sessionId == id` via `terminateObservation()` and emit `activeObservationsSignal`.
- `CoapServer`: retain `pendingSeparateResponses` cleanup from C3, remove `observers` with `key.peer.sessionId == id` via `removeObserverAndEmit()`, and erase `incomingBlock1`/`retainedBlock2` entries with `key.peer.sessionId == id`.

**Existing tests:** None for Observe or Blockwise over DTLS.

**Task: Write or extend protocol tests.** Add two `.test` files on `TapPairNetwork`:
- `tests/protocol/coapdtls/Rfc7641DtlsObserve.test`: `CoapDtlsClient` subscribes to `/sensors/temp` (`Observe=0`), receives `CON` notifications across a server `updateResourceValue()` and a `KeyUpdate` (`rekeyAfterRecords = 2`), tests supersession when a notification ACK is dropped by `tap`, and then closes/replaces the DTLS association; assert `onObserveTerminated` fires on client, `activeObservers` drops to `0` on server, and subsequent server mutations emit zero notifications on the replacement session.
- `tests/protocol/coapdtls/Rfc7959DtlsBlockwise.test`: Exercise `Block1` `PUT` (3 KiB body, `preferredBlockSize = 1024B`, default `udpPayloadLimit = 1200B`), `Block2` `GET`, two concurrent `Request-Tag` `Block1` uploads over one DTLS association, `Observe + Block2` notification retrieval, and incomplete `Block1` cleanup when a `CoapDtlsTestPeer` closes session 1 after block 0 (`M=1`) and sends block 1 (`NUM=1`) on session 2 -> assert server rejects session 2's orphaned block with `4.08 Request Entity Incomplete`.

**Task: Write the reviewer document.** Add `tests/protocol/coapdtls/dtls-observe-and-blockwise.md` associated with `C5` and both tests, with sequence diagrams for Observe across `KeyUpdate` + session closure teardown and `Block1` incomplete-upload isolation across DTLS sessions.

**Verification:** Run proposed `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coapdtls$' -f 'Rfc(7641|7959)Dtls.*\.test$'`.

---

### C6: coap: verify DTLS endpoint lifecycle and statistics accounting

Dependencies: C2, C5.
Purpose and scope: Verify graceful stop, crash, and restart (`ModuleStopOperation`, `ModuleCrashOperation`, `ModuleStartOperation`) and CoAP/DTLS signal/statistic invariants for `CoapDtlsClient` and `CoapDtlsServer`.

**Current protocol bug or gap:** Lifecycle transitions (`requestAborted` vs `requestFailed`, delayed `close_notify` when `protectOutDelay > 0`, server state cleanup on peer `close_notify`, crash without `close_notify`, and restart with fresh `AssociationId` and preserved `tokenCounter`/`nextMessageId`/`nextEtagRevision`) and combined CoAP/DTLS statistics have not been verified end-to-end.

**Proposed correction:** Relative to C5, `C6` adds the lifecycle and statistics verification suites without altering the `C2`–`C5` protocol design (`CoapEndpointBase::handleStopOperation()` aborting unfinished requests via `clearTransientState(false)` before `Dtls::handleStopOperation()`, `handleCrashOperation()` clearing state without `close_notify`, and `operationalState` guards ignoring post-stop `Dtls` callbacks).

**Existing tests:** [CoapLifecycle.test](../../tests/protocol/coap/CoapLifecycle.test) and [CoapStatistics.test](../../tests/protocol/coap/CoapStatistics.test) cover UDP `NoSec` only.

**Task: Write or extend protocol tests.** Add two `.test` files on `TapPairNetwork` (driven via `LifecycleController` with `*.host*.hasStatus = true`, matching [CoapLifecycle.test](../../tests/protocol/coap/CoapLifecycle.test)):
- `tests/protocol/coapdtls/CoapDtlsLifecycle.test`:
  - *Stop during handshake and during active transfers (`protectOutDelay = 0.2s`):* Stop `host1` (`CoapDtlsClient`) while a handshake is delayed and separately while `CON`, `Block1`, and established `Observe` are active. Assert unfinished requests emit `requestAborted == 1` each (and `requestFailed == 0`), established Observe emits `onObserveTerminated` (not `requestAborted`), `Dtls` transmits `close_notify`, `CoapDtlsServer` cleans up `observers`/`incomingBlock1` on receipt, and restarting `host1` opens a new `AssociationId` and completes requests.
  - *Server crash & restart:* Crash `host2` (`CoapDtlsServer`); assert zero `close_notify` emitted, XML resources reset to configured defaults, duplicate history clears, and restarting `host2` accepts a fresh client handshake.
- `tests/protocol/coapdtls/CoapDtlsStatistics.test`:
  - Verify `requestStarted`, `requestCompleted`, `requestFailed`, `requestAborted`, `requestLatency`, `retransmission`, `duplicateReceived`, `activeObservations`, `activeObservers`, and DTLS `associationEstablished` / `recordOverhead` across handshake setup, cached Observe hit (`0` DTLS records / `0` latency samples), dropped-ACK retry, and stop abort.

**Task: Write the reviewer document.** Add `tests/protocol/coapdtls/dtls-lifecycle-and-statistics.md` associated with `C6` and both tests, with sequence/state diagrams comparing graceful `CoapDtlsClient` shutdown (`STAGE_APPLICATION_LAYER` abort -> `STAGE_TRANSPORT_LAYER` `close_notify`) against `ModuleCrashOperation`.

**Verification:** Run proposed `opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coapdtls$' -f 'CoapDtls(Lifecycle|Statistics)\.test$'`.

---

### C7: coap: add CoAP over DTLS examples, documentation, and conformance update

Dependencies: C6.
Purpose and scope: Provide runnable `examples/coapdtls/` scenarios, declare `CoapDtlsExamples` in `.oppfeatures`, update user documentation and release notes, and record final conformance/coverage evidence in `doc/project/evidence/model/coap/`.

Protocol test and reviewer-document fields do not apply because C7 adds examples, user documentation, release notes, and final evidence records without modifying protocol behavior.

- Add `examples/coapdtls/package.ned`, `CoapDtlsNetwork.ned`, `resources.xml`, and `omnetpp.ini` (`PskBasic`, `CertificateMutualAuth`, `LossyObserveBlockwise`, `ProcessingCost`, `Ipv6`) plus `CoapDtlsExamples` in `.oppfeatures` (`requires = "CoapDtls Ethernet Ieee8022 Ipv4 Ipv6 Loopback PhysicalLayerWiredCommon Queueing Udp"`).
- Update [ch-apps.rst](../../doc/src/users-guide/ch-apps.rst#L501-L565) to document `CoapDtlsClient`/`CoapDtlsServer` and link to [ch-dtls.rst](../../doc/src/users-guide/ch-dtls.rst).
- Add `doc/whatsnew/compatible/coap-dtls.txt` following [coap-udp-profile.txt](../../doc/whatsnew/compatible/coap-udp-profile.txt).
- Update `doc/project/evidence/model/coap/coverage.md`, `conformance.md`, and `results.md` with observed run verdicts from C2–C6.

Verification: run all checks in Section 7 (`debug` + `release` builds, full CoAP & DTLS suites, feature-matrix builds, example configs, and mechanical gates).

## 7. Verification methods and expected results

All execution commands below are **proposed** (`NOT_RUN` at plan time). Run from `/home/miguel/workspace/inet-coap-dtls` per [run-the-gates.md](../../doc/project/guide/run-the-gates.md), rebuilding before every test run after C++ changes:

```sh
# 1. Build both modes
make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release

# 2. Focused DTLS unit, module, and protocol checks (C2 regression + new order test)
opp_run_unit_tests --load . -p inet -m debug -f '(TlsHandshake(Serializer|Parameters)|DtlsRecord(Serializer|State))_1\.test'
opp_run_module_tests --load . -p inet -m debug -f 'Dtls.*Lifecycle_1\.test'
opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/dtls$' -f 'Rfc9147.*\.test$'

# 3. Full CoAP over UDP and CoAP over DTLS protocol suites
opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coap$' -f '.*\.test$'
opp_run_protocol_tests --load . -p inet -m debug -w '^tests/protocol/coapdtls$' -f '.*\.test$'

# 4. Feature-matrix compilation checks (C3, C7; restore enabled features afterward or run in an isolated worktree)
opp_featuretool disable Dtls CoapDtls CoapDtlsExamples && make makefiles && make -j$(nproc) MODE=debug
opp_featuretool enable Dtls && opp_featuretool disable Coap CoapExamples CoapTests && make makefiles && make -j$(nproc) MODE=debug
opp_featuretool enable Coap CoapExamples CoapDtls CoapDtlsExamples && make makefiles && make -j$(nproc) MODE=debug

# 5. Runnable examples (C7, fixed seed-set = 0, with . setenv active)
cd examples/coapdtls && inet -u Cmdenv -c PskBasic -r 0 --sim-time-limit=10s
# Repeat for CertificateMutualAuth, LossyObserveBlockwise, ProcessingCost, Ipv6

# 6. Mechanical gates (before push)
doc/project/enforcement/check-architecture.sh src/inet/applications/coap
doc/project/enforcement/check-architecture.sh src/inet/applications/coapdtls
doc/project/enforcement/check-naming.sh --base origin/master
doc/project/enforcement/check-interfaces.sh
doc/project/enforcement/check-pull-request.sh --base origin/master
doc/project/enforcement/check-source-seals.sh --base origin/master
doc/project/enforcement/check-links.sh
python3 doc/project/enforcement/whatsnew.py preview
```

Commit-specific test selections and assertions are defined in `C2`–`C6`.

## 8. Risks, unresolved decisions, and approval scope

1. **DTLS epoch vs session binding (`RFC 7252 §9.1.1/§9.1.2` vs `RFC 9147` + `RFC 9175 §4`):** Because `IDtls` does not expose record epoch numbers and DTLS 1.3 `KeyUpdate` preserves peer identity within an `AssociationId`, this plan scopes CoAP matching, duplicate suppression, Observe, and Blockwise state to `IDtls::AssociationId` and lets exchanges/Observe survive in-session `KeyUpdate` rotations (paired with 8-byte non-reused tokens per `RFC 9175 §4.2`). If a maintainer requires strict RFC 7252 §9.1.1/§9.1.2 cross-epoch exchange/Observe abortion, `IDtls::ICallback` and `Dtls` would need an epoch metadata tag or epoch-changed callback (a material design change under `PLR-REVISION`).
2. **Dtls lifecycle stage change (`C2`):** Shifting `Dtls` from `STAGE_APPLICATION_LAYER` to `STAGE_TRANSPORT_LAYER` touches `src/inet/transportlayer/dtls/Dtls.h` outside `coap/`. It is isolated in `C2` (`PR-SPLIT-UPSTREAM`) and verified against all existing DTLS lifecycle and protocol tests before `C3` depends on it.
3. **`udpPayloadLimit` vs `preferredBlockSize`:** Default `Dtls.udpPayloadLimit` (`1200B`) accommodates default `CoapEndpointBase.preferredBlockSize` (`1024B` + CoAP headers <= `1178B`). Configurations that shrink `dtls.udpPayloadLimit` (e.g. `500B`) must also configure a smaller `coap.preferredBlockSize`; oversized single CoAP messages fail promptly via `IDtls::ICallback::sendError`.
4. **Repository/skill-infrastructure note:** `AGENTS.md` references `scripts/validate_skill_suite.py` and `tests/skill-suite/`, which are absent from this INET checkout; only INET's `doc/project/enforcement/` checks apply here.
5. **Approval scope:** Pending human maintainer approval covering `C1`–`C7`, specifically confirming decision #1 (session-scoped `AssociationId` binding across DTLS 1.3 `KeyUpdate`) and decision #2 (`Dtls.h` lifecycle stage alignment in `C2`). No sealed-path exception is requested.
