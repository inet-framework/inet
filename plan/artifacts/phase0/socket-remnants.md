# Socket Command Remnants — `new Request(` sites remaining after branch conversion

Branch: `topic/infrastructure`
Survey date: 2026-07-13

## Architecture background

The `at` (application-layer) MessageDispatcher connects apps to transport modules.
It is a `PacketProcessorBase` that only implements `pushPacket` for `Packet` objects.
It does **not** define `handleMessage`, so any raw `cMessage`/`Request` sent via a gate
connected to `at` hits `cSimpleModule::handleMessage()` default, which throws:

> "Redefine handleMessage() or specify non-zero stack size to use activity()"

The conversion pattern used in the reference commits (f4f8c8f3b2, e409faa29f) is:
1. Add the method to the protocol's C++ interface (`ITcp`, `IUdp`, `ISctp`).
2. Implement it in the module by calling `handleUpperCommand` directly (bypassing the dispatcher).
3. Replace `new Request(...) + sendToX(request)` in the Socket class with a direct `proto->method(socketId, ...)` call.

For commands that need no-op on already-removed sockets (like TCP destroy), the module
implementation must guard against resurrecting a non-existent socket (see `Tcp::destroy`).

---

## Files checked with no remaining `new Request(` sites

| File | Status |
|------|--------|
| `src/inet/networklayer/contract/ipv4/Ipv4Socket.cc` | Clean — already converted |
| `src/inet/networklayer/contract/ipv6/Ipv6Socket.cc` | Clean — already converted |
| `src/inet/linklayer/ethernet/common/EthernetSocket.cc` | Clean — already converted |
| `src/inet/linklayer/ieee8022/Ieee8022LlcSocket.cc` | Clean — already converted |
| `src/inet/linklayer/ieee8021q/Ieee8021qSocket.cc` | Clean — already converted |

---

## Remaining sites — full table

### Key for columns

- **Crash path**: whether the un-converted message path goes through the `at` MessageDispatcher (YES = crash) or calls a direct method on the protocol module bypassing `at` (NO = safe).
- **Dead**: no callers exist anywhere in `src/`.

