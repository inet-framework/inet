# Pull Requests and Commits

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `PR-*` · **Stands on:** [architecture.md](architecture.md), [testing.md](testing.md)
How to divide a change into commits, how to write the commit messages, and what a pull
request must contain. The rules exist for the *reader* of the change: the reviewer who must
judge it now, the developer who bisects a regression two years later, and the developer who
reads `git log` to learn why the code looks as it does. A change that is correct but badly
divided costs all three of them time, and it hides real defects.

Each rule has a stable identifier of the form `PR-<AREA>-<NAME>`, a one-line statement, and a
short rationale. The other documents in this folder say what the *code* must look like —
[architectural-requirements.md](architecture.md),
[naming-conventions.md](naming.md), [sealing.md](sealing.md). This one says what
the *change* must look like, and [classification.md](classification.md) says what the change *is*.
It is the concrete form of step 4 of the *Contributor workflow* (smallest change surface) and of the reviewable-patch clauses of
[AR-QUAL-FINGERPRINT](architecture.md) and
[AR-QUAL-TRACEABILITY](architecture.md).

**The commit is the unit of review, not the pull request.** A reviewer reads a series of
commits, one at a time, and asks one question per commit: *is this change right?* That
question has an answer only if the commit contains one change. When a commit contains two
changes, the reviewer must first separate them mentally, and the reviewer does that work
again for every later reader of the history. Divide the work in advance, because you are the
only person who knows where the boundaries are.

## The reviewer's questions

A reviewer meets every change with the same four questions. The opening of each commit body and of
each pull request description answers them, before any detail:

1. **Which problem does the change solve?** What is wrong or missing today, and for whom.
2. **Is it a real problem, and do we want to solve it?** How we know: a symptom with a way to
   reproduce it, a failing test, a standard clause, a requirement, a user report, a measurement.
   What it costs to leave the problem, and why the change is worth its own cost. For a commit that
   prepares a later one: which later commit needs it, and for what.
3. **What changes?** What a user or a developer sees after the change: behavior, contracts,
   configuration, recorded results.
4. **How does the change reach its goal?** The approach in a sentence or two, and why this approach
   and not the obvious alternative.

**The answers scale with the change.** A typo fix answers all four in its subject. A small fix
answers them in a few sentences. A feature answers them in the first paragraphs of each commit and
of the pull request. Headings such as "Why", "What" and "How" can help a long text; a short text
does not need them. What counts is that the answers are there, that they come first, and that a
reviewer without the author's context understands them.

