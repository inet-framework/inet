# The project documentation structure

> **Kind:** decision · **Status:** implemented 2026-08-31 · **Stands on:** the documents in `doc/architecture/`, and the
> `omnet-julia/` document set as a source of ideas

A new structure for the documents that govern the INET Framework: the requirements, the designs, the
rules, and the audit and seal process. The structure replaces `doc/architecture/`. It does not keep
the old paths. The `omnet-julia/` folder is a copy from another project. Its ideas apply here; its
content does not.

## 1. What is wrong with the structure of today

The folder is called `doc/architecture/`, but only two of its documents are about architecture. The
rest are requirements, naming rules, commit rules, seal rules, ledgers, checklists and audit reports.
The name stopped describing the content some time ago.

Six more problems follow from that:

1. **One document holds four kinds of text.** `architectural-requirements.md` holds the rules
   (`AR-*`), the design rationale (§*How the requirements compose*), the enforcement ladder and map,
   and the contributor workflow. A reader who wants one of the four reads all four. Each of the four
   changes for a different reason.
2. **Rules and reality are mixed in some places and split in others.** `sealing.md` and
   `sealing-status.md` split correctly. `naming-conventions.md` and `naming-exceptions.md` split
   correctly. But the enforcement map inside the rule document is reality, not rule, and it drifts.
3. **Only two rule families have identifiers.** `AR-*` and `PR-*`. The naming rules have none, so a
   reviewer cannot cite the naming rule that a name breaks. The seal rules have none. The quality,
   test, release and documentation rules do not exist as documents at all.
4. **Two kinds of report share one folder.** `reports/` holds subsystem audits and pull request
   audits. They have different inputs, different rules and different lifetimes.
5. **There is no decision log.** The requirements say what INET must do. Nothing says why the packet
   model is a chunk tree and not something else, or which alternative was turned down. A question of
   the form "why not X?" has no home, so it is asked again every year.
6. **Nothing says where a fact belongs.** `doc/src/` (the published guides), `doc/architecture/`, the
   NED comments, the Doxygen comments and `.agent/rules/` all hold overlapping text. The
   `.agent/rules/architecture.md` file is a second, older copy of the repository map.

## 2. The two documentation roots

INET holds two document sets with two audiences. The design separates them and states the boundary.

| Root | Audience | Answers | Format |
| --- | --- | --- | --- |
| `doc/src/` | the user of the product | How do I use INET? How do I write a model? | `.rst`, published |
| `doc/project/` | the contributor, the reviewer, the AI agent | What must INET do? How is it built? What is my change checked against? | `.md`, not published |

**One fact lives in one root.** The product documents describe the models and the API as they are
today. The project documents describe the requirements, the decisions, the rules and their
enforcement. When a project document needs to explain a mechanism, it cites the chapter of the
product document. It does not repeat it.

The new root is `doc/project/`. The name says what the documents are about: the project itself. The
folder `doc/architecture/` becomes one folder inside it.

## 3. The chain

Each folder answers one question. The folders form a chain. Each row stands on the row above it. A
reader who asks *why is this so* walks up. A reader who asks *what must I build* walks down.

```
requirement/   What must INET do?
  └─ design/   How is it built, why not otherwise, and what is each part?
       ├─ rule/     What is a change checked against?
       │    └─ domain/   What more is checked in one protocol family?
       └─ the code
```

Beside the chain, and checked against every row of it:

```
enforcement/   Which gate checks which rule, and how do I run it?
audit/         What did an audit find, what deviations are known, and what is sealed?
evidence/      Which test backs each claim?
guide/         How do I do one task?
history/       How did the current state come to be?
```

## 4. The folders and the files

The full inventory. The **Owns** column gives the identifier prefix that the document owns. A file
marked *new* does not exist today.

### `doc/project/README.md`

The map. It draws the chain, lists every document with the question it answers, states the document
header, and states the seal classes. A reader starts here. *(new)*

### `requirement/` — what INET must do

| File | Kind | Owns | Answers |
| --- | --- | --- | --- |
| `accepted-requirements.md` | what | `R-*` | What must INET do for a simulation user? The accepted set. |
| `candidate-requirements.md` | what | `C-*` | Which requirements are proposed and not yet accepted? The backlog. *(new)* |

**No "why" layer.** A young project needs a document that argues why anyone would want it. INET is
twenty years old and has its users, so the argument is settled and the two documents that would carry
it are not written. A requirement stands on its own merit, and `evidence/claim-coverage.md` says
which test demonstrates it.

### `design/` — how it is built

