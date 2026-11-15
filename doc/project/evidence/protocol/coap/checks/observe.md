# CoAP — English check procedures: Observe resource observation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7641/catalog.md](../../../standard/rfc7641/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## Observe registration and initial notification

- **Checks** — [RFC7641-REG-1](../../../standard/rfc7641/catalog.md#rfc7641-reg-1), [RFC7641-REG-2](../../../standard/rfc7641/catalog.md#rfc7641-reg-2), [RFC7641-NOTIFY-1](../../../standard/rfc7641/catalog.md#rfc7641-notify-1).
- **Requirement** — `GET` with `Observe=0` registers the `(client endpoint, token)` on an observable resource and returns `2.05 Content` (or `2.03 Valid` on matching `ETag`) with an `Observe` option; re-registering with the same `(client endpoint, token)` updates the entry without duplicating it; non-observable resources or full observer tables return `2.05` without `Observe`; and resource state changes emit notifications carrying the registration Token and increasing 24-bit `Observe` sequence numbers.

### Procedure

1. Build the plain pair mockup with an observable resource `/sensors/temp` and a non-observable resource `/info`.
2. Send `GET /sensors/temp` with `Observe=0`, re-send `GET /sensors/temp` with `Observe=0` and the same Token (and with a matching `ETag`), send `GET /info` with `Observe=0`, test observer-table capacity fallback, and trigger a state update on `/sensors/temp`.
3. Observe all responses and notifications on the wire.

### Expected observations

1. `GET /sensors/temp` with `Observe=0` returns `2.05 Content` with an `Observe` option (`6`), and re-registration with the same Token and matching `ETag` returns `2.03 Valid` with `Observe` and creates no duplicate observer entry.
2. When `/sensors/temp` changes state, `S` sends exactly one notification carrying the client's registration Token and a strictly larger `Observe` sequence number.
3. `GET /info` with `Observe=0` (and `GET` when `maxObservers` is full) returns `2.05 Content` without an `Observe` option and registers no observer.

## Notification ordering and 128-second rule

- **Checks** — [RFC7641-ORDER-1](../../../standard/rfc7641/catalog.md#rfc7641-order-1).
- **Requirement** — a client accepts a notification with `Observe` value `V2` at local time `T2` over the previous accepted `(V1, T1)` if and only if `(V1 < V2 && V2 - V1 < 2^23) || (V1 > V2 && V1 - V2 > 2^23) || (T2 > T1 + 128s)`.

### Procedure

1. Build the plain pair mockup and register an observation.
2. Deliver notifications out of order (`V=10` then stale `V=9` within `128s`), deliver a notification that wraps across the 24-bit boundary (`V=16777210` then `V=5`), deliver an older serial number at exactly `T1 + 128s`, and deliver an older serial number at `T1 + 128.1s`.
3. Observe which notifications the client accepts versus discards.

### Expected observations

1. The stale notification `V=9` arriving after `V=10` within `128s` is discarded, while the wrapped notification `V=5` after `V=16777210` is accepted as newer.
2. An older serial arriving at `T2 == T1 + 128s` is discarded (`T2 > T1 + 128s` is strict), whereas an older serial arriving at `T2 > T1 + 128s` is accepted.

## Notification freshness and re-registration

- **Checks** — [RFC7641-FRESH-1](../../../standard/rfc7641/catalog.md#rfc7641-fresh-1).
- **Requirement** — when no notification arrives before the representation's `Max-Age` plus a random jitter in `[5s, 15s]` expires, the client re-registers by sending a new `GET` request with `Observe=0` and the same registration Token.

### Procedure

1. Build the plain pair mockup with an observable resource whose `Max-Age` is `10s` and no scheduled server updates.
2. Register an observation at `t = 0.1s` and observe the wire over `30s`.

### Expected observations

1. After the initial `2.05 Content` notification (`Max-Age = 10s`), no re-registration leaves `C` before `15s` (`Max-Age + 5s`) have elapsed.
2. Within `[15s, 25s]` (`[Max-Age + 5s, Max-Age + 15s]`) after the initial notification, `C` sends a new `GET` request with `Observe=0` and the original registration Token, and `S` answers with a fresh `2.05` notification.

## Confirmable notification supersession and retry removal

- **Checks** — [RFC7641-SUPER-1](../../../standard/rfc7641/catalog.md#rfc7641-super-1).
- **Requirement** — if an observed resource changes while a `CON` notification is awaiting `ACK`, the server supersedes the pending representation and updates the `Observe` option and Message ID on the next retransmission attempt without resetting the retry backoff schedule; if a `CON` notification exhausts `MAX_RETRANSMIT` without `ACK`, the server removes the observer registration.

### Procedure

1. Build the plain pair mockup with `CON` notifications enabled on an observable resource.
2. Drop the first `CON` notification on relay `T`, update the resource before the first retry timeout fires, and let the retry pass; in a second phase, drop all transmissions of a subsequent `CON` notification (`1 + MAX_RETRANSMIT` attempts), and then update the resource again.
3. Observe all `CON` notifications, Message IDs, `Observe` values, and post-timeout silence.

### Expected observations

1. The retry after the mid-backoff resource update carries the newer payload, a newer `Observe` sequence number, and a new Message ID at the original retry deadline, and `C` acknowledges it.
2. When all `1 + MAX_RETRANSMIT` (`5`) transmissions of a `CON` notification are dropped, `S` removes the observer registration and emits no notification on the subsequent resource update.

## Observation cancellation by Observe=1, Reset, and error

- **Checks** — [RFC7641-CANCEL-1](../../../standard/rfc7641/catalog.md#rfc7641-cancel-1).
- **Requirement** — an observation is cancelled when the client sends `GET` with `Observe=1` and the registration Token, when the client rejects a notification with `RST`, or when the server sends a non-`2.xx` notification (`4.04 Not Found` on resource deletion or `4.06 Not Acceptable` on incompatible format change).

### Procedure

1. Build the plain pair mockup and exercise three observation lifecycles in sequence: explicit `GET` deregistration with `Observe=1`, local cancellation where `C` answers the next notification with `RST`, and server-side resource deletion (`DELETE`) emitting a `4.04 Not Found` notification without an `Observe` option.
2. Trigger a further resource update after each cancellation and observe the wire.

### Expected observations

1. `GET` with `Observe=1` and the registration Token returns `2.05 Content` without an `Observe` option and stops future notifications.
2. Answering a notification with `RST` echoing its Message ID removes the observer at `S` so subsequent resource updates produce no notifications.
3. Deleting an observed resource causes `S` to emit a `4.04 Not Found` notification carrying the registration Token and no `Observe` option, terminating the observation at both endpoints.
