# Derive tests from a protocol standard

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test-anatomy.md](../design/test-anatomy.md), [testing.md](../rule/testing.md)

How to turn a protocol standard — an IETF RFC, an IEEE standard, or another protocol
specification — and its family of related documents into protocol tests. The workflow is
repeatable: each new pass adds detail and coverage on top of the artifacts of the earlier
passes. The worked example is IPv4 (RFC 791) in
[`../evidence/standard/ipv4/`](../evidence/standard/ipv4/).

The tests land in `tests/protocol/`; the test framework itself is documented in
[`AUTHORING.md`](../../../tests/protocol/lib/AUTHORING.md). This guide describes the
process around it, and the document artifacts land in `doc/project/evidence/standard/`.

## Principle: specification first

The standards map (step 2), the catalog (step 3), and the English check descriptions
(step 5) come from the standard texts only. They contain no INET module names, no
parameters, no signals, and no code references.

The feature map (step 4) also comes from the standard texts only, except for its support
column, which the run of step 7 fills in later.

- A look at the code is permitted only for one purpose: to select practical candidates from
  the catalog.
- INET names first appear in the `.test` file (step 6).
- Code analysis (file and line references) first appears in the run-analysis document
  (step 7).
- The conformance document (step 8) reads the model documentation by design: its claims
  part records which standards the model intends to implement. The claims must not leak
  back into the earlier artifacts.

Reason: the tests must check the protocol against the specification, not against the
implementation. A test derived from the code can only confirm the code. The English check
description is also the review gate: a person can compare it against the standard without
INET knowledge.

## Steps at a glance

| Step | Action | Artifact |
| --- | --- | --- |
| 1 | Download the standard and its relatives | `evidence/standard/<proto>/<doc>.txt` |
| 2 | Map the standards family, select the in-scope set | `evidence/standard/<proto>/standards.md` |
| 3 | Extract checkable statements, per document | `evidence/standard/<proto>/<doc>-checklist.md` |
| 4 | Map the features across the combined standards | `evidence/standard/<proto>/features.md` |
| 5 | Write the English check procedure with a mockup | `evidence/standard/<proto>/<doc>-checks.md` |
| 6 | Write the protocol test | `tests/protocol/<proto>/<Doc><Name>.test` |
| 7 | Run the test, analyze the model, update feature support | `evidence/standard/<proto>/<doc>-results.md` |
| 8 | Extract the model claims, cross-check at feature level | `evidence/standard/<proto>/conformance.md` |
| 9 | Decide the category of each check | `evidence/standard/<proto>/<doc>-categories.md` |

Naming:

- `<proto>` is the protocol name, for example `ipv4`. It names both the evidence folder
  here and the test suite folder under `tests/protocol/`.
- `<doc>` is the document slug: the document identity in lowercase, without the version —
  `rfc791`, `rfc1122`, `ieee8021q`. `<Doc>` is the same slug in camel case (`Rfc791`,
  `Ieee8021q`), and `<DOC>` in capitals is the ID prefix (`RFC791`, `IEEE8021Q`).
- The document identity stays stable across versions; the standards map (step 2) pins the
  exact version or edition behind each slug. Only the cached text file carries the version
  when the body revises in place, for example `ieee8021q-2022.txt`.

## Step 1 — download the standards

Cache the standard text in the evidence folder:

```sh
curl -s https://www.rfc-editor.org/rfc/rfc791.txt -o doc/project/evidence/standard/ipv4/rfc791.txt
```

- Record the source URL and the download date in `standards.md` (step 2).
- Also cache the companion documents that the primary document delegates to. Error signals
  often live in a companion (IPv4 delegates error reports to ICMP, RFC 792).
- Also cache the relatives of the primary document: the documents that update, obsolete,
  or amend it. For an RFC, the RFC-editor metadata (the `Updated by` and `Obsoleted by`
  lines) lists them. For an IEEE standard, the amendments and the later editions are the
  relatives.
- Not every standards body gives a free text; IEEE sells most standards and gives some
  through the IEEE GET program. Cache what the license permits. When you cannot cache a
  text, record the source and the exact version in `standards.md`, and quote by clause
  number instead of line number in all later artifacts.
- A cached file gives stable line numbers. All quotes in later artifacts point into it,
  for example `rfc791.txt:1013-1014`.

## Step 2 — map the standards family (`standards.md`)

A protocol rarely lives in one document. RFCs get updates and replacements (RFC 1122
updates RFC 791). IEEE standards get editions and amendments that replace clauses. The
standards map is the one place that records this family and pins what the tests target.
One file per protocol; it contains:

- **Document list** — every related document: number, title, version or edition, date, the
  source URL, the download date, and the relationship to the base document: `base`,
  `updates`, `obsoletes`, `amends`, or `companion`. Take the relationships from the
  RFC-editor metadata or from the register of the standards body, and record the retrieval
  date.
- **Override table** — one row per clause-level conflict: the area, the base clause, the
  clause of the later document that changes it, and the document that governs. Give line
  references into both cached texts, or clause references when a text is not cached.
- **In-scope set** — the exact document versions that the current pass tests against. All
  later artifacts are pinned to this set. A related document outside the set is out of
  scope, not unknown; list it with a reason.

The low-level artifacts stay per document: each in-scope document gets its own catalog file
(step 3), and the catalog IDs carry the document identity (`RFC791-...`, `RFC1122-...`,
`IEEE8021Q-...`). The overview across the combination of the documents is the feature map
(step 4).

## Step 3 — extract checkable statements (`<doc>-checklist.md`)

Write one catalog file per in-scope document, and one catalog entry per checkable
statement of that document. The document follows the
conventions of this tree: the header line ([DR-HEADER](../rule/documentation.md#dr-header)),
an index of the identifiers ([DR-INDEX](../rule/documentation.md#dr-index)), and a bare
identifier as each heading ([DR-ID-HEADING](../rule/documentation.md#dr-id-heading)) with
the statement as the bold lead sentence. An entry contains:

- **ID** — `<DOC>-AREA-n`, for example `RFC791-TTL-1` or `IEEE8021Q-VLAN-1`. The ID is
  stable forever. Never renumber; append new entries at the end of an area.
- **Quote** — the verbatim sentence of the standard with a line reference into the cached
  file, or a clause reference when no text is cached.
- **Strength** — the word the document uses: `must`, `shall`, `should`, `may`, or
  `description` for normative prose without a keyword. Old RFCs predate RFC 2119, and each
  standards body has its own keyword conventions; record the words as written.
- **Class** — how a test can observe the statement:
  - `wire` — fields of datagrams on a link between two nodes;
  - `end-to-end` — what the destination accepts and delivers upward;
  - `error-signal` — an ICMP or other report message;
  - `internal` — state inside a module, not visible from outside;
  - `encoding` — the exact bit layout; a serializer concern.
- **Check idea** — one or two sentences, still without implementation names.
- **Status** — `selected`, `covered`, `candidate`, or `later` (needs injection, faults, or
  several flows).
- **Overridden by** — present only when a later in-scope document changes this statement:
  the catalog ID of the statement that replaces it, from the override table of the
  standards map. Keep the entry and its ID; do not remove overridden entries. A test
  targets the governing statement.

Close the catalog with a **coverage ledger**: a table from ID to check section and test
file. The ledger is the frontier of the work; each pass extends it.

Do not aim for completeness in the first pass. Extract the statements of a few areas well,
mark the rest out of scope, and note that at the end of the catalog.

## Step 4 — map the features (`features.md`)

The catalogs are flat, fine-grained, and per document. The feature map is the high-level
view above them: one file per protocol, across the whole in-scope set of the standards
map. It lists the capabilities that the combined standards define, with the requirement
level of each, and the cross reference into the catalogs. The map answers two questions:

- **Down:** which checks tell whether a feature works? The map selects them from the
  catalogs.
- **Up:** after the tests run, which features does the simulation model support? Step 7
  fills in the answer, and step 8 compares the answer with the claims of the model.

Write the feature list from the standard texts only, before you look at the code. The
document follows the same conventions as the catalogs: header line, index, one section per
feature with a bare identifier as the heading. An entry contains:

- **ID** — `<PROTO>-F-NAME`, for example `IPV4-F-FRAGMENTATION`. The ID names the protocol,
  not one document: a feature can span documents. The ID is stable forever. Never
  renumber; append new features at the end.
- **Sources** — the documents and clauses that define the feature, in precedence order
  from the standards map. When a later document overrides the base text, cite both and
  name the one that governs.
- **Level** — the requirement level in the governing text: `mandatory`, `optional`, or
  `unstated` when the text gives no keyword. Quote the words with a line reference. When a
  later document changes the level (RFC 1122 turns prose into a MUST), record the change
  with both citations.
- **Description** — one or two sentences on what the feature does, in the terms of the
  standard only.
- **Checks** — the catalog IDs that establish the feature, from the coverage ledgers. The
  IDs carry the document identity, so one list can mix documents. Mark each ID as `core`
  (the feature does not work without this behavior) or `supporting` (detail or edge case).
  Do not list an overridden entry as `core`; point to the entry that governs. A feature
  with an empty list is a coverage gap; keep the entry and note the gap.
- **Support** — empty at first. Step 7 fills it in; see the derivation rule there.

Open the document with a summary table: feature, level, sources, checks, support. This
table is the cross-reference matrix of the whole workflow. Also close the loop in the
other direction: every area of every in-scope catalog must appear in at least one feature,
or the map must mark the area as out of scope.

## Step 5 — write the English check description (`<doc>-checks.md`)

One file per document, with one section per check. A check can cover several catalog
entries when one scenario shows them together. A common-mockup section at the top serves
all checks; each check section contains:

- **Checks** — the catalog IDs with their strength.
- **Requirement** — a short restatement of the clauses of the standard.
- **Mockup** — the abstract network around the checked behavior: named nodes (host A,
  gateway R, host B), links, and the scenario constants (MTU values, sizes, TTL, time
  limits). Add a small diagram.
- **Size or value arithmetic** — when the expected field values follow from the procedure
  of the standard, derive them here (for example, fragment sizes and offsets from the MTU).
- **Procedure** — numbered imperative steps: build, configure, send, observe.
- **Expected observations** — a numbered list. Each item names the link or node, the
  fields, and the expected values. **The test program of step 6 maps its steps one to one
  onto this list.**
- **Notes** — strength nuances (a `may` clause is not a violation when absent), tolerance
  decisions, and out-of-scope variants.

Make one expected observation confirm the stimulus itself (for example: the DF flag is
visible on link 1). A configuration error that voids the stimulus then fails the test
instead of a silent pass for the wrong reason.

Keep the document free of INET names. Use protocol names (UDP, ICMP, VLAN) and the field
names of the standard.

## Step 6 — write the protocol test (`<Doc><Name>.test`)

Translate the English document into a self-contained `opp_test` file in
`tests/protocol/<proto>/`, with the framework in
[`tests/protocol/lib/`](../../../tests/protocol/lib/AUTHORING.md):

- File name: `<Doc><CheckName>.test`, for example `Rfc791TtlDecrement.test` or
  `Ieee8021qVlanTag.test`. Program name: `<doc>_<check_name>`, for example
  `rfc791_ttl_decrement`.
- The `%description` names the catalog IDs, points to the check section, and gives the
  mockup mapping (host A = `hostA`, gateway R = `router`, ...).
- One program step per numbered expected observation, in order, with a comment that names
  the observation. Use `capture` for recorded values and relative comparisons.
- When a mapping is imperfect (for example, a window that cannot cover an instant), write
  the deviation into the `%description` and into `<doc>-results.md` (step 7). Do not
  silently weaken the English expectation.
- When the faithful assertion fails because the model lacks a feature, keep the assertion
  and declare `%# expected-result: FAIL` (see AUTHORING.md). Never invert the assertion.
- Check the printed program (`printDescription = true`) against the English document. The
  two texts must tell the same story.
- Expression pitfalls (a wrong expression is a silent non-match, and the step times out):
  a unit-bearing field needs a unit literal (`udp.totalLengthField == 1008B`); the
  protocol prefix is the INET dissector name (`icmpv4`, not `icmp`); an ini key with a
  wrong path applies nothing. On a deadline miss, run the tester without `testName` first
  and read the real frames in the trace.

## Step 7 — run and analyze (`<doc>-results.md`)

Build the framework library once per INET build, then run the suite:

```sh
cd tests/protocol/lib && ./build.sh
inet_run_protocol_tests -p inet -w ipv4
```

(`-p inet` skips the project discovery; discovery crashes on a `~/.omnetpp` directory.)

Record in `<doc>-results.md`:

- the date, the INET commit, and the verdict of every test;
- for each failure, its class: **test error** (fix the test), **model gap** (keep the
  faithful test, mark `%# expected-result: FAIL`, file the gap), or **specification
  misread** (fix the catalog and the check document);
- the simulation model analysis: where the model implements the checked behavior, with
  file and line references. This is the first artifact that may reference code;
- sharpening candidates for the next pass.

Then update the support column of the feature map (`features.md`) from the verdicts. The
rule, per feature:

- `supported` — every `core` check of the feature ran and passed;
- `partial` — at least one `core` check passed, and at least one `core` check failed as a
  model gap or is not yet tested;
- `not supported` — every `core` check that ran failed as a model gap;
- `untested` — no `core` check of the feature ran.

Record the date and the INET commit next to the support value. The support statement is
bounded by the checks: a `supported` feature is supported as far as the checks reach, not
proven complete. A failure of a `supporting` check does not lower the support value by
itself; note it in the feature entry instead.

## Step 8 — extract the model claims and cross-check (`conformance.md`)

The tests tell what the model does. This step adds what the model intends: which standards
the model claims to implement, and how the claim compares with the measured feature
support. One file per protocol, with two parts.

**Part 1 — claims.** Collect the intended standards from the model itself: NED
documentation comments, the module documentation, source comments, and the release notes.
Record each claim with a source reference (file and line). Record the claimed document and
version precisely: a model that claims RFC 791 does not claim the RFC 1122 updates, and a
model that claims IEEE 802.1Q-2018 does not claim a later amendment. Map each claim onto
the standards map (step 2). A claimed document outside the in-scope set is a scope gap for
a later pass, not a verdict.

**Part 2 — conformance matrix.** Cross-check the claims against the feature map, feature
by feature — not test by test. The tests already aggregate into feature support in step 7;
the matrix works only on that level. A feature is `claimed` when the claims of part 1
cover its governing source document. Combine the claim with the support value of
`features.md` into a verdict:

| Claimed | Support | Verdict |
| --- | --- | --- |
| yes | supported | `confirmed` |
| yes | partial | `partial` — list the gap checks |
| yes | not supported | `defect` — the model does not do what it claims |
| yes | untested | `unverified` — a coverage gap, not a model verdict |
| no | supported | `undocumented` — the model does more than it claims |
| no | partial, not supported, or untested | `out of claim` |

Record the date, the INET commit, and the feature-map state that the matrix comes from. A
`defect`, and an `unverified` feature with level `mandatory`, are the headlines for the
next pass. An `undocumented` feature is a documentation task for the model, not a test
task.

## Step 9 — decide the category (`<doc>-categories.md`)

Judge every check after it runs. The categories and what each can establish are
[test-anatomy.md](../design/test-anatomy.md#the-categories); the rule that the category
must match the claim is [TR-CAT-MATCH](../rule/testing.md#tr-cat-match). The usual mapping
from observation class to category:

| Observation class | Usual category |
| --- | --- |
| wire, end-to-end, error-signal | protocol test |
| encoding, algorithm on one message | unit test |
| internal state with a scalar signal | protocol test with a state-signal step |
| internal state without a signal | module test |
| timer distributions, throughput bounds | statistical test |
| whole-trajectory regression lock | fingerprint test |

Record the decision and the reason per check. When a check does not fit the protocol suite,
keep its catalog entry and note the target category; write the test in the other suite in a
later pass.

## Iteration model

The workflow is a loop, not a one-shot process. A later pass:

1. extends the standards map — a new update, amendment, or edition enters the in-scope
   set, brings its own catalog file, and adds rows to the override table;
2. extends the catalogs — new sections of the in-scope documents;
3. promotes `candidate` and `later` entries to `selected` as the toolset allows (injection,
   interception, and state signals unlock the `later` class);
4. deepens selected checks — sharper field assertions, timing bounds, fault variants;
5. re-runs everything and updates the ledgers, `<doc>-results.md`, the support column of
   `features.md`, and the conformance matrix.

The catalog IDs, the feature IDs, and the ledgers carry the state between passes. A pass
is complete when the standards map, the ledgers, the feature map, the conformance matrix,
the results, and the plan document agree.

## Where everything lives

```
doc/project/evidence/standard/<proto>/  the document artifacts
  rfc791.txt, rfc792.txt                step 1: cached standard texts
  standards.md                          step 2: standards family + in-scope set, one per protocol
  rfc791-checklist.md                   step 3: catalog + coverage ledger, one per document
  features.md                           step 4: feature map + support matrix, one per protocol
  rfc791-checks.md                      step 5: English procedures, one section per check
  rfc791-results.md                     step 7: verdicts + model analysis
  conformance.md                        step 8: model claims + conformance matrix, one per protocol
  rfc791-categories.md                  step 9: category decisions

tests/protocol/<proto>/                 the tests
  Rfc791TtlDecrement.test               step 6: one test per check
  Rfc791FragmentReassembly.test
  Rfc791DontFragment.test
```

## Pass log

| Pass | Date | Scope | Result |
| --- | --- | --- | --- |
| 1 | 2026-09-02 | RFC 791 + RFC 792 error signals; 17 catalog entries; 3 checks; 3 tests | see [`rfc791-results.md`](../evidence/standard/ipv4/rfc791-results.md) |