| File | Kind | Owns | Answers |
| --- | --- | --- | --- |
| `decisions.md` | decision | `D-*` | How is INET built? Each decision, what it serves, what it costs. *(new)* |
| `rejected-designs.md` | decision | `REJ-*` | Which designs were considered and turned down, and why? *(new)* |
| `repository-layout.md` | reference | — | Where does everything live: the source tree, the tests, the examples, the tools, the build files? *(new)* |
| `node-anatomy.md` | design | — | What is a network node made of? *(new)* |
| `protocol-anatomy.md` | design | — | What is one protocol model made of, from NED to serializer to test? *(new)* |
| `packet-anatomy.md` | design | — | What is a packet made of: chunks, tags, signals, and the wire boundary? *(new)* |
| `test-anatomy.md` | design | — | What is a test made of, and what does each of the twelve categories cover? *(new)* |

An anatomy document is short and structural. It states the settled form and cites the Developer's
Guide chapter for the detail. It never repeats the chapter.

A decision names the requirements it serves and what it costs. A design that carries no cost is not
described honestly.

### `rule/` — what a change is checked against

Every rule prefix ends in `R`. Every rule is a heading, so a citation can link to the rule itself.

| File | Owns | Answers |
| --- | --- | --- |
| `architecture.md` | `AR-*` | What must the structure of the code respect? Layers, contracts, packages, folders, dependency direction, features. |
| `naming.md` | `NR-*` | How is every kind of artifact named? |
| `quality.md` | `QR-*` | How does the code read? Comments, size budgets, error idioms, logging. *(new)* |
| `testing.md` | `TR-*` | Which test category backs which claim, and when may a baseline change? *(new)* |
| `pull-request.md` | `PR-*` | How is a change divided into commits, and what must a pull request contain? |
| `release.md` | `RR-*` | How is a version numbered, what goes in WHATSNEW, and what does a breaking change owe the user? *(new)* |
| `sealing.md` | `SR-*` | What does a seal mean, and what is the audit before it? |
| `documentation.md` | `DR-*` | Where does a fact live, how is a document shaped, and how is an identifier formed? *(new)* |

Three of these are splits of `architectural-requirements.md`, not new text:

- The `AR-*` rules stay in `rule/architecture.md`.
- §*How the requirements compose* becomes the head of `design/decisions.md`. It is rationale.
- §*Enforcement tiers* and §*Enforcement map* move to `enforcement/README.md`. They are reality.
- §*Contributor workflow* becomes `guide/contribute-a-change.md`. It is a procedure.

### `domain/` — what more is checked in one family

| File | Owns | Scope |
| --- | --- | --- |
| `README.md` | — | What a domain document is, and when a family earns one. *(new)* |
| `ieee80211.md` | `AR-WLAN-*` | `src/inet/linklayer/ieee80211/`, `src/inet/physicallayer/wireless/ieee80211/` |

A domain document holds four things: the paths it covers, the rules that apply there in addition to
`rule/`, a link to its review checklist, and a link to its audit and seal state. One document per
family. The next ones are likely `tsn.md` and `physicallayer.md`.

### `enforcement/` — the machinery

| File | Answers |
| --- | --- |
| `README.md` | What are the five tiers? Which gate exists, what does it check, and how do I run it? *(new)* |
| `check-architecture.sh` | The dependency direction check for `AR-ORG-DOMAINS` and `AR-ORG-VIS-SPLIT`. |
| `check-naming.sh` | The mechanical part of `NR-*`: file names, package names, generated pairs. *(new)* |
| `check-commits.sh` | The mechanical part of `PR-*`: whitespace, move, baseline, linear, subject. *(new)* |
| `check-seals.sh` | The seal flags: placement, coverage, and the generated index. *(new)* |
| `check-cpp.sh` | Runs clang-tidy over a subtree with the root `.clang-tidy`. *(new)* |
| `checklist/general.md` | The tier 4 checklist: the semantic rules an agent judges. |
| `checklist/ieee80211.md` | The tier 4 checklist for the IEEE 802.11 subtrees. |

Two rules hold for this folder:

- **It holds only what a gate runs.** Heavy tooling stays in `python/inet/` and `_scripts/`. This
  folder holds the direct transcription of a numbered rule.
- **No copies.** A tool file that must sit at the repository root sits at the root, and this folder
  cites it. clang-tidy finds its configuration by a walk up from the source file, so the
  configuration must sit at the root.

**The two clang-tidy check sets become one.** Today they are split and neither runs in CI:

| Today | What it checks | What happens |
| --- | --- | --- |
| `doc/architecture/enforcement/.clang-tidy` | `readability-identifier-naming` — the `NR-*` rules a linter can see | becomes the root `/.clang-tidy` |
| `_scripts/clang-tidy/run.sh` | `bugprone-*` and `modernize-*`, less nine exclusions | its check list moves into that same file |
| `_scripts/clang-tidy/run.sh`, `prepare.sh` | a hand-run tool with `--fix` | deleted, and `enforcement/check-cpp.sh` replaces it |

The merged file is one authority for two rule families: the name rules of `rule/naming.md` and the
bug and modernization rules of `rule/quality.md`. `check-cpp.sh` runs it without `--fix`, because a
gate reports and does not rewrite. It keeps a `--fix` flag for the hand-run sweep, which lands as a
mechanical commit under `PR-SPLIT-MECHANICAL`.

