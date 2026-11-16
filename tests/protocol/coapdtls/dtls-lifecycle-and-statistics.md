# CoAP over DTLS Lifecycle and Statistics Verification

This document summarizes the C5 lifecycle and statistics protocol tests verifying graceful shutdown, crash recovery, and signal accounting for `CoapDtlsClient` and `CoapDtlsServer`.

## Lifecycle Order

```
ModuleStopOperation (CoapDtlsClient: coap -> dtls)
  coap::handleStopOperation():
    1. Abort in-flight requests (requestAborted)
    2. Terminate established Observe relations (onObserveTerminated)
    3. dtlsSocket.close(id) -> queues DTLS close_notify (with protectOutDelay)
  dtls::handleStopOperation():
    1. Retain already-closing associations in closingIds
    2. Flush close_notify before closing UDP socket

ModuleCrashOperation (CoapDtlsServer)
  dtls / coap::handleCrashOperation():
    1. Immediately clear associations and transient state without close_notify
    2. Reload XML default resource state and clear duplicate history
```

## Test Verification

- **Lifecycle Shutdown, Crash, and Restart ([CoapDtlsLifecycle.test](CoapDtlsLifecycle.test)):** Verifies `ModuleStopOperation` during a delayed handshake aborts the request with zero CoAP packets emitted, `ModuleStopOperation` during active `CON GET` + `Block1 PUT` + established `Observe` (with `protectOutDelay = 0.2s`) emits `requestAborted == 2` and `onObserveTerminated == 1`, transmits `close_notify` cleanly so the server removes `observers` and `incomingBlock1`, restarts cleanly with a higher `AssociationId`, and `ModuleCrashOperation` emits zero `close_notify` alerts while resetting server resources and clearing duplicate history.
- **Signals & Statistics ([CoapDtlsStatistics.test](CoapDtlsStatistics.test)):** Verifies `requestStarted`, `requestCompleted`, `requestFailed`, `requestAborted`, `requestLatency`, `retransmission`, `duplicateReceived`, `activeObservations`, `activeObservers`, DTLS `associationEstablished`, and `recordOverhead` across Observe registration, local cached hit (`0` DTLS records / `0` latency samples), dropped-ACK retry, `sendError` oversize failure, and stop abort.
