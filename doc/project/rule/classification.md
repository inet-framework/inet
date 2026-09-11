# Change Classification

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `CR-*` · **Stands on:** [pull-request.md](pull-request.md), [testing.md](testing.md), [release.md](release.md)

What a commit *is*, said in a form that a reader can scan and a gate can check. Three dimensions
answer three questions: **where** does the change reach, **how deep** does it go, and **what must
move with it**. Every commit states all three in one line at its end.

The three dimensions are not bookkeeping. Each one is already the input of a rule that exists.
Scope decides which tests must run ([TR-CAT-MATCH](testing.md#tr-cat-match)). Depth decides whether
a recorded expectation may move ([TR-BASELINE-DELIBERATE](testing.md#tr-baseline-deliberate)) and
whether the commit is divided correctly ([PR-SPLIT-MECHANICAL](pull-request.md#pr-split-mechanical)).
Obligation decides what the release owes its users ([RR-BREAK-MIGRATE](release.md#rr-break-migrate)).
A dimension that changes nothing anyone does does not belong here.

**The classification is a claim, not a label.** The author knows one thing that no tool knows:
whether a change to the source keeps the behavior. Everything else in the three dimensions follows
from the diff. The value of the claim is that a test suite can prove it wrong.

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
| [CR-TAG-TRAILER](#cr-tag-trailer) | Every commit ends with one `Change:` line |
| [CR-TAG-FORM](#cr-tag-form) | The trailer has three fields in a fixed order |
| [CR-TAG-SUBJECT](#cr-tag-subject) | The subject carries only what the author judges important |

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
| 3 | `name` | identifiers | the structure |
| 4 | `structure` | how the code is organized | what the artifact does |
| 5 | `behavior` | what the artifact does | — |

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
| restructure the internals | no | no |

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

The check is cheap and it is currently green. Of master's last 300 commits, **9 claim inertness in
their subject** with words such as refactor, cleanup, rename, whitespace, unindent and reorder, and
**all 9 touch no baseline**. The check is a guard against a future defect, not a backlog of present
ones.

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

## The trailer (CR-TAG)

### CR-TAG-TRAILER

**Every commit ends with one `Change:` line.**

The line is the last line of the message. One empty line comes before it. It states all three
dimensions in a form that a reader scans in one second and a gate parses with one regular
expression.

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

**The trailer has three fields in a fixed order, separated by `|`.**

```
Change: <area>[.<position>] | <depth>[.<direction>][.fix] | <obligations>
```

| Field | Values |
| --- | --- |
| area | `src`, `tests`, `doc`, `examples`, `build` |
| position | the feature, subsystem, module or class; omitted outside `src` |
| depth | `comment`, `format`, `location`, `name`, `structure`, `behavior` |
| direction | `add`, `remove`, `change`; more than one joined by `+`; only on `behavior` |
| intent | `fix`, or omitted for a deliberate change |
| obligations | any of `fingerprint`, `statistical`, `expected`, `test`, `whatsnew`, `migration`, separated by a space; or `-` for none; or `?` |

Write `?` rather than a guess. A guess that a reviewer trusts is worse than a question that a
reviewer answers.

Six real shapes:

```
Change: src.ieee80211.Dcf | behavior.change.fix | fingerprint
Change: src.transportlayer.tcp | behavior.add | test whatsnew
Change: src.networklayer.L3AddressResolver | name | whatsnew migration
Change: src.visualizer | format | -
Change: tests.fingerprint | behavior.change | -
Change: doc | behavior.change | -
```

*Enforced at T3 — one regular expression, and a comparison of each field with the diff.*

### CR-TAG-SUBJECT

**The subject carries only what the author judges important.**

The trailer holds the whole classification, so the subject does not repeat it. This **relaxes**
[PR-MSG-SUBJECT](pull-request.md#pr-msg-subject): the area prefix and the kind marker are both
optional, and the author chooses which parts earn their place on the one line that
`git log --oneline` shows.

Keep in the subject what a reader who scans a hundred subjects needs to find this commit. Leave out
what the trailer already states and what nobody scans for.

| Keep it when | Leave it out when |
| --- | --- |
| the area separates this commit from its neighbours in a mixed series | the whole series is in one area, and the subject reads better without it |
| the subject is unclear without the component name | the subject already names the component in its own words |
| `fix:` tells a reader of the release branch what they need | the body and the trailer already say `fix` and nobody greps the subject for it |

**Why the relaxation.** The kind marker is not INET practice: of master's last 1500 subjects, about
**20 carry a real kind marker, which is 1.3%**. The area prefix is practice: **1354 of 1500, which
is 90%**. A rule that demands the first would import a convention that the project does not use,
and it would put the same fact in two places. The trailer is the one place.

*Enforced at T3 — the length band of [PR-MSG-SUBJECT](pull-request.md#pr-msg-subject) still holds:
aim for 72 characters, and a gate fails above 80.*