### `audit/` — the findings, the ledgers, the seals

| File | Kind | Owns | Answers |
| --- | --- | --- | --- |
| `README.md` | procedure | — | What is an audit, what are the four kinds, and what state can a path be in? *(new)* |
| `architecture-exceptions.md` | ledger | `AS-*`, `AV-*` | Which couplings depart from `rule/architecture.md`? |
| `naming-exceptions.md` | ledger | `NS-*`, `NV-*` | Which names depart from `rule/naming.md`? |
| `seal-list.md` | ledger | — | Which paths are sealed, against which audit? Plus a generated index of the seal flags inside the documents. |
| `report/subsystem/<slug>.md` | report | — | What did the audit of one directory tree find? |
| `report/pull-request/pr-<n>.md` | report | — | What did the audit of one pull request find? |
| `report/sweep/<rule>.md` | report | — | What did one rule find across the whole tree? |
| `report/document/<slug>.md` | report | — | What did the audit of one project document find? |

A ledger is append-only and its identifiers are permanent. A report is a snapshot with a date and a
commit. A re-audit rewrites the report file; git holds the older text.

### `evidence/` — the claims

| File | Kind | Owns | Answers |
| --- | --- | --- | --- |
| `claim-coverage.md` | measurement | — | For each `R-*` requirement: which test, example or showcase demonstrates it? *(new)* |

`claim-coverage.md` is the traceability that `AR-QUAL-TRACEABILITY` asks for, in one table. It points
into `tests/validation/`, `showcases/` and `examples/`. It creates no new folder.

**No risk register yet.** A `risks.md` with `RISK-*` identifiers is the natural neighbour of
`design/decisions.md`, because a decision names the risks it accepts. INET is a mature framework, and
its risks are not the risks of a new simulator. The document waits until someone needs it. The
`RISK-*` prefix stays reserved, so nobody gives it another meaning.

### `guide/` — one task per file

A guide file name is the task, and it starts with a verb.

| File | Answers |
| --- | --- |
| `contribute-a-change.md` | The nine steps from scope to seal. *(from the contributor workflow)* |
| `audit-a-subsystem.md` | How do I audit a directory and write the report? *(new)* |
| `review-a-pull-request.md` | How do I run the `PR-*` checks and write the report? *(new)* |
| `add-a-protocol.md` | Which contracts, registrations, tests and documents does a new protocol need? *(new)* |
| `change-a-baseline.md` | How do I change a fingerprint or a statistical baseline, and what must go with it? *(new)* |
| `run-the-gates.md` | How do I run every check before I push? *(new)* |

### `history/` — how it came to be

| File | Kind | Answers |
| --- | --- | --- |
| `design-history.md` | history | How did the current state come to be? One section per subject, one link per plan in `plan/done/`. *(new)* |

**A document describes what is.** How it came to be is in `history/`. A reader who wants the current
state does not read the history. A reader who wants the history finds it in one place.

### At the repository root

| Path | What it holds |
| --- | --- |
| `plan/pending/`, `plan/done/` | The design and implementation plans. A done plan is one step of the history. *(new)* |
| `AGENTS.md` | A short pointer into `doc/project/`. It holds no rules of its own. *(new)* |
| `.agent/rules/` | Pointers into `doc/project/`. The current copy of the repository map is deleted. |

## 5. The document header

Every document starts with one block quote under its title:

```markdown
> **Kind:** rule · **Status:** current · **Seal:** by section · **Owns:** `NR-*` · **Stands on:** [architecture.md](architecture.md), [decisions.md](../design/decisions.md)
```

**Kind** is one of eleven. The kind says which question the document answers, and it decides how the
document may change.

| Kind | The document answers | How it changes |
| --- | --- | --- |
| what | What must INET do? | by an accepted requirement |
| decision | How is it built, and why not otherwise? | by a new decision |
| rule | What is a change checked against? | by a new decision |
| design | What is each part, in its settled form? | with the code |
| reference | Where is what, and how is it used? | with the tree |
| ledger | Which known deviations exist? | append only; ids are permanent |
| report | What did one audit find, on one date? | a re-audit rewrites it |
| risk | What can go wrong? | when a risk fires or a mitigation lands |
| measurement | What did we measure? | when we measure again |
| procedure | How do I do this task? | when the task changes |
| history | How did we get here? | append only |

**Status** is `current`, `current; <qualifier>`, `draft`, `snapshot <date>`,
`superseded by <document>`, or `generated, do not edit`.

**Seal** declares the *unit* that this document seals by. It is a property of the document, not its
current state. Section 9 gives the values and the flags. The short form:

| Value | The unit that carries a flag |
| --- | --- |
| `none` | the document carries no flags |
| `whole` | the document, as one unit |
| `by section` | each `##` or `###` section |
| `by requirement`, `by rule`, `by decision` | each item that owns an identifier |
| `by row` | each row of a table |

