# Documentation rules

> **Kind:** rule · **Status:** current · **Seal:** whole · **Owns:** `DR-*` · **Stands on:** [sealing.md](sealing.md), [naming.md](naming.md)

These rules define document ownership, structure, identifiers, and clear explanations. They govern `doc/project/` and its boundary with `doc/src/`. [DR-EXPLAIN](#dr-explain) also applies to contributor prose, including PR descriptions, commit messages, plans, and reports.

A rule has a stable identifier `DR-<AREA>`, and the identifier is the heading: [DR-HEADER](documentation.md#dr-header).

## Index

**Where a fact lives**

| Rule | Statement |
| --- | --- |
| [DR-TWO-ROOTS](#dr-two-roots) | Two documentation roots, one fact in one of them |
| [DR-CITE-DONT-REPEAT](#dr-cite-dont-repeat) | Each fact has one authoritative definition. |
| [DR-WHAT-IS](#dr-what-is) | A document describes what is; the history lives apart |
| [DR-NO-SECOND-COPY](#dr-no-second-copy) | An agent instruction file points here; it holds no rules of its own |

**The shape of a document**

| Rule | Statement |
| --- | --- |
| [DR-EXPLAIN](#dr-explain) | Contributor prose explains the change to a human reader. |
| [DR-HEADER](#dr-header) | Every document carries the header block |
| [DR-KIND](#dr-kind) | The kind is one of eleven, and it decides how the document may change |
| [DR-OWNS](#dr-owns) | Exactly one document owns an identifier prefix |
| [DR-INDEX](#dr-index) | A document with identifiers carries an index |
| [DR-TIER](#dr-tier) | A rule states the tier it is enforced at, on its last line |

**Identifiers and names**

| Rule | Statement |
| --- | --- |
| [DR-ID-FORM](#dr-id-form) | An identifier is `SCREAMING-KEBAB-CASE`, permanent, and never reused |
| [DR-ID-HEADING](#dr-id-heading) | An identifier heading holds the identifier alone |
| [DR-NAME](#dr-name) | The folder says the kind; the file says the subject |

**Links**

| Rule | Statement |
| --- | --- |
| [DR-LINK-RELATIVE](#dr-link-relative) | Every link is relative and resolves |
| [DR-CITE-EXCEPTIONS-ONLY](#dr-cite-exceptions-only) | Cite a rule in source only to mark an exception |

## Where a fact lives

### DR-TWO-ROOTS

**INET has two documentation roots, and one fact lives in exactly one of them.**

| Root | Audience | Answers | Format |
| --- | --- | --- | --- |
| `doc/src/` | the user of the product | How do I use INET? How do I write a model? | `.rst`, published |
| `doc/project/` | the contributor, the reviewer, the AI agent | What must INET do? How is it built? What is my change checked against? | `.md`, not published |

The product documents describe the current models and APIs. The project documents describe requirements, decisions, rules, and enforcement. Cite the product documentation for the full explanation of a mechanism. Include the brief local context permitted by [DR-CITE-DONT-REPEAT](#dr-cite-dont-repeat) when the argument needs it. For example, `doc/src/developers-guide/ch-packets.rst` explains use of the chunk API. [packet-anatomy.md](../design/packet-anatomy.md) defines the representation and its reasons.

*Enforced at T4 — agent review: does this text belong in the other root?*

### DR-CITE-DONT-REPEAT

**Each fact has one authoritative definition.**

Link to that definition from other documents. Independent copies can diverge, so the reader cannot tell which one to trust. A brief local explanation may repeat the context needed to understand an argument under [DR-EXPLAIN](#dr-explain). Cite the authoritative source beside that explanation. Preserve its conditions and limits. Keep full API descriptions and policy definitions in their authoritative documents.

For example, a plan may state that a component retains its timer object across stop/start. That fact explains why the proposed stop handler cancels the event and keeps the object. The plan links to the source for the complete lifecycle behavior.

A bare `§N` means a section of the current document. Cite another document by name and link.

*Enforced at T4 — agent review.*

### DR-WHAT-IS

**A document describes what is. How it came to be is in [history/](../history/design-history.md).**

No "changed in", no "previously", no record of an argument that was settled. A reader who wants the current state is not made to walk through the history, and a reader who wants the history finds it in one place with a link to the plan that made each step. This is [QR-CMT-NO-HISTORY](quality.md#qr-cmt-no-history) for documents.

The exceptions are the kinds whose *nature* is historical: a report is a dated snapshot, a ledger keeps its resolved rows, and the history document is the history.

*Enforced at T4 — agent review.*

### DR-NO-SECOND-COPY

**An agent instruction file — `AGENTS.md`, `CLAUDE.md`, `.agent/rules/` — points into `doc/project/`. It never holds a rule of its own.**

A second copy of the repository map is a copy that nobody updates, and an agent that reads it acts on last year's structure. These files say where to look and what to read first; the rules stay where they are owned.

Agent skills may supply tool invocation recipes, implementation discovery, filter semantics and diagnostic artifact handling. They discover project obligations through the active checkout's project map and cite their owners; they do not maintain independent copies of build-mode policy, test acceptance criteria, evidence requirements or baseline approval rules. Contributor decisions and procedures must remain usable without an installed skill. Published model and API mechanisms remain in `doc/src/` under [DR-TWO-ROOTS](#dr-two-roots).

*Enforced at T3 — a size and content check on the pointer files.*

## The shape of a document

### DR-EXPLAIN

**Contributor prose explains the change to a human reader.**

Use the same explanatory style as a direct answer to a developer who asks how the change works. Assume general knowledge of software engineering and communication protocols, including IEEE 802.11. Assume no knowledge of the specific implementation, change, or conversation. Explain specialized protocol conditions and local terms when the reader needs them.

State the problem or intended result first. Explain the cause when evidence establishes it. State uncertainty when the cause remains unknown. Connect the trigger, responsible component, action, and observable result. Introduce an API name through its role in that mechanism.

Use a concrete scenario to explain behavior, a cause, or a design choice. A failure scenario or reproduction already serves as that example when it states the initial condition, action, and result. Use one scenario for closely related points. Add another only when a distinct condition needs explanation. Label hypothetical examples. A simple fact or status needs no example.

Use a diagram when it clarifies an interaction or sequence.

For example, a hypothetical TXOP admission rule requires the complete exchange to fit within the remaining TXOP. Only 200 μs remain, but the exchange needs 240 μs. The MAC refuses the exchange before it transmits RTS. This preserves the proposed duration bound. The example describes a proposed model rule; it makes no claim about an IEEE requirement or current INET behavior.

Keep related sentences in connected paragraphs. Use lists for steps or parallel items. Use tables for comparisons. Preserve required document structures, trailers, and evidence fields. Fill templates with explanations that connect the facts. A list of files, API names, or checklist verdicts alone does not explain a change.

Write each prose paragraph on one source line. Put exactly one blank line between paragraphs. This rule applies to documents, pull request descriptions, and commit messages ([PR-MSG-WRAP](pull-request.md#pr-msg-wrap)). Use editor or viewer display options to wrap long lines without source line breaks. Preserve line breaks required by lists, tables, code blocks, quoted output, trailers, and other structured content. Sentence length limits do not require line breaks within a paragraph.

Keep relevant conditions, scope limits, and distinctions between requirements, model choices, proposals, and observed results explicit. Links support the explanation; the reader must understand the main argument without them. Check whether the document alone explains what happens, why it happens, and what the result means.

*Enforced at T4 — prose review of the document and its technical evidence.*

### DR-HEADER

**Every document starts with one block-quote line under its title.**

```markdown
> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `NR-*` · **Stands on:** [architecture.md](architecture.md)
```

**Status** is `current`, `current; <qualifier>`, `draft`, `snapshot <date>`, `superseded by <document>`, or `generated, do not edit`.

**Stands on** names the documents this one depends on directly, by their path from this file. It is the citation direction, which can differ from the order of the chain.

**Seal** declares the unit the document seals by; the values and their meaning are [SR-FLAG-COVERAGE](sealing.md#sr-flag-coverage).

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) reads the header and fails without it.*

### DR-KIND

**The kind is one of eleven, and it says how the document may change.**

| Kind | The document answers | How it changes |
| --- | --- | --- |
| what | What must INET do? | by an accepted requirement |
| decision | How is it built, and why not otherwise? | by a new decision |
| rule | What is a change checked against? | by a new decision |
| design | What is each part, in its settled form? | with the code |
| reference | Where is what, and how is it used? | with the tree |
| ledger | Which known deviations exist? | append only; identifiers are permanent |
| report | What did one audit find, on one date? | a re-audit rewrites it |
| risk | What can go wrong? | when a risk fires or a mitigation lands |
| measurement | What did we measure? | when we measure again |
| procedure | How do I do this task? | when the task changes |
| history | How did we get here? | append only |

The set is closed. A document that fits none of them is a sign that a kind is missing, which is a decision, not a free choice at writing time.

*Enforced at T3 — [check-seals.sh](../enforcement/check-seals.sh) rejects an unknown kind.*

### DR-OWNS

**Exactly one document owns an identifier prefix, and the `Owns` field names it.**

| Prefix | Means | Lives in |
| --- | --- | --- |
| `R-…` | a requirement INET promises the user | [accepted-requirements.md](../requirement/accepted-requirements.md) |
| `C-…` | a candidate requirement, not yet accepted | [candidate-requirements.md](../requirement/candidate-requirements.md) |
| `D-…` | a design decision, with what it costs | [decisions.md](../design/decisions.md) |
| `REJ-…` | a design considered and turned down | [rejected-designs.md](../design/rejected-designs.md) |
| `AR-…` | an architecture rule | [architecture.md](architecture.md) |
| `AR-<DOMAIN>-…` | a domain extension of the architecture rules | [domain/](../domain/README.md) |
| `NR-…` | a naming rule | [naming.md](naming.md) |
| `QR-…` | a code quality rule | [quality.md](quality.md) |
| `TR-…` | a test rule | [testing.md](testing.md) |
| `PLR-…` | an implementation plan rule | [planning.md](planning.md) |
| `PR-…` | a commit and pull request rule | [pull-request.md](pull-request.md) |
| `CR-…` | a change classification rule | [classification.md](classification.md) |
| `RR-…` | a release rule | [release.md](release.md) |
| `SR-…` | a sealing rule | [sealing.md](sealing.md) |
| `DR-…` | a documentation rule | this document |
| `AS-…` / `AV-…` | a sanctioned architecture exception / an open violation | [architecture-exceptions.md](../audit/architecture-exceptions.md) |
| `NS-…` / `NV-…` | a sanctioned naming exception / an open violation | [naming-exceptions.md](../audit/naming-exceptions.md) |
| `RISK-…` | a cost the framework accepts | reserved; no document yet |

Every rule prefix ends in `R`. A bare `R-` is a requirement.

*Enforced at T3 — a check that no prefix appears as a heading in two documents.*

### DR-INDEX

**A document that holds identifiers opens with an index of them.**

The index gives the overview that an identifier-only heading cannot: one row per identifier, its link, and its statement. It is what a reader scans when they do not yet know which rule they need. Generate it rather than typing it, so it cannot fall behind.

*Enforced at T3 — a check that every identifier appears in the index.*

### DR-TIER

**A rule closes with the tier it is enforced at, and the gate that does it.**

`*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*` on the last line of the rule. The tier lives with the rule and not in a central table, because a central table drifts. The gate inventory in [enforcement/README.md](../enforcement/README.md) lists the machinery from the other side, which is the side that can be measured.

*Enforced at T3 — a check that every rule heading is followed by an enforcement line.*

## Identifiers and names

### DR-ID-FORM

**An identifier is `SCREAMING-KEBAB-CASE`, it is permanent, and it is never reused.**

Retiring a rule retires its identifier; a deleted one is never reassigned to something else, because a citation somewhere still means the old thing. The identifier names the rule and not its position, so a regroup moves nothing else and no citation changes.

*Enforced at T3 — a check for a reused identifier across the git history.*

### DR-ID-HEADING

**An identifier heading holds the identifier alone. The statement is the bold lead sentence of the body.**

```markdown
### NR-NED-GATE

**A gate is `<stem>In` or `<stem>Out`, and the stem names the peer it faces.**
```

The anchor is then the lowercased identifier, so a citation is short, writable by hand, and survives a rewording: `[NR-NED-GATE](naming.md#nr-ned-gate)`. A heading that also carried the statement would anchor as `#nr-ned-gate--a-gate-is-stemin-or-stemout`, which nobody writes and which breaks the moment the wording improves. It also leaves the first body line free for the seal flag ([SR-FLAG-PLACEMENT](sealing.md#sr-flag-placement)).

*Enforced at T3 — a check on every heading that starts with an identifier.*

### DR-NAME

**The folder says the kind; the file says the subject.**

A folder is singular and lowercase: `rule/`, `design/`, `audit/`, `guide/`. A file is lowercase with hyphens and says its subject: `rule/naming.md`, `design/packet-anatomy.md`. A basename is unique across the whole tree, because a citation shows only the basename in its link text — `[naming.md]` must be unambiguous.

| Kind | The name is |
| --- | --- |
| a guide | the task, verb first: `audit-a-subsystem.md` |
| a subsystem report | the path, slashes to hyphens, `src/inet/` stripped: `common-packet.md` |
| a pull request report | `pr-<number>.md` |
| a sweep report | the rule family: `naming.md` |
| a document report | the document path, slashes to hyphens: `rule-naming.md` |
| a plan | the subject, in `plan/pending/`, moving to `plan/done/` when it lands |

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

## Links

### DR-LINK-RELATIVE

**Every link inside this tree is relative, and every link resolves.**

Relative, so the tree can move as a whole and so a link works in a checkout, on the web and in an editor. Resolving, including the `#anchor`: a broken anchor is worse than a missing link, because it silently lands the reader at the top of the right document and they believe they are in the right place.

*Enforced at T3 — a link checker over `doc/project/`.*

### DR-CITE-EXCEPTIONS-ONLY

**Cite a rule in the source only to mark an exception, or a constraint a reader would otherwise "correct".**

Never to announce compliance. `// AR-COM-DIRECT: deliberately a message, this crosses the medium` earns its line, because the next reader would otherwise replace it with a call. A comment that says the code follows a rule adds nothing and goes stale when the rule moves.

*Enforced at T4 — agent review.*
