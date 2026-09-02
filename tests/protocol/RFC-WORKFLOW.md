# RFC Test Workflow

This document describes how to turn an RFC into protocol tests for INET. The workflow is
repeatable: each new pass adds detail and coverage on top of the artifacts of the earlier
passes. The worked example is IPv4 (RFC 791) in [`ipv4/rfc/`](ipv4/rfc/).

The test framework itself is documented in [`lib/AUTHORING.md`](lib/AUTHORING.md). This
document describes the process around it.

## Principle: specification first

The catalog (step 2) and the English check descriptions (step 3) come from the RFC text
only. They contain no INET module names, no parameters, no signals, and no code references.

- A look at the code is permitted only for one purpose: to select practical candidates from
  the catalog.
- INET names first appear in the `.test` file (step 4).
- Code analysis (file and line references) first appears in the run-analysis document
  (step 5).

Reason: the tests must check the protocol against the specification, not against the
implementation. A test derived from the code can only confirm the code. The English check
description is also the review gate: a person can compare it against the RFC without INET
knowledge.

## Steps at a glance

| Step | Action | Artifact |
| --- | --- | --- |
| 1 | Download the RFC and its companions | `<proto>/rfc/rfcNNN.txt` |
| 2 | Extract checkable statements | `<proto>/rfc/checklist.md` |
| 3 | Write the English check procedure with a mockup | `<proto>/rfc/checks/<name>.md` |
| 4 | Write the protocol test | `<proto>/RfcNNN<Name>.test` |
| 5 | Run the test and analyze the simulation model | `<proto>/rfc/results.md` |
| 6 | Decide the category of each check | `<proto>/rfc/categories.md` |

`<proto>` is a suite folder under `tests/protocol/`, for example `ipv4/`.

## Step 1 — download the RFC

Cache the RFC text in the suite folder, under `rfc/`:

```sh
curl -s https://www.rfc-editor.org/rfc/rfc791.txt -o tests/protocol/ipv4/rfc/rfc791.txt
```

- Record the source URL and the download date in `checklist.md`.
- Also cache the companion RFCs that the primary RFC delegates to. Error signals often live
  in a companion (IPv4 delegates error reports to ICMP, RFC 792).
- The cached file gives stable line numbers. All quotes in later artifacts point into it,
  for example `rfc791.txt:1013-1014`.

## Step 2 — extract checkable statements (`checklist.md`)

Read the RFC and write one catalog entry per checkable statement. An entry contains:

- **ID** — `RNNN-AREA-n`, for example `R791-TTL-1`. The ID is stable forever. Never
  renumber; append new entries at the end of an area.
- **Quote** — the verbatim RFC sentence with a line reference into the cached file.
- **Strength** — the word the RFC uses: `must`, `may`, or `description` for normative prose
  without a keyword. Old RFCs predate RFC 2119; record the words as written.
- **Class** — how a test can observe the statement:
  - `wire` — fields of datagrams on a link between two nodes;
  - `end-to-end` — what the destination accepts and delivers upward;
  - `error-signal` — an ICMP or other report message;
  - `internal` — state inside a module, not visible from outside;
  - `encoding` — the exact bit layout; a serializer concern.
- **Check idea** — one or two sentences, still without implementation names.
- **Status** — `selected`, `covered`, `candidate`, or `later` (needs injection, faults, or
  several flows).

Close the catalog with a **coverage ledger**: a table from ID to check document and test
file. The ledger is the frontier of the work; each pass extends it.

Do not aim for completeness in the first pass. Extract the statements of a few areas well,
mark the rest out of scope, and note that at the end of the catalog.

## Step 3 — write the English check description (`checks/<name>.md`)

One document per check. A check can cover several catalog entries when one scenario shows
them together. The document contains:

- **Checks** — the catalog IDs with their strength.
- **Requirement** — a short restatement of the RFC clauses.
- **Mockup** — the abstract network around the checked behavior: named nodes (host A,
  gateway R, host B), links, and the scenario constants (MTU values, sizes, TTL, time
  limits). Add a small diagram.
- **Size or value arithmetic** — when the expected field values follow from the RFC
  procedure, derive them here (for example, fragment sizes and offsets from the MTU).
- **Procedure** — numbered imperative steps: build, configure, send, observe.
- **Expected observations** — a numbered list. Each item names the link or node, the
  fields, and the expected values. **The test program of step 4 maps its steps one to one
  onto this list.**
- **Notes** — strength nuances (a `may` clause is not a violation when absent), tolerance
  decisions, and out-of-scope variants.

