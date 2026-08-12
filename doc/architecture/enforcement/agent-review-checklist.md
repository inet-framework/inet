# Agent-Review Checklist (T4 enforcement)

The tier-4 gate from [architectural-requirements.md](../architectural-requirements.md) §*Quality
attributes and enforcement*. It enforces the **semantic** architectural requirements — the ones no
compiler or linter can express — by having an LLM reviewer judge a diff against each item. Run it as
a CI step on every change (and locally before pushing).

## How to run

Input: the change under review (a diff / PR / working tree). For **each** checklist item, output one
of:

- `PASS` — the change plainly complies (or the item doesn't apply).
- `FLAG — <file>:<line> — <one-line reason>` — a clear violation.
- `QUESTION — <file>:<line> — <what to check>` — plausibly a violation but genuinely a judgment call;
  escalate to human review (T5), don't block on it.

Ground rules:

1. **Precision over recall.** Only `FLAG` on a clear violation; when unsure, `QUESTION`. A noisy gate
   gets ignored.
2. **Judge only what static checks miss.** The compiler, `clang-tidy`, and `check-architecture.sh`
   already cover the mechanical rules; you cover *semantics* (intent, logic, duplication).
3. **Respect the ledgers.** Couplings already recorded in
   [architecture-exceptions.md](../architecture-exceptions.md) or names in
   [naming-exceptions.md](../naming-exceptions.md) are known — don't re-flag them; flag only *new*
   deviations, and propose them as new ledger rows.
4. **Scope to the diff.** Review what the change adds or moves, not the whole pre-existing tree.

## Checklist

**[AR-ORG-VIS-SPLIT] Does protocol/model code contain visualization or instrumentation logic?**
FLAG if a protocol/mobility/physical module draws on a canvas, builds a figure, or references a
visualizer beyond emitting a signal. *Not a violation:* emitting a `@signal` that a visualizer
consumes from outside.

**[AR-ORG-KERNEL] Does the change reimplement or patch an OMNeT++ kernel facility inside INET?**
FLAG a private reimplementation of event scheduling, RNG, suspend/resume, breakpoints, or a hand-patch
of kernel internals. *Not a violation:* consuming a kernel API, or a documented shim with a linked
upstream issue.

**[AR-MOD-COMPOSITION] Is new behavior added by composition, or by inheritance / a growing god-module?**
FLAG a new deep inheritance chain, or a simple module/class that accretes several unrelated
responsibilities. *Not a violation:* extending a `*Base` for genuine shared machinery.

**[AR-COM-SOCKETS] Does a new application talk to a transport protocol via raw messages?**
FLAG an app that hand-rolls command/indication messages instead of using `UdpSocket`/`TcpSocket`/peer.
*Not a violation:* a new protocol implementing the socket-facing side.

**[AR-COM-DIRECT] Is a zero-time message standing in for a direct call?**
FLAG `scheduleAt(simTime(), …)` or a zero-delay `send()` used for same-instant, same-node coordination
between sibling submodules. *Not a violation:* a message that advances simulation time or crosses the
medium.

**[AR-OBS-NED-TRUTH] Does prose/code duplicate what a NED declaration owns?**
FLAG doc text that restates parameters/gates/signals/statistics already in NED, or C++ that hardcodes a
value that should be a NED parameter. *Not a violation:* referencing the NED declaration.

**[AR-OBS-INTROSPECTION] Does a new protocol ship its introspection support?**
FLAG a new protocol header/chunk added without a registered serializer, dissector, and printer.
(Partly covered by a completeness test; you catch the "registered but empty/incorrect" case.)

**[AR-CFG-INFER / DRY] Is a derivable fact restated instead of inferred?**
FLAG a manually configured value that the model could infer (e.g. interface counts), or the same
constant/parameter duplicated across sites instead of set once and propagated.

**[AR-CFG-PARAMS] Are new parameters/fields well-formed?**
FLAG a physical-quantity parameter without `@unit`, a parameter without a `default()`, or one field
that means both "user override" and "computed value." *Not a violation:* a dimensionless count.

**[AR-EXT-NOCORE] Does adding a protocol require editing core code?**
FLAG a change that adds a protocol by modifying `common/` or a dispatcher/registry switch, rather than
registering through existing contract/registration points.

**[AR-BUILD-DECLARATIVE] Are build values hardcoded?**
FLAG absolute machine paths, `-march=native`, or per-machine flags baked into build scripts instead of
declared in the build descriptors.

**[AR-QUAL-NAMING] Do new NED/`.msg`/semantic names follow the conventions?**
FLAG names that break [naming-conventions.md](../naming-conventions.md) on the NED/message side that
`clang-tidy` can't see (wrong role suffix, `Msg`/`Message` packet, abbreviated field). Propose new
findings as `naming-exceptions.md` rows.

**[AR-QUAL-LOGGING] Is a programming error logged instead of thrown?**
FLAG a violated invariant / impossible state that is written to the log and execution continues, where
it should `throw`/`ASSERT`/`check_and_cast`. *Not a violation:* informational logging.

**[AR-QUAL-TESTS] Does the change ship with tests matching its nature?**
FLAG new behavior with no accompanying unit/module/statistical/validation test (fingerprints alone
detect *that* behavior changed, not *whether it is correct*).

**[AR-QUAL-DISPLAY] Does a new module have a distinguishing icon?**
FLAG a new module type with no `@display("i=…")`, or one reusing a generic catch-all icon for a
semantically distinct role.

## Output footer

End with a one-line verdict: `REVIEW: n PASS, n FLAG, n QUESTION` and, for any `FLAG`, a suggested
ledger row (`AV-*` or `NV-*`) so the finding lands in the backlog rather than being lost.
