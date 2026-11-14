# DSDV optional model behavior

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [source scope and version-1 wire profile](../../protocol/dsdv/standards.md), [statement catalog](../../standard/perkins1994/catalog.md)

This document defines the model choices for optional DSDV behavior. The statement catalog owns the paper's routing requirements; these choices specify how the model connects them to its surrounding modules. Test execution and measured support belong in the [coverage ledger](coverage.md) and [results](results.md), not here.

## MAC failure feedback

Enabling `useMacFailureFeedback` makes `Dsdv` subscribe to the existing node-level `linkBrokenSignal` and resolve `arpModule` through `IArp`. Otherwise, it retains silence-based neighbor detection without resolving an ARP dependency for this option. Parameter declarations and defaults belong to [Dsdv.ned](../../../../../src/inet/routing/dsdv/Dsdv.ned).

A usable hint is a failed unicast IPv4 frame emitted by a module within the configured ad hoc interface. The handler reads the borrowed packet's retained `MacAddressReq` receiver and `NetworkProtocolInd` header. It uses `IArp::getL3AddressFor` to find the failed receiver's IPv4 identity without initiating another resolution. This profile assumes ordinary local ARP and one ad hoc IPv4 identity per neighbor MAC. Inactive modules, unrelated interfaces, non-IPv4 frames, group receivers and absent or unresolved metadata do not cause route invalidation; the existing timeouts remain active.

The table removes that neighbor's liveness record and invalidates each finite route whose current successor is the failed receiver. Each withdrawal advances its even destination sequence once to odd infinity and becomes immediately eligible for advertisement. Repeated hints do not repeatedly increment an already-invalid route. The adapter removes the corresponding owned IPv4 routes and schedules the existing advertisement callback without settling delay. It does not infer the failed receiver from a fresh route lookup for the packet's final destination, because that route may have switched successors while the frame was queued.

This is an optional heuristic. AckingMac emits a hint after an acknowledgment timeout, and ad hoc DCF emits one after retry exhaustion. Other producers can give hints for causes such as local contention; receipt of the signal is not proof that the physical link is permanently broken. The option changes neither MAC retry policy nor the [version-1 UDP encoding](../../protocol/dsdv/standards.md#version-1-wire-profile-engineering-choices).

Subscriptions remain available across stop/start. While DSDV is inactive, failure hints have no effect; normal startup and retained withdrawal floors govern recovery. No failed packet or borrowed route pointer is retained beyond its callback.

## Adaptive periodic full dumps

Enabling `adaptiveFullDump` adjusts periodic scheduling from the initial `fullDumpInterval`, within the configured interval bounds and bitrate target. The declarations and defaults of these settings belong to [Dsdv.ned](../../../../../src/inet/routing/dsdv/Dsdv.ned). Intervals and budget must be finite and positive, with minimum <= initial <= maximum. This is an engineering policy for the paper's qualitative suggestions, not a formula specified by the paper or a measurement of channel occupancy.

The table counts significant selected-route transitions: newly reachable destinations, finite-to-infinite withdrawals and metric or successor changes. Count one transition once even when several conditions change. Duplicate updates, same-path sequence refreshes, periodic self-sequence advances and corrective resends without a selected-route change do not count. The counter saturates at the entry capacity K of one outgoing datagram; a zero limit disables collection. Startup resets the history after initializing the table, while later neighbor discovery belongs to the first observation window.

At each periodic callback, advance the self sequence once and submit the complete full dump. Let C be the accumulated change count and B the sum of the encoded DSDV payload bytes in its fragments. Choose the next interval in three steps:

1. Propose 2 × I for C = 0, I / 2 for C >= K, or I otherwise.
2. Raise the proposal to at least 8 × B / fullDumpBitrateBudget, then clamp it to the configured minimum and maximum using representable simulation times.
3. Store that interval, clear the consumed history and schedule the existing periodic timer at the selected interval plus its usual jitter draw.

The threshold K counts transition events, not necessarily distinct destinations or the exact next incremental size. As a configured arithmetic example, choose I = 1 s, minimum 0.5 s, maximum 4 s, K = 2, B = 80 bytes and a 11200 bps budget. The result is 2 s for C = 0 and 0.5 s for C = 2. Changing that example's budget to 320 bps raises either proposal to at least 2 s; a 128 bps budget requests 5 s, but the 4 s maximum takes precedence. These example settings are not a list of parameter defaults. The target can be exceeded.

The bitrate target covers periodic DSDV payload bytes only. It excludes UDP/IP headers, radio airtime and triggered traffic. Urgent updates, corrections and required-entry overflow dumps use the existing advertisement path; sending them does not reset the periodic timer or advance the self sequence. Preparing a dump consumes dissemination state, so byte accounting uses the submitted fragments rather than preparing a second copy. `fullDumpIntervalSelected` records the selected interval once per adaptive periodic callback. The disabled path uses neither the new statistic nor another random draw or timer.

Receiver liveness depends on peer sending intervals, not the receiver's own interval. In a loss-free fixture, its neighbor timeout should exceed the largest peer periodic interval plus peer jitter and a delivery allowance; its route timeout must also allow the relevant settling delay. Include fixed-period peers in that calculation. The model retains positive-timeout validation but does not reject a receiver whose own full-dump interval exceeds its receive timeout. It cannot derive a network-wide timing bound from local parameters, and losses or unbounded latency can still cause expiry.

An illustrative heterogeneous fixture uses A with initial/minimum 2 s, maximum 4 s, neighbor timeout 1.5 s and route timeout 2 s; B uses initial/minimum 0.25 s, maximum 0.5 s and both timeouts 6 s. With zero jitter, a 0.1 s settling cap and delivery allowance below 0.05 s, each receiver's timeouts cover its peer's gap. A's receive timeout can legitimately be shorter than A's own sending interval. This is a configuration oracle, not a claim that this scenario has been executed.

Stop, crash and ad hoc interface loss cancel the existing timers. Restart resets the selected interval to `fullDumpInterval` and clears adaptation history while preserving the protocol's retained sequence floors. No performance improvement is implied by enabling the policy; delivery, convergence and total control traffic require their own measurements.
