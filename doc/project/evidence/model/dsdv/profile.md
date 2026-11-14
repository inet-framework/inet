# DSDV optional model behavior

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [source scope and version-1 wire profile](../../protocol/dsdv/standards.md), [statement catalog](../../standard/perkins1994/catalog.md)

This document defines the model choices for optional DSDV behavior. The statement catalog owns the paper's routing requirements; these choices specify how the model connects them to its surrounding modules. Test execution and measured support belong in the [coverage ledger](coverage.md) and [results](results.md), not here.

## MAC failure feedback

Enabling `useMacFailureFeedback` makes `Dsdv` subscribe to the existing node-level `linkBrokenSignal` and resolve `arpModule` through `IArp`. Otherwise, it retains silence-based neighbor detection without resolving an ARP dependency for this option. Parameter declarations and defaults belong to [Dsdv.ned](../../../../../src/inet/routing/dsdv/Dsdv.ned).

A usable hint is a failed unicast IPv4 frame emitted by a module within the configured ad hoc interface. The handler reads the borrowed packet's retained `MacAddressReq` receiver and `NetworkProtocolInd` header. It uses `IArp::getL3AddressFor` to find the failed receiver's IPv4 identity without initiating another resolution. This profile assumes ordinary local ARP and one ad hoc IPv4 identity per neighbor MAC. Inactive modules, unrelated interfaces, non-IPv4 frames, group receivers and absent or unresolved metadata do not cause route invalidation; the existing timeouts remain active.

The table removes that neighbor's liveness record and invalidates each finite route whose current successor is the failed receiver. Each withdrawal advances its even destination sequence once to odd infinity and becomes immediately eligible for advertisement. Repeated hints do not repeatedly increment an already-invalid route. The adapter removes the corresponding owned IPv4 routes and schedules the existing advertisement callback without settling delay. It does not infer the failed receiver from a fresh route lookup for the packet's final destination, because that route may have switched successors while the frame was queued.

This is an optional heuristic. AckingMac emits a hint after an acknowledgment timeout, and ad hoc DCF emits one after retry exhaustion. Other producers can give hints for causes such as local contention; receipt of the signal is not proof that the physical link is permanently broken. The option changes neither MAC retry policy nor the [version-1 UDP encoding](../../protocol/dsdv/standards.md#version-1-wire-profile-engineering-choices).

Subscriptions remain available across stop/start. While DSDV is inactive, failure hints have no effect; normal startup and retained withdrawal floors govern recovery. No failed packet or borrowed route pointer is retained beyond its callback.
