# Derive tests from a protocol standard

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test-anatomy.md](../design/test-anatomy.md), [testing.md](../rule/testing.md)

How to turn a protocol standard — an IETF RFC, an IEEE standard, or another protocol
specification — and its family of related documents into protocol tests. The workflow is
repeatable: each new pass adds detail and coverage on top of the artifacts of the earlier
passes. The worked example is IPv4 (RFC 791) in
[`../evidence/protocol/ipv4/`](../evidence/protocol/ipv4), with the catalogs of its
two documents in [`../evidence/standard/`](../evidence/standard).

The tests land in `tests/protocol/`; the test framework itself is documented in
[`AUTHORING.md`](../../../tests/protocol/lib/AUTHORING.md). This guide describes the
process around it. The artifacts land in three trees under `doc/project/evidence/`:
`standard/<doc>/` holds what belongs to one standard document, `protocol/<proto>/` what
belongs to the protocol as a whole, and `model/<proto>/` everything that depends on the
simulation model.

## Principle: specification first

The standards map (step 2), the catalog (step 3), and the English check descriptions
(step 5) come from the standard texts only. They contain no INET module names, no
parameters, no signals, and no code references.

The feature map (step 4) also comes from the standard texts only. It carries no run data
at all: what a run showed lives in the coverage ledger.

