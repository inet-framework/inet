# What a test is made of

> **Kind:** design · **Status:** current · **Seal:** by section · **Owns:** — · **Stands on:** [rule/testing.md](../rule/testing.md), [repository-layout.md](repository-layout.md)

The twelve test categories under `tests/`, what each one can establish, and what it cannot. Which one
a change owes is [rule/testing.md](../rule/testing.md); how to run them is
`doc/src/developers-guide/ch-testing.rst`.

The categories are not twelve flavours of the same thing. Each answers a different question, and a
test in the wrong category is persuasive and empty
([TR-CAT-MATCH](../rule/testing.md#tr-cat-match)).

## The categories

| Category | Establishes | Cannot establish |
| --- | --- | --- |
| `unit` | a computation, a serializer round-trip, a data structure | anything about a running network |
| `module` | one module behaves so, given these inputs | that the module composes with others |
| `protocol` | an interaction between peers follows this sequence | a distribution, or a rate |
| `queueing` | datapath elements chain and transfer correctly | end-to-end behavior |
| `packet` | the chunk algebra holds | how a protocol uses it |
| `networks` | a pre-assembled network builds and runs | that its results are right |
| `statistical` | a measured quantity has this distribution | which mechanism produced it |
| `validation` | the model agrees with the real world or an analytical result | that nothing else changed |
| `fingerprint` | **nothing else changed** | whether the behavior is correct |
| `speed` | a run costs this much time | correctness of any kind |
| `features` | a feature builds with its neighbours off | that it works |
| `misc` | what does not fit above | — |

## The one that is different

**A fingerprint is not a test of correctness.** It hashes the event trajectory of a configuration and
compares it against a recorded value. That is a wide net for an *unintended* change, and it is the
only instrument that covers everything at once. It is also blind in a specific way: a model that has
been wrong since the day it was written has a perfectly stable fingerprint, and a repair and a
regression look identical to it.

This is why [TR-FP-NOT-ENOUGH](../rule/testing.md#tr-fp-not-enough) exists, and why
[REJ-09](rejected-designs.md#rej-09) records the argument for fingerprints as the whole suite and why
it lost.

## What every test holds

1. **A network**, or the fixture the category uses. The smallest one that shows the behavior.
2. **A configuration** that names the scenario, and nothing beyond it.
3. **The claim**, as an assertion, an expected output, or a recorded baseline.
4. **A reason the claim is right**, where a reader could not derive it: the standard clause, the
   analytical formula, the reference measurement.

The fourth is the one most often left out, and it is the one that decides whether the test can be
maintained. A baseline with no reason is a number that the next person will regenerate.

## The recorded expectations

Three categories compare against a recorded value rather than an assertion: `fingerprint`,
`statistical` and any `.test` with an expected output. All three are claims that *these values are
correct*, and all three change only under
[TR-BASELINE-DELIBERATE](../rule/testing.md#tr-baseline-deliberate),
[TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance) and
[TR-BASELINE-COMMIT](../rule/testing.md#tr-baseline-commit) — deliberately, with a stated reason, in
a commit of their own.

## What holds it all up

Every category rests on [TR-DETERMINISTIC](../rule/testing.md#tr-deterministic): the same seed gives
the same trajectory, on every platform and at every thread count. Without it a fingerprint is noise,
a statistical test measures the scheduler, and a defect report cannot be reproduced. Determinism is
not one property among twelve; it is the precondition for all of them.