Make one expected observation confirm the stimulus itself (for example: the DF flag is
visible on link 1). A configuration error that voids the stimulus then fails the test
instead of a silent pass for the wrong reason.

Keep the document free of INET names. Use protocol names (UDP, ICMP) and RFC field names.

## Step 4 — write the protocol test (`RfcNNN<Name>.test`)

Translate the English document into a self-contained `opp_test` file with the framework in
[`lib/`](lib/) (see `AUTHORING.md` for the API):

- File name: `RfcNNN<CheckName>.test`, for example `Rfc791TtlDecrement.test`. Program name:
  `rfcNNN_<check_name>`.
- The `%description` names the catalog IDs, points to the check document, and gives the
  mockup mapping (host A = `hostA`, gateway R = `router`, ...).
- One program step per numbered expected observation, in order, with a comment that names
  the observation. Use `capture` for recorded values and relative comparisons.
- When a mapping is imperfect (for example, a window that cannot cover an instant), write
  the deviation into the `%description` and into `results.md`. Do not silently weaken the
  English expectation.
- When the faithful assertion fails because the model lacks a feature, keep the assertion
  and declare `%# expected-result: FAIL` (see AUTHORING.md). Never invert the assertion.
- Check the printed program (`printDescription = true`) against the English document. The
  two texts must tell the same story.
- Expression pitfalls (a wrong expression is a silent non-match, and the step times out):
  a unit-bearing field needs a unit literal (`udp.totalLengthField == 1008B`); the
  protocol prefix is the INET dissector name (`icmpv4`, not `icmp`); an ini key with a
  wrong path applies nothing. On a deadline miss, run the tester without `testName` first
  and read the real frames in the trace.

## Step 5 — run and analyze (`results.md`)

Build the framework library once per INET build, then run the suite:

```sh
cd tests/protocol/lib && ./build.sh
inet_run_protocol_tests -p inet -w ipv4
```

(`-p inet` skips the project discovery; discovery crashes on a `~/.omnetpp` directory.)

Record in `results.md`:

- the date, the INET commit, and the verdict of every test;
- for each failure, its class: **test error** (fix the test), **model gap** (keep the
  faithful test, mark `%# expected-result: FAIL`, file the gap), or **specification
  misread** (fix the catalog and the check document);
- the simulation model analysis: where the model implements the checked behavior, with
  file and line references. This is the first artifact that may reference code;
- sharpening candidates for the next pass.

## Step 6 — decide the category (`categories.md`)

The INET test tree has several categories. Judge every check after it runs:

| Observation class | Usual category |
| --- | --- |
| wire, end-to-end, error-signal | protocol test (this suite) |
| encoding, algorithm on one message | unit test (`tests/unit`) |
| internal state with a scalar signal | protocol test with a state-signal step |
| internal state without a signal | module test (`tests/module`) |
| timer distributions, throughput bounds | statistical test (`tests/statistical`) |
| whole-trajectory regression lock | fingerprint test (`tests/fingerprint`) |

Record the decision and the reason per check. When a check does not fit the protocol suite,
keep its catalog entry and note the target category; write the test in the other suite in a
later pass.

## Iteration model

The workflow is a loop, not a one-shot process. A later pass:

1. extends the catalog — new RFC sections, companion RFCs, updates such as RFC 1122;
2. promotes `candidate` and `later` entries to `selected` as the toolset allows (injection,
   interception, and state signals unlock the `later` class);
3. deepens selected checks — sharper field assertions, timing bounds, fault variants;
4. re-runs everything and updates the ledger and `results.md`.

The catalog IDs and the ledger carry the state between passes. A pass is complete when the
ledger, the results, and the plan document agree.

## Directory layout

```
tests/protocol/
  RFC-WORKFLOW.md              this document
  lib/                         the test framework (AUTHORING.md)
  ipv4/
    Rfc791TtlDecrement.test          step 4: one test per check
    Rfc791FragmentReassembly.test
    Rfc791DontFragment.test
    rfc/
      rfc791.txt               step 1: cached RFC texts
      rfc792.txt
      checklist.md             step 2: catalog + coverage ledger
      checks/
        ttl-decrement.md       step 3: English procedures with mockups
        fragment-reassembly.md
        dont-fragment.md
      results.md               step 5: verdicts + model analysis
      categories.md            step 6: category decisions
```

## Pass log

| Pass | Date | Scope | Result |
| --- | --- | --- | --- |
| 1 | 2026-09-02 | RFC 791 + RFC 792 error signals; 17 catalog entries; 3 checks; 3 tests | see `ipv4/rfc/results.md` |