A `by <unit>` value may add `, complete`. Then every unit must carry a flag, and a gate checks it.

**Owns** names the identifier prefix that the document owns, or `—`. Exactly one document owns a
prefix. A gate can check this.

**Stands on** names the documents that this one depends on directly, by their path from this file.

## 6. The identifiers

An identifier is `SCREAMING-KEBAB-CASE`. Each identifier is a heading, so a citation links to the
rule and not to the file:
`[AR-ORG-DOMAINS](../rule/architecture.md#ar-org-domains)`.

Three rules govern every identifier:

- **An identifier is permanent and is never reused.** A retired rule retires its identifier.
- **The identifier names the rule, not its position.** A regroup moves nothing else.
- **One document owns one prefix**, and the prefix says which document to open.

| Prefix | Means | Lives in |
| --- | --- | --- |
| `R-…` | a requirement INET promises the user | `requirement/accepted-requirements.md` |
| `C-…` | a candidate requirement, not yet accepted | `requirement/candidate-requirements.md` |
| `D-…` | a design decision, with what it costs | `design/decisions.md` |
| `REJ-…` | a design considered and turned down | `design/rejected-designs.md` |
| `AR-…` | an architecture rule | `rule/architecture.md` |
| `AR-<DOMAIN>-…` | a domain extension of the architecture rules | `domain/<domain>.md` |
| `NR-…` | a naming rule | `rule/naming.md` |
| `QR-…` | a code quality rule | `rule/quality.md` |
| `TR-…` | a test rule | `rule/testing.md` |
| `PR-…` | a commit and pull request rule | `rule/pull-request.md` |
| `RR-…` | a release rule | `rule/release.md` |
| `SR-…` | a sealing rule | `rule/sealing.md` |
| `DR-…` | a documentation rule | `rule/documentation.md` |
| `AS-…` / `AV-…` | a sanctioned architecture exception / an open violation | `audit/architecture-exceptions.md` |
| `NS-…` / `NV-…` | a sanctioned naming exception / an open violation | `audit/naming-exceptions.md` |
| `RISK-…` | a cost the framework accepts | reserved; `evidence/risks.md` is not written yet |

**Cite a rule in the source only to mark an exception**, or to mark a constraint that a later reader
would otherwise "correct". Never cite a rule to announce compliance.

## 7. What each artifact kind is ruled by

The design must cover every kind of thing in the repository. This table is the coverage check. Every
row has an owner, and no row has two.

| Artifact | Its name | Where it goes | Other rules |
| --- | --- | --- | --- |
| directory, NED package, C++ namespace | `NR-PKG-*` | `AR-ORG-DOMAINS` | — |
| C++ header and source file | `NR-FILE-*` | `AR-ORG-*` | `QR-*` for the shape |
| C++ type, function, member, macro, constant | `NR-CPP-*` | — | `QR-*` |
| NED type, gate, parameter, property, signal, statistic | `NR-NED-*` | `AR-ORG-DOMAINS` | `AR-OBS-NED-TRUTH` |
| MSG type and field | `NR-MSG-*` | `AR-PKT-*` | — |
| generated `_m.h` / `_m.cc` | `NR-GEN-*` | beside the `.msg` | `SR-*`: the seal follows the source |
| INI file, config section, iteration variable | `NR-INI-*` | `AR-CFG-*` | — |
| feature id in `.oppfeatures` | `NR-FEATURE-*` | `AR-EXT-FEATURES` | `RR-*` for compatibility |
| test file, test case, fingerprint tag | `NR-TEST-*` | `design/test-anatomy.md` | `TR-*` |
| example, showcase, tutorial folder | `NR-EXAMPLE-*` | `AR-ORG-*` | `R-DOC-RUNNABLE` |
| icon and image | `NR-ASSET-*` | `images/` | `AR-QUAL-DISPLAY` |
| Python module and script | `NR-TOOL-*` | `python/`, `_scripts/` | `QR-*` |
| CI workflow file | `NR-CI-*` | `.github/workflows/` | `TR-CI-*` |
| branch name | `PR-BRANCH-*` | — | — |
| commit | `PR-MSG-*` | — | `PR-SPLIT-*`, `PR-SERIES-*` |
| pull request | `PR-REQ-*` | — | `PR-REVIEW-*`, `PR-MERGE-*` |
| release tag, version, WHATSNEW entry | `RR-*` | — | `R-DIST-COMPAT` |
| project document | `DR-NAME-*` | `doc/project/` | `DR-*`, `SR-*` |
| published chapter | `DR-NAME-*` | `doc/src/` | `DR-WHERE-*` |
| audit report | `DR-NAME-*` | `audit/report/<kind>/` | `audit/README.md` |
| plan | `DR-NAME-*` | `plan/pending/`, `plan/done/` | — |
| identifier | `DR-ID-*` | its owning document | — |

