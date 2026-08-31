# Test rules

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `TR-*` · **Stands on:** [architecture.md](architecture.md), [test-anatomy.md](../design/test-anatomy.md)

Which test backs which claim, when a recorded expectation may change, and what CI must run. What the
twelve test categories *are* is [design/test-anatomy.md](../design/test-anatomy.md); this document
says what a change owes them.

The rules exist because of one asymmetry: **a fingerprint proves that behavior changed, never that
behavior is correct.** A model that has been wrong since the day it was written has a perfectly
stable fingerprint. The wide net and the specific claim are two different instruments, and a change
needs both.

A rule has a stable identifier `TR-<AREA>`, and the identifier is the heading:
[TR-CAT-MATCH](testing.md#tr-cat-match).

## Index

Every test rule in document order.

**Which test**

| Rule | Statement |
| --- | --- |
| [TR-CAT-MATCH](#tr-cat-match) | The test category matches the kind of claim the change makes |
| [TR-SHIP-WITH](#tr-ship-with) | New behavior ships with its test, in the same pull request |
| [TR-FP-NOT-ENOUGH](#tr-fp-not-enough) | A fingerprint is never the only test of new behavior |
| [TR-VALIDATE-EXTERNAL](#tr-validate-external) | A claim about the real world is checked against something outside INET |

**Recorded expectations**

| Rule | Statement |
| --- | --- |
| [TR-BASELINE-DELIBERATE](#tr-baseline-deliberate) | A baseline changes on purpose, never as a side effect |
| [TR-BASELINE-PROVENANCE](#tr-baseline-provenance) | A baseline change names its cause and its reason |
| [TR-BASELINE-COMMIT](#tr-baseline-commit) | A baseline update is its own commit |

**Determinism**

| Rule | Statement |
| --- | --- |
| [TR-DETERMINISTIC](#tr-deterministic) | The same seed gives the same trajectory, on every platform and every thread count |
| [TR-NO-AMBIENT](#tr-no-ambient) | A model draws only from its own random number generators |
| [TR-SELF-CONTAINED](#tr-self-contained) | A test depends on nothing outside its own directory |

**CI**

| Rule | Statement |
| --- | --- |
| [TR-CI-EVERY-COMMIT](#tr-ci-every-commit) | CI builds and tests every commit of a branch, not only its head |
| [TR-CI-FEATURE-MATRIX](#tr-ci-feature-matrix) | A feature is built with its neighbours switched off |
| [TR-FLAKY](#tr-flaky) | A flaky test is repaired or disabled with an issue, never re-run until green |

## Which test

### TR-CAT-MATCH

**The test category matches the kind of claim the change makes.**

| The change claims | The test that establishes it |
| --- | --- |
| a computation is right | `unit` |
| a module behaves so, given these inputs | `module` |
| a protocol interaction follows this sequence | `protocol` |
| a serializer round-trips | `unit`, plus the fingerprint `D` ingredient |
| a datapath element chains correctly | `queueing` |
| a statistic has this distribution | `statistical` |
| the model matches the real world or an analytical result | `validation` |
| a feature builds alone | `features` |
| nothing else changed | `fingerprint` |

A test in the wrong category is persuasive and empty. A module test cannot establish a distribution,
and a statistical test cannot establish that a field is encoded correctly.

*Enforced at T4 — agent review: does the test type match the claim?*

### TR-SHIP-WITH

**New behavior ships with its test, in the same pull request.**

Not in a follow-up, and not in an issue. A test written later is written against the code as it
turned out, not against the behavior that was intended, and it will pass over a defect that was in
the code from the start. The test is also the only durable statement of what the change was *for*.

*Enforced at T4 — agent review; T2 once coverage is gated.*

### TR-FP-NOT-ENOUGH

**A fingerprint is never the only test of new behavior.**

A fingerprint says the trajectory differs from the recorded one. It cannot say which of the two is
right, and for new behavior there is no recorded one at all — the first baseline simply records
whatever the new code does, including its bugs. Fingerprints are the net that catches an *unintended*
change; the claim needs its own test. See
[REJ-09](../design/rejected-designs.md#rej-09) for the case in full.

*Enforced at T4 — agent review.*

### TR-VALIDATE-EXTERNAL

**A claim about the real world is checked against something outside INET.**

An analytical result, a measurement, a reference implementation, or a figure from the standard or a
paper. A model checked only against itself is checked against nothing: it will reproduce its own
error exactly. The external reference and the tolerance both go in the test, so a later reader can
judge whether the agreement is good.

*Enforced at T2 — the validation suite; T5 for whether the reference is the right one.*

## Recorded expectations

### TR-BASELINE-DELIBERATE

**A recorded expectation changes because someone decided it should, never as a side effect of
another change.**

A fingerprint `.csv`, a statistical baseline, an expected output: each is a claim that *these values
are correct*. Regenerating one to make CI green is the single fastest way to lose every regression
guarantee the suite provides, and it is invisible in a large diff — which is precisely why
[PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline) puts it in a commit of its own.

*Enforced at T3 — a per-commit check that source and baseline do not change together.*

### TR-BASELINE-PROVENANCE

**A baseline change names the commit that causes it and the reason the new values are right.**

"Fingerprints updated" is not a reason. Name the change, say which behavior moved, and say why the
new trajectory is the correct one — a standard clause, a repaired defect, a deliberate model change.
Two years later this line is the only thing that can tell a bisecting developer whether the change
was intended.

*Enforced at T4 — agent review of the message against the diff.*

### TR-BASELINE-COMMIT

**A baseline update is its own commit, directly after the commit that changes behavior.**

The rule is [PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline); it is repeated here because it is
a test rule as much as a commit rule. Inside a source commit the update is invisible, and "the
fingerprint changed" stops being a conscious decision.

*Enforced at T3 — the same per-commit check.*

## Determinism

### TR-DETERMINISTIC

**The same seed gives the same trajectory: on every platform, in every build mode, at every thread
count.**

Determinism is the precondition for every other guarantee in this document. A fingerprint over a
non-deterministic model is noise, a bug report that cannot be reproduced cannot be repaired, and a
published result that nobody can re-run is not a result. Anything that varies between runs —
iteration over a container ordered by pointer value, a time or a hostname read at run time, a
floating-point path that depends on thread count — is a defect, not a quirk.

*Enforced at T2 — fingerprint stability across seeds, modes and platforms.*

### TR-NO-AMBIENT

**A model draws only from the random number generators the kernel gives it.**

No `rand()`, no `std::random_device`, no static generator of its own. The number and the order of the
draws are part of the result: a model that takes one extra draw in a branch changes every later draw
in the run, so a refactor that "cannot change behavior" does.

*Enforced at T3 — a grep for the forbidden generators; T2 by fingerprint drift.*

### TR-SELF-CONTAINED

**A test depends on nothing outside its own directory, and leaves nothing behind.**

No file from another test's output, no fixed absolute path, no network, and no dependence on the
order the suite happens to run in. A test that passes only after its neighbour has run is a test that
will fail alone, in parallel, and on the machine of whoever is trying to reproduce a defect.

*Enforced at T2 — the suite runs in parallel and in isolation.*

## CI

### TR-CI-EVERY-COMMIT

**CI builds and tests every commit of a branch, not only its head.**

`git bisect` is the fastest tool for a regression nobody can explain, and one broken middle commit
makes it useless. This is the mechanical half of
[PR-SERIES-BUILDS](pull-request.md#pr-series-builds), which is otherwise a rule that nothing checks.

*Enforced at T2 — not yet configured; the workflows test the head only.*

### TR-CI-FEATURE-MATRIX

**A feature is built with its neighbours switched off, not only in the full build.**

The full build hides every accidental dependency, because everything a model reaches for happens to
be present. The feature-off build is the only check that
[AR-EXT-FEATURES](architecture.md#ar-ext-features) and the dependency direction of
[AR-ORG-DOMAINS](architecture.md#ar-org-domains) are still real.

*Enforced at T3 — the feature workflow and `inet_featuretool`.*

### TR-FLAKY

**A flaky test is repaired, or disabled with an issue number. It is never re-run until it passes.**

A re-run until green teaches the whole project that a red CI means nothing, and from then on a real
regression is waved through with the same reflex. A test that is flaky is either testing something
non-deterministic — which is a model defect under
[TR-DETERMINISTIC](#tr-deterministic) — or testing it wrongly. Both are worth finding.

*Enforced at T5 — human discipline; nothing can check this one.*
