# The INET project documentation

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** every document in this folder

This folder holds the documents that govern the INET Framework: what it must do, how it is built and
why, what a change is checked against, and what has been audited and sealed. **This file is the map.**

It is one of two documentation roots, and the boundary matters
([DR-TWO-ROOTS](rule/documentation.md#dr-two-roots)):

| Root | Audience | Answers |
| --- | --- | --- |
| `doc/src/` | the user of the product | How do I use INET? How do I write a model? |
| **`doc/project/`** | the contributor, the reviewer, the AI agent | What must INET do? How is it built? What is my change checked against? |

One fact lives in exactly one of them. When a document here needs to explain a mechanism, it cites
the guide chapter rather than restating it.

## The chain

Each folder answers one question, and each row stands on the row above it. A reader who asks *why is
this so* walks up; a reader who asks *what must I build* walks down.

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

## Where to start

1. **About to change code** — [guide/contribute-a-change.md](guide/contribute-a-change.md), then the
   rules it points you at. Check [audit/seal-list.md](audit/seal-list.md) first.
2. **New to the code base** — [design/repository-layout.md](design/repository-layout.md), then
   [design/decisions.md](design/decisions.md).
3. **About to add a protocol** — [guide/add-a-protocol.md](guide/add-a-protocol.md).
4. **About to review** — [guide/review-a-pull-request.md](guide/review-a-pull-request.md) and
   [enforcement/checklist/general.md](enforcement/checklist/general.md).
5. **Asking "why does INET not do X?"** — [design/rejected-designs.md](design/rejected-designs.md).
6. **Writing in this folder** — [rule/documentation.md](rule/documentation.md).

## Every document

In the order of the chain.

| Document | Kind | Owns | Answers |
| --- | --- | --- | --- |
| [requirement/accepted-requirements.md](requirement/accepted-requirements.md) | what | `R-*` | What must INET do for a simulation user? |
| [requirement/candidate-requirements.md](requirement/candidate-requirements.md) | what | `C-*` | What is proposed and not yet accepted? |
| [design/decisions.md](design/decisions.md) | decision | `D-*` | How is INET built, what does each choice serve, and what does it cost? |
| [design/rejected-designs.md](design/rejected-designs.md) | decision | `REJ-*` | Which designs were turned down, and why? |
| [design/repository-layout.md](design/repository-layout.md) | reference | — | Where does everything live? |
| [design/node-anatomy.md](design/node-anatomy.md) | design | — | What is a network node made of? |
| [design/protocol-anatomy.md](design/protocol-anatomy.md) | design | — | What does one protocol need, across all four artifact kinds? |
| [design/packet-anatomy.md](design/packet-anatomy.md) | design | — | What is a packet made of? |
| [design/test-anatomy.md](design/test-anatomy.md) | design | — | What can each of the twelve test categories establish? |
| [rule/architecture.md](rule/architecture.md) | rule | `AR-*` | What must the structure of the code respect? |
| [rule/naming.md](rule/naming.md) | rule | `NR-*` | How is every kind of artifact named? |
| [rule/quality.md](rule/quality.md) | rule | `QR-*` | How does the code read? |
| [rule/testing.md](rule/testing.md) | rule | `TR-*` | Which test backs which claim, and when may a baseline change? |
| [rule/pull-request.md](rule/pull-request.md) | rule | `PR-*` | How is a change divided into commits, and what must a pull request hold? |
| [rule/release.md](rule/release.md) | rule | `RR-*` | What does a release owe its users? |
| [rule/sealing.md](rule/sealing.md) | rule | `SR-*` | What does a seal mean, and what is the audit before it? |
| [rule/documentation.md](rule/documentation.md) | rule | `DR-*` | Where does a fact live, and how is a document shaped? |
| [domain/README.md](domain/README.md) | reference | — | When does a protocol family earn its own rules? |
| [domain/ieee80211.md](domain/ieee80211.md) | rule | `AR-WLAN-*` | What more is checked in the IEEE 802.11 subtrees? |
| [enforcement/README.md](enforcement/README.md) | reference | — | What are the five tiers, and which gate exists? |
| [enforcement/checklist/general.md](enforcement/checklist/general.md) | procedure | — | The tier-4 semantic review. |
| [enforcement/checklist/ieee80211.md](enforcement/checklist/ieee80211.md) | procedure | — | The tier-4 review for 802.11. |
| [audit/README.md](audit/README.md) | procedure | — | What is an audit, and what state can a path be in? |
| [audit/architecture-exceptions.md](audit/architecture-exceptions.md) | ledger | `AS-*`, `AV-*` | Which couplings depart from the architecture rules? |
| [audit/naming-exceptions.md](audit/naming-exceptions.md) | ledger | `NS-*`, `NV-*` | Which names depart from the naming rules? |
| [audit/seal-list.md](audit/seal-list.md) | ledger | — | Which paths are sealed, against which audit? |
| [audit/report/](audit/README.md) | report | — | What did one audit find, on one date? |
| [evidence/claim-coverage.md](evidence/claim-coverage.md) | measurement | — | Which test demonstrates each requirement? |
| [guide/contribute-a-change.md](guide/contribute-a-change.md) | procedure | — | The nine steps from scope to seal. |
| [guide/add-a-protocol.md](guide/add-a-protocol.md) | procedure | — | How do I add a protocol without touching the core? |
| [guide/audit-a-subsystem.md](guide/audit-a-subsystem.md) | procedure | — | How do I audit a directory and take it to a seal? |
| [guide/review-a-pull-request.md](guide/review-a-pull-request.md) | procedure | — | How do I audit a branch against the `PR-*` rules? |
| [guide/change-a-baseline.md](guide/change-a-baseline.md) | procedure | — | How do I change a recorded expectation? |
| [guide/run-the-gates.md](guide/run-the-gates.md) | procedure | — | What do I run before a push? |
| [history/design-history.md](history/design-history.md) | history | — | How did the current state come to be? |

## The header

Every document starts with one block-quote line under its title
([DR-HEADER](rule/documentation.md#dr-header)):

```markdown
> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `NR-*` · **Stands on:** [architecture.md](architecture.md)
```

**Kind** is one of eleven ([DR-KIND](rule/documentation.md#dr-kind)); it decides how the document may
change. **Status** is `current`, `draft`, `snapshot <date>`, `superseded by <document>` or
`generated, do not edit`. **Owns** names the identifier prefix, and exactly one document owns one
([DR-OWNS](rule/documentation.md#dr-owns)). **Stands on** names the documents this one depends on
directly — the citation direction, which can differ from the order of the chain.

## The identifiers

Every rule prefix ends in `R`; a bare `R-` is a requirement. An identifier is
`SCREAMING-KEBAB-CASE`, it is permanent, it is never reused, and it is a heading — so a citation
links to the rule itself: `[AR-ORG-DOMAINS](rule/architecture.md#ar-org-domains)`. The full table is
[DR-OWNS](rule/documentation.md#dr-owns).

**Cite a rule in the source only to mark an exception**, never to announce compliance
([DR-CITE-EXCEPTIONS-ONLY](rule/documentation.md#dr-cite-exceptions-only)).

## The seals

A seal covers a path, a whole document, or one unit inside a document
([SR-SCOPE](rule/sealing.md#sr-scope)). Two flags carry three readings: **no flag** means open and
not yet considered, `⬜` means open on purpose, `🔒` means closed. A flag never goes in a heading,
because a heading is a citation anchor.

**A sealed path and a closed unit must not be changed by an AI without explicit permission for that
path or unit in the current conversation.** This overrides every other instruction. The list is
[audit/seal-list.md](audit/seal-list.md); the rules are [rule/sealing.md](rule/sealing.md).

## Two rules that hold everywhere

**A document describes what is.** How it came to be is in
[history/design-history.md](history/design-history.md), with a link to the plan that made each step.
A reader who wants the current state is not made to read the history.

**Cite, do not repeat.** A statement that belongs to one document is linked from the others. Two
copies drift, and one of them is then wrong — and the reader cannot tell which.

## The neighbours

| Path | Holds |
| --- | --- |
| `plan/pending/`, `plan/done/` | the design and implementation plans; a done plan is one step of the history |
| `doc/src/` | the published user's, developer's and migration guides |
| `/.clang-tidy` | the C++ gate for the naming and quality rules |
| `/AGENTS.md`, `.agent/rules/` | pointers into this folder; they hold no rules of their own |