Four rule families do not exist today and this table needs them: `QR-*`, `TR-*`, `RR-*` and `DR-*`.
`NR-*` exists as prose without identifiers, and it stops at the code; it must reach the branch names,
the CI files and the documents.

## 8. The audit process

An audit is the act that turns "we believe this complies" into "this complies, and here is the
evidence". `audit/README.md` owns the process. It has four kinds and five states.

### The four kinds

| Kind | Input | Checked against | Output |
| --- | --- | --- | --- |
| subsystem | a directory tree | `rule/architecture.md`, `rule/naming.md`, `rule/quality.md`, the domain document | `report/subsystem/<slug>.md` |
| pull request | a branch or a pull request | `rule/pull-request.md`, plus the rules the diff touches | `report/pull-request/pr-<n>.md` |
| sweep | one rule family, the whole tree | one rule document | `report/sweep/<rule>.md` |
| document | one project document | `rule/documentation.md` | `report/document/<slug>.md` |

### The five states of a path

```
unaudited ──audit──► audited ──fix or sanction──► compliant ──the user seals──► sealed
                                                                                  │
                                                              a cited rule changes │
                                                                                  ▼
                                                                              re-audit
```

| State | What it means |
| --- | --- |
| `unaudited` | The default. Nothing is claimed. |
| `audited` | A dated report exists. Every finding is a ledger row. |
| `compliant` | Every finding is repaired, or is sanctioned as an `AS-*` or `NS-*` row. |
| `sealed` | Compliant, and frozen by the user. An AI must not change it. |
| `re-audit` | Sealed, but a rule that the report cites changed after the audit. |

### What a report holds

Scope, date, the exact commit, the commands that were run, the rules that were checked, one row per
finding with its ledger id, and one verdict line. The verdict is `PASS` or `FAIL` with counts. A
finding that is not in a ledger is lost, so every finding gets a row.

### The rule that keeps the ledgers honest

A finding is either repaired or sanctioned. It is never dropped. A `Done` row keeps its identifier
and moves to a resolved section, so the open part of the ledger stays short and readable.

## 9. The seal process

A seal says: *this was audited, it complied, and it is now frozen against AI change*.
`rule/sealing.md` owns the rules as `SR-*`.

### 9.1 Three scopes, and where each one keeps its state

A seal can cover a path, a whole document, or one unit inside a document. Each scope keeps its state
in the place a reader already looks. **The artifact is the authority. The registry is an index.**

| Scope | Examples | Where the state lives |
| --- | --- | --- |
| a path | `common/packet/`, `Ipv4.cc` | `audit/seal-list.md`, because a source file cannot carry a flag |
| a whole document | `rule/sealing.md`, `design/decisions.md` | the `Seal` field of its header |
| a unit in a document | one `R-*` requirement, one section of an anatomy, one `NR-*` rule, one sanctioned ledger row | an inline flag on the unit |

`audit/seal-list.md` therefore holds two parts: the **sealed paths**, which it owns, and a
**generated index** of every document with its counts of closed and open units. The index is marked
`generated, do not edit`, and `enforcement/check-seals.sh` writes it. Nobody keeps two lists by hand.

### 9.2 The flags

Two flags, and three readings. This is what lets a document carry flags on the units that need them
and stay quiet everywhere else.

| Flag | Reading |
| --- | --- |
| no flag | **open, not yet considered.** The default. Nothing is claimed. |
| ⬜ | **open on purpose.** The unit was looked at and left open: still under discussion, or waiting for a decision. |
| 🔒 | **closed.** The unit was audited, it complies, and the user froze it. |

The default keeps the cost at zero. A document that needs no flags carries `**Seal:** none` and
nothing else changes. A document that settles one requirement at a time carries a flag on that one
requirement and leaves the rest bare.

⬜ is not maintenance work; it is a statement. Use it to mark the frontier — the units that are being
worked on now — so a reader can tell "we left this open" from "we never got here".

**Do not change a closed unit. Do not edit its words, do not reformat it, and do not change it as
part of a larger task. A closed unit changes only when the user gives explicit permission for that
unit in the conversation.** This is the same rule as for a sealed file, at the granularity of one
requirement or one section.

### 9.3 Where the flag goes

**A flag never goes in a heading.** A heading is the citation anchor, and a citation such as
`[NR-NED-GATE](../rule/naming.md#nr-ned-gate)` must not break because a unit closed. The rule by unit
kind:

| Unit | Placement |
| --- | --- |
| a section or a rule with a heading | the flag opens the first line of the body under the heading |
| a bullet item | the flag opens the item |
| a table row | a first column named `Seal` |
| the whole document | the `Seal` field of the header, as `whole; closed` |

```markdown
### NR-NED-GATE — A gate is named for the peer it faces

🔒 A gate name is `<stem>In` or `<stem>Out`, and the stem names the peer, not the packet.
```

