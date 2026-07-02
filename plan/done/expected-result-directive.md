# Plan: `%# expected-result` — expected-vs-actual via the opp_repl wrapper (no opp_test change)

## Status: DONE (2026-07-02)

## Implementation outcome

Implemented across two repos (branch `topic/expected-result` in each), verified end-to-end.

- **opp_repl** (`af35bd5`): `OppTestTask.get_expected_result()` parses a
  `%# expected-result: PASS|FAIL|ERROR` comment and threads it into every `TestTaskResult`
  (reusable for any suite run via `opp_run_opp_tests`); plus the display fix in
  `common/task.py` so an *unexpected PASS* is annotated `(unexpected)` (was silently green).
  Exit already keys off `is_all_results_expected()`. Unit-verified.
- **inet-protocoltest** (`b873296bec`, `895b737aa3`, `39706cc081`, `9cb9d46b9b`): the 35
  NOT-MODELED tests now assert the honest `PROTOCOLTEST <name>: PASS` line and declare
  `%# expected-result: FAIL`; README + AUTHORING document the model.

**Invocation decision (resolved with the user):** running our nested, two-library,
single-binary suite through `opp_run_opp_tests` would require generalizing `OppTestTask` to
link multiple libraries (a shared opp_repl build-path change, regression-risk to INET's own
suites) **and** a `.opp` descriptor, **and** would force 74 slow per-test builds. Chose the
plan's documented **thin-delegator** fallback instead: `run-tests.sh` keeps its fast
single-binary build and pipes `opp_test`'s per-test output to `report_expected_results.py`,
which reuses opp_repl's `TestTaskResult` model to pair each result with its declaration.

**Verified:** full suite = 39 `PASS` + 35 `FAIL (expected)` → "All results as expected", exit 0.
A synthetic closed-gap → `PASS (unexpected)` and a regression → `FAIL (unexpected)`, exit 1.
`opp_test gen` still accepts the `%#` comment (no "invalid entry").

**Follow-up for the user:** the opp_repl changes live on `topic/expected-result`; merge/install
opp_repl so `run-tests.sh` finds the fixed model without `OPP_REPL_PATH=<checkout>`.

The decision points below were resolved: (1) fix the shared display — done; (2) `%# expected-result`
syntax — done; (3) invocation — thin delegator (above).

## Problem

Our WiFi conformance suite marks the 35 NOT-MODELED tests with
`%contains: PROTOCOLTEST <name>: FAIL`. That **lies**: the program genuinely prints `FAIL`
(the spec assertion did not hold), but asserting the `FAIL` line makes opp_test report the
test as **PASS**. "The assertion string matched" is conflated with "the protocol conformed."
Worse, when INET later *implements* the feature the program prints `PASS`, the
`%contains: FAIL` rule stops matching and the test flips to **FAIL** — a gap *closing* looks
like a *regression*. Backwards.

