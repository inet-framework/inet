# Code quality rules

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `QR-*` · **Stands on:** [naming.md](naming.md), [architecture.md](architecture.md)

How the code of INET reads, and what keeps it readable. A human reads this code to learn what a
protocol does, and often to learn what the *standard* says. Every rule here serves that reader.

[naming.md](naming.md) is the authority on every name. This document starts where a name ends: the
shape of a file, what a comment is for, the size budgets, and the idioms for errors and logging.
[architecture.md](architecture.md) says where code goes; this one says how it reads once it is there.

A rule has a stable identifier `QR-<AREA>`, and the identifier is the heading, so a citation links to
the rule: [QR-ERR-THROW](quality.md#qr-err-throw).

## What this document does not cover

| Subject | Owner |
| --- | --- |
| Every name | [naming.md](naming.md) |
| Where a file goes, and what may depend on what | [architecture.md](architecture.md) |
| Which test a change ships with | [testing.md](testing.md) |
| How a change is divided into commits | [pull-request.md](pull-request.md) |
| What a document must look like | [documentation.md](documentation.md) |

## Index

Every quality rule in document order.

**Comments and structure**

| Rule | Statement |
| --- | --- |
| [QR-CMT-WHY](#qr-cmt-why) | A comment says why, because the code already says what |
| [QR-CMT-STANDARD](#qr-cmt-standard) | Normative behavior cites the standard clause it realizes |
| [QR-CMT-UNIT](#qr-cmt-unit) | A field that has a unit or a range says so |
| [QR-CMT-NO-HISTORY](#qr-cmt-no-history) | A comment describes what is, not what changed |
| [QR-CMT-NO-DEAD](#qr-cmt-no-dead) | Commented-out code is deleted |

**Errors and logging**

| Rule | Statement |
| --- | --- |
| [QR-ERR-THROW](#qr-err-throw) | A programming error throws; it is never only logged |
| [QR-ERR-CAST](#qr-err-cast) | A downcast that must succeed is a `check_and_cast` |
| [QR-LOG-LEVEL](#qr-log-level) | A log level means the same thing everywhere |
| [QR-LOG-NO-COST](#qr-log-no-cost) | Logging must not change behavior, and must not cost when off |

**Size and shape**

| Rule | Statement |
| --- | --- |
| [QR-SIZE](#qr-size) | A function, a file and a line have budgets |
| [QR-STATE-OWNER](#qr-state-owner) | One piece of state has one owner |
| [QR-DUP](#qr-dup) | A constant is defined once and propagated |
| [QR-TODO](#qr-todo) | A TODO carries an issue number |

## Comments and structure

### QR-CMT-WHY

**A comment says why the code is as it is; the code already says what it does.**

The expensive knowledge in a protocol model is not what a line does but why it must do it: which case
it handles, which peer misbehaves without it, which alternative was tried and failed. A comment that
restates the line costs the reader time and rots at the first edit. A comment that carries the reason
survives every refactor of the code beneath it.

*Enforced at T4 — agent review.*

### QR-CMT-STANDARD

**Code that realizes normative behavior cites the standard, the revision and the clause.**

`// IEEE Std 802.11-2020, 10.3.2.3` beside a timing formula turns an unfalsifiable line into a
checkable one: a reviewer can compare it against the clause instead of against intuition or against
another simulator. Untraceable normative logic is where invented behavior enters a model — a timeout
that seems right, a validity check borrowed from a different amendment — and review cannot catch it,
because there is nothing to check against. A modeling simplification is exempt, but it must be
*labeled* as one.

The domain documents make this sharper where the risk concentrates; see
[AR-WLAN-STD-TRACE](../domain/ieee80211.md#ar-wlan-std-trace).

*Enforced at T4 — agent review.*

### QR-CMT-UNIT

**A field or a parameter that carries a unit, a range or an encoding says so where it is declared.**

A `double` does not say "seconds", and an `int` does not say "in units of 32 microseconds, as the
standard encodes it". Where the type system can carry it, it should — `@unit` in NED, the unit types
of `common/Units.h` in C++ — and where it cannot, the comment carries it. A wrong unit is the
cheapest bug to write and one of the most expensive to find, because the model runs and the numbers
look plausible.

*Enforced at T1 — `@unit` and the unit types, where they apply; T4 for the rest.*

### QR-CMT-NO-HISTORY

**A comment describes what the code is, not what it used to be.**

No `// changed 2019`, no `// was int before`, no name of whoever made a change. Git holds the history
with more precision than a comment can, and a history comment is wrong from the first edit that
nobody updates. The reason a change was made belongs in its commit message
([PR-MSG-WHY](pull-request.md#pr-msg-why)), where it stays attached to the diff it explains.

*Enforced at T4 — agent review.*

### QR-CMT-NO-DEAD

**Commented-out code is deleted, not kept.**

A block behind `//` or `#if 0` is code that no compiler checks, no test runs and no reader can trust:
it may not even build. Git holds it if it is ever wanted again. Keeping it costs every later reader a
decision about whether it matters.

*Enforced at T3 — [check-cpp.sh](../enforcement/check-cpp.sh); T4 for the cases a linter misses.*

## Errors and logging

### QR-ERR-THROW

**A violated invariant throws; it is never logged while execution continues.**

There are two kinds of bad news, and they need different handling. A **modeling condition** — a
malformed frame, an unreachable destination, a timeout — is behavior, and the model handles it and
may log it. A **programming error** — an impossible state, a broken invariant, a null that cannot be
null — is a defect, and continuing past it produces results that look valid and are not. Throw
`cRuntimeError`, or assert. The simulation stops at the first cause instead of at the tenth symptom.

*Enforced at T4 — agent review; the checklist has an item for it.*

### QR-ERR-CAST

**A downcast that must succeed is `check_and_cast`, never a bare `dynamic_cast` whose result is not
tested.**

`check_and_cast` throws with the type it wanted and the type it got, at the site where the assumption
broke. An untested `dynamic_cast` gives a null pointer that travels until it is dereferenced
somewhere else, and the stack trace then names the wrong module.

*Enforced at T3 — [check-cpp.sh](../enforcement/check-cpp.sh) (`bugprone-*`).*

### QR-LOG-LEVEL

**A log level means the same thing in every model.**

`EV_ERROR` for a condition that breaks the model's contract with its peer. `EV_WARN` for a condition
worth noticing that the model handles. `EV_INFO` for the events a user follows to understand a run.
`EV_DETAIL` and below for the internals. A model that logs its normal operation at `EV_ERROR` makes
the level useless for every other model, because a user cannot filter on it any more.

*Enforced at T4 — agent review.*

### QR-LOG-NO-COST

**Logging never changes behavior, and it costs nothing when it is off.**

No side effect inside a log statement — no increment, no state change, no call that allocates or
consumes a random number. A model whose behavior differs between a run with logging and one without
breaks [AR-QUAL-DETERMINISM](architecture.md#ar-qual-determinism) and makes every fingerprint
meaningless. Build the expensive string inside the `EV_` guard, not before it.

*Enforced at T2 — a fingerprint run with logging on and off; T4 for the side-effect case.*

## Size and shape

### QR-SIZE

**A function, a file and a line have budgets, and going over one is a signal to divide.**

| Thing | Budget |
| --- | --- |
| a line | 120 characters |
| a function | 60 lines |
| a file | 800 lines |

A budget is a signal, not a law: a switch over forty frame types is one idea and may run long, and a
state machine may not divide cleanly. But a function over the budget usually holds two ideas, and a
file over it usually holds two responsibilities — which is
[AR-MOD-COMPOSITION](architecture.md#ar-mod-composition) arriving from the other direction.

**Where the code sits today is not written here.** That is a measurement, and a measurement copied
into a document is wrong by the next commit. Run the count when you want the number; do not bring the
answer back.

*Enforced at T3 — a size report, advisory rather than blocking.*

### QR-STATE-OWNER

**One piece of state has exactly one owner, and everyone else reads it through that owner.**

Two modules that both maintain "the current data rate" will disagree, and the run in which they
disagree is the one nobody can explain. Decide the owner before you write the field. This is what
makes a model's behavior attributable to a module, which every debugging session and every
sequence chart depends on.

*Enforced at T4 — agent review.*

### QR-DUP

**A constant, a limit or a default is defined once, and propagated.**

The same number written in the NED default, in the C++ initializer and in the test is three numbers
that will diverge. Define it where it belongs — the NED parameter, or one named constant — and let
everything else read it. This is [AR-CFG-INFER](architecture.md#ar-cfg-infer) at the scale of a
single value.

*Enforced at T4 — agent review.*

### QR-TODO

**A TODO carries an issue number, or it is not a TODO.**

`// TODO fix this properly` is a note to nobody: it has no owner, no date and no way to be found
again. `// TODO(#1234)` is a promise that something tracks. A branch that carries a bare TODO is not
ready ([PR-REQ-CLEAN](pull-request.md#pr-req-clean)).

*Enforced at T3 — a grep for a TODO with no number.*