### 9.4 What the seal covers, and what it does not

The seal covers the **text of the unit and its identifier**. It does not cover the unit's position.
A regroup that moves a closed unit without changing one word of it is allowed, and it lands as a
mechanical commit under `PR-SPLIT-MECHANICAL`. This is what makes the identifiers worth having: a
document can be reordered while every closed unit and every citation survives.

Reopening a closed unit is the same act as changing it. Replace 🔒 with ⬜, get explicit permission,
and say in the commit message why the unit reopened.

### 9.5 Which document seals by what

| Document | Seal | Why |
| --- | --- | --- |
| `requirement/accepted-requirements.md` | `by requirement, complete` | every requirement should show whether its wording is settled |
| `requirement/candidate-requirements.md` | `none` | a candidate is by definition not settled |
| `design/decisions.md` | `by decision` | a decision closes when it is taken |
| `design/*-anatomy.md` | `by section` | an anatomy settles one layer at a time |
| `design/repository-layout.md` | `none` | it follows the tree |
| `rule/*.md` | `by rule` | most rules settle one at a time; a document that closes every rule can be promoted to `whole` |
| `rule/sealing.md`, `rule/documentation.md` | `whole` | they change only by a new decision |
| `domain/*.md` | `by rule` | same as the general rules |
| `audit/*-exceptions.md` | `by row` | a **sanctioned** row is a settled decision and may close; an open violation row must stay editable |
| `audit/seal-list.md`, `audit/report/**` | `none` | a ledger of paths and a dated report |
| `evidence/**`, `guide/**`, `history/**` | `none` | they change with the facts |

A document reaches `whole` by a promotion: once every unit is closed, the header becomes
`**Seal:** whole; closed` and the inline flags come out in the same commit.

### 9.6 Three things the design adds to the rules of today

**Each seal cites its audit report.** A path row and a closed unit both name the report that audited
them. The report names the rules it checked and the commit it examined. Without that citation a seal
is a claim with no evidence.

**A rule change can make a seal stale.** When a rule changes in a way that changes what compliance
means, the change marks every seal whose report cites that rule as `re-audit`. The reports name their
rules, so a script finds the rows. This stops the seal list from becoming a list of old opinions.

**A folder seal stays recursive** and covers files added later. That is why a new file under a sealed
folder needs permission.

### 9.7 The gate

`enforcement/check-seals.sh` is a tier 3 check. It fails when:

- a flag sits in a document whose header declares `none`, or a unit kind that does not match;
- a flag sits inside a heading line;
- a document marked `complete` has a unit with no flag;
- the generated index in `audit/seal-list.md` does not match the flags in the tree.

It also writes the index, so the same script checks and regenerates.

## 10. Enforcement

The five tiers stay, and they move to `enforcement/README.md`, because two rule documents cite them.

- **T1 — will not build.** The compiler or the NED toolchain rejects it.
- **T2 — fails a CI test.**
- **T3 — a deterministic check flags it.** A linter or a script.
- **T4 — an agent reviewer flags it.** The semantic rules.
- **T5 — a human decides.** Genuine design judgment.

The enforcement map changes direction. Today one big table lists every rule and how it is enforced,
and it drifts because it lives in the rule document.

- **Each rule states its own tier and its own gate, on one line under the rule.** That is
  authoritative, and it cannot drift from the rule.
- **`enforcement/README.md` holds the gate inventory**: one row per check that really exists, what it
  runs, and which rules it covers. The table then lists machinery, not intent, and it shows the
  coverage from the side that can be measured.

## 11. Why this structure is better

| Problem of today | What the design does |
| --- | --- |
| the folder name does not fit the content | `doc/project/`, beside `doc/src/`, with a stated boundary |
| one document holds four kinds of text | one kind per document, and the kind is in the header |
| rules and reality mix | `rule/` is the target, `audit/` is reality, `enforcement/` is the machinery |
| only three rule families have identifiers | eight rule documents, eight prefixes, every rule a heading |
| naming stops at the code | `NR-*` covers documents, branches, CI files and reports |
| two report kinds share a folder | one folder per report kind |
| no decision log | `design/decisions.md` and `design/rejected-designs.md` |
| a seal is a claim with no evidence | each seal row cites its report; a rule change marks it `re-audit` |
| only a whole source path can seal | a path, a whole document or one unit in a document can close, and the flags are optional |
| no home for the history | `history/design-history.md` and `plan/done/` |
| `.agent/rules/` is a second copy | it becomes a pointer |

## 12. The move from today