**Length is not an answer.** Ten pages that never state the problem leave the reviewer with the
questions. Three sentences that answer them are enough. A link to a plan or an issue supports the
answers but does not replace them ([PR-MSG-STANDALONE](#pr-msg-standalone)).

## Index

Every rule in document order. The identifier links to the rule; the statement is its lead sentence.

**Commit content (PR-SPLIT)**

| Rule | Statement |
| --- | --- |
| [PR-SPLIT-ONE-CHANGE](#pr-split-one-change) | One commit makes exactly one change |
| [PR-SPLIT-SIZE](#pr-split-size) | A commit above 400 changed source lines says why it does not divide |
| [PR-SPLIT-WHITESPACE](#pr-split-whitespace) | A whitespace change touches only whitespace |
| [PR-SPLIT-MECHANICAL](#pr-split-mechanical) | A mechanical sweep is separate from work that needs thought |
| [PR-SPLIT-MOVE](#pr-split-move) | A file move is its own commit |
| [PR-SPLIT-PREPARE](#pr-split-prepare) | Preparation comes before the change that needs it |
| [PR-SPLIT-UPSTREAM](#pr-split-upstream) | A shared-component change is separate from, and before, the model that needs it |
| [PR-SPLIT-BASELINE](#pr-split-baseline) | A regenerated baseline travels with the change that causes it |
| [PR-SPLIT-DRIVEBY](#pr-split-driveby) | No unrelated fixes |

**Commit series (PR-SERIES)**

| Rule | Statement |
| --- | --- |
| [PR-SERIES-BUILDS](#pr-series-builds) | Every commit builds and passes its tests |
| [PR-SERIES-ORDER](#pr-series-order) | Prerequisites first, no fixup commits |
| [PR-SERIES-LINEAR](#pr-series-linear) | Rebase the topic branch; do not merge into it |

**Commit messages (PR-MSG)**

| Rule | Statement |
| --- | --- |
| [PR-MSG-SUBJECT](#pr-msg-subject) | `area: what the commit does` |
| [PR-MSG-BODY](#pr-msg-body) | A commit whose subject cannot carry its reason has a body |
| [PR-MSG-SUMMARY](#pr-msg-summary) | The body starts with a summary that a reviewer understands in two minutes |
| [PR-MSG-WHY](#pr-msg-why) | The body gives the reason, not the content |
| [PR-MSG-REPRODUCE](#pr-msg-reproduce) | A fix says how to reproduce the defect |
| [PR-MSG-PLAN](#pr-msg-plan) | A commit that implements a plan names it |
| [PR-MSG-GENERIC](#pr-msg-generic) | A shared-component commit explains itself in generic terms |
| [PR-MSG-STANDALONE](#pr-msg-standalone) | The message carries its own context |
| [PR-MSG-FACTS](#pr-msg-facts) | The message contains only facts about the change |

**The pull request (PR-REQ)**

| Rule | Statement |
| --- | --- |
| [PR-REQ-TOPIC](#pr-req-topic) | One pull request, one topic |
| [PR-REQ-STORY](#pr-req-story) | The description starts with a summary, then gives the commits and the evidence |
| [PR-REQ-ARCH](#pr-req-arch) | The description names the architectural surface |
| [PR-REQ-CLEAN](#pr-req-clean) | No leftovers |

## Commit content (PR-SPLIT)

### PR-SPLIT-ONE-CHANGE

**One commit makes exactly one change**

A commit contains one self-contained change, and the whole of that change. Half a change is not
a commit: the tree after the commit must build, and the model after the commit must be consistent
(PR-SERIES-BUILDS). "One change" means one *decision*, not one file — a decision that touches eight
files is still one commit.

**Divide a commit wherever a part of it can stand alone.** A part stands alone when it builds,
passes its tests, and has a reason of its own that its own message can give. That part is a commit
of its own, even when the larger feature needs it. One change is the smallest step that a reviewer
can judge alone, not the largest feature that the steps serve: a feature that five steps build is
five commits.

The subject line is a quick test, but a weak one. If you cannot say what the commit does in one
line without "and" or a list, the commit holds more than one change. But an abstract subject such
as "enforce the TXOP limit" passes this test and can still cover several changes. These signs show
a second change:

| Sign | What it usually shows |
| --- | --- |
| the moved baseline rows need more than one explanation | one behavior change for each explanation |
| a new test shows a symptom that the rest of the commit does not need | a fix or a feature that can land first |
| the commit adds a mechanism and also turns on its first production user | two steps ([PR-SPLIT-PREPARE](#pr-split-prepare)) |
| the subject carries a mixed marker such as `add+change:` | two changes, unless the parts cannot be divided ([CR-TAG-SUBJECT](classification.md#cr-tag-subject)) |
| the plan of the work lists the content as more than one step | one commit for each step |
| the body needs a paragraph for each of several topics | one topic for each commit |

A sign is a question, not a verdict. Some changes do not divide: a contract and the update of every
implementation of it must build together, for example. The body of such a commit says why it does
not divide.

*Enforced at T4 — agent review: can a part of the commit stand alone? T3 gives a note:
[check-commits.sh](../enforcement/check-commits.sh) names a commit whose fingerprint rows move in
more than one way.*

### PR-SPLIT-SIZE

**A commit above 400 changed source lines says why it does not divide**

Count the changed lines of `.cc`, `.h`, `.ned` and `.msg` files under `src/`, without the generated
`_m.h` and `_m.cc` files. Tests, recorded expectations and documentation do not count. Above 400
lines, the body of the commit says why the commit cannot be divided
([PR-SPLIT-ONE-CHANGE](#pr-split-one-change)).

The number is measured, not chosen. Across master's last 1000 commits, 95 % of the commits that
change source change at most 425 source lines, and the median is 34. Most commits above the line add
a new protocol, import reference code, or do a sweep. A reviewer can hold a commit below the line in
the head at once. Above it, the reviewer reads the commit in pieces and must find the boundaries that
the author did not draw.

The limit asks a question; it does not forbid a large commit. A new protocol module, imported
reference code, a mechanical sweep ([PR-SPLIT-MECHANICAL](#pr-split-mechanical)) and a move
([PR-SPLIT-MOVE](#pr-split-move)) can be larger, and one sentence in the body says so. A large
commit that builds one feature in several steps is the case that this rule is for: divide it.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh) gives a note above 400 changed
source lines, except on a `comment`, `format`, `location` or `name` commit; T4 — agent review: is
the reason in the body true?*

### PR-SPLIT-WHITESPACE

**A whitespace change touches only whitespace**

A commit that changes whitespace changes nothing else. It may change whitespace in many
files, but it must not contain one line of functional change.

Whitespace that travels with a real change is expensive. Every touched file appears in the
file list of the pull request, and files of a central component — a base class, a contract
header, a shared utility — then look as if their behavior changed. The reviewer opens them,
reads the diff, and finds an indentation fix. That attention is lost, and the real change
hides in the noise. A whitespace-only commit costs the reviewer one step: `git show -w` is
empty, so the whole commit is verified at once. A mixed commit gives that step to nobody, and
it points `git blame` at the wrong commit forever.

The rule also applies in the small: do not tidy the lines around your edit. Put the tidy-up in
its own commit. Put that commit *before* the functional commits, so the functional diffs apply
to the final layout.

### PR-SPLIT-MECHANICAL

**A mechanical sweep is separate from work that needs thought**

A large mechanical edit — a rename across the tree, a signature sweep, a re-generation of
generated code, a header or copyright update, an automatic reformat — is its own commit and
changes nothing else.

A mechanical commit is checked differently from a normal one. The reviewer does not read 400
files; the reviewer reads the *rule* in the commit message ("every caller of `X` gets the new
argument"), spot-checks some sites, and trusts the tests. State that rule in the message. When
three hunks of new logic hide among 400 mechanical hunks, the reviewer must read everything to
find the three, and normally does not.

### PR-SPLIT-MOVE

**A file move is its own commit**

Move or rename files in a commit that does not change their content. Change the content in the
next commit.

Git detects a rename from content similarity. A move plus an edit in one commit shows as a
deleted file and a new file: the diff disappears, and the history of the file breaks at that
point. Two commits keep the rename visible and keep the real edit small. This holds for `.ned`,
`.msg` and C++ files, and for whole directories.

**Rename the type in its own commit too, after the move.** A file whose name carries a type name
takes three commits, not two: move the files, rename the type everywhere, then change the content.
The move commit will not compile, and [PR-SERIES-BUILDS](#pr-series-builds) exempts it — the loss
from a rename git cannot see is permanent, and the loss from a broken middle commit is one
`git bisect skip`.

### PR-SPLIT-PREPARE

**Preparation comes before the change that needs it**

When a change needs preparation, commit the preparation first, in one or more commits of its own.
Preparation has three forms:

- **A refactor.** Keep it behavior-preserving. The fingerprint tests then answer the question *is
  the refactor safe?*, and the change that follows is a diff of a few lines.
- **A prerequisite fix.** A defect that the change exposes, or that the change needs repaired, is a
  fix of its own. It has its own symptom, its own reproduction
  ([PR-MSG-REPRODUCE](#pr-msg-reproduce)) and often its own baselines. Commit it before the change.
- **A new mechanism before its first user.** A contract, a data structure or a procedure that the
  change needs can land before the commit that uses it, with tests that reach it directly. No
  behavior moves, so every recorded expectation stays where it is, and the reviewer judges the
  mechanism alone. Say in the body which later commit of the series uses it; a mechanism whose user
  is not in the pull request is speculative ([AR-EXT-REUSE](architecture.md#ar-ext-reuse)).

A last, small commit then turns the behavior on. It carries the baselines that move
([PR-SPLIT-BASELINE](#pr-split-baseline)), and its diff shows exactly where the behavior changes.

The reviewer then answers several simple questions instead of one hard one. In a mixed commit no
question has a safe answer, because every changed line is a candidate cause of the behavior change.

*Enforced at T4 — agent review: does a refactor commit change behavior, and does one commit both
add a mechanism and turn on its first user?*

### PR-SPLIT-UPSTREAM

**A shared-component change is separate from, and before, the model that needs it**

When a fix in one protocol model needs a new capability in a shared component, divide the work
into two commits. The first commit adds the capability to the shared component, and explains
the *generic* need. The second commit changes the protocol model that uses it.

Example: a bug in the IEEE 802.11 model needs a new feature in the queueing model. These are
two changes with two audiences. The queueing commit belongs to the queueing contracts and must
stand on its own: it must be correct for *every* user of the queue, and its message must
describe the new capability in general terms. The 802.11 commit is then small, and it shows
exactly how the new capability repairs the bug.

The division is also a design check. If you cannot describe the first commit without the words
"802.11", the feature is in the wrong place, or it is too narrow — a shared component must not
gain a feature that makes sense for one protocol only (AR-ORG-DOMAINS, AR-ORG-CONTRACTS). The
order matters for the same reason: the shared commit must build and be correct alone, and a
later revert of the protocol fix must leave the framework in a working state.

### PR-SPLIT-BASELINE

**A regenerated baseline travels with the change that causes it**

When one source change moves a recorded expectation — a fingerprint (`tests/fingerprint/*.csv`), a
statistical baseline, an expected output — the regenerated values go in the same commit as that
source change. The message of that commit says which behavior moved and why the new values are
correct.

A baseline update is a claim, not a side effect: *these values change on purpose, and the new values
are correct*. The claim lives in the message. It does not live in the commit boundary. One commit
also makes the attribution exact, which is what [AR-QUAL-TRACEABILITY](architecture.md) asks for: the
change that justifies the new values is the change that carries them.

The commit is the unit that must be true on its own. A source commit without its baselines fails its
own fingerprint test, and that breaks [PR-SERIES-BUILDS](#pr-series-builds): `git bisect` over the
suite then names the behavior commit as the first bad commit, and the per-commit CI build goes red in
the middle of the series. The two halves also separate under every operation that moves a commit. A
revert of the source commit leaves the new values in the tree. A cherry-pick to a maintenance branch
takes the code and forgets the values. They are one decision, so they are one commit, and `git blame`
on a moved value lands on the commit that explains it.

The cost of this rule is the size of the diff. A large re-record buries the source change under
thousands of lines, so read the source alone:

```bash
git show <sha> -- . ':!tests'
```

What the earlier form of this rule protected — that a baseline never moves unnoticed
([TR-BASELINE-DELIBERATE](testing.md#tr-baseline-deliberate)) — now rests on the message and on
review, not on the commit boundary.

**A baseline update stands alone only when no single source commit causes it.** A re-record after a
compiler, tool or solver version change; drift that came from outside the branch; a mass re-record
over unrelated configurations. No commit beside it can carry the reason, so the message carries the
whole reason ([TR-BASELINE-PROVENANCE](testing.md#tr-baseline-provenance)). A baseline-only commit
that stands directly after the source commit that moves the values is the divided form of one change:
squash the two.

A value that is recorded for the first time is not a baseline update. A new fingerprint row for a new
example is the test of that example, and it belongs to the commit that adds the example.

### PR-SPLIT-DRIVEBY

**No unrelated fixes**

Do not add a fix that you found on the way to a commit that does something else.

An unrelated fix inside another commit shares that commit's fate: a revert of the main change
removes the fix too, and a cherry-pick of the fix pulls in the main change. It also widens the
review to a topic the reviewer did not prepare for. A small obvious fix may stay in the same
pull request as a separate commit. A fix that needs its own discussion needs its own pull
request.

## Commit series (PR-SERIES)

### PR-SERIES-BUILDS

**Every commit builds and passes its tests**

Every commit in the series compiles and passes the test categories that apply to it, not only
the last commit.

`git bisect` is the fastest tool for a regression that nobody can explain, and one broken
middle commit makes it useless. The rule also protects review itself: a reviewer can judge
commit N only if the tree after commit N is consistent.

**One exemption: a pure move or rename commit may fail to build.**
[PR-SPLIT-MOVE](#pr-split-move) asks for a commit that moves files and changes no content, and
when the moved file declares the type being renamed, no such commit can compile — renaming
`TcpBaseAlg.h` while it still declares `class TcpBaseAlg` breaks every file that includes it.
The move is worth more than the build here, because a rename git cannot see costs the file its
history permanently, and a broken build costs one `git bisect skip`.

The exemption is narrow. It covers a commit whose diff is **only** moves and renames, it does not
extend to the content commit that follows, and the run of broken commits is as short as the
change allows — a move and then its rename, not a move and then twenty commits.

### PR-SERIES-ORDER

**Prerequisites first, no fixup commits**

Order the commits so that each one depends only on the commits before it. A commit that
corrects an earlier commit of the same series must not survive to review: rebase the correction
into the commit it repairs (`git commit --fixup` and `git rebase --autosquash`).

The series is the author's final reasoning, not a record of how the author got there. "Fix typo
in previous commit" teaches nobody anything, and it breaks PR-SERIES-BUILDS in the middle of
the series.

### PR-SERIES-LINEAR

**Rebase the topic branch; do not merge into it**

Keep the series linear on top of the target branch. Do not merge the target branch into your
topic branch to resolve a conflict — rebase the series instead.

A merge commit inside the series mixes other developers' changes into the diff of the pull
request, and a bisect crosses into unrelated history. How the finished branch enters the target
branch afterwards is the maintainer's decision; the branch you submit stays linear.

## Commit messages (PR-MSG)

### PR-MSG-SUBJECT

**`area: what the commit does`**

One line: the component or tree area, a colon, then what the commit does, in the present tense. End
it without a full stop. Then one empty line, then the body.

**Aim for 72 characters. A gate fails above 80.** The 72 is where `git log --oneline` still fits an
80-column terminal, once the abbreviated hash and its space are counted. The 80 is where the subject
stops fitting on its own. Between the two is a matter of taste and nothing is gained by arguing it:
the length limit is a proxy for [PR-SPLIT-ONE-CHANGE](#pr-split-one-change) — a subject that will not
fit usually describes two changes — and at 73 characters that proxy tells you nothing. A gate that
reports a one-character overrun beside a real defect teaches the reader to skim both.

The area is the NED or C++ component (`ExternalProcess:`, `Ipv4:`, `visualizer:`) or the part of
the tree (`tests:`, `doc:`, `build:`, `examples/mpls/net37:`). An optional kind word may follow
the area (`ospfv3: fix:`, `python: refactor:`). Name the *behavior*, never the mechanics: write
`ExternalProcess: don't kill the process group when a spawned command fails`, not "update
ExternalProcess.cc" and not a list of file names or links.

**Both the area and the kind word are optional, and both are free within a bound.**
[CR-TAG-SUBJECT](classification.md#cr-tag-subject) gives the grammar: the author may write any
consecutive segments of the trailer's scope and any consecutive segments of its depth, direction
and intent, so `EthernetMac: fix:`, `linklayer: refactor:` and `showcases.tsn: format:` are all
correct. The one thing a subject must not do is disagree with the trailer. The obligations never
appear in a subject.

### PR-MSG-BODY

**A commit whose subject cannot carry its reason has a body**

A subject says what the commit does. Where that is the whole story, the commit is finished. Where it
is not, the body carries the rest, and [PR-MSG-WHY](#pr-msg-why) says what the rest is.

**A body is required when the change is substantial, and whenever it repairs a defect, changes
behavior, or implements a standard** — the last three at any size, because each has a reason that no
subject has room for. A one-line fix for a null dereference still owes the reader the crash.

**A body is not required when the subject is the whole story.** A rename, an include ordering, a
whitespace commit, a mechanical sweep whose rule fits the subject. Nor when the change *is* its own
explanation: a plan or documentation commit, a regenerated file, a `WHATSNEW` entry. In each of
those the reader's next step is to read the file, not the message.

**Where the line falls.** The gate fails an empty body above **50 changed lines**, and that number
is measured rather than chosen: across master's last 300 commits the share with no body is flat at
3 to 4 % for every threshold from 50 upward, so 50 is where the project already draws the line
itself. Of the nine commits above it that carry no body, six are the exempt kinds above.

**On what a body is for**, since it is the usual question:

| | Where it belongs |
| --- | --- |
| **why** it was done | the summary gives the problem, and how we know that it is real and worth solving ([PR-MSG-SUMMARY](#pr-msg-summary)). The rest of the body adds the cause, the alternative rejected, and what the change deliberately leaves unrepaired. Nothing else carries the reason. |
| **what** it changes | the subject names it, and the summary says what a user or a developer sees after the change, at the level of components and contracts. The diff shows the lines; the body does not restate them. |
| **how** it reaches the goal | the summary gives the approach in a sentence or two. The body says *which* mechanism, and *why that one and not the obvious alternative*. The diff shows the code. |

A body that restates the subject in longer words is worse than no body, because it costs a reader
the time to discover that it says nothing.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh) fails an empty body above 50
changed lines, outside the exempt kinds; T4 for whether the body gives a reason at all.*

### PR-MSG-SUMMARY

**The body starts with a summary that a reviewer understands in two minutes**

Start the body with a summary of one to three short paragraphs. The summary answers the
[reviewer's questions](#the-reviewers-questions) for this commit: the problem, how we know that it
is real and worth solving, what changes, and how. A reviewer who reads only the summary knows what
the commit does and why, without the diff.

Details come after the summary: the mechanism and why it was chosen, edge cases, standard clauses,
migration notes, the account of moved baselines. A reader who needs them reads on. A reader who
scans `git log` stops after the summary.

The summary is not a list of actions. "Add X. Change Y. Remove Z." repeats the diff and leaves the
reader to find the reason. Write the problem first, then the idea of the solution.

**Keep the body short.** The first paragraph aims for about 100 words, and the whole body for about
300. Both numbers are measured: across master's last 1000 commits, 99 % of first paragraphs have at
most 121 words, and 95 % of bodies have at most 291. A body that needs much more usually describes
more than one change ([PR-SPLIT-ONE-CHANGE](#pr-split-one-change)), or carries evidence that belongs
in the pull request ([PR-MSG-FACTS](#pr-msg-facts)).

A commit whose subject is the whole story needs no body ([PR-MSG-BODY](#pr-msg-body)). In a body of
one short paragraph, that paragraph is the summary.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh) gives a note when the first
paragraph has more than 120 words or the body more than 300, and when the body opens with an action
and names no problem; T4 — agent review: can a reviewer restate the answers to the reviewer's
questions from the summary alone?*

### PR-MSG-WHY

**The body gives the reason, not the content**

The diff already shows what changed, line by line. The body says why: the symptom, the cause, why this
solution and not an obvious alternative, and what the change deliberately does not repair.

For a bug fix, write the symptom in the words a future reader will search for — the error
message, the wrong packet, the failed assertion. For a behavior change, name the standard
clause or the reference that makes the new behavior the correct one. For a new feature, say who
needs it and how we know: a requirement, a standard clause, a user report, or the later commit that
uses it. A commit that prepares a later one names that commit and what it needs from this one;
"needed later" alone is not a reason.

### PR-MSG-REPRODUCE

**A fix says how to reproduce the defect**

A commit that repairs a defect carries, in its body, the way to see the defect happen. Two forms
are acceptable and the choice is the author's:

1. **Steps.** The configuration, the scenario, and what goes wrong — *"run
   `examples/inet/tcpwindowscale` with `sackSupport=true`; the sender stalls at t=2.4 s because
   `nextSeg` rule (2) underflows"*. A few lines, and it is enough for most defects.
2. **A standalone regression test**, committed beside the repair.

**The test is justified only when the defect is serious enough and likely to bite again.** Most
are not. A defect earns a regression test when it sits on a path many things cross, when a future
refactor could reintroduce it without noticing, or when it came from a misreading of a standard
that the next reader could repeat. A one-off typo in a log message earns steps and nothing more.

**Why the steps are required even when the test is not.** A fix is not new behavior, so
[TR-SHIP-WITH](testing.md#tr-ship-with) does not reach it, and nothing else in the rule set asks a
fix for evidence. Without the steps a reviewer must take the defect on trust, and a later reader
who suspects a regression in the same area has no way to tell whether it is the old defect
returning.

Write the symptom in the words a future reader will search for: the error message, the wrong
packet, the failed assertion. That is [PR-MSG-WHY](#pr-msg-why) applied to a fix, and this rule
says the searchable symptom is not optional.

A fix is exactly the commit whose trailer carries `.fix` under
[CR-DEPTH-FIX](classification.md#cr-depth-fix), so the two rules select the same commits from
opposite directions: one asks the author to declare the intent, the other asks for the evidence.

*Enforced at T4 — agent review; T3 can check that a `.fix` commit has a body, not what is in it.*

### PR-MSG-PLAN

**A commit that implements a plan names it**

Where the work follows a plan under `plan/pending/` or `plan/done/`, the message gives the plan's
repository-relative path on a `Plan:` line, above the `Change:` trailer:

```
Plan: plan/pending/pr-1155-resolve-audit-findings.md
Change: src.tcp.Rfc6675Recovery | behavior.change.fix | fingerprint | pr-1155-findings
```

A plan holds what no commit body has room for: the alternatives that were weighed, the order the
steps must run in, the decisions a person made and why. A commit that implements step 5 of
something is unreadable without step 1 to step 4, and the plan is where they are.

**This does not weaken [PR-MSG-STANDALONE](#pr-msg-standalone).** That rule forbids a message that
*replaces* its reason with a pointer. A plan reference is an addition: the body still gives the
reason for this commit, and the plan gives the reason for the shape of the series. The difference
from a ticket is that the plan is in the repository — it is fetched with the history, it survives
the tracker, and `git log` and the file move together.

`Plan:` comes before `Change:`, because
[CR-TAG-TRAILER](classification.md#cr-tag-trailer) puts the classification last.

*Enforced at T3 — a check that the path a `Plan:` line names exists in the tree at that commit.*

### PR-MSG-GENERIC

**A shared-component commit explains itself in generic terms**

The message of a PR-SPLIT-UPSTREAM commit describes the new capability and the general need for
it. It may name the model that needs it first, but a reader must understand the commit without
that model.

### PR-MSG-STANDALONE

**The message carries its own context**

Do not write "as discussed", "review comments", "address feedback", or "see the ticket". Write
the fact itself. An issue or pull request number is a useful addition, never a replacement.

### PR-MSG-FACTS

**The message contains only facts about the change**

No attribution trailers for tools or assistants, no progress notes, no apologies, and no
speculation about future work. Keep a `Fixes #<n>` style reference when it is accurate.

**The message describes the final change, not the way to it.** A pull request goes through
revisions, and the commits that land show only the result ([PR-SERIES-ORDER](#pr-series-order)). A
section such as "audit correction", "after review" or "the earlier revision" records the history of
the pull request, and it means nothing to a reader of `git log`. Write the message for the final
change.

**Test logs belong in the pull request.** The commands that ran, their results, the run counts and
the seeds go in the description ([PR-REQ-STORY](#pr-req-story)). They describe one run on one
machine, and they are often the same for every commit of a series. A message names a test only when
the test is part of the reason: the regression test of a fix
([PR-MSG-REPRODUCE](#pr-msg-reproduce)), or the case that shows that a moved baseline is right.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh) fails an attribution trailer
and gives a note for words of revision history and for a `Validation:` paragraph; T4 — agent review
for other progress notes.*

### The classification trailer

Every commit also ends with one `Change:` line that states its scope, its depth, what must move
with it, and — where the commit is one of several in a larger change — the label of that group.
See [classification.md](classification.md), which owns `CR-*`. The trailer is not part of
the message rules above: [PR-MSG-WHY](#pr-msg-why) governs the *reason*, and the trailer governs
the *classification*. Neither one says what the other says.

## The pull request (PR-REQ)

### PR-REQ-TOPIC

**One pull request, one topic**

A pull request carries one topic, at a size a reviewer can hold in the head at once. Two topics
are two pull requests, even when the same developer wrote them on the same day. A long series
on one topic is fine; a short series on three topics is not.

**A step that is useful alone and moves behavior outside the topic goes first, in a pull request of
its own.** A fix that moves the fingerprints of configurations that the topic does not reach is the
usual case. The reviewers of that behavior are not always the reviewers of the topic, and the fix
can land while the topic is still in review. The topic pull request then builds on it. When a pull
request keeps such a step, the description says why.

*Enforced at T4 — agent review: does a commit move behavior outside the topic, and can it land
alone? T5 — the size of the topic is human judgment.*

### PR-REQ-STORY

**The description starts with a summary, then gives the commits and the evidence**

A description has three parts, in this order:

1. **A summary that answers the [reviewer's questions](#the-reviewers-questions)** for the whole
   pull request: the problem, how we know that it is real and worth solving, what changes, and how.
   Then the risk: what can break, and for whom. A few short paragraphs: a reviewer reads them in
   about five minutes and then knows what to expect from the commits. The summary describes the
   change at the level of components and contracts; it does not repeat the commit messages.
2. **The commits, in the order to read them**, one line each. Say which commits only prepare and
   which commit moves behavior ([PR-SPLIT-PREPARE](#pr-split-prepare)). The architectural surface
   ([PR-REQ-ARCH](#pr-req-arch)) follows the commits.
3. **The evidence.** The tests that ran, with the exact commands and the resulting status, every
   baseline update (*Contributor workflow*, step 6), and what remains unverified.

A reviewer who stops after the summary must still be able to answer the reviewer's questions. Long
material — a table for each test, a log excerpt, the complete account of moved baselines — goes to
the end of the description, or into the plan.

**The description scales with the pull request.** A pull request of one small commit can use that
commit's message as its whole description. A series needs a summary of its own, because no single
commit answers the questions for the whole series. Headings are optional.

*Enforced at T4 — agent review: can a reviewer restate the answers to the reviewer's questions, and
the risk, from the summary alone, in about five minutes?*

### PR-REQ-ARCH

**The description names the architectural surface**

Name the contracts, the packet content, the configuration surface, and the feature descriptors
that the change touches. Record genuinely new deviations as `AV-*` or `NV-*` rows in
[architecture-exceptions.md](../audit/architecture-exceptions.md) or
[naming-exceptions.md](../audit/naming-exceptions.md), and name them here. If the change touches a
sealed path, state the permission for it (see [sealing.md](sealing.md)).

### PR-REQ-CLEAN

**No leftovers**

The branch contains no debug output, no commented-out code, no `#if 0` block, no build output
or IDE files, and no TODO without an issue number. A change that is not ready for this stays a
draft.

## Quick reference

| Situation | Do not | Do |
|---|---|---|
| You reformat a file you also fix | one commit | whitespace commit first, fix after (PR-SPLIT-WHITESPACE) |
| You rename a class in 200 files and add a feature | one commit | mechanical commit, then feature commit (PR-SPLIT-MECHANICAL) |
| You move a file and edit it | one commit | move commit, then edit commit (PR-SPLIT-MOVE) |
| A refactor makes the fix possible | one commit | behavior-preserving refactor, then fix (PR-SPLIT-PREPARE) |
| A feature needs a defect repaired first | fold the fix into the feature | the fix with its own baselines, then the feature (PR-SPLIT-PREPARE) |
| A feature needs a new contract | the contract and its first user in one commit | the contract with its tests, then a small commit that turns the behavior on (PR-SPLIT-PREPARE) |
| An 802.11 fix needs a queueing feature | one commit | generic queueing feature, then 802.11 fix (PR-SPLIT-UPSTREAM) |
| Your fix changes fingerprints | a baseline commit after the fix | the source and the `.csv` in one commit, with the reason in the message (PR-SPLIT-BASELINE) |
| A compiler update moves fingerprints | fold them into the next fix | a baseline-only commit that names the cause (PR-SPLIT-BASELINE) |
| You see an unrelated bug on the way | fold it in | separate commit, or separate pull request (PR-SPLIT-DRIVEBY) |
| A reviewer finds a defect in commit 2 of 5 | add commit 6 | rebase the correction into commit 2 (PR-SERIES-ORDER) |
| The target branch moved under you | merge it in | rebase the series (PR-SERIES-LINEAR) |
| The subject needs an "and" | write the "and" | divide the commit (PR-SPLIT-ONE-CHANGE) |
| The commit changes more than 400 source lines | leave the size unexplained | divide it, or say in the body why it does not divide (PR-SPLIT-SIZE) |
| A part of the commit builds and has its own reason | keep it inside the feature commit | make it a commit of its own (PR-SPLIT-ONE-CHANGE) |

## Enforcement

The tiers are the ones defined in
[architectural-requirements.md](architecture.md) §*Enforcement tiers*. Most of
these rules are mechanically checkable, which makes them cheap to enforce and unnecessary to
argue about.

| Rule | Tier | Enforced by (→ how to raise) |
|---|---|---|
| PR-SPLIT-WHITESPACE | T3 | per-commit check: a file whose diff is empty under `git diff -w` but not otherwise, in a commit that also has real changes |
| PR-SPLIT-MOVE | T3 | per-commit check: a delete/add pair with high similarity plus a content change |
| PR-SPLIT-BASELINE | T3+T4 | per-commit check: a baseline-only commit directly after a source commit, or one with no reason in the body (T3) + agent review that the commit which moves the values explains the movement (T4) |
| PR-SPLIT-MECHANICAL | T3+T4 | diff-size and hunk-uniformity heuristic (T3) + agent review |
| PR-SPLIT-SIZE | T3+T4 | note: more than 400 changed source lines (T3) + agent review of the reason in the body (T4) |
| PR-SERIES-BUILDS | T2 | CI builds and tests every commit of the branch, not only the head |
| PR-SERIES-ORDER | T3 | subject-line check for `fixup!`, `squash!`, "typo", "address review" |
| PR-SERIES-LINEAR | T3 | branch check: no merge commit between the merge base and the head |
| PR-MSG-SUBJECT | T3 | commit-message lint: `area: summary`, no file paths, no links; length fails above 80 and is a note above 72 |
| PR-MSG-FACTS | T3+T4 | commit-message lint: no attribution trailers; note for words of revision history and for a `Validation:` paragraph (T3) + agent review for other progress notes (T4) |
| PR-SPLIT-ONE-CHANGE | T3+T4 | note: the fingerprint rows of one commit move in more than one way (T3) + agent review: can a part of the commit stand alone? The rule lists the signs (T4) |
| PR-SPLIT-UPSTREAM | T4 | agent review: does the commit change a shared component to serve one protocol? |
| PR-SPLIT-PREPARE | T4 | agent review: does a "refactor" commit change behavior, and does one commit both add a mechanism and turn on its first user? |
| PR-SPLIT-DRIVEBY | T4 | agent review: is a hunk unrelated to the subject line? |
| PR-MSG-BODY | T3+T4 | commit-message lint: an empty body above 50 changed lines, outside the exempt kinds; agent review for a body that restates the subject |
| PR-MSG-SUMMARY | T3+T4 | note: a first paragraph above 120 words, a body above 300 words, or a body that opens with an action and names no problem (T3) + agent review: can a reviewer restate the answers to the reviewer's questions from the summary alone? (T4) |
| PR-MSG-WHY, PR-MSG-GENERIC, PR-MSG-STANDALONE | T4 | agent review of the message against the diff |
| PR-REQ-* | T4→T5 | agent review for completeness; topic and size are human judgment |