| # | file:line | Socket method | Command kind | Target module | Module has direct method? | Crash via `at`? | Active callers? | Proposed conversion | Difficulty |
|---|-----------|---------------|--------------|---------------|--------------------------|-----------------|-----------------|---------------------|------------|
| 1 | `TcpSocket.cc:138` | `read(int32_t numBytes)` | `TCP_C_READ` | `Tcp` | NO — not in `ITcp`; handled in `Tcp::handleUpperCommand` → `TcpConnection::processAppCommand(TCP_C_READ)` | YES → CRASH | YES: `TcpServerSocketIo`, `TcpClientSocketIo`, `TcpEchoApp`, `TcpSessionApp`, `TcpSinkApp`, `TcpBasicClientApp`, `TelnetApp` | Add `read(int socketId, int32_t numBytes)` to `ITcp` + `Tcp`; call `tcp->read(connId, numBytes)` | Mechanical |
| 2 | `TcpSocket.cc:191` | `requestStatus()` | `TCP_C_STATUS` | `Tcp` | NO — not in `ITcp`; handled in `Tcp::handleUpperCommand` → `TcpConnection::processAppCommand(TCP_C_STATUS)` | YES → CRASH | DEAD — zero callers in `src/` | Add `requestStatus(int socketId)` to `ITcp` + `Tcp`; call `tcp->requestStatus(connId)` | Mechanical (but dead, low priority) |
| 3 | `TcpSocket.cc:208` | `setDscp(short dscp)` | `TCP_C_SETOPTION / TcpSetDscpCommand` | `Tcp` | NO — not in `ITcp`; handled in `Tcp::handleUpperCommand` → `TcpConnection::processAppCommand(TCP_C_SETOPTION)` | YES → CRASH | YES: `TcpAppBase` (called from all TCP-based apps on connect) | Add `setDscp(int socketId, short dscp)` to `ITcp` + `Tcp`; call `tcp->setDscp(connId, dscp)` | Mechanical |
| 4 | `TcpSocket.cc:217` | `setTos(short tos)` | `TCP_C_SETOPTION / TcpSetTosCommand` | `Tcp` | NO — not in `ITcp`; handled via same `TCP_C_SETOPTION` path as `setDscp` | YES → CRASH | YES: `TcpAppBase` (called from all TCP-based apps on connect) | Add `setTos(int socketId, short tos)` to `ITcp` + `Tcp`; call `tcp->setTos(connId, tos)` | Mechanical |
| 5 | `SctpSocket.cc:200` | `listen(uint32_t requests, bool fork, …, bool options, int32_t fd)` | `SCTP_C_OPEN_PASSIVE` | `Sctp` | Partial — `ISctp::listen(...)` exists but this is a separate 5-param overload that sends via `sendToSctp` (the overload with `bool options` and `int32_t fd` args). | YES → CRASH | DEAD — all current callers (`SctpServer`, `SctpNatServer`, `SctpNatPeer`, `SctpPeer`, `NetPerfMeter`) use the `(bool fork, bool reset, uint32_t requests, uint32_t messagesToPush)` overload which already calls `sctp->listen(...)` directly. | Remove this overload or delegate to `sctp->listen(...)` after extracting needed fields (ignoring `fd` which is a legacy POSIX artifact unused by the module). Needs analysis of `options` / `contextPointer` semantic. | Needs-judgment (legacy overload with POSIX `fd`; should confirm `contextPointer(sOptions)` is irrelevant before removal) |
| 6 | `SctpSocket.cc:272` | `connect(int32_t fd, L3Address, int32_t port, uint32_t numRequests, bool options)` | `SCTP_C_ASSOCIATE` | `Sctp` | Partial — `ISctp::connect(...)` exists but this is the `fd`-first overload that sends via `sendToSctp`. `contextPointer(sOptions)` is set when `options=true`. | YES → CRASH | DEAD — all current callers use the `(L3Address, int32_t port, bool streamReset, int32_t prMethod, uint32_t numRequests)` overload which calls `sctp->connect(...)` directly. | Remove overload or convert to `sctp->connect(...)` mapping `fd/options` semantics. | Needs-judgment (same POSIX-fd concern as #5) |
| 7 | `SctpSocket.cc:301` | `accept(int32_t assId, int32_t fd)` | `SCTP_C_ACCEPT` | `Sctp` | Partial — `ISctp::accept(int socketId)` exists but this 2-arg overload sends via `sendToSctp`. | YES → CRASH | DEAD — only caller is `SctpServer.cc:616` which calls the 1-arg form `socket->accept(newSocketId)` (maps to `acceptSocket()` → `sctp->accept()`). | Remove overload or convert to `sctp->accept(assId)` (ignore `fd`). | Needs-judgment (confirm `fd` is unused at module side) |
| 8 | `SctpSocket.cc:400` | `destroy()` | `SCTP_C_DESTROY` | `Sctp` | NO — `destroy` is not in `ISctp`; `SCTP_C_DESTROY` is not handled in `SctpAssociation::preanalyseAppCommandEvent` (falls to `throw cRuntimeError("Unknown message kind")`). | YES → CRASH (double fault: `at` dispatcher AND unhandled command) | DEAD — no callers in `src/inet/applications/`. | Add `destroy(int socketId)` to `ISctp` + `Sctp` (no-op if assoc already removed, per TCP pattern). Call `sctp->destroy(assocId)`. | Needs-judgment (lifecycle: must not resurrect; SCTP_C_DESTROY is also unimplemented in `Sctp.handleMessage`, so the module side needs implementing too) |
| 9 | `SctpSocket.cc:416` | `requestStatus()` | `SCTP_C_STATUS` | `Sctp` | NO — `SCTP_C_STATUS` is not in `preanalyseAppCommandEvent` switch (throws "Unknown message kind"). | YES → CRASH (double fault) | DEAD — zero callers in `src/`. | Add `requestStatus(int socketId)` to `ISctp` + `Sctp` with a synthesized `SCTP_I_STATUS` reply path. | Needs-judgment (status reply path is non-trivial) |
| 10 | `SctpSocket.cc:599` | `setStreamPriority(uint32_t stream, uint32_t priority)` | `SCTP_C_SET_STREAM_PRIO` | `Sctp` | NO — not in `ISctp`; `SCTP_C_SET_STREAM_PRIO` is handled in `SctpAssociation::preanalyseAppCommandEvent` → `SCTP_E_SET_STREAM_PRIO`. | YES → CRASH | YES — could be called from SCTP apps that use stream priorities (no direct app callers found in `src/inet/applications/sctpapp/`, but the API is public). Mark as potentially-live. | Add `setStreamPriority(int socketId, uint32_t stream, uint32_t priority)` to `ISctp` + `Sctp` (delegate to `handleMessage` internally). Call `sctp->setStreamPriority(assocId, stream, priority)`. | Mechanical |
| 11 | `SctpSocket.cc:613` | `setRtoInfo(double initial, double max, double min)` | `SCTP_C_SET_RTO_INFO` | `Sctp` | NO — not in `ISctp`; `SCTP_C_SET_RTO_INFO` is handled in `SctpAssociation::preanalyseAppCommandEvent` → `SCTP_E_SET_RTO_INFO`. | YES → CRASH | DEAD — no app callers found in `src/` (the `sOptions` member stores values but no app appears to call `setRtoInfo` and have the socket connected). | Add `setRtoInfo(int socketId, double initial, double max, double min)` to `ISctp` + `Sctp`. | Mechanical |
| 12 | `QuicSocket.cc:59` | `bind(L3Address, uint16_t)` | `QUIC_C_CREATE_PCB` | `Quic` | NO — there is no C++ `IQuic` class; `Quic` uses `handleMessageWhenUp` → `handleMessageFromApp` → `AppSocket::processAppCommand`. `Quic` module has `handleMessage` (via `OperationalMixin`). | YES → CRASH (`at` dispatcher) | YES: `QuicClient`, `QuicZeroRttClient`, `QuicTrafficgen`, `QuicOrderedReceiver`, `QuicDiscardServer`, indirectly from `accept()`. | Need a C++ `IQuic` interface + direct methods in `Quic`. This is the largest single conversion task for QUIC. OR add `handleMessage` to `MessageDispatcher` (wrong direction). | Needs-judgment (no IQuic C++ class exists; requires designing the entire direct-method interface for QUIC) |
| 13 | `QuicSocket.cc:67` | `listen()` | `QUIC_C_OPEN_PASSIVE` | `Quic` | NO — same as above. | YES → CRASH | YES: `QuicDiscardServer`, `QuicOrderedReceiver`. | Same as #12. | Needs-judgment |
| 14 | `QuicSocket.cc:80` | `connect(L3Address, uint16_t)` | `QUIC_C_OPEN_ACTIVE` | `Quic` | NO — same as above. | YES → CRASH | YES: `QuicClient`, `QuicZeroRttClient`, `QuicTrafficgen`. | Same as #12. | Needs-judgment |
| 15 | `QuicSocket.cc:139` | `recv(int64_t length, uint64_t streamId)` | `QUIC_C_RECEIVE` | `Quic` | NO — same as above. | YES → CRASH | YES: `QuicOrderedReceiver`, `QuicDiscardServer`. | Same as #12. | Needs-judgment |
| 16 | `QuicSocket.cc:146` | `close()` | `QUIC_C_CLOSE` | `Quic` | NO — same as above. | YES → CRASH | YES: `QuicClient`, `QuicZeroRttClient`, `QuicTrafficgen`, `QuicOrderedReceiver`, `QuicDiscardServer`. | Same as #12. | Needs-judgment |
| 17 | `QuicSocket.cc:191` | `accept()` | `QUIC_C_ACCEPT` (+ internal `QUIC_C_CREATE_PCB` for new socket) | `Quic` | NO — same as above. | YES → CRASH | YES: `QuicOrderedReceiver`. | Same as #12. Each `accept()` call also triggers a `bind()` (#12) on the new socket. | Needs-judgment |
| 18 | `L3Socket.cc:67` | `bind(const Protocol*, L3Address)` | `L3_C_BIND` | `Ipv4`/`Ipv6`/`NextHopForwarding` | YES — `Ipv4::handleRequest(Ipv4SocketBindCommand*)`, `NextHopForwarding` has similar handling. | YES → CRASH | YES: `PingApp` (when `networkProtocol != IPv4 && != IPv6`, i.e. generic IP). Note: `Ipv4Socket` and `Ipv6Socket` are already converted; L3Socket is the generic fallback. | Add `bind(int socketId, const Protocol*, L3Address)` to protocol modules; OR add `handleMessage` dispatch in `Ipv4` that accepts messages from `at` (requires INetworkProtocol interface extension). | Needs-judgment (generic fallback for arbitrary L3 protocols; target module varies) |
| 19 | `L3Socket.cc:79` | `connect(L3Address)` | `L3_C_CONNECT` | `Ipv4`/`Ipv6`/`NextHopForwarding` | YES — `Ipv4::handleRequest(Ipv4SocketConnectCommand*)`. | YES → CRASH | RARELY: `PingApp.cc:187` binds but does not connect for generic L3 sockets. No direct caller for connect found. | Same approach as #18. | Needs-judgment |
| 20 | `L3Socket.cc:101` | `close()` | `L3_C_CLOSE` | `Ipv4`/`Ipv6`/`NextHopForwarding` | YES — `Ipv4::handleRequest(Ipv4SocketCloseCommand*)`. | YES → CRASH | YES: `PingApp.cc:177, 317` calls `socket.second->close()` on all socket types including L3Socket. | Same approach as #18. | Needs-judgment |
| 21 | `L3Socket.cc:110` | `destroy()` | `L3_C_DESTROY` | `Ipv4`/`Ipv6`/`NextHopForwarding` | YES — `Ipv4::handleRequest(Ipv4SocketDestroyCommand*)`. | YES → CRASH | YES: `PingApp.cc:336` calls `socket.second->destroy()` on all socket types including L3Socket. | Same approach as #18. | Needs-judgment |
| 22 | `TunSocket.cc:81` | `close()` | `TUN_C_CLOSE` | `Tun` | YES — `Tun::handleUpperCommand(TunCloseCommand*)` via `MacProtocolBase → handleMessage`. | YES → CRASH | DEAD — no app currently calls `tunSocket.close()`. (`TunLoopbackApp` and `TunnelApp` use `open()` and `send()` only.) | Add `close(int socketId)` to `ITun`; implement in `Tun`; call `tun->close(socketId)` from `TunSocket`. | Mechanical |
| 23 | `TunSocket.cc:90` | `destroy()` | `TUN_C_DESTROY` | `Tun` | YES — `Tun::handleUpperCommand(TunDestroyCommand*)`. | YES → CRASH | DEAD — no app currently calls `tunSocket.destroy()`. | Add `destroy(int socketId)` to `ITun`; implement in `Tun`; call `tun->destroy(socketId)` from `TunSocket`. | Mechanical |

---

## Per-file summary

### `TcpSocket.cc` (4 sites)

- **Mechanical**: 1, 3, 4 (read, setDscp, setTos) — called by widely-used TCP apps, will crash on every simulation that uses them.
- **Mechanical/dead**: 2 (requestStatus) — dead code, zero callers.
- All 4 paths go through the `at` dispatcher → crash.
- `Tcp` does NOT need `handleMessage` restored; all ITcp methods already call `handleUpperCommand` directly. Only need to add `read`, `requestStatus`, `setDscp`, `setTos` to `ITcp` + `Tcp`.

### `SctpSocket.cc` (7 sites)

- **Dead / needs-judgment**: 5, 6, 7 (listen2, connect2, accept2 — legacy `fd`-bearing overloads). No active callers. Safest action: remove these overloads.
- **Dead / needs-judgment**: 8, 9 (destroy, requestStatus) — SCTP_C_DESTROY and SCTP_C_STATUS are also not implemented on the module side; double-fault. Dead callers.
- **Mechanical**: 10 (setStreamPriority) — SCTP_C_SET_STREAM_PRIO is handled in SctpAssociation; no app callers found but API is public.
- **Mechanical / dead**: 11 (setRtoInfo) — dead callers.

### `QuicSocket.cc` (6 sites)

- **All 6 are needs-judgment** — QUIC has no C++ `IQuic` interface at all. `Quic` communicates entirely via `handleMessage`/`handleMessageWhenUp`→`handleMessageFromApp`. The `Quic` module DOES have `handleMessage` (through `OperationalMixin`/`OperationalBase`), so these messages would reach the Quic module IF they could pass the `at` dispatcher. The blocker is purely the `at` dispatcher.
- **All 6 sites are in active use** (bind/listen/connect/recv/close/accept are called by real QUIC apps). QUIC simulations will crash immediately on socket creation.
- Conversion requires designing a C++ `IQuic` interface class (like `ITcp`), adding direct methods to `Quic`, and updating `QuicSocket` to hold a `ModuleRefByGate<IQuic>`.

### `L3Socket.cc` (4 sites)

- **All 4 are needs-judgment** — the target module varies (Ipv4, Ipv6, NextHopForwarding depending on protocol). The `Ipv4Socket` / `Ipv6Socket` already have direct method paths; L3Socket is the generic fallback.
- **Active callers**: `PingApp` uses L3Socket when `networkProtocol` is neither IPv4 nor IPv6. bind (#18) + close (#20) + destroy (#21) are actively called.
- Conversion approach: either extend the L3-level interface modules with direct methods, or route L3Socket through the already-existing `Ipv4` / `Ipv6` typed sockets when possible (PingApp probably falls through to L3Socket only for non-IP protocols).

### `TunSocket.cc` (2 sites)

- **Mechanical / dead**: 22, 23 (close, destroy) — `ITun` only has `open()`; adding `close()` and `destroy()` is straightforward. No active callers today, but close/destroy paths must work before any lifecycle-aware TUN app can run correctly.

---

## QUIC verdict

QUIC was **not converted** to the direct-method pattern on this branch.

- There is no C++ `IQuic` class (only a NED `moduleinterface IQuic`).
- `QuicSocket` holds only a raw `cGate*`, not a `ModuleRefByGate<IQuic>`.
- All 6 QuicSocket command methods send raw `Request` messages via `sendToQuic` → `app.socketOut` → `at` dispatcher → CRASH.
- Every QUIC simulation (`QuicClient`, `QuicDiscardServer`, `QuicOrderedReceiver`, `QuicTrafficgen`, `QuicZeroRttClient`) crashes at socket initialization (the first call is `bind()` → `QuicSocket.cc:59`).

The QUIC module itself (`Quic::handleMessageWhenUp` + `AppSocket::processAppCommand`) correctly handles all command kinds (`QUIC_C_CREATE_PCB`, `QUIC_C_OPEN_PASSIVE`, `QUIC_C_OPEN_ACTIVE`, `QUIC_C_RECEIVE`, `QUIC_C_CLOSE`, `QUIC_C_ACCEPT`), so the module side is complete. Only the socket-side and the C++ interface definition are missing.

---

## Sites currently crashing real simulations

In order of severity:

1. **All QuicSocket operations** (sites 12–17) — every QUIC simulation crashes at first `bind()`. Highest priority.
2. **TcpSocket::read** (site 1) — crashes any TCP app that disables `autoRead` (i.e., uses manual read mode): `TcpServerSocketIo`, `TcpClientSocketIo`, `TcpEchoApp`, `TcpSessionApp`, `TcpSinkApp`, `TcpBasicClientApp`, `TelnetApp`.
3. **TcpSocket::setDscp + setTos** (sites 3, 4) — crashes any TCP app that sets `dscp` or `tos` parameters (all `TcpAppBase`-derived apps when those parameters are non-zero). Very common in QoS simulations.
4. **L3Socket::bind + close + destroy** (sites 18, 20, 21) — crashes `PingApp` when using a non-IPv4/IPv6 network protocol (generic L3 case).
5. **SctpSocket::setStreamPriority** (site 10) — crashes any SCTP app that calls `setStreamPriority()`. Not found in bundled apps but the API is public.
6. **TunSocket::close + destroy** (sites 22, 23) — dead today; will crash if lifecycle shutdown is implemented for TUN apps.
7. **SCTP dead overloads / unimplemented commands** (sites 5–9, 11) — dead code; will crash if any code path exercises them.

---

## Resurrection / lifecycle notes

- **SctpSocket::destroy** (site 8): `SCTP_C_DESTROY` is not handled by `SctpAssociation::preanalyseAppCommandEvent` (the switch falls to `throw cRuntimeError("Unknown message kind")`). Even if the `at` dispatcher were bypassed, the command would throw in the module. Implementation needed in both `ISctp`/`Sctp` AND `SctpAssociation`. Must guard against resurrecting an already-removed association (TCP precedent: `Tcp::destroy` checks `findConnForApp(socketId) == nullptr`).
- **TcpSocket::read** (site 1): requires care around autoRead mode — `TcpConnection` tracks `autoRead` state; `read()` is only meaningful when `autoRead=false`. No resurrection risk.
- **QuicSocket::destroy** (not in the 6 listed sites — `QuicSocket::destroy()` at line 182 throws `cRuntimeError("not implemented")`): a seventh crash path, separate from the message-based ones, if `destroy()` is ever called.