Goal (user's ask): **expected-vs-actual is a separate dimension from pass/fail**, done the way
`opp_repl`/`opp_ci` already do it — implemented in the **opp_repl wrapper**, not by modifying
the shared opp_test tool.

## Key facts (researched)

### 1. opp_test hard-errors on unknown directives — but ignores `%#` comments
`omnetpp/python/omnetpp/test.py::parse_testfile` validates every `%key` against `entries` and
fails on unknown ones (**351-352**). So a bare `%expected-result:` would make opp_test error.
But it skips `%#` comment lines first (**312-314**) and strips `%#…` in bodies (385). So the
expectation must be encoded as a **comment** — exactly opp_repl's own trick
(`#expected-result = "ERROR"` commented INI key, `project.py:941`).

### 2. opp_repl ALREADY wraps opp_test
`opp_repl/opp_repl/test/opp.py` — `OppTestTask.run_protected()` runs `opp_test gen`, makemake,
make, `opp_test run`, then reads `Aggregate result: (\w+)` → `result ∈ {PASS,FAIL,ERROR}` and
builds a `TestTaskResult(self, result=result, …)` (**opp.py:115-119**; ERROR/ CANCEL/ FAIL
early-returns at 84-96, 120-123). Exposed as the `opp_run_opp_tests` entry point
(`pyproject.toml:102`, `bin/opp_run_opp_tests`). Each `OppTestTask` runs **one** `.test`, so
its `Aggregate result` is that test's raw actual outcome. It reads the `.test` file path
(`self.test_file_name` under `self.working_directory`) — so it can parse our comment.

### 3. opp_repl's result model ALREADY carries expected-vs-actual
`TestTaskResult.__init__(task, result, expected_result="PASS", …)` (**test/task.py:23**) →
`TaskResult` sets `self.expected_result` and `self.expected = expected_result == result`
(**common/task.py:232-233**), vocabulary `PASS/SKIP/CANCEL/FAIL/ERROR`.
`MultipleTaskResults.get_unexpected_results()` / `filter_results(exclude_expected_test_result=…
→ expected_result != result)` (**485-492**) already isolate divergences, **including an
unexpected PASS** (result=PASS, expected_result=FAIL are not equal). The model is complete;
`OppTestTask` simply never supplies a declared `expected_result`, so every test defaults to
expect-PASS.

**Two reporting weak spots to decide on:**
- **Display suffix** (**common/task.py:256-258**): the one-line render adds `(unexpected)` only
  when `not self.expected and self.color != COLOR_GREEN`. An *unexpected PASS* is green → shows
  neither suffix → silently green (opp_repl's known asymmetry; opp_ci fixes it by treating any
  divergence as `UNEXPECTED`). The *data* is still captured by `get_unexpected_results()`.
- **Aggregate label** (**common/task.py:347-356**): the crude two-pass sets the run's `result`
  to the worst *expected* code, so an all-expected suite containing expected-FAILs yields an
  aggregate label of `FAIL`. Success/exit must therefore be driven by
  `get_unexpected_results()` (empty ⇒ success), not the aggregate label.

## Design

opp_test stays untouched. Encode the expectation as a `%#` comment; teach the existing
`OppTestTask` to parse it and feed `expected_result` into the result it already builds. The
expected/actual model then does the rest.

### Encoding (in the `.test` file)
```
%# expected-result: FAIL        # opp_test ignores it; OppTestTask reads it. default = PASS
```
Values `PASS | FAIL | ERROR` (opp_ci's `TestResultCode` minus SKIPPED — skip stays orthogonal).
Absent ⇒ `PASS`. Human reason stays in `%description`.

### `%contains` stays honest
`%contains:` always asserts `PROTOCOLTEST <name>: PASS`, so opp_test's raw actual = PASS iff
the protocol conformed. No line lies.

### Verdict matrix (already computed by the model, once fed the declared value)
| declared \ actual | PASS | FAIL | ERROR |
|---|---|---|---|
| **PASS** (default) | PASS ✅ | FAIL ❌ | ERROR ❌ |
| **FAIL** | **UNEXPECTEDPASS** ⚠️ | EXPECTEDFAIL ✅ | ERROR ❌ |
| **ERROR** | **UNEXPECTEDPASS** ⚠️ | FAIL ❌ | EXPECTEDERROR ✅ |

`expected = (declared == actual)`: green when equal (incl. expected-FAIL), red/surfaced when
not (incl. the unexpected PASS "gap-closed" alarm).

## Implementation

### Phase 1 — teach `OppTestTask` the declared expectation (`opp_repl/test/opp.py`)
1. Add a helper to read `^%#\s*expected-result\s*:\s*(PASS|FAIL|ERROR)\s*$` from the `.test`
   file (case-insensitive value; validate; default `PASS`; error on unknown value).
2. In `OppTestTask.__init__`, store `self.expected_result` from that helper (path =
   `os.path.join(working_directory, test_file_name)`).
3. Thread `expected_result=self.expected_result` into **every** `self.task_result_class(self,
   result=…, …)` construction in `run_protected()` (the 3 build-ERROR returns 85/92/96, the
   final `Aggregate result` return 119, and the CANCEL/FAIL returns 121/123). Now
   `result.expected` reflects the declared expectation.
   - Cleaner option to evaluate: give `TestTask`/`OppTestTask` a `get_expected_result()` and
     have the result construction consult it, mirroring the `SimulationTask.get_expected_result`
     pattern — avoids threading the kwarg through six call sites.

### Phase 1b — make the verdict/report honest end-to-end
4. Drive suite success + process exit from `get_unexpected_results()` (empty ⇒ success),
   not the crude aggregate label. Verify the `opp_run_opp_tests` CLI path already does this;
   if not, adjust that reporting entry point.
5. **UNEXPECTEDPASS visibility** (decision below): either (a) fix the shared display at
   `common/task.py:257` so an *unexpected* result is annotated `(unexpected)` even when green
   (closest to opp_ci; affects all task types → re-verify their output), or (b) surface it only
   in the opp_test-run summary by listing `get_unexpected_results()` loudly. Prefer (a) for a
   true fix; (b) if we must not touch shared display.

### Phase 2 — migrate the suite (`tests/protocol/wifi/`)
- **35 NOT-MODELED** `.test`: `%contains` body `… : FAIL` → `… : PASS`; add
  `%# expected-result: FAIL` near `%description`. (Use `ERROR` for any that truly ERROR —
  none seen; confirm in Phase 4.)
- **39 CONFORMS**: unchanged (no comment ⇒ declared PASS).

### Phase 3 — run path + docs + self-test
- Decide how the suite is invoked with expectations (decision 3): run it through
  `opp_run_opp_tests` (needs the suite usable as an opp_repl simulation project — check for an
  existing `.opp`/project descriptor), and/or update `run-tests.sh` to call the wrapper.
- `README.md`: replace the "%contains expects FAIL" paragraph with the `%# expected-result`
  semantics + matrix; note UNEXPECTEDPASS surfacing.
- `AUTHORING.md`: document `%# expected-result`.
- Self-test: fixture pair — a `%# expected-result: FAIL` test that fails (→ EXPECTEDFAIL, green)
  and one that passes (→ UNEXPECTEDPASS, red/exit-nonzero).

### Phase 4 — verify
- Full wifi suite through the wrapper: **39 expected-PASS + 35 EXPECTEDFAIL**, zero unexpected,
  process exit 0, every verdict honest (no fake PASS).
- Negative check: temporarily weaken one NOT-MODELED assertion so its program prints PASS →
  wrapper reports **UNEXPECTEDPASS** and exits nonzero (the stale-matrix alarm).
- Confirm `opp_test gen` still accepts the migrated files (the `%#` comment causes no
  "invalid entry" error).
- Regression: run another existing opp_test suite via `opp_run_opp_tests` unchanged — no
  behavior change for tests without the comment (default expect-PASS).

## Backward compatibility & risk
- **Zero** change to opp_test / omnetpp. opp_repl change is additive: absent comment ⇒ declared
  PASS ⇒ identical to today for every other opp_test suite.
- If Phase 1b(a) touches `common/task.py` display, it affects all opp_repl task renders →
  re-verify their summaries. Phase 1b(b) avoids that.
- `run-tests.sh` output/entry may change (now via the wrapper) → update any CI grep.

## Decision points to confirm
1. **UNEXPECTEDPASS visibility:** fix the shared display asymmetry (`common/task.py:257`) so a
   closed gap is loudly flagged, or surface it only in the opp_test-run summary? *Rec: fix the
   display — it's the whole point, and matches opp_ci.*
2. **Comment syntax:** `%# expected-result: FAIL` (mirrors opp_repl's `expected-result` key).
3. **Invocation:** should the wifi suite now run through `opp_run_opp_tests` (as an opp_repl
   project) so expectations apply, with `run-tests.sh` delegating to it? Or keep `run-tests.sh`
   as the entry and port the tiny parse+verdict there too? *Rec: run via `opp_run_opp_tests`;
   make `run-tests.sh` a thin delegator so both paths agree.*

## Rejected alternatives
- **Add a real `%expected-result` directive to opp_test.** Cleanest semantically but the user
  ruled out modifying the shared tool, and an unknown directive hard-errors (test.py:351).
- **Native `%expected-failure`.** Zero-code, removes the lie today, but presence-based,
  fail-only, no UNEXPECTEDPASS — the closed-gap signal we want. (Fallback if the wrapper work
  is deferred.)
- **Keep `%contains: FAIL`.** Status quo; lies and inverts the gap-closing signal.
