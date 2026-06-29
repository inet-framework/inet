# Pattern-language refactor — orthogonal selector functionals

Status: **pending** — design converged (brainstormed with the user), not yet implemented.

## Motivation

The current `EventPattern` selector overloads several distinct concerns onto a few names,
which makes the vocabulary confusing and non-orthogonal:

- `on("host1")` conflates *where we subscribe* with a *source-node filter*.
- `.layer(L)` is a coarse heuristic for the **source module's** stack position (it never
  looked at the packet protocol).
- `.sentToLower()` / `.receivedFromUpper()` / `.dropped()` encode the **signal** in the
  method name.
- the state channel (`state()` + `reaches()`/`neverReaches()`) is a parallel, separate
  mechanism for scalar signals.

Refactor to **one functional per concern**.

## New vocabulary (one functional per concern)

| Concern | Now | After |
|---|---|---|
| Where to subscribe (listener scope) | implicit — global at network root | `on(path)` |
| Which signal | name-encoded kinds + separate `state()` channel | `signal(name)` (packet + state + marker) |
| Source (emitting module) | `on("host1")` (node) + `.layer(L)` (stack guess) | `source(path)` |
| Packet protocol (`PacketProtocolTag`) | `.protocol()` / `on("n.x")` | `protocol(name)` |
| Dispatch protocol (`DispatchProtocolReq`) | — | `dispatch(name)` |
| Direction | `.inbound()` / `.outbound()` | `direction(in/out)` — or implied by signal |
| Interface | `.iface("eth0")` | `iface(name)` |
| Packet content (value is a packet) | `.match("expr")` | `packet(expr)` (PacketFilter; asserts the value is a Packet) |
| Generic signal value (non-packet) | `.is(v)` on `state(...)`; `.match(lambda)` | `match(expr)` (general value predicate) |
| Description point of view | — *(framework guessed)* | `attributeTo(path)` |
| Timing | `.within()` `.after()` `.notBefore()` | unchanged |
| Capture / doc | `.capture()` `.describe()` | unchanged |

`layer` is removed (subsumed by `source`).

## `attributeTo(path)`

A **description-only** hint naming the module whose point of view the clause is narrated
from. It never affects matching. The describer derives the verb from the signal direction
relative to the attributed module: for a `…FromUpper` / `…ToUpper` signal, attributing to the
*counterpart* (the upper module) flips send↔receive. The readable subject name comes from the
step's `protocol(name)` (via the Protocol registry) when present, else the module path.

- with `attributeTo("MN[0].ipv6.mipv6")` on a `packetReceivedFromUpper` + `protocol("mobileipv6")`
  step → *"MN[0]'s Mobile IPv6 sends a Binding Update (…)."*
- without it → *"MN[0].ipv6 is handed a `mobileipv6` Binding Update from above."* (honest source POV)

## `on` vs `source` (the key distinction)

- **`on(path)`** = the module we `subscribe(signal, listener)` on. Bounds which emissions
  reach us (only `on`'s subtree, since signals propagate up to ancestors). Default: network
  root. Today the tester subscribes globally and `on` is a post-filter, so today's `on`
  behaves like a coarse `source`; after the refactor `on` is the real subscribe target.
- **`source(path)`** = a filter on the `source` argument of `receiveSignal(source, …)` — the
  actual emitting component — within the subscribed scope.

## `signal(name)` unification

Parameterize the signal instead of name-encoding it: `signal("packetSentToLower")`,
`signal("controlStateChanged")` (scalar/FSM state), `signal("mipv6RoCompleted")` (marker).
One mechanism for the packet, state, and marker channels — retires the kind-named methods
and the separate `state()`/`reaches()` channel.

## `packet(expr)` vs `match(expr)`

- `packet(expr)` — PacketFilter content expression; asserts the signal value is a `Packet`.
- `match(expr)` — general value predicate (non-packet signal values, e.g. an FSM state
  index), retiring the special-case `.is()`.

## `dispatch(name)`

Filter on the `DispatchProtocolReq` tag (where the packet is *headed*), distinct from
`protocol(name)` (`PacketProtocolTag`, what it *is*). Also the clean primitive for
receive-side module attribution (a packet *dispatched to* `mobileipv6`).

## Migration

- **No back-compat aliases** — all call sites are in-repo (same approach as the cardinality
  rename). Migrate all sample tests in `ProtocolTests.cc`, the `.test` files, and `AUTHORING.md`.
- Update the matcher (`EventPattern`/`ProtocolTester`) and the describer to the new primitives.

## Subject rendering — resolved by `attributeTo`

The earlier (a) honest-source vs (b) attribute-to-mipv6 dilemma is dissolved: the selector
stays honest/precise, and `attributeTo(path)` lets the *author* opt into a different narration
point of view per step. Default = honest source POV; add `attributeTo(...)` for mipv6 (or any
counterpart) as the subject. No global policy needed.