| Today | New path | Work |
| --- | --- | --- |
| `architectural-requirements.md` | `rule/architecture.md` | keep the `AR-*` rules only |
| " §How the requirements compose | `design/decisions.md` | becomes the head of the decision log |
| " §Enforcement tiers, §Enforcement map | `enforcement/README.md` | invert the map to a gate inventory |
| " §Contributor workflow | `guide/contribute-a-change.md` | move |
| `requirements.md` | `requirement/accepted-requirements.md` | rename; add the seal markers |
| `naming-conventions.md` | `rule/naming.md` | add `NR-*` identifiers; extend to the new artifact kinds |
| `naming-exceptions.md` | `audit/naming-exceptions.md` | move; add a resolved section |
| `architecture-exceptions.md` | `audit/architecture-exceptions.md` | move; add a resolved section |
| `pull-requests.md` | `rule/pull-request.md` | add `PR-BRANCH-*`, `PR-REVIEW-*`, `PR-MERGE-*` |
| `sealing.md` | `rule/sealing.md` | add `SR-*` identifiers and the staleness rule |
| `sealing-status.md` | `audit/seal-list.md` | add the report citation column and the document section |
| `ieee80211-architectural-requirements.md` | `domain/ieee80211.md` | move |
| `enforcement/agent-review-checklist.md` | `enforcement/checklist/general.md` | move |
| `enforcement/ieee80211-agent-review-checklist.md` | `enforcement/checklist/ieee80211.md` | move |
| `enforcement/check-architecture.sh` | `enforcement/check-architecture.sh` | keep |
| `enforcement/.clang-tidy` | `/.clang-tidy` at the root | add the `bugprone-*` and `modernize-*` list from `_scripts/clang-tidy/run.sh` |
| `_scripts/clang-tidy/` | — | delete; `enforcement/check-cpp.sh` replaces it |
| `reports/common-packet.md` | `audit/report/subsystem/common-packet.md` | move |
| `reports/pr-11*.md` | `audit/report/pull-request/` | move |

Every move is `git mv` plus a link repair. Only three documents need a content split, and all three
splits come out of `architectural-requirements.md`.

## 13. The order of the work

**Every phase below is implemented.** Section 15 records what the implementation changed about the
design, and what it found.


**Phase 1 — the frame.** Create `doc/project/`, move every existing document, repair the links, and
write `README.md`. The set works after this step, with no new text.

**Phase 2 — the splits.** Divide `architectural-requirements.md` into the rule document, the head of
the decision log, the enforcement README and the workflow guide.

**Phase 3 — the identifiers.** Give `NR-*` identifiers to the naming rules and `SR-*` to the seal
rules. Add the header block to every document. Add the `Owns` field. Merge the two clang-tidy check
sets into the root `.clang-tidy`, write `enforcement/check-cpp.sh`, and delete `_scripts/clang-tidy/`.

**Phase 4 — the audit and seal upgrade.** Write `audit/README.md`. Add the report citation and the
`re-audit` state to `audit/seal-list.md`. Divide `report/` by kind. Add the `Seal` header field, the
inline flags and `enforcement/check-seals.sh`.

**Phase 5 — the missing rule documents.** `rule/quality.md`, `rule/testing.md`, `rule/release.md`,
`rule/documentation.md`.

**Phase 6 — the design layer.** `design/decisions.md`, `design/rejected-designs.md`,
`design/repository-layout.md`, and the four anatomy documents.

**Phase 7 — the rest.** `evidence/`, `guide/`, `history/`, `domain/README.md`, the new gate scripts,
and the pointers at the repository root.

Phases 1 to 4 make the structure real. Phases 5 to 7 fill the slots that the structure opens, and
each one can wait for the day someone needs it.

## 14. Decisions

The questions the drafts raised, and the answers. All were settled on 2026-08-31.

1. **The root folder is `doc/project/`.** Good enough for now. A later rename costs one `git mv` and
   a link sweep, because every citation is a relative path.
2. **`doc/src/` stays as it is.** It is the Sphinx source root, and the rename touches the build. The
   design does not need it. The boundary of §2 holds whatever the folder is called.
3. **One clang-tidy configuration, at the repository root.** The check list of
   `_scripts/clang-tidy/run.sh` moves into it, and `_scripts/clang-tidy/` is deleted. See
   §4 *`enforcement/`*.
4. **No risk register yet.** The `RISK-*` prefix stays reserved. See §4 *`evidence/`*.
5. **No "why" layer.** `requirement/user-tasks.md` and `requirement/user-benefits.md` are dropped,
   with the `T-*` and `B-*` prefixes. The project is mature and does not need to argue for itself.
   `requirement/` now holds the accepted set and the candidate backlog, and the chain starts at
   *what must INET do*.
6. **The design pair keeps its names**: `design/decisions.md` and `design/rejected-designs.md`. A
   symmetric `accepted-designs.md` was considered and turned down. The two pairs split on two
   different axes: a requirement moves from candidate to accepted, which is a lifecycle, while a
   design is taken or rejected, which is the outcome of one comparison. `accepted` would then mean
   *not a candidate* in `requirement/` and *not rejected* in `design/`. One word, two meanings, in
   one document set. `decisions.md` also carries the name of its own kind, which no other file does
   as directly.