- A look at the code is permitted for two purposes, and no other: to select practical
  candidates from the catalog, and to answer the one question of
  [the third principle](#principle-a-claimed-feature-gets-a-test) — does the model claim this
  behavior? Neither answer may be written into an artifact of steps 2 to 5.
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

## Principle: the information flows one way

No step edits an artifact of an earlier step. A new test must not force an edit to the
catalog, and a new run must not force an edit to the feature map. Otherwise the two
spec-derived documents never settle, and the workflow becomes circular editing instead of a
flow.

The changing state therefore lives in one artifact of its own, the **coverage ledger**
`model/<proto>/coverage.md`:

- which statement a check targets (`selected`, `covered`, `candidate`, `owed`, `later`, `no check`);
- which check section and which test file carry it;
- the verdict of the last run;
- the support value of every feature;
- the level each feature reaches, and the level of the protocol as a whole.

`owed` is the status for a statement the model claims and no check reaches yet. It is the
coverage debt of the pass, it says what a check would need, and it is the one status that a
later pass is obliged to clear.

A row of `no check` carries the reason, and only two reasons are legal:
[the third principle](#principle-a-claimed-feature-gets-a-test) allows the status for a statement
the model does not claim, and step 9 allows it for one whose category is another suite. Anything
else the tooling cannot reach gets a failing test instead, so the status stays rare and a reader
can trust it.

Steps 5, 6 and 7 write their outcome there. The catalog states what the standard says. The
feature map states which capabilities the standards define. Neither one ever mentions a
test or a verdict.

## Principle: a claimed feature gets a test

**If the model claims a behavior, that behavior gets a test. A test is never skipped because
the model would fail it.** The claim counts whether it is explicit — the NED documentation, a
source comment, the release notes — or implicit, which means only this: **there is code for the
behavior.** Code is an implementation effort, and an effort is a claim.

A failing test is the point of the exercise and not an accident of it. A test that can be
written and that fails is the **most valuable** result this workflow produces: it is the one
output that tells somebody what to fix. So it is written, it is kept, and it fails.

Three cases, and they cover everything:

| The state of the model | What to write | The verdict it carries |
| --- | --- | --- |
| it claims the behavior, and the test can be written | the faithful test | it fails, and nothing declares the failure expected |
| it claims the behavior, and the test **cannot** be written because something needed to test against is missing | the test, failing unconditionally, with the missing part named in its description | it fails, and nothing declares the failure expected |
| it does **not** claim the behavior | the faithful test | it fails, and the failure **is** declared expected |

**The middle row is the one that is easy to get wrong.** When a check cannot be built — the
stimulus is unreachable, a node type does not exist, a field the check must set has no
representation — the answer is not to leave the statement without a test. The answer is a test
that fails and says why, in its own description, in one sentence a reader can act on. A missing
part is a finding about the model and about the tooling, and a test is where a finding lives. A
note in a document is not a finding; nothing fails when it goes stale.

The third row is the only licence to declare a failure expected, and "does not claim" is
narrow. It holds when the model **says** it does not support the behavior — a `TODO`, a stated
limitation in the release notes, a document it never names — or when it has made **no effort**:
no code path, no function, no field. It does not hold merely because a mechanism is incomplete.
A half-written mechanism is a claim.

Claiming a protocol is not claiming every feature of it. A model whose documentation says
"implements RFC 2131" has not thereby claimed each of the five messages that document defines;
the code decides, one behavior at a time. Read the question narrowly, about the behavior the
check tests, and the three rows stay decidable.

### What this principle rules out

- **A statement with no test, where code exists for it.** The escape of step 5 — a statement
  that no check carries, with the reason recorded — is bounded by this principle. It is for a
  statement the model does not claim, or one whose category is another suite (step 9). It is
  never for a statement the model claims and would fail.
- **A verdict of `untested` on a statement whose test exists and fails.** The test failed; the
  statement is not established. Say that, and say which observation the failure kept the check
  from reaching. A reader who sees `untested` reasonably concludes that nobody wrote a test.
- **A declared expected failure over a defect.** See
  [the class of a failure](#the-class-of-a-failure-and-when-to-declare-it-expected) in step 7.

## Levels of depth

The workflow applies at several depths. A pass declares which one it aims at, so "do IPv4
and TCP at level 2" is a scope you can give and a state you can check.

Four things separate a shallow pass from a deep one: which documents are in scope, how much
of each document the catalog extracts, what a check may observe, and how rich the scenario
is. The third one jumps in steps rather than degrees, so the **toolset a check may use**
defines the level.

| Level | Name | What it adds | Toolset a check may use | Exit criterion |
| --- | --- | --- | --- | --- |
| 1 | **Survey** | The standards family and the model's claim. No test at all. | none | The standards map pins an in-scope set, every model claim is mapped onto it, and every obsolete claim is named. |
| 2 | **Core** | The normal path of every mandatory mechanism, with its boundary values. | observe a normal exchange on the wire, including the absence of a packet | Every normal-path mandatory mechanism of the base document appears as a feature, and every mandatory feature has a core check that ran and has a verdict. |
| 3 | **Edge** | Everything that needs a fault to show: negative requirements, error reports, corrupted or crafted input. | and injection, interception | The catalogs hold every mandatory statement of the in-scope documents, including each `MUST NOT`, and each one has a check. |
| 4 | **Dynamics** | Timers, distributions, control loops. | and statistical tests, state signals | The catalogs hold every timer and every control loop of the in-scope set, and each one has a check with a stated tolerance. |
| 5 | **Complete** | The optional features, the options, the whole state machine. | and module tests, fingerprints | No area of an in-scope catalog stays out of scope, and the conformance matrix holds no `unverified`. |

Every criterion has two halves: what the catalogs must **hold**, and what must have
**run**. The first half is the one that matters. Without it a small catalog reaches any
level, and four features supported out of four declared reads as finished when a fifth
mechanism was never written down.

**The run half asks that a check exists and ran, not that it passed.** A level is a measure of
how deeply the pass looked, and a failing check looked. By
[the third principle](#principle-a-claimed-feature-gets-a-test) a claimed behavior always has a
check, so a pass that finds many failures can still reach its level; a pass that left claimed
behaviors without a check cannot, whatever its verdicts say.

Each step up is a different kind of work, not more of the same:

- **1 to 2** adds simulation. Level 1 needs no run and no build, which is why it is worth
  having on its own. The TCP survey found that the model claims RFC 793 and never names
  RFC 9293, and that finding cost no simulation.
- **2 to 3** adds control over the network. You must drop, corrupt, delay, or craft a
  packet. An absence step alone stays at level 2: a don't-fragment discard or a TTL expiry
  needs only observation, and the rule of step 5 — one observation confirms the stimulus —
  guards against a vacuous pass. Debugging at level 3 is harder, because a missing packet
  and a wrong filter look the same.
- **3 to 4** adds statistics. A timer or a congestion window is a distribution, not a value,
  so every check needs a tolerance and a defence against a false failure.
- **4 to 5** adds no new tool. It differs by exhaustiveness alone, so it is the one level
  that is quantity rather than kind.

### The level tells you which documents to bring in

| Level | Documents it needs | IPv4 | TCP |
| --- | --- | --- | --- |
| 1 to 2 | the base document, and the companion that carries its error reports | RFC 791, RFC 792 | RFC 9293 |
| 3 | the host-requirements and errata documents, where the edge rules live | RFC 1122, RFC 6864 | RFC 1122 is already folded into RFC 9293 |
| 4 | the timer and control-loop documents | the reassembly timer | RFC 6298, RFC 5681 |
| 5 | the whole family | RFC 2474, RFC 1191 | RFC 7323, RFC 2018 |

### Which steps a level runs

| Level | Steps |
| --- | --- |
| 1 | 1, 2, and part 1 of 8. No catalog, no feature map, no test. |
| 2 and up | all nine, and the ledger |

Level 1 has one rule of its own, and it is the reason the level finds anything: **choose
the in-scope set from the standards register, never from what the model claims.** The
register says which document governs; the model says which one it followed. The gap between
the two is the first finding of every pass, and a set chosen from the claim can never show
it.

### Where the level is declared

The boundary rule decides this:

- The **target level** goes in `protocol/<proto>/standards.md`. It is a scope decision
  about the standard, so it stays free of INET names.
- The **achieved level** goes in `model/<proto>/coverage.md`, because only a run can
  establish it. The ledger records it per feature too, so a protocol that reaches level 2
  for three features and level 1 for a fourth reports that, instead of rounding up.

A level is reached only when its exit criterion holds. Until then the ledger says
`level N, partial` and names what blocks it. Rounding up hides exactly the gap the workflow
exists to find.

## Steps at a glance

| Step | Action | Artifact |
| --- | --- | --- |
| 1 | Download the standard and its relatives | `evidence/standard/<doc>/<doc>.txt` |
| 2 | Map the standards family, select the in-scope set, declare the target level | `evidence/protocol/<proto>/standards.md` |
| 3 | Extract checkable statements, per document | `evidence/standard/<doc>/catalog.md` |
| 4 | Map the features across the combined standards | `evidence/protocol/<proto>/features.md` |
| 5 | Write the English check procedure with a mockup | `evidence/protocol/<proto>/checks.md` |
| 6 | Write the protocol test | `tests/protocol/<proto>/<Doc><Name>.test` |
| 7 | Run the test, analyze the model, update feature support | `evidence/model/<proto>/results.md` |
| 8 | Extract the model claims, cross-check at feature level | `evidence/model/<proto>/conformance.md` |
| 9 | Decide the category of each check | `evidence/model/<proto>/categories.md` |
| — | Record the state of steps 5, 6 and 7 | `evidence/model/<proto>/coverage.md` |

Three trees, and which one an artifact belongs to. The first two hold nothing that depends
on the simulation model; the third holds everything that does.

- `evidence/standard/<doc>/` holds what belongs to **one standard document**: the cached
  text and the catalog of its statements. A document serves every protocol that uses it.
  RFC 792 belongs to IPv4 and to ICMP; RFC 1122 states rules for IP, ICMP, UDP, and TCP.
  One folder per document keeps one copy of each catalog.
- `evidence/protocol/<proto>/` holds what belongs to **the protocol as a whole**, across
  the documents of its in-scope set: the standards map, the feature map, and the checks. A
  check may observe two documents at once, so it cannot live under either one.
- `evidence/model/<proto>/` holds everything that depends on **the simulation model**: the
  coverage ledger, the run results, the conformance matrix, and the category decisions.

**The boundary rule.** No document under `standard/` or `protocol/` may state anything
derived from the model: no INET name, no test file name, no status, no verdict. Those two
trees are the specification-first zone, and a reader can review them against the standard
without any INET knowledge. A constant pointer that says where the model-side data lives is
allowed, because it never changes when a test or a run changes.

Two things follow. The specification-first principle stops being a convention and becomes a
path you can check. And the spec-derived half is reusable: a second implementation adds its
own folder under `model/` and reuses `standard/` and `protocol/` unchanged.

Naming:

- `<proto>` is the protocol name, for example `ipv4`. It names both the evidence folder
  under `evidence/protocol/` and the test suite folder under `tests/protocol/`.
- `<doc>` is the document slug: the document identity in lowercase, without the version —
  `rfc791`, `rfc1122`, `ieee8021q`. It names the folder under `evidence/standard/`.
  `<Doc>` is the same slug in camel case (`Rfc791`, `Ieee8021q`), and `<DOC>` in capitals
  is the ID prefix (`RFC791`, `IEEE8021Q`).
- The folder carries the document identity, so the files inside it do not repeat it:
  `rfc791/catalog.md`, not `rfc791/rfc791-catalog.md`. The cached text keeps its full
  name, because every quote in every later artifact cites it by name and line number.
- The document identity stays stable across versions; the standards map (step 2) pins the
  exact version or edition behind each slug. Only the cached text file carries the version
  when the body revises in place, for example `ieee8021q-2022.txt`.

## Step 1 — download the standards

Cache the standard text in the evidence folder:

```sh
curl -s https://www.rfc-editor.org/rfc/rfc791.txt -o doc/project/evidence/standard/rfc791/rfc791.txt
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
- **In-scope set** — the exact document versions that the current pass tests against,
  taken from the standards register and not from the model's claim. All later artifacts
  are pinned to this set. A related document outside the set is out of scope, not unknown;
  list it with a reason.
- **Target level** — the depth this pass aims at, from the table above. The level decides
  which documents the in-scope set needs, so record the two together.

The low-level artifacts stay per document: each in-scope document gets its own catalog file
(step 3), and the catalog IDs carry the document identity (`RFC791-...`, `RFC1122-...`,
`IEEE8021Q-...`). The overview across the combination of the documents is the feature map
(step 4).

An override belongs to a pair of documents, not to a protocol. When the same row would
appear in the maps of two protocols — RFC 1122 against RFC 791 for IPv4, and against
RFC 793 for TCP — keep the row in one map and link the other map to it, so the two cannot
drift apart.

## Step 3 — extract checkable statements (`standard/<doc>/catalog.md`)

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

  The class also predicts the test category, by the table of step 9. Read that table now:
  an `internal` or `encoding` statement rarely becomes a protocol check, and it is cheaper
  to know that before step 5 than after step 6.
- **Check idea** — one or two sentences, still without implementation names.
- **Overridden by** — present only when a later in-scope document changes this statement:
  the catalog ID of the statement that replaces it, from the override table of the
  standards map. Keep the entry and its ID; do not remove overridden entries. A test
  targets the governing statement.

Do not close the catalog with a coverage table, and do not give an entry a status. Both
belong to the coverage ledger; see the one-way principle above. Close the catalog instead
with the areas of the document that it leaves out of scope, which is a statement about the
standard and therefore belongs here.

Extract what the target level needs, and mark the rest out of scope at the end of the
catalog. At level 2 that is every normal-path mandatory mechanism of the base document. A
first pass may stop short of that, but then it is level 2 partial, and the ledger says so.

## Step 4 — map the features (`features.md`)

The catalogs are flat, fine-grained, and per document. The feature map is the high-level
view above them: one file per protocol, across the whole in-scope set of the standards
map. It lists the capabilities that the combined standards define, with the requirement
level of each, and the cross reference into the catalogs. The map answers two questions:

- **Down:** which checks tell whether a feature works? The map selects them from the
  catalogs.
- **Up:** after the tests run, which features does the simulation model support? Step 7
  writes the answer into the ledger, and step 8 compares it with the claims of the model.

Write the feature list from the standard texts only, before you look at the code. The
document follows the same conventions as the catalogs: header line, index, one section per
feature with a bare identifier as the heading. An entry contains:

- **ID** — `<PROTO>-F-NAME`, for example `IPV4-F-FRAGMENTATION`. The ID names the protocol,
  not one document: a feature can span documents. The ID is stable forever. Never
  renumber; append new features at the end.
- **Sources** — the documents and clauses that define the feature, in precedence order
  from the standards map. When a later document overrides the base text, cite both and
  name the one that governs.
- **Level** — `mandatory`, `optional`, or `unstated`, derived from the core statements by
  one rule, so that two reviewers reach the same value: `mandatory` when any core statement
  says `must` or `shall`, or when the mechanism is the only path the document gives to a
  state or an outcome; `optional` when every core statement says `may` or `should`;
  `unstated` otherwise. Name the reason next to the value (`keyword` or `only path`), and
  quote the words with a line reference. The catalog keeps `should` distinct from `may`, so
  the grouping loses nothing. When a later document changes the level (RFC 1122 turns prose
  into a MUST), record the change with both citations.
- **Description** — one or two sentences on what the feature does, in the terms of the
  standard only.
- **Checks** — the catalog IDs that establish the feature, from the catalogs. The IDs
  carry the document identity, so one list can mix documents, and one statement may be
  `core` to several features. Mark each ID as `core`
  (the feature does not work without this behavior) or `supporting` (detail or edge case).
  Do not list an overridden entry as `core`; point to the entry that governs. A feature
  with an empty list is a coverage gap; keep the entry and note the gap.

Open the document with a summary table: feature, level, sources, checks. The support of
each feature belongs to the coverage ledger, not here. Also close the loop in the
other direction: every area of every in-scope catalog must appear in at least one feature,
or the map must mark the area as out of scope.

## Step 5 — write the English check description (`protocol/<proto>/checks.md`)

One file per protocol, with one section per check. When the file passes about ten checks,
split it by feature into `checks/<feature>.md`; the section names stay, so the links of the
ledger survive the split. A check can cover several catalog entries when one scenario shows
them together. A common-mockup section at the top serves
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

### Which statements get a check

Every statement of the in-scope set that the target level demands gets a check, with two
exceptions and no others:

1. **The model does not claim the behavior.** The third principle defines what a claim is; the
   one code look it permits answers the question. Record the statement with the reason, in a
   closing section of `checks.md` that lists every statement no check carries.
2. **Its category is another suite.** An `encoding` statement belongs to a serializer unit test
   and a distribution to a statistical test; step 9 records the target category. The statement
   still gets a test, in the other suite, in a later pass.

**"A check of two nodes on a link cannot observe it" is not a third exception.** It is a
statement about the tooling, and by
[the third principle](#principle-a-claimed-feature-gets-a-test) a claimed behavior the tooling
cannot reach gets a test that fails unconditionally and names what is missing. Write the check
document for it anyway: the procedure says what would be observed, and the notes say what stops
it. A later pass that gains the tool then has the procedure ready.

The closing list is therefore short. **It says what a check would need and nothing about the
model**, because `checks.md` is inside the specification-first zone: whether the absence of a check
is legal is a judgment about a claim, and that judgment belongs to the ledger. So the list reads
"this needs a second client in the mockup" and never "the model does not implement this". The
ledger then gives each statement `owed` or `no check`, and a reader who wants to know which
compares the two documents.

A list that grows past a handful of statements is a sign that the first exception is being
stretched to cover the third.

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
  the deviation into the `%description` and into `results.md` (step 7). Do not
  silently weaken the English expectation.
- When the faithful assertion fails because the model lacks a feature, keep the assertion
  and declare `%# expected-result: FAIL` (see AUTHORING.md). Never invert the assertion.
- Check the printed program (`printDescription = true`) against the English document. The
  two texts must tell the same story.
- Expression pitfalls (a wrong expression is a silent non-match, and the step times out):
  a unit-bearing field needs a unit literal (`udp.totalLengthField == 1008B`); the
  protocol prefix is the INET dissector name (`icmpv4`, not `icmp`); a protocol prefix
  resolves to the **last** chunk of that protocol in the packet, so read a field of an
  Ethernet frame through its chunk class name (`EthernetMacHeader.dest`, and not
  `ethernetmac.dest`, which lands on the frame check sequence); an ini key with a wrong
  path applies nothing. On a deadline miss, run the tester without `testName` first
  and read the real frames in the trace.

## Step 7 — run and analyze (`model/<proto>/results.md`)

Source the `setenv` script of OMNeT++ and then the one of INET; the runner is not on the
path otherwise. Build the framework library once per INET build, then run the suite:

```sh
cd tests/protocol/lib && ./build.sh
inet_run_protocol_tests -p inet -w ipv4
```

(`-p inet` skips the project discovery; discovery crashes on a `~/.omnetpp` directory.)

### The run record

Every document under `model/<proto>/` carries a run record at its head. A date alone is
not enough. A reader who sees only a date must guess which code ran, and the usual guess
is "the latest version". That guess is wrong as soon as the branch moves, and the reader
has no way to see the mistake.

The run record answers one question: what ran, and where? These fields are mandatory.

| Field | What to write |
| --- | --- |
| Date | The date and the time of the run, with the time zone. |
| INET | The branch, the commit, and the state of the working tree. |
| OMNeT++ | The version, and the commit when the checkout is a git repository. |
| Build | The mode, `debug` or `release`, and where the libraries come from. |
| Compiler | The name and the full version. |
| Platform | The distribution, the kernel and the machine architecture. |
| Command | The runner command, with every option. |

Do not write a commit hash for a working tree that has uncommitted changes. The hash then
names code that did not run. Write `dirty` beside the hash and list the changed files.

Check that the build is newer than the sources. A stale `libINET.so` gives you the
verdicts of an older commit under the name of the new one, and it gives no warning.

Two fields decide more results than a reader expects. The compiler version changes
floating-point results, and the build mode changes which library the test links. Write
both, even when they look unimportant.

This command prints the block. Run it in the worktree that you tested.

```sh
MODE=${MODE:-debug}
LIB=src/libINET$([ "$MODE" = debug ] && echo _dbg).so
if [ -z "$(git status --porcelain)" ]; then TREE=clean; else TREE="DIRTY -- the commit does not name the code that ran"; fi
if [ -n "$(find src -name '*.cc' -newer "$LIB" -print -quit)" ]; then BUILD="STALE -- rebuild before you trust the run"; else BUILD="built from this commit"; fi
echo "- Date: $(date '+%Y-%m-%d %H:%M %z')"
echo "- INET: branch \`$(git symbolic-ref --quiet --short HEAD || echo detached)\`, commit \`$(git rev-parse --short HEAD)\`, tree $TREE"
echo "- OMNeT++: $(opp_run -v | sed -n 's/^Version: \([^,]*\),.*/\1/p')"
echo "- Build: $MODE, $BUILD"
echo "- Compiler: $(clang++ --version | head -1)"
echo "- Platform: $(. /etc/os-release; echo "$PRETTY_NAME"), $(uname -srm)"
```

It gives a block of this shape:

```markdown
- Date: 2026-09-10 15:11 +0200
- INET: branch `master`, commit `7c6e39829e`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (...)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w ipv4`
```

Add the command line yourself, and add anything else that changed the run: a
configuration option, a patched source file, or a container image.

Three documents carry the run record: `results.md`, `coverage.md` and `conformance.md`.
`categories.md` records decisions, not run data, so it does not need one.

### The class of a failure, and when to declare it expected

Every failure falls in one of five classes, and the class decides what you do about it. The
classes follow from [the third principle](#principle-a-claimed-feature-gets-a-test); this is that
principle applied to a run.

| Class | What it means | What to do |
| --- | --- | --- |
| **test error** | the check is right and the program does not implement it | fix the test |
| **specification misread** | the check asks for something the standard does not say | fix the catalog and the check document |
| **defect** | the model claims the behavior and gets it wrong | keep the faithful test, **declare nothing**, file the gap |
| **untestable claim** | the model claims the behavior and the check cannot be built | keep the test failing, name the missing part in its description, **declare nothing**, file the gap against the tooling as well |
| **unimplemented feature** | the model does not claim the behavior | keep the faithful test, declare `%# expected-result: FAIL`, file the gap |

Only the last row declares anything. Three of the five classes end in a failure the run reports,
and that is the normal, intended outcome of a deep pass against a model that was not written
against these checks.

An expected-result declaration means one thing only: *this feature is known to be unimplemented,
so the failure is not a regression*. That is the wording of
[`AUTHORING.md`](../../../tests/protocol/lib/AUTHORING.md), in its section on declaring an
expected result, and it is the whole of what the declaration is for.

**Never declare a defect as an expected failure.** A defect must make the suite red and keep it
red until somebody fixes it. Declaring one turns the suite green over a bug and hides it, which is
the opposite of what a suite is for, and it is worse than having no test at all: the test now
states that the wrong behavior is the intended one.

The line is decidable, and this is where to draw it:

> **Does code exist for this specific behavior?** If it exists, the model claims the behavior:
> the failure is a **defect**, or an **untestable claim** when the check could not be built at
> all. If the behavior is absent — no code path, no function, no field, or a `TODO` or a stated
> limitation that says it is absent on purpose — it is an **unimplemented feature**.

Read the question about the *specific* behavior and not about the protocol claim, for the reason
[the principle](#principle-a-claimed-feature-gets-a-test) gives: claiming a protocol is not
claiming every feature of it.

**A half-written mechanism is a claim, and its failures are defects.** This is the trap. A
function that builds the right message and that nothing calls, a timer that fires and does the
wrong thing on expiry, a branch that exists and is never reached — each one is an effort, so each
one claims the behavior, so each one fails without a declaration. "Not finished" is not the same
as "not claimed", and only the second may be declared.

Two shapes to watch for, because both look like one class and are the other:

- A field that is **set to a wrong value** is a defect, not a missing feature. The setter is
  there; the value is wrong. A reply that carries a constant where the standard asks for a copy
  of the request is the clearest case.
- A branch that exists but is **unreachable for the case the check tests** is a defect. A server
  that tests two conditions in the wrong order has the code for both and reaches the wrong one.
- A **complete function that nothing calls** is a defect. The message builder is written, the
  trigger is not; the model claims the message and never sends it.
- A **mechanism present with the wrong law** is a defect. A retransmission that repeats at a
  constant interval, where the standard demands a randomized exponential one, has the code for
  the delay and the wrong value in it.

The word `defect` also appears in the conformance matrix of step 8, and it means something else
there: a whole mandatory **feature** that the model claims and does not support. A statement-level
defect usually sits inside a feature that otherwise works, which the matrix then reads as
`partial`. Say which of the two you mean wherever both could be read.

### What results.md holds

Record in `results.md`:

- the run record, and the verdict of every test;
- for each failure, its class, from the four of [the section above](#the-class-of-a-failure-and-when-to-declare-it-expected);
- the simulation model analysis: where the model implements the checked behavior, with
  file and line references. This is the first artifact that may reference code;
- sharpening candidates for the next pass.

Then write the verdict of every check and the support of every feature into the coverage
ledger (`coverage.md`). Do not edit the feature map. The rule, per feature:

- `supported` — every `core` check of the feature ran and passed;
- `partial` — at least one `core` check passed, and at least one `core` check failed as a
  model gap or is not yet tested;
- `not supported` — every `core` check that ran failed as a model gap;
- `untested` — **no `core` check of the feature exists**.

The last value is narrower than it looks, and it is the one to be careful with. A feature whose
checks all failed is `not supported`, never `untested`: the run answered the question, and the
answer was no. `untested` is for a feature nobody has written a check for — which, by
[the third principle](#principle-a-claimed-feature-gets-a-test), can only be a feature the model
does not claim, or one whose checks belong to a suite this pass did not write.

The same care applies to a statement row. When a check fails early and never reaches the
observation that a second statement needs, that second statement is **not** `untested`: its test
exists and its test failed. Give the row the verdict of its test and say, in the same cell, which
observation the failure kept the check from reaching. `untested` on a row whose test exists and
fails tells a reader that nobody wrote a test, which is the opposite of what happened.

Record the run record at the head of the ledger. The support statement is
bounded by the checks: a `supported` feature is supported as far as the checks reach, not
proven complete. A failure of a `supporting` check does not lower the support value by
itself; note it in the ledger, next to the feature.

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
cover its governing source document. Combine the claim, the support value of the ledger
(`coverage.md`), and the level of the feature into a verdict:

| Level | Claimed | Support | Verdict |
| --- | --- | --- | --- |
| any | yes | supported | `confirmed` |
| any | yes | partial | `partial` — list the gap checks |
| mandatory, unstated | yes | not supported | `defect` — the model does not do what it claims |
| optional | yes | not supported | `declined` — the model chose not to; a `should` deserves a reason in the results |
| any | yes | untested | `unverified` — a coverage gap, not a model verdict |
| any | no | supported | `undocumented` — the model does more than it claims |
| any | no | partial, not supported, or untested | `out of claim` |

The level row matters: a model that skips a `may` has not failed anything, and a matrix
that calls it a `defect` teaches the reader to ignore defects.

Record the run record, and the ledger state that the matrix comes from. A
`defect`, and an `unverified` feature with level `mandatory`, are the headlines for the
next pass. An `undocumented` feature is a documentation task for the model, not a test
task.

## Step 9 — decide the category (`model/<proto>/categories.md`)

Confirm, after the run, the category that the observation class predicted at step 3. The
categories and what each can establish are
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
5. re-runs everything and updates the coverage ledger, `results.md`, and the conformance
   matrix, and raises the achieved level when an exit criterion starts to hold. The
   catalogs and the feature map change only when a standard enters or leaves the in-scope
   set.

The catalog IDs and the feature IDs are stable names; the coverage ledger carries the
state between passes. A pass is complete when the ledger, the results, and the conformance
matrix agree.

### When a document is replaced

The case is real: RFC 9293 replaced RFC 793 while the model still cites RFC 793. When a
document leaves the in-scope set and its successor enters:

- The old catalog stays, and its IDs stay valid forever. Its header changes to
  `Status: superseded by <doc>`; nothing else in it changes.
- The successor gets its own folder and catalog, with its own IDs. The override table maps
  each old statement to the statement that replaces it.
- The feature map rewrites its `Sources` to the successor. This is the one edit to the
  feature map that the one-way principle permits, because a standard changed, not a test.
- A test whose `%description` names an old ID is not wrong, but it targets a text that no
  longer governs. The ledger marks its row `superseded`, and the next pass points the test
  at the governing ID and re-reads the check against the new text.

## Where everything lives

```
doc/project/evidence/standard/<doc>/     one folder per standard document, INET-free
  rfc791/rfc791.txt                      step 1: the cached text
  rfc791/catalog.md                      step 3: the statements, no run data
  rfc792/rfc792.txt                      step 1
  rfc792/catalog.md                      step 3

doc/project/evidence/protocol/<proto>/   one folder per protocol, INET-free
  ipv4/standards.md                      step 2: standards family + in-scope set
  ipv4/features.md                       step 4: feature map, no run data
  ipv4/checks.md                         step 5: English procedures, one section per check

doc/project/evidence/model/<proto>/      one folder per protocol, INET-dependent
  ipv4/coverage.md                       the ledger: status, test, verdict, support
  ipv4/results.md                        step 7: verdicts + model analysis
  ipv4/conformance.md                    step 8: model claims + conformance matrix
  ipv4/categories.md                     step 9: category decisions

tests/protocol/<proto>/                  the tests
  Rfc791TtlDecrement.test                step 6: one test per check
  Rfc791FragmentReassembly.test
  Rfc791DontFragment.test
```

## Pass log

Each protocol keeps its own pass log at the end of its coverage ledger,
`model/<proto>/coverage.md#pass-log`. The guide does not repeat it. A global table grows
with every protocol and every pass, and the ledger is the artifact that changes on every
pass in any case.
