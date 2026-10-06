# What a test is made of

> **Kind:** design · **Status:** current · **Seal:** by section · **Owns:** — · **Stands on:** [rule/testing.md](../rule/testing.md), [repository-layout.md](repository-layout.md)

The twelve test categories under `tests/`, what each one can establish, and what it cannot. Which one
a change owes is [rule/testing.md](../rule/testing.md); how to run them is
`doc/src/developers-guide/ch-testing.rst`.

The categories identify typical test infrastructure and uses. Assess each claim from the actual fixture, executed path, inputs, and assertions under [TR-CAT-MATCH](../rule/testing.md#tr-cat-match).

## The categories

| Category | Establishes | Cannot establish |
| --- | --- | --- |
| `unit` | a computation, a serializer round-trip, a data structure | anything about a running network |
| `module` | module behavior, including integration when the fixture connects production components | behavior outside the exercised components, paths, and assertions |
| `protocol` | an interaction between peers follows this sequence | a distribution, or a rate |
| `queueing` | datapath elements chain and transfer correctly | end-to-end behavior |
| `packet` | the chunk algebra holds | how a protocol uses it |
| `networks` | a pre-assembled network builds and runs | that its results are right |
| `statistical` | recorded simulation statistics match their baseline; a distribution only when the test explicitly checks it | distributional validity from baseline equality alone, or which mechanism produced a difference |
| `validation` | the model agrees with the real world or an analytical result | that nothing else changed |
| `fingerprint` | selected runs match recorded hashes for the chosen ingredients | correctness, unselected behavior, or changes outside those ingredients |
| `speed` | a run costs this much time | correctness of any kind |
| `features` | a feature builds with its neighbours off | that it works |
| `misc` | what does not fit above | — |

For example, [udpapp_lifecycle_6.test](../../../tests/module/udpapp_lifecycle_6.test) connects two `StandardHost` nodes. It changes their lifecycle state and checks received traffic. Its module-test location does not prevent integration evidence; its configured operations and assertions bound that evidence.

## The production path

**A test that calls a helper directly establishes only the helper's contract.** It does not establish
that a production module invokes the helper, supplies the intended inputs, or lets the result affect
observable behavior. A fixture that reproduces the production selection or dispatch logic has the
same limitation: it tests the reproduced path, not the integrated one.

A claim that a helper is integrated into model behavior therefore needs module or protocol evidence
that enters through the production gate, API or configuration and observes the resulting behavior.
Helper tests remain useful for computation boundaries that the integration test does not cover. Select complementary checks for the actual claims under [TR-FOCUSED-EVIDENCE](../rule/testing.md#tr-focused-evidence). Do not duplicate a sufficient assertion solely to populate another test category.

## The one that is different

**A fingerprint is not a test of correctness.** It hashes selected ingredients from a configured run and compares the result with a recorded value. A match supplies regression evidence within that scope; it does not prove that every behavior remains unchanged. A model can preserve the same defect and produce a stable fingerprint. A mismatch alone cannot distinguish a correction from a regression.

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

The Python `inet_run_statistical_tests` runner compares recorded scalar results, including results
converted from vectors, with the separate `statistics` baseline checkout. Its default selection is
run 0. Equality is regression evidence for those recorded results, not a hypothesis test or evidence
from independent repetitions. The legacy R-based `.test` checks under `tests/statistical/` are a
different harness; their assertions determine what they establish. For a distributional claim,
define the experiment and uncertainty under
[analyze-simulation-results.md](../guide/analyze-simulation-results.md).

Three categories compare against a recorded value rather than an assertion: `fingerprint`,
`statistical` and any `.test` with an expected output. All three are claims that *these values are
correct*, and all three change only under
[TR-BASELINE-DELIBERATE](../rule/testing.md#tr-baseline-deliberate),
[TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance) and
[TR-BASELINE-COMMIT](../rule/testing.md#tr-baseline-commit) — deliberately, with a stated reason, in
the commit that moves them.

## What holds it all up

Every category rests on [TR-DETERMINISTIC](../rule/testing.md#tr-deterministic): the same seed gives
the same trajectory, on every platform and at every thread count. Without it a fingerprint is noise,
a statistical test measures the scheduler, and a defect report cannot be reproduced. Determinism is
not one property among twelve; it is the precondition for all of them.
