# Change Classification

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `CR-*` · **Stands on:** [pull-request.md](pull-request.md), [testing.md](testing.md), [release.md](release.md)

What a commit *is*, said in a form that a reader can scan and a gate can check. Three dimensions
answer three questions: **where** does the change reach, **how deep** does it go, and **what must
move with it**. A fourth, optional dimension answers a question about a commit's neighbours:
**which other commits belong with it**. Every commit states them in one line at its end.

The dimensions are not bookkeeping. Each one is already the input of a rule that exists. Scope
decides which tests must run ([TR-CAT-MATCH](testing.md#tr-cat-match)). Depth decides whether
a recorded expectation may move ([TR-BASELINE-DELIBERATE](testing.md#tr-baseline-deliberate)) and
whether the commit is divided correctly ([PR-SPLIT-MECHANICAL](pull-request.md#pr-split-mechanical)).
Obligation decides what the release owes its users ([RR-BREAK-MIGRATE](release.md#rr-break-migrate)).
The group decides what a reader must read together. A dimension that changes nothing anyone does
does not belong here.

**The classification is a claim, not a decoration.** The author knows two things that no tool
knows: whether a change to the source keeps the behavior, and which commits make one larger change.
Everything else follows from the diff. The value of a claim is that a reader, a run or a gate can
prove it wrong.

**Three dimensions classify one commit. The fourth relates commits to each other.** That is why the
group is different in kind, why it is optional, and why it comes last.

## Index

| Rule | Statement |
| --- | --- |
| [CR-SCOPE-AREA](#cr-scope-area) | Every commit names one area |
| [CR-SCOPE-POSITION](#cr-scope-position) | A model commit names the deepest position it reaches |
| [CR-SCOPE-ABOUT](#cr-scope-about) | The scope is what the change is about, not every path it touches |
| [CR-DEPTH-SCALE](#cr-depth-scale) | Depth is one of six ordered levels |
| [CR-DEPTH-ONE](#cr-depth-one) | One commit reaches one level |
| [CR-DEPTH-DIRECTION](#cr-depth-direction) | A behavior commit says what it adds, removes and changes |
| [CR-DEPTH-FIX](#cr-depth-fix) | A commit that repairs a defect says so |
| [CR-DEPTH-NOT-BREAK](#cr-depth-not-break) | What breaks a user is independent of the depth |
| [CR-OBL-DERIVED](#cr-obl-derived) | The obligation follows from the scope and the depth |
| [CR-OBL-INERT](#cr-obl-inert) | A commit below the behavior level moves no recorded expectation |
| [CR-OBL-BASELINE](#cr-obl-baseline) | The author says which recorded expectations move |
| [CR-GROUP-LABEL](#cr-group-label) | A commit that is one of several in a larger change names the group |
| [CR-GROUP-SPANS](#cr-group-spans) | A group is not a pull request |
| [CR-GROUP-STABLE](#cr-group-stable) | The label does not change, and it carries no ordinal |
| [CR-GROUP-STANDALONE](#cr-group-standalone) | The label does not excuse a partial commit |
| [CR-TAG-TRAILER](#cr-tag-trailer) | Every commit ends with one `Change:` line |
| [CR-TAG-FORM](#cr-tag-form) | The trailer has three fields in a fixed order |
| [CR-TAG-SUBJECT](#cr-tag-subject) | The subject may repeat any part of the classification except the obligations |

## Scope (CR-SCOPE)

### CR-SCOPE-AREA

**Every commit names one area.**

The area is the top level of the scope. There are five, and the whole repository falls into them.

| Area | What it holds |
| --- | --- |
| `src` | the model source, `src/inet` |
| `tests` | `tests/`, of every category |
| `doc` | `doc/`, and `WHATSNEW` |
| `examples` | `examples/`, `showcases/`, `tutorials/` |
| `build` | Makefiles, `python/`, `.github/`, the scripts |

The area matters because it is most of the corpus. A mechanical pass over master's last 300
commits puts **111 in `src`, 88 in `tests`, 89 in `doc` and 4 in `build`**. A taxonomy that can
name only source changes cannot classify two thirds of the history.

*Enforced at T3 — the area is derivable from the paths in the diff, so a gate can compare the claim
with the diff.*

### CR-SCOPE-POSITION

**A model commit names the deepest position it reaches.**

Inside `src`, four positions exist. Each one is more precise than the one above it, and the author
gives the most precise one that holds the whole change. The ancestors are implied, so
`ieee80211.Dcf` does not repeat the subsystem chain above `ieee80211`.

| Position | Where the value comes from | How much of `src/inet` it covers |
| --- | --- | --- |
| feature | `.oppfeatures` | 78% of the files, that is 3842 of 4875 |
| subsystem | the directory | all of them |
| module | the NED type | only a `.ned` change |
| class | the C++ type | only a `.cc` or `.h` change |

**A feature is not the parent of a subsystem.** Both name sets of directories. `.oppfeatures`
curates its set for the build; the architecture curates its set for the design. The two sets agree
most of the time, and the rules do not depend on the agreement. Of the 124 features, 123 name at
least one directory.

*Enforced at T4 — the position is derivable, but which position is the right level of precision is
a judgment.*

### CR-SCOPE-ABOUT

**The scope is what the change is about, not every path it touches.**

A behavior change ships with its test ([TR-SHIP-WITH](testing.md#tr-ship-with)) and, when the
behavior moves a recorded expectation, with its baseline
([PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline)). Those paths are the *obligation* of the
change, and the third dimension records them. They do not make the commit a test commit.

The opposite case is a real `tests` commit: the change is about the test itself. A new test for
behavior that already exists, a repaired test, a renamed configuration. Nothing in `src` moves.

*Enforced at T4.*

## Depth (CR-DEPTH)

### CR-DEPTH-SCALE

**Depth is one of six ordered levels.**

Each level reaches further into the artifact than the one above it, and each one keeps everything
that the levels above it keep.

| Level | Name | What the commit changes | What it keeps |
| --- | --- | --- | --- |
| 0 | `comment` | comments and documentation text | every token the compiler reads |
| 1 | `format` | whitespace, line breaks, brace position | the sequence of tokens |
| 2 | `location` | which file holds the code | the text of the code |
| 3 | `name` | identifiers | the organization of the code |
| 4 | `refactor` | how the code is organized | what the artifact does |
| 5 | `behavior` | what the artifact does | — |

**`location` and `name` are the two refactors that earn their own level.** `refactor` is every
other structural change. The word is broader in ordinary use than the level is — of master's last
3000 subjects, 71 carry it and **13 of those 71 also say rename, move, hoist or split**. Those 13
are level 2 or level 3 work. [CR-DEPTH-ONE](#cr-depth-one) resolves the overlap: the level is the
deepest one the commit *reaches*, and a pure rename does not reach level 4.

The level is named for what a reader recognizes and not for the naming axis of the other five. A
noun for what changed would be more consistent; `refactor` is the word every author already knows,
and a field that people type from memory is worth more when it is guessed right the first time.

**Level 5 reads per area.** In `src` it is the behavior of the model. In `tests` it is what the
test checks. In `doc` it is what the document states. In each area it is the same question: does a
reader of the artifact get a different answer after the commit?

**A `doc` change is an area, not a depth.** A comment inside `src/inet/linklayer/ieee80211` is
`src` at level 0. A rewritten page under `doc/` is `doc` at level 5. The two are not the same
category, and one scale cannot hold both.

*Enforced at T3 — levels 0 to 3 are derivable from the diff. A gate can compare the claim with the
diff for those four.*

### CR-DEPTH-ONE

**One commit reaches one level.**

This is not a new demand. [PR-SPLIT-WHITESPACE](pull-request.md#pr-split-whitespace) separates
level 1. [PR-SPLIT-MOVE](pull-request.md#pr-split-move) separates level 2.
[PR-SPLIT-MECHANICAL](pull-request.md#pr-split-mechanical) separates level 3.
[PR-SPLIT-ONE-CHANGE](pull-request.md#pr-split-one-change) covers the rest. The depth scale is
those four rules seen from the other side: **two levels in one commit is the violation itself.**

When a commit does reach two levels, its depth is the deepest one, and the audit reports the split
rule that it breaks.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh) already fails a whitespace
change mixed with a content change.*

### CR-DEPTH-DIRECTION

**A behavior commit says what it adds, removes and changes.**

Three directions, and they are not exclusive. One commit can add and remove in the same change.

| Direction | Means |
| --- | --- |
| `add` | something exists that did not exist |
| `remove` | something no longer exists |
| `change` | something exists in both states and does a different thing |

The change summary computes all three per pull request, as its Added, Removed and Changed
sections. The trailer states them per commit, which the summary cannot do for free, because it
compares two trees and not two commits.

*Enforced at T3 for a public interface, where the summary derives the direction; T4 for a private
one.*

### CR-DEPTH-FIX

**A commit that repairs a defect says so.**

Level 5 needs a second attribute, because the direction does not carry the intent. `change` covers
two different things:

- a **fix** — the old behavior was wrong, and the new one is right;
- a **deliberate change** — the old behavior was right, and the new one is different.

Write `fix` for the first. Write nothing for the second, because most changes are deliberate and a
marker on the common case costs every author a word for no reading value.

The distinction is what the release notes need. A fix goes under repaired defects. A deliberate
change goes under changes, and it may owe a migration note
([RR-BREAK-MIGRATE](release.md#rr-break-migrate)).

A `fix` also earns a body under [PR-MSG-BODY](pull-request.md#pr-msg-body), at any size, and the
body gives the symptom in the words a future reader will search for.

*Enforced at T4 — no tool can read the intent. A gate can check that a `fix` carries a body.*

### CR-DEPTH-NOT-BREAK

**What breaks a user is independent of the depth.**

A rename does not change the simulation, and it breaks a user's code. A behavior change moves the
simulation, and it can break nothing. The two effects are orthogonal, so the depth scale must not
try to carry both.

| The commit | The model moves | The user must repair |
| --- | --- | --- |
| rename a public class | no | yes |
| change a private algorithm | yes | no |
| remove a public parameter | yes | yes |
| refactor the internals | no | no |

The break is **derived, not declared**. The change summary computes it from the removed and renamed
public members, so the author does not repeat it. It reaches the trailer only through the third
field, as `whatsnew` and `migration`.

*Enforced at T3 — the change summary derives the break.*

## Obligation (CR-OBL)

### CR-OBL-DERIVED

**The obligation follows from the scope and the depth.**

The third dimension is a consequence of the first two, and not an independent axis. One table gives
almost all of it.

| Scope and depth | What must move |
| --- | --- |
| any area, levels 0 to 2 | nothing |
| `src`, level 3, a public name | `whatsnew`, `migration` |
| `src`, level 3, a private name | nothing |
| `src`, level 4 | nothing |
| `src`, level 5, `add` | `test`, `whatsnew` |
| `src`, level 5, `remove` or `change` of a public thing | `test`, `whatsnew`, `migration` |
| `src`, level 5, and a covered scenario moves | `fingerprint` or `statistical` |
| `tests`, level 5, and a module test output changes | `expected` |
| `doc`, level 5, and the changed text is a rule | `seals`, for every seal that rests on it |

The values, in full:

| Value | What it names | Rule |
| --- | --- | --- |
| `fingerprint` | a fingerprint baseline under `tests/fingerprint/` | [TR-BASELINE-PROVENANCE](testing.md#tr-baseline-provenance) |
| `statistical` | a statistical baseline | the same |
| `expected` | the expected output inside a `.test` file | [TR-SHIP-WITH](testing.md#tr-ship-with) |
| `test` | a new or changed test | [TR-SHIP-WITH](testing.md#tr-ship-with), [TR-CAT-MATCH](testing.md#tr-cat-match) |
| `whatsnew` | an entry in `WHATSNEW` | [RR-WHATSNEW](release.md#rr-whatsnew) |
| `migration` | an entry in `doc/src/migration-guide/` | [RR-BREAK-MIGRATE](release.md#rr-break-migrate) |
| `seals` | a seal that rests on a changed rule, and is now stale | [SR-RULE-CHANGE-STALES](sealing.md#sr-rule-change-stales) |
| `-` | nothing moves | — |
| `?` | the author does not know yet | — |

**The last row of the first table is the one cell that no tool can fill.** Whether a behavior
change moves a covered scenario depends on the trajectory of that scenario, and only a run answers
it. Everything above that row is derivable.

*Enforced at T3 for every value except `fingerprint` and `statistical`; T5 for those two, because a
run decides them.*

### CR-OBL-INERT

**A commit below the behavior level moves no recorded expectation.**

This is the check that the whole classification exists for. A commit at level 0 to 4 claims that the
model produces the same results. A fingerprint or statistical baseline in the same series falsifies
the claim. One of the two is then wrong: the classification, or the split under
[PR-SPLIT-BASELINE](pull-request.md#pr-split-baseline).

**The check is cheap and it fires about once in a thousand commits.** Master's last 300 hold 9
commits that claim inertness in their subject, and none of the 9 touches a baseline. Widen the
window to 3000 and the contradiction appears: of the 71 subjects that carry the word `refactor`,
two carry a fingerprint baseline as well.

```
IPv6: refactor: extension headers to chunk-based architecture
IPv6: refactor: replace pointer-ordered sets with vectors (deterministic ordering)
```

Both are honest commits that chose the wrong word. The second one repairs a determinism defect,
which is a behavior change and not a refactor. That is the whole value of the check: it finds the
commit whose word and whose diff disagree, and it finds about one per thousand.

The contrary case exists and is legitimate: **12 of the 300 move a baseline and change no source**,
for example `tests: synchronize stale JSON fingerprints with CSV baselines`. Those are `tests` at
level 5, and the obligation field is where the reason belongs.

*Enforced at T3 — a gate compares the claimed depth with the presence of a baseline in the diff.*

### CR-OBL-BASELINE

**The author says which recorded expectations move.**

Where the obligation table gives `fingerprint` or `statistical`, the author names them, or states
that none move. A branch that changes behavior and says nothing about the baselines is the single
most common blocking finding in the audit reports so far.

Three answers are acceptable, and silence is not one of them:

1. **They do not move**, with the configurations that were run.
2. **They move**, with a baseline commit and the reason under
   [TR-BASELINE-PROVENANCE](testing.md#tr-baseline-provenance).
3. **`?`** — the author has not run them yet. This is honest and a reviewer can act on it.

*Enforced at T4 — the reviewer asks. T5 once a per-commit run exists
([TR-CI-EVERY-COMMIT](testing.md#tr-ci-every-commit)).*

## The group (CR-GROUP)

### CR-GROUP-LABEL

**A commit that is one of several in a larger change names the group.**

The label is a stable slug: lower case, words joined by a hyphen, one to four words. It names the
change and not the mechanism, which is the test that
[PR-MSG-SUBJECT](pull-request.md#pr-msg-subject) applies to a subject. Write
`tcp-algorithm-hierarchy`, not `move-files` and not `part-2`.

Where the work has a plan file under `plan/pending/`, **use the stem of that file**. The two names
then agree for free, and a reader who finds one finds the other.

```
Change: src.transportlayer.tcp | refactor | - | tcp-algorithm-hierarchy
```

A commit that stands alone has no group, and the fourth field is absent. Most commits are of that
kind, so the field costs nothing when it is not needed.

**What the label buys the reader.** A larger change arrives as five or ten commits, and each one is
correct on its own. The reader who bisects to the third of them needs to know that seven more
exist, because the answer to "why does this look half-finished" is in the other seven. Nothing else
in the history says so.

*Enforced at T4 — no tool can know which commits belong together. A gate can check the shape of the
slug, and it can list the groups it finds in a series.*

### CR-GROUP-SPANS

**A group is not a pull request.**

All three relations occur, so a tool cannot derive the label from the branch:

- one pull request holds **one** group. This is the common case.
- one pull request holds **two or more** groups. A stacked request, or a series that first prepares
  a shared component and then changes the model that needs it
  ([PR-SPLIT-UPSTREAM](pull-request.md#pr-split-upstream)).
- one group spans **two or more** pull requests. The author divided the work to keep each request
  small enough to review ([PR-REQ-TOPIC](pull-request.md#pr-req-topic)).

**The label is what survives.** A merge with a squash or a rebase drops the number of the pull
request from the history. The label stays in the body of every commit, so `git log --grep` finds
the whole group years later, across the request boundaries and across every rebase.

*Enforced at T4.*

### CR-GROUP-STABLE

**The label does not change, and it carries no ordinal.**

Do not write `tcp-cleanup-3-of-7`. A rebase reorders the commits, a review inserts one, and a split
turns one into two. Each of those makes the ordinal wrong. **A wrong ordinal is worse than no
ordinal**, because a reader counts the commits, finds six, and looks for a seventh that never
existed.

Do not rename the label after the first commit carries it. A group with two names is two groups to
everyone who greps for it, and the history keeps both names forever.

*Enforced at T3 — a gate can check the shape of the slug and report two labels that differ by one
character in the same series.*

### CR-GROUP-STANDALONE

**The label does not excuse a partial commit.**

A group is a cross-reference, not a licence. Every commit in the group still builds and passes its
tests ([PR-SERIES-BUILDS](pull-request.md#pr-series-builds)), still makes exactly one change
([PR-SPLIT-ONE-CHANGE](pull-request.md#pr-split-one-change)), and still carries its own reason
without the others ([PR-MSG-STANDALONE](pull-request.md#pr-msg-standalone)).

The test: forget every other commit of the group and read this one. If it no longer makes sense,
the label hides a division fault. It does not repair one.

*Enforced at T4 — the same review that judges the division of the series.*

## The trailer (CR-TAG)

### CR-TAG-TRAILER

**Every commit ends with one `Change:` line.**

The line is the last line of the message. One empty line comes before it. It states the three
dimensions of the commit, and the group where the commit has one, in a form that a reader scans in
one second and a gate parses with one regular expression.

```
ieee80211: make rate control adapt per receiver

A single rate per station blends the conditions of every peer, so a node
that talks to a near and a far station converges on a rate that suits
neither. Hold the state per receiver address instead.

Change: src.ieee80211.IRateControl | behavior.change | fingerprint whatsnew migration
```

The trailer is not the body and it does not replace it. The body gives the reason
([PR-MSG-WHY](pull-request.md#pr-msg-why)); the trailer gives the classification. Neither one says
what the other says.

*Enforced at T3 — [check-commits.sh](../enforcement/check-commits.sh); no gate yet.*

### CR-TAG-FORM

**The trailer has three fields in a fixed order, separated by `|`, and an optional fourth.**

```
Change: <area>[.<position>] | <depth>[.<direction>][.fix] | <obligations> [| <group>]
```

The fourth field is last, so its absence is unambiguous and it needs no placeholder. A commit that
belongs to no group writes three fields.

| Field | Values |
| --- | --- |
| area | `src`, `tests`, `doc`, `examples`, `build` |
| position | the feature, subsystem, module or class; omitted outside `src` |
| depth | `comment`, `format`, `location`, `name`, `refactor`, `behavior` |
| direction | `add`, `remove`, `change`; more than one joined by `+`; only on `behavior` |
| intent | `fix`, or omitted for a deliberate change |
| obligations | any of `fingerprint`, `statistical`, `expected`, `test`, `whatsnew`, `migration`, separated by a space; or `-` for none; or `?` |
| group | a stable slug, one to four lower-case words joined by a hyphen; the whole field is omitted when the commit stands alone |

Write `?` rather than a guess. A guess that a reviewer trusts is worse than a question that a
reviewer answers.

Eight real shapes. The last three are one group, in three commits, in the order a reader reads
them:

```
Change: src.ieee80211.Dcf | behavior.change.fix | fingerprint
Change: src.transportlayer.tcp | behavior.add | test whatsnew
Change: src.networklayer.L3AddressResolver | name | whatsnew migration
Change: src.visualizer | format | -
Change: tests.fingerprint | behavior.change | -
Change: doc | behavior.change | -

Change: src.transportlayer.tcp | location | - | tcp-algorithm-hierarchy
Change: src.transportlayer.tcp | name | whatsnew migration | tcp-algorithm-hierarchy
Change: src.transportlayer.tcp | refactor | - | tcp-algorithm-hierarchy
```

Read the three together and the group tells its own story: the files move, then the types take
their new names, then the hierarchy changes shape. Each commit holds one depth level
([CR-DEPTH-ONE](#cr-depth-one)), and no commit changes the behavior, so no baseline moves
([CR-OBL-INERT](#cr-obl-inert)).

*Enforced at T3 — one regular expression, and a comparison of each field with the diff.*

### CR-TAG-SUBJECT

**The subject may repeat any part of the classification except the obligations.**

`git log --oneline` shows the subject and nothing else. **The trailer is invisible in the one view
the subject exists to serve**, so "the trailer already says it" is no reason to leave a thing out.
The author decides what a log reader needs, and the rule only bounds the choice.

```
[<scope>: ][<kind>: ]<what the commit does>
```

Both prefixes are optional and each one is free within its bounds.

| Prefix | What the author may write | Taken from |
| --- | --- | --- |
| scope | one or more **consecutive segments** of the trailer's area and position, joined by a dot, in their order | field 1 |
| kind | one or more **consecutive segments** of the trailer's depth, direction and intent, joined by a dot, in their order | field 2 |
| the group | the label, shortened as far as it stays recognizable | field 4 |
| the obligations | **never** | field 3 |

The kind is one word in almost every case, because `behavior` is implied by the direction that
follows it and is not written:

| The trailer says | The subject writes |
| --- | --- |
| `behavior.add` | `add:` |
| `behavior.change` | `change:` |
| `behavior.remove` | `remove:` |
| `behavior.change.fix`, `behavior.add.fix` | `fix:` |
| `refactor`, `comment`, `format`, `location`, `name` | the same word |
| a mixed depth or direction | both, as in `add+change:` or `name+refactor:` |

A mixed marker is meant to look wrong. `add+change:` and `name+refactor:` are the subjects of
commits that hold two changes, and [CR-DEPTH-ONE](#cr-depth-one) and
[PR-SPLIT-ONE-CHANGE](pull-request.md#pr-split-one-change) both say so.

Five shapes, each one legal:

```
EthernetMac: fix: drop the frame when the carrier goes before the preamble ends
linklayer: refactor: hoist the shared frame-sequence steps into the base
showcases.tsn: format: unindent the gate-schedule tables
add: Tail Loss Probe (RFC 8985 section 7.2)
split: fold DcTcp onto the shared ACK path
```

The first writes the class and drops the area, the second writes the subsystem and drops the
class, the third joins an area and a position, the fourth drops the scope and keeps the kind, and
the fifth writes the group where a scope would go, on a commit whose group matters more than its
subsystem.

**Write the kind. Drop the scope first when the line is tight.** The two prefixes are equally
optional in the grammar and they are not equally valuable, so the rule states the order.

A missing kind is not a third way of saying something. In a scheme with three markers, absence can
carry the fourth value — this is what simu5g does, where an unmarked subject is a feature. **This
scheme has six depths and three directions, so absence carries nothing**: an unmarked subject may
be `behavior.add`, `behavior.change`, `behavior.add+change`, or an author in a hurry. A gate cannot
see a marker that is not there either, so an optional kind leaves the check silent on every subject
that omits it.

The scope is different. A reader recovers most of it from the topic and all of it from the file
paths, so a subject survives its loss. Give up the scope first, the kind last, and the topic never.
Where even `fix: ` will not fit, the topic is too long, and that is the thing to repair.

**What the author must not do is disagree with the trailer.** A subject that says `fix:` over a
trailer with no `.fix`, or `linklayer:` over a trailer that says `src.tcp`, is one commit with two
classifications. That, and not the choice of prefix, is what a gate checks.

**Why the obligations stay out.** They are a list of what travels *with* the commit, and a subject
names what the commit *is*. They also move: a rebase or a split changes which commit carries a
baseline, and a subject that named one goes stale in place. Nobody scans a log for them.

Take care with one look-alike. `tests(fingerprint): update the MIPv6 roaming baseline` is INET
practice and it is correct: `fingerprint` there is the **scope** — the commit changes
`tests/fingerprint/` — and not the obligation.

**Why the rule was widened.** An earlier version told the author to leave out what the trailer
states. Applying it to the 61 commits of `topic/tcp-new` showed the fault
([classification-on-tcp-new.md](../audit/report/sweep/classification-on-tcp-new.md), F-1): it
permitted dropping `tcp:` from 47 subjects, and dropping it would have made `git log --oneline`
worse in every one. A rule that pushes an author toward a worse log is wrong, whatever the trailer
holds.

*Enforced at T3 — a gate compares each prefix with the trailer and reports a disagreement. The
length band of [PR-MSG-SUBJECT](pull-request.md#pr-msg-subject) still holds: aim for 72
characters, and a gate fails above 80.*
