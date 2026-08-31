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
the *change* must look like. It is the concrete form of step 4 of the *Contributor workflow*
(smallest change surface) and of the reviewable-patch clauses of
[AR-QUAL-FINGERPRINT](architecture.md) and
[AR-QUAL-TRACEABILITY](architecture.md).

**The commit is the unit of review, not the pull request.** A reviewer reads a series of
commits, one at a time, and asks one question per commit: *is this change right?* That
question has an answer only if the commit contains one change. When a commit contains two
changes, the reviewer must first separate them mentally, and the reviewer does that work
again for every later reader of the history. Divide the work in advance, because you are the
only person who knows where the boundaries are.

## Index

Every rule in document order. The identifier links to the rule; the statement is its lead sentence.

**Commit content (PR-SPLIT)**

| Rule | Statement |
| --- | --- |
| [PR-SPLIT-ONE-CHANGE](#pr-split-one-change) | One commit makes exactly one change |
| [PR-SPLIT-WHITESPACE](#pr-split-whitespace) | A whitespace change touches only whitespace |
| [PR-SPLIT-MECHANICAL](#pr-split-mechanical) | A mechanical sweep is separate from work that needs thought |
| [PR-SPLIT-MOVE](#pr-split-move) | A file move is its own commit |
| [PR-SPLIT-PREPARE](#pr-split-prepare) | Preparation comes before the change that needs it |
| [PR-SPLIT-UPSTREAM](#pr-split-upstream) | A shared-component change is separate from, and before, the model that needs it |
| [PR-SPLIT-BASELINE](#pr-split-baseline) | Updated expected results are their own commit |
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
| [PR-MSG-WHY](#pr-msg-why) | The body gives the reason, not the content |
| [PR-MSG-GENERIC](#pr-msg-generic) | A shared-component commit explains itself in generic terms |
| [PR-MSG-STANDALONE](#pr-msg-standalone) | The message carries its own context |
| [PR-MSG-FACTS](#pr-msg-facts) | The message contains only facts about the change |

**The pull request (PR-REQ)**

| Rule | Statement |
| --- | --- |
| [PR-REQ-TOPIC](#pr-req-topic) | One pull request, one topic |
| [PR-REQ-STORY](#pr-req-story) | The description states the topic, the reason, and the reading order |
| [PR-REQ-ARCH](#pr-req-arch) | The description names the architectural surface |
| [PR-REQ-CLEAN](#pr-req-clean) | No leftovers |

## Commit content (PR-SPLIT)

### PR-SPLIT-ONE-CHANGE

**One commit makes exactly one change**

A commit contains one self-contained change, and the whole of that change.

Use the subject line as the test: if you cannot say what the commit does in one line without
"and" or a list, the commit holds more than one change. The opposite fault counts too. Half a
change is not a commit: the tree after the commit must build, and the model after the commit
must be consistent (PR-SERIES-BUILDS). "One change" means one *decision*, not one file — a
decision that touches eight files is still one commit.

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

### PR-SPLIT-PREPARE

**Preparation comes before the change that needs it**

When a fix needs a refactor first, commit the refactor alone, and keep it behavior-preserving.
The fix follows in the next commit.

The reviewer then answers two simple questions instead of one hard one: *is the refactor
safe?* — and the fingerprint tests answer it — and *is the fix right?*, on a diff of a few
lines. In a mixed commit neither question has a safe answer, because every changed line is a
candidate cause of the behavior change.

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

**Updated expected results are their own commit**

Regenerated fingerprints (`tests/fingerprint/*.csv`), statistical baselines, and other recorded
expectations go in a commit that contains no source change.

A baseline update is a claim, not a side effect: *these values change on purpose, and the new
values are correct*. The message must name the commit that causes the change and give the
reason. Put the baseline commit directly after the commit that changes behavior. Inside a
source commit the same update is invisible, and "the fingerprint changed" stops being a
conscious decision — which is the whole point of AR-QUAL-FINGERPRINT and AR-QUAL-TRACEABILITY.

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

One line: the component or tree area, a colon, then what the commit does, in the present tense.
Keep it below about 72 characters and end it without a full stop. Then one empty line, then the
body.

The area is the NED or C++ component (`ExternalProcess:`, `Ipv4:`, `visualizer:`) or the part of
the tree (`tests:`, `doc:`, `build:`, `examples/mpls/net37:`). An optional kind word may follow
the area (`ospfv3: fix:`, `python: refactor:`). Name the *behavior*, never the mechanics: write
`ExternalProcess: don't kill the process group when a spawned command fails`, not "update
ExternalProcess.cc" and not a list of file names or links.

### PR-MSG-WHY

**The body gives the reason, not the content**

The diff already shows what changed. The body says why: the symptom, the cause, why this
solution and not an obvious alternative, and what the change deliberately does not repair.

For a bug fix, write the symptom in the words a future reader will search for — the error
message, the wrong packet, the failed assertion. For a behavior change, name the standard
clause or the reference that makes the new behavior the correct one.

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

## The pull request (PR-REQ)

### PR-REQ-TOPIC

**One pull request, one topic**

A pull request carries one topic, at a size a reviewer can hold in the head at once. Two topics
are two pull requests, even when the same developer wrote them on the same day. A long series
on one topic is fine; a short series on three topics is not.

### PR-REQ-STORY

**The description states the topic, the reason, and the reading order**

The description says what the change achieves and why it is needed, and it names the order in
which the commits should be read when that order is not obvious. It lists the tests that were
run, with the exact commands and the resulting status, and it names every baseline update
(*Contributor workflow*, step 6).

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
| An 802.11 fix needs a queueing feature | one commit | generic queueing feature, then 802.11 fix (PR-SPLIT-UPSTREAM) |
| Your fix changes fingerprints | source and `.csv` together | source commit, then baseline commit (PR-SPLIT-BASELINE) |
| You see an unrelated bug on the way | fold it in | separate commit, or separate pull request (PR-SPLIT-DRIVEBY) |
| A reviewer finds a defect in commit 2 of 5 | add commit 6 | rebase the correction into commit 2 (PR-SERIES-ORDER) |
| The target branch moved under you | merge it in | rebase the series (PR-SERIES-LINEAR) |
| The subject needs an "and" | write the "and" | divide the commit (PR-SPLIT-ONE-CHANGE) |

## Enforcement

The tiers are the ones defined in
[architectural-requirements.md](architecture.md) §*Enforcement tiers*. Most of
these rules are mechanically checkable, which makes them cheap to enforce and unnecessary to
argue about.

| Rule | Tier | Enforced by (→ how to raise) |
|---|---|---|
| PR-SPLIT-WHITESPACE | T3 | per-commit check: a file whose diff is empty under `git diff -w` but not otherwise, in a commit that also has real changes |
| PR-SPLIT-MOVE | T3 | per-commit check: a delete/add pair with high similarity plus a content change |
| PR-SPLIT-BASELINE | T3 | per-commit check: `tests/**/*.csv` and `src/**` changed in the same commit |
| PR-SPLIT-MECHANICAL | T3+T4 | diff-size and hunk-uniformity heuristic (T3) + agent review |
| PR-SERIES-BUILDS | T2 | CI builds and tests every commit of the branch, not only the head |
| PR-SERIES-ORDER | T3 | subject-line check for `fixup!`, `squash!`, "typo", "address review" |
| PR-SERIES-LINEAR | T3 | branch check: no merge commit between the merge base and the head |
| PR-MSG-SUBJECT | T3 | commit-message lint: `area: summary`, length, no file paths, no links |
| PR-MSG-FACTS | T3 | commit-message lint: no attribution trailers |
| PR-SPLIT-ONE-CHANGE | T4 | agent review: does the commit contain two independent decisions? |
| PR-SPLIT-UPSTREAM | T4 | agent review: does the commit change a shared component to serve one protocol? |
| PR-SPLIT-PREPARE | T4 | agent review: does a "refactor" commit change behavior? |
| PR-SPLIT-DRIVEBY | T4 | agent review: is a hunk unrelated to the subject line? |
| PR-MSG-WHY, PR-MSG-GENERIC, PR-MSG-STANDALONE | T4 | agent review of the message against the diff |
| PR-REQ-* | T4→T5 | agent review for completeness; topic and size are human judgment |
