# Claim coverage

> **Kind:** measurement · **Status:** current; skeleton · **Seal:** none · **Owns:** — · **Stands on:** [requirement/accepted-requirements.md](../requirement/accepted-requirements.md), [design/test-anatomy.md](../design/test-anatomy.md)

For each `R-*` requirement, what demonstrates it. This is the traceability that
[AR-QUAL-TRACEABILITY](../rule/architecture.md#ar-qual-traceability) asks for, in one table.

It creates no new folder and no new test. It points into `tests/`, `showcases/` and `examples/`,
which already hold the evidence — the gap it closes is that **nothing said which requirement each one
serves**, so a requirement could quietly stop being true and no failing test would say so.

## How to read a row

| Column | Means |
| --- | --- |
| Requirement | the `R-*` identifier |
| Demonstrated by | the test, showcase or example, by path |
| Kind | which test category, or `showcase` / `example` |
| Establishes | *that it works* (a test), or *what it looks like* (a showcase or example) |

A requirement with no row is not necessarily untested — it is **unmapped**, which is a different and
smaller problem. Mapping one is cheap: find the test that would fail if the requirement stopped
holding, and add the row.

## The table

| Requirement | Demonstrated by | Kind | Establishes |
| --- | --- | --- | --- |
| *unmapped* | | | |

<!--
| R-RUN-REPRO | tests/fingerprint/ | fingerprint | that it works |
| R-SCOPE-TSN | showcases/tsn/ | showcase | what it looks like |
-->

## Status

**This document is a skeleton.** Nothing is mapped yet, and that is the honest state rather than an
oversight — the mapping is a pass over 27 requirements and twelve test categories, and it is worth
doing deliberately rather than guessed at.

The order worth doing it in: the requirements whose failure would be **silent**. `R-RUN-REPRO`,
`R-VIS-NEUTRAL` and `R-RESULT-BUILTIN` are the ones where a regression produces results that look
fine and are not.