Nothing else is open. The design is ready to build.

## 15. What the implementation changed

Implemented on 2026-08-31, in the `topic/project-documentation` worktree, as eleven commits. Four
things differ from the design above, and each is a fact the design could not have known.

### 15.1 An identifier heading holds the identifier alone

The design said "each identifier is a heading, so a citation links to the rule itself". That is not
enough. `### AR-ORG-DOMAINS — Layered, domain-partitioned source tree with acyclic dependencies`
anchors as `#ar-org-domains--layered-domain-partitioned-source-tree-with-acyclic-dependencies` —
nobody writes that by hand, and it breaks the moment the statement is reworded.

The heading now holds the identifier alone, and the statement becomes the bold lead sentence of the
body. The anchor is then the lowercased identifier. It also leaves the first body line free for the
seal flag, which is where `SR-FLAG-PLACEMENT` puts it. An Index table at the head of each document
restores the overview that the heading used to give, and it is generated.

101 headings were converted, in five documents. The rule is
[DR-ID-HEADING](../../doc/project/rule/documentation.md#dr-id-heading).

### 15.2 The requirements were not citable at all

They were bullet items, so nothing could link to one, and no requirement could carry a seal flag.
The 27 accepted requirements are now headings, each with a `⬜` flag, and the document declares
`Seal: by requirement, complete`.

### 15.3 The naming rule identifiers, as they came out

The design's coverage table proposed identifiers by artifact kind. The document's own section
boundaries were a better fit, so the 23 `NR-*` rules follow them: `NR-CASE`, `NR-WORD`, `NR-PKG`,
`NR-NED-ROLE`, `NR-NED-TYPE`, `NR-NED-GATE`, `NR-NED-PARAM`, `NR-NED-PROP`, `NR-NED-SIGNAL`,
`NR-MSG-TYPE`, `NR-MSG-FIELD`, `NR-CPP-TYPE`, `NR-CPP-NAME`, `NR-CPP-REG`, `NR-CPP-TIMER`, `NR-GEN`,
`NR-INI`, `NR-FEATURE`, `NR-DIR`, `NR-TEST`, `NR-ASSET`, `NR-TOOL`, `NR-CI`. The last three are new;
branch and commit names stayed with `PR-*`, and document names with `DR-NAME`, as designed.

### 15.4 A sixth gate: the link checker

The design named five gate scripts. Writing the documents needed a sixth immediately — a link checker
— because a broken `#anchor` is silent: it lands the reader at the top of the right document, and
they believe they are in the right place. It found 114 broken links after the move.
`enforcement/check-links.sh` is now the `T3` gate for
[DR-LINK-RELATIVE](../../doc/project/rule/documentation.md#dr-link-relative).

## 16. What the implementation found

Four facts about the repository, none of which were visible before the documents were arranged to
make them visible.

**The clang-tidy configuration had never run.** `doc/architecture/enforcement/.clang-tidy` said in
its own header that it had to be moved to the repository root to take effect, and it never was.
clang-tidy finds its configuration by walking up from the source file. Meanwhile
`_scripts/clang-tidy/run.sh` held a second, different check list inline, behind a hand-run `--fix`,
which CI also never called. Both are now one file at the root.

**No rule gate runs in CI.** `.github/workflows/` holds twelve test and build workflows and not one
rule check. Every `T3` row in the gate inventory is a script a person must remember to run, which is
the weakest form of every rule it covers. This is the largest remaining gap, and
[enforcement/README.md](../../doc/project/enforcement/README.md) says so in the inventory rather than
implying coverage that does not exist.

**The one sealed path does not satisfy the seal rules.** `common/packet/` was sealed over two
`AV-ORG` clusters that are still `Open (decide)` rather than sanctioned, which
[SR-AUDIT-FIRST](../../doc/project/rule/sealing.md#sr-audit-first) forbids. It is recorded in
[audit/seal-list.md](../../doc/project/audit/seal-list.md) rather than quietly repaired: either the
two clusters get sanctioned as `AS-*` rows, or the couplings are inverted, or the seal comes off.
That is the user's call.

**The commit gate failed on its own history.** `check-commits.sh`, run over the commits that built
this structure, flagged a 74-character subject against `PR-MSG-SUBJECT`. It was repaired by a replay
rather than left, which is the smallest possible demonstration that the gate is real.

## 17. What is left

| Work | Why it waits |
| --- | --- |
| wire the gates into `.github/workflows/` | the largest gap; needs a CI decision, not a document |
| fill `evidence/claim-coverage.md` | a deliberate pass over 27 requirements and twelve test categories |
| decide `common/packet/`: sanction, invert, or unseal | the user's call |
| audit and close the first units | nothing in the set has been through a document audit; every flag is `⬜` or absent |
| `evidence/risks.md` | deferred by decision 4; the `RISK-*` prefix stays reserved |
