# Agent-Review Checklist (T4 enforcement)

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/architecture.md](../../rule/architecture.md), [README.md](../README.md)
The tier-4 gate from [architectural-requirements.md](../../rule/architecture.md) §*Quality
attributes and enforcement*. It enforces the **semantic** architectural requirements — the ones no
compiler or linter can express — by having an LLM reviewer judge a diff against each item. Run it as
a CI step on every change (and locally before pushing). For diffs touching
`src/inet/linklayer/ieee80211/` or `src/inet/physicallayer/wireless/ieee80211/`, additionally run
the [IEEE 802.11 checklist](ieee80211.md).

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
   [architecture-exceptions.md](../../audit/architecture-exceptions.md) or names in
   [naming-exceptions.md](../../audit/naming-exceptions.md) are known — don't re-flag them; flag only *new*
   deviations, and propose them as new ledger rows.
4. **Scope to the diff.** Review what the change adds or moves, not the whole pre-existing tree.

## Checklist

**[AR-ORG-CONTRACT-PURITY] Does a contract header declare anything that is not part of the role?**
FLAG a `static` helper, a utility function, a non-trivial inline body, or a policy decision added to a
C++ interface or a NED `moduleinterface`. Ask where it goes instead: the `*Base` class if it serves
implementors, the caller's side if it serves callers, the owning module if it encodes a modeling
decision. *Not a violation:* a pure virtual, a nested type the role needs, a `static simsignal_t`
signal identity, or a trivial virtual destructor.

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

**[AR-COM-DIRECT] Is a zero-time message standing in for a required direct call?**
FLAG `scheduleAt(simTime(), …)` or a zero-delay `send()` used for a same-instant command, query,
return value or required handshake between sibling submodules. *Not a violation:* a message that
represents a modeled delivery or explicit event boundary, including at the same simulation time; a
message that crosses the medium; or a fire-and-forget signal that satisfies AR-COM-NOTIFY.

**[AR-COM-NOTIFY] Is a behavior-driving signal really an independent notification?**
FLAG a publisher that depends on a particular listener, reply or relative invocation order among
listeners; a signal used for ownership transfer, buffering, modeled delay or listener-ordered
coordination; emission before the publisher restores its invariants; a behavioral module listener
that omits `Enter_Method`/`Enter_Method_Silent`, retains a pointer beyond its declared lifetime,
mutates a borrowed payload, or modifies the currently firing subscription list; or a broad ancestor
subscription that fails to filter unrelated sources. *Not a violation:* an independent consumer
reacting synchronously to completed facts in emission order without participating in the
publisher's operation.

**[AR-OBS-SIGNALS] Can attaching an observer change modeled behavior?**
FLAG a recorder, visualizer or analyzer that mutates modeled state, schedules a modeled action, or
calls back into the producer. *Not a violation:* a separate behavioral listener that satisfies
AR-COM-NOTIFY, even when it consumes a signal that is also recorded or visualized.

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
FLAG names that break [naming-conventions.md](../../rule/naming.md) on the NED/message side that
`clang-tidy` can't see (wrong role suffix, `Msg`/`Message` packet, abbreviated field). Propose new
findings as `naming-exceptions.md` rows.

**[AR-QUAL-LOGGING] Is a programming error logged instead of thrown?**
FLAG a violated invariant / impossible state that is written to the log and execution continues, where
it should `throw`/`ASSERT`/`check_and_cast`. *Not a violation:* informational logging.

**[AR-QUAL-TESTS] Does the change ship with tests matching its nature?**
FLAG new behavior with no accompanying unit/module/statistical/validation test (fingerprints alone
detect *that* behavior changed, not *whether it is correct*).

**[AR-QUAL-TRACEABILITY] Does a commit that moves a recorded expectation explain the movement?**
FLAG a commit that changes a fingerprint `.csv`, a statistical baseline or an expected output and
whose message does not say which behavior moved and why the new values are right. FLAG a
baseline-only commit that stands directly after the source commit that moves the values, and ask for
a squash: that is one change in two commits
([PR-SPLIT-BASELINE](../../rule/pull-request.md#pr-split-baseline)). *Not a violation:* a first
recording for new content, or a re-record that names a cause outside the branch, such as a compiler
or solver version change.

**[AR-QUAL-DISPLAY] Does a new module have a distinguishing icon?**
FLAG a new module type with no `@display("i=…")`, or one reusing a generic catch-all icon for a
semantically distinct role.

## Output footer

End with a one-line verdict: `REVIEW: n PASS, n FLAG, n QUESTION` and, for any `FLAG`, a suggested
ledger row (`AV-*` or `NV-*`) so the finding lands in the backlog rather than being lost.
