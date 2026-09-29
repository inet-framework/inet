# Resolve the audit findings of PR #1155

Status: **in progress** — steps 1a to 1j (all of step 1), 2, 3 and 4 done; the rebase left the work of
steps 2a to 2c; step 5, 5b and 6 follow. The plan lives on the branch `topic/tcp-new-audit-fixes`
since 2026-09-29 (owner's decision); older copies are on `topic/audit` and on `master`.
`/home/levy/workspace/inet-tcp-new-audit-fixes`, branch `topic/tcp-new-audit-fixes`.
Audit: `audit/pull-request/pr-1155.md`, third pass, 2026-09-11.
Branch: `topic/tcp-new`, head `33e8b0d073`, merge base `434658d729`, 61 commits, 183 files, +15840 / −3685.

## 1. What this plan resolves

Eight findings: **F-1, F-2, F-6** blocking, **F-3, F-4, F-5, F-7** findings, **F-8** blocking for
the audit tooling rather than for the branch.

| | What it is | Where the work is |
| --- | --- | --- |
| F-1 | the branch edits sealed `common/packet/` and states no permission | a decision, then one line in the description |
| F-2 | no release note for four kinds of break | `WHATSNEW`, the migration guide, one alias |
| F-3 | two baseline-only commits, and 9 commits owe a fingerprint | the expensive step |
| F-4 | a rename of three files inside a 25-file edit | split one commit |
| F-5 | `ITcpRecovery` holds five no-op defaults | 5 declarations, 3 implementors |
| F-6 | the branch no longer merges | rebase over 99 commits, 3 conflicting paths |
| F-7 | 17 commits have no body, 12 of them above 50 lines | the author writes 12 bodies |
| F-8 | the summary tool fails silently without its parsers | `opp_repl`, not this repository |

Two rules written on 2026-09-11 also apply to this branch, and neither was in the audit because
neither existed when it was written:

| Rule | What it asks of this branch |
| --- | --- |
| [PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce) | the 14 `fix` commits say how to see the defect — step 1f |
| [PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan) | every commit this plan touches names this plan — step 1g |

[TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance) was strengthened
the same day, and that is D-2 below.

## 2. The two decisions, both now made

### D-1 — `common/packet/`: the three edits are permitted — **decided 2026-09-11**

The seal owner validated the three edits — `chunk/BitCountChunk.cc`, `chunk/ByteCountChunk.cc` and
`recorder/PcapRecorder.cc` — and allowed them, in the words "unseal for the edit and reseal after".

**Reading the rule showed that no unsealing is wanted, and step 4 was reworked because of it.**
[SR-PR-APPROVAL](../../doc/project/rule/sealing.md#sr-pr-approval) treats a
current-conversation permission as authority to *prepare* the change, which is exactly what was
given; the registry is not touched, and merge authorization is a separate, protected, human
decision bound to the head. Physically removing the row would leave the path unprotected for
everybody in the meantime and record nothing.

The decision unblocks F-1 and does not resolve the underlying question: `AV-ORG-01` and
`AV-ORG-02` remain `Open (decide)`, so the seal still rests on two clusters the ledger calls
invalid. **This decision is about the branch, not about the seal.**

### D-2 — Attribute every moved row — **decided 2026-09-11, the hard way**

Every commit that moves a fingerprint carries the new values, and **every moved row is analyzed and
explained** from the change that causes it and from what that fingerprint records.

This is the rule, not a preference for this branch:
[TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance) was strengthened
on 2026-09-11 to say it. **A row that moves and cannot be explained from the diff is the finding**
— an unexplained row is an unintended change until somebody shows otherwise, because a fingerprint
says only that the trajectory differs and never which of the two is right. Rows that share one
explanation are named together; a count is not an explanation.

Step 5 is therefore the long step, and the plan says so rather than hiding it.

## 3. The order, and why

```
  step 1  rewrite history   (F-4, F-5, F-7, the F-2 alias, the trailers,
                             9 of the 14 reproductions, the Plan: lines)
  step 2  rebase onto master (F-6)
  step 3  the release note   (F-2)
  step 4  unseal, edit, reseal (F-1)
  step 5  the baselines, every row explained (F-3)   <- the long step
  step 5b the five reproductions that step 5 supplies (F-3 -> PR-MSG-REPRODUCE)
  step 6  verify
```

Three constraints fix this order.

**Split the rename before the rebase.** Git detects the three renames in `ed2c8df034` at **53 %,
56 % and 64 %** similarity. A rebase replays that commit over 99 commits of master, and a few more
changed lines take it under the detection threshold, which ends the `git blame` history of three
central TCP files. Splitting first gives a pure-rename commit that rebases with no detection at
all.

**The baselines come after the rebase.** `tests/fingerprint/showcases.csv` is one of the three
conflicting paths, so master has re-recorded rows the branch also re-records. Every value measured
before the rebase is stale.

**One history pass, not three.** F-4, F-5 and F-7 all edit the same series. Doing them together
costs one rebase; doing them apart costs three, and each one risks the rename detection again.

**Five reproductions wait for step 5.** Commits 44, 45, 49, 50 and 51 each move fingerprint rows,
and the row analysis step 5 must do anyway *is* the reproduction those commits owe. Writing them
before step 5 means writing them twice.

## 4. The steps

### Step 1 — One history pass

Work in a dedicated worktree, on a copy of the branch, so the original stays reachable.

**1a. Split `ed2c8df034` (F-4).** Two commits: a rename-only commit, then the content rewrite.

**Six files move, not three.** The audit's first two passes counted the three renames git detects;
preparing this step found three more that git does not, and whose history has therefore already
ended at this commit:

| From | To | git sees it |
| --- | --- | --- |
| `TcpBaseAlg.h` | `TcpAlgorithmBase.h` | `R056` |
| `TcpBaseAlgState.msg` | `TcpAlgorithmBaseState.msg` | `R053` |
| `TcpTahoeRenoFamilyState.msg` | `TcpClassicAlgorithmBaseState.msg` | `R064` |
| `TcpBaseAlg.cc` | `TcpAlgorithmBase.cc` | **no** — 39 %, under the 50 % default |
| `TcpTahoeRenoFamily.h` | `TcpClassicAlgorithmBase.h` | **no** — not at `-M10%` either |
| `TcpTahoeRenoFamily.cc` | `TcpClassicAlgorithmBase.cc` | **no** |

`Changes-20051129.txt` is deleted and is not a rename; it stays in the content commit.

**Three commits, and the first one will not compile.** Renaming `TcpBaseAlg.h` while it still
declares `class TcpBaseAlg` breaks every file that includes it, so a content-free move cannot
build. [PR-SERIES-BUILDS](../../doc/project/rule/pull-request.md#pr-series-builds) was given an
exemption for exactly this on 2026-09-11: **a rename git cannot see costs the file its history
permanently, and a broken middle commit costs one `git bisect skip`.**

```
tcp: location: move the TcpBaseAlg and TcpTahoeRenoFamily files to their new names
    Change: src.tcp.flavours | location | - | tcp-algorithm-split
tcp: name: rename TcpBaseAlg and TcpTahoeRenoFamily for the split architecture
    Change: src.tcp.flavours | name | whatsnew migration | tcp-algorithm-split
tcp: refactor: move the classic flavours onto the split architecture
    Change: src.tcp.flavours | refactor | - | tcp-algorithm-split
```

The first commit is `git mv` and nothing else, so every one of the six is `R100`. The second
substitutes the two type names everywhere and restores the build. The third is the architecture
change that is the point of the series.

The body of the move commit says it does not build and why, so a bisecting reader reaches for
`git bisect skip` instead of a bug report.

**Done when** `git show --name-status -M` on the move commit shows six `R100` lines and no `A`/`D`
pair, the tree builds in both modes **after the rename commit**, and `check-commits.sh` no longer
flags `PR-SPLIT-MOVE`.

**1b. Add the deprecation alias (F-2, part).** In the rename commit, one line per renamed type:

```cpp
using TcpBaseAlg = TcpAlgorithmBase;            // deprecated, remove after the next release
using TcpTahoeRenoFamily = TcpClassicAlgorithmBase;
```

That satisfies [RR-DEPRECATE-FIRST](../../doc/project/rule/release.md#rr-deprecate-first) and keeps
every out-of-tree TCP algorithm compiling for one release.
**Done when** a translation unit that names `TcpBaseAlg` still compiles against the branch.

**1c. Make `ITcpRecovery` pure (F-5).** Amend `96b310f5f3`, the commit that introduces the
interface — not a fix-up at the end, which
[PR-SERIES-ORDER](../../doc/project/rule/pull-request.md#pr-series-order) forbids.

Remove the five `{}` bodies from `segmentsAcked`, `dataSent`, `segmentRetransmitted`,
`onRexmitTimeout` and `reoTimeout`, and add an empty override in each of `Rfc5681Recovery`,
`Rfc6582Recovery` and `Rfc6675Recovery` where the behavior really is nothing. Each override then
states its own answer, and a later rename of any of the five breaks an out-of-tree implementor
loudly instead of silently dropping its override.

This is a **new interface with no out-of-tree implementor yet**, so this is the cheapest moment it
will ever have.
**Done when** `check-interfaces.sh --verbose` reports 16 at the head, down from 17 — the remaining
one is `IIeee80211Band`, which master repaired after this branch's base and which this branch does
not touch.

**1d. Write the bodies (F-7). — done 2026-09-15.** An attempt on 2026-09-14 found that the
reasons are not recoverable per commit, because the commits were not per reason (F-10). Steps 1i
and 1j made them so, and then each body wrote itself from its feature.

Twenty-two bodies were wrong rather than missing after the two re-cut steps: they claimed content
that had moved to another commit. The RACK commit still promised the queue's lost mark and
`getRecovery()`; the sizing remainder still promised the socket options, the zero-copy record and
the Linux congestion state; the signals commit still claimed a move it only half made. Each of
those now says what its commit holds, and each names where the piece came from, so a reader who
remembers the original branch can follow the move.

One commit had no body at all -- the plumbing remainder. Its body is a list of fourteen unrelated
changes, and it says so: **it is the commit to split next if this series is cut again.**

One commit was split. The clamp of an explicit read at a received FIN is a repair of a defect
older than the branch, and it rode inside the commit that adds the zero-copy completions and the
receive timestamp tag. A feature commit that also carries a fix cannot state one intent, and
[CR-DEPTH-FIX](../../doc/project/rule/classification.md#cr-depth-fix) says which one it must
state, so the clamp is its own `fix:` commit now. **The series is 75 commits.**

**Done:** `check-commits.sh` reports no `PR-MSG-BODY` violation.

**1e. Add the `Change:` trailers. — done 2026-09-15.** All 75, the 61 of the branch from the
reconstruction in `audit/sweep/classification-on-tcp-new.md`, which already wrote them, and the
rest from the re-cut. The subject of every commit carries its kind, which the reconstruction
supplies as well.

This is not tidiness. With the trailers in place,
[CR-OBL-INERT](../../doc/project/rule/classification.md#cr-obl-inert) guards step 5: a commit that
claims no behavior change and carries a baseline row fails the gate. Without them, step 5 has no
mechanical check at all.

**Done:** `check-classification.sh` passes over the series -- trailer, form, subject, area, one
depth per commit, and inertness. `check-commits.sh` passes too, except the two baseline-only
commits that [PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline) refuses;
they are F-3 and step 5 owns them. Fifteen subjects were over the 80-character limit and are
shorter now.

**1h. Declare every parameter where it is first read (F-9). — done 2026-09-14.**

Eight commits read 62 of the branch's 67 new parameters before any NED declares them, so
seventeen commits compile and no TCP simulation runs across them. Each declaration moved to the
commit that first reads it, grouped under a `// parameters of: <subject>` header.

The whole series was rebuilt tree by tree rather than replayed as patches, so there were no
conflicts and every file except `Tcp.ned` is byte-identical to before.
[check-ned-params.sh](../../doc/project/enforcement/check-ned-params.sh) went from 8 violations to
`PASS`, and commit 35 shrank from declaring 63 parameters to changing 12 defaults — which is
exactly the second half of its own subject.

**1h2. Declare every state field where it is first used (F-9). — done 2026-09-14.**

The state is the worse half. **104 of the 175 state fields the branch adds are used in code before
any `.msg` declares them**, which is a compile error rather than a run-time throw, so **commits 11
to 33 do not build.**

The same tree-by-tree rebuild moved each field's declaration to the commit that first uses it,
under a `// declared here for the code above that already uses it:` header. **The final tree is
byte-identical to the audited head** — the repair adds nothing at the end, only earlier.

**1h3. Repair the four forward includes (F-9). — done 2026-09-14, and it was not small.**

Commits 11 to 14 include `TcpClassicAlgorithmBase.h`, `TcpAlgorithmBase.h` and `TcpCubic.h`
before those files exist, which
[check-includes.sh](../../doc/project/enforcement/check-includes.sh) now reports. Phase B was
written against the names the rename would later give the files.

Two ways to fix it, and the second is better. Point the includes at the names that exist at those
commits and let the rename commit update them — which is what the rename commit is for, and it
already touches those files' neighbours. The other way, moving the rename earlier, reorders the
series for a smaller reason than it deserves.

The gate also reports the repair's own move commit, commit 15, and that one is correct: a move
that changes no content cannot fix its own includes, and
[PR-SERIES-BUILDS](../../doc/project/rule/pull-request.md#pr-series-builds) exempts it.

**What it took.** Three of the four were the rename, and pointing them at the names that exist
before commit 16 was mechanical. The fourth was not: `Rfc5681Recovery.cc` at commit 11 includes
`TcpCubic.h` because it sniffs the concrete type —

```cpp
// TODO move this to a derived class or use function pointer for ssthresh calculation?
if (TcpCubic *tcpCubic = dynamic_cast<TcpCubic *>(conn->getTcpAlgorithmForUpdate()))
```

— and `TcpCubic` arrives at commit 14, which itself needs commits 12 and 13. **A cycle no
reordering can break.** The series already contains the repair, at commit 57, *"pick the
fast-retransmit ssthresh by virtual, not by concrete-type sniffing"*, so the fix was to apply
commit 57's virtual at commit 11 and carry it through the 46 commits between. The author's own
`TODO` says they knew.

**Result:** the include gate goes from 5 violations to 1, the one being the exempt move commit,
and commit 11 goes from failing after 200 sources to compiling 1685.

**1h4. The fifth shape: symbols, not files. — commit 11 done, the rest open.**

Building the repaired commit 11 got 1685 sources in and then failed on six symbols:
`getBytesInFlight`, which enters `TcpAlgorithm.h` at commit 19, and `cwndSignal` and
`ssthreshSignal`, which exist at commit 9 but whose include is not added until commit 17.

Both were repaired for commit 11, and **commit 11 now builds**: the signals include was added
where the signals are used, and `getBytesInFlight` moved to its first user together with the two
implementations `TcpBaseAlg` and `DumbTcp` owe it.

`check-series-builds.sh` then measured the rest. Commits 10 and 11 build; 12 fails on
`TcpSackRexmitQueue::markHeadLost`, 13 on `TcpAlgorithm::calculateSsthreshForFastRecovery` and
`TcpSackRexmitQueue::updateLost`, and 14 onward cascade.

**Continuing the same move failed, and the failure is the answer.** Moving those three methods to
their first users was tried on 2026-09-14 and reverted. It did not converge:

- `markHeadLost` and `updateLost` are **`protected` at the commit that declares them** and public
  later, so moving the declaration text also moves its access, and commit 12 traded *"no member
  named `markHeadLost`"* for *"`markHeadLost` is a protected member"*.
- `markHeadLost`'s body reads `Region::lost`, a struct field added later still, so the method
  drags a field, which drags whatever the field's users need.
- Commit 13 then wanted `getRegion` as well, and commit 17 revealed `override` markers on three
  methods that are not yet virtual.

**A method does not move alone.** Its dependency closure at commit 12 reaches well into commits 13
to 34, so each move opens two more. That is not a patch that is merely long; it is the re-cut
wearing a disguise, and doing it one symbol at a time hides the design decisions inside mechanical
edits.

**So the loop stops here, at commits 10 and 11 building.** The rest belongs to step 1i, where the
unit of work is a feature rather than a symbol, and where somebody decides what `TcpSackRexmitQueue`
should expose and when — which is the question `markHeadLost` being protected was really asking.

This is the same defect one layer deeper, and it is the point at which patching stops paying.
Each shape repaired reveals the next, because they are all the one fault: **phase B was written
against the finished tree.** Step 1i is the repair; 1h4 is not a separate task but a measure of
how much of phase B the re-cut has to touch.

**1i. Re-cut phases B and C by feature (F-10). — done 2026-09-15, 15 of 15.** The list is
fifteen features, not thirteen: keepalive, delayed ACK, TCP_NOTSENT_LOWAT, timing, receive buffer,
reordering, TLP, F-RTO, undo, PRR, RACK, segment sizing, PSH and cork, Fast Open, Accurate ECN.

**Keepalive — done 2026-09-15.** The feature was in three places: its parameters, state, parameter
reads, establish-time arming and handler body inside *"move the classic flavours onto the split
architecture"*, and its probe sender inside a later commit named *"keepalive probing"* that also
carried the cork flush, the RTO loss marking and a rewrite of `sendProbe()`. It is now one commit,
`tcp: add: keepalive probing (RFC 1122 4.2.3.6)`, +85 / −12 across five files, and the commit
that had its name is renamed for the three things it still holds.

Verified three ways: the head tree is unchanged apart from one comment line of the parameter
header; no commit before the feature commit references its state, parameters or probe; and
**commit 19's compile-error set is byte-identical before and after** — the same 65 distinct
errors — so the removal took nothing the refactor needed. The timer, its dispatch and an empty
handler stub predate the branch and stay where they are.

**Delayed ACK — done 2026-09-15.** The author's one subject covered two things: making the
classic path's frame count and timeout parameters, and the Linux adaptive receiver (ATO, quickack
budget, pingpong mode). The adaptive logic was in the refactor across eleven functions of
`TcpAlgorithmBase` — four whole functions and seven with a block each — its two parameters in
*"size segments"*, and its timeout parameter in the commit named for it, which mostly carried the
retransmission and persist timer parameters. It is now one commit, +205 / −4 across six files;
the interim commits get the pre-branch constant and threshold back; and the commit that had its
name holds the seven timer parameters under an honest subject until the timing feature moves them.
Verified the same three ways: head identical, zero references before the feature commit, and the
same 65-error set at commit 19.

**A regression of my own, found and fixed 2026-09-15.** The include repair of step 1h3 inserted
`calculateSsthresh` into `TcpAlgorithm.h` after the line `receivedDuplicateAck() = 0;`, and
commit 22 removes that line, so the insertion silently skipped commits 22 to 60 — thirty-nine
commits calling a virtual their base class did not declare. The script used `if` where every
other rule used `assert`. It is now anchored on `calculateSsthreshForFastRecovery()`, which the
earlier move placed in every commit from 13 on, and the declaration is present from 13 through
the head with no gap; commit 61, which originally added it, no longer touches the file.

**TCP_NOTSENT_LOWAT — done 2026-09-15.** Parameter, state, handler, initialisation and the
send-side check were in *"size segments"*; the connection member and the arm-on-enqueue in the
commit named for socket options. One commit, +39 across five files. The command type and
`TcpSocket::setNotsentLowat()` stay in the socket-contract commit, which is what that commit is
for. The commit that had the feature's name is renamed for the members, cork hooks and rx
timestamping it still holds.

**Retransmission, persist and handshake timing — done 2026-09-15.** The largest so far, and the
first with a behavior change buried in the refactor: the refactor replaced five constants with
members *and changed three of their values* (RTO floor 1 s → 200 ms, ceiling 240 s → 120 s, and
the Linux `SRTT + max(4·RTTVAR, floor)` formula), and it silently fixed a persist-timer slip that
wrote the clamped value into `rexmit_timeout`. The interim commits get the merge-base code back
at six sites; the feature commit, +99 / −59 across five files, says which defaults move and that
the persist fix is folded in; the SYN parameters come from *"judge loss by transmission time"*
and `synLinearTimeouts` from a later commit of the same name, which is renamed for the STATUS
change it still holds. Verified at commit 19 with locale-safe set comparison: the six timer-member
errors are gone and nothing else changed.

**Two more regressions of my own, both found and fixed 2026-09-15.** The state-field pass moved
`fastopenCookieToSend` to commit 20 without the `typedef` and `@existingClass` line its type needs,
so the message compiler failed at commits 20 to 26 and no C++ built there at all — every "build"
of commit 20 before the fix compiled zero sources, including one I had reported as an identical
error set. Both lines now move with the field. And the include repair's `calculateSsthresh`
insertion silently skipped commits 22 to 60, recorded above.

**A latent defect the timers cut exposed, not caused.** Six errors about `rttSignal`, `rtoSignal`
and their siblings appear at commit 19 once the timer errors are gone. Nothing
`TcpAlgorithmBase.cc` includes at 19 reaches `TcpSimsignals.h`, so they were undeclared before
too and clang's recovery had hidden them. The include arrives with a later commit; the repair is
the same as the signals pass already done for the flavour files, extended past the refactor.

**Receive buffer — done 2026-09-15.** The feature was in four commits: its three parameters,
three state variables and their initialisation in *"size segments"*; the STATUS line in *"judge
loss by transmission time"*; the occupancy accounting, the over-accept flag and the growth of the
offer on arrival in *"undo a reduction"*; and the members, the release on read, the offer
arithmetic and the acceptance check in the commit of its name. It is now one commit, +245 / −2
across six files, built as commit 28 plus the owner's pieces. The commit that had its name keeps
the MSG_EOR, PSH and zero-copy sets, the sndbuf-limited chrono, the receive-timestamp tag and a
clamp of explicit reads at a received FIN, until the cuts of those features take them. **The FIN
clamp is a fix, and step 1f owes it a reproduction.**

Verified with complete error sets, the first cut so verified (see the third lesson below). The
strip removes exactly seven errors at commit 28 — the six members and `appOwned`, which sits
inside the moved block — and adds none. The feature commit adds exactly the two forward
references the original commit also had: `appOwned`, declared in the socket-option commit, and
`getAcknowledgedDataLength`, declared in the plumbing commit. And the feature commit and the
original differ by exactly the remainder's six declarations.

**Reordering — done 2026-09-15.** The learning, its state and the changed loss rule sat in the
RFC 6675 engine of *"move SACK loss recovery into Rfc6675Recovery"*, the two parameters and their
reads in *"size segments"*, the STATUS line in *"judge loss by transmission time"*, and only the
new signature of `setSackedBit()` in the commit of its name — which declared the value it returns
and never computed or returned it. The computation was written into *"connection plumbing the
modern features share"*, eighteen commits later; in between the function had no return statement.
It is now one commit, +93 / −6 across eight files, built as commit 21 plus the feature, and it
carries the computation with the signature. The interim engine uses `dupthresh` where the learned
degree goes; the SACK-side F-RTO check that reads the same value is out of commits 15 to 24 and
first appears in the F-RTO commit. The commit that had the name keeps the callback rename and
`addInferredSack()`, which nothing calls until the plumbing commit.

Verified with complete sets: the strip removes exactly one error at commit 21 — the `void` result
assigned to `uint32_t` — and adds none; the feature commit adds and removes nothing; and the
feature commit and the original differ by exactly the rename's effect, two stale callers on one
side and seventeen flavours that already use the new names on the other. That rename is needed
from commit 12 on and moves there with the forward-reference pass.

Two findings for the audit report: the return value that no commit computed for eighteen commits
(a defect of the original series, repaired by the re-cut), and the old doc comment of
`setSackedBit()` left above the new one, still in the head.

**Tail Loss Probe — done 2026-09-15.** The timer, its arming, its handler and the outcome check
were in the refactor with the state; the parameter and its read in *"size segments"*; the
declaration of the probe sender in *"judge loss by transmission time"*; and the sender itself in
the commit of its name, which otherwise carried the Fast Open cookie cache and the SYN-SENT paths
that use it. One commit, +203 / −1 across seven files. The probe's delayed-ACK allowance read
`minRexmitTimeout`, a member the timing commit introduces later in the series; the interim uses
the constant the timing commit then replaces. The remainder is the Fast Open material, named as
such, for the Fast Open cut to absorb.

**F-RTO — done 2026-09-15.** The episode, its state and the cumulative check were in the engine
of commit 15; the parameter in *"size segments"*; the SACK-side check moved with the reordering
cut. One commit, +85 / −1 across seven files. Its undo of a spurious timeout calls
`resetLostBit()`, which only the plumbing commit defined; the definition comes with its caller
now, as `setSackedBit()`'s body did. The commit of its name held one AccECN fix, which stays as a
one-hunk fix commit for the AccECN cut to absorb.

**Loss undo — done 2026-09-15.** The sender side — undo context, D-SACK counting, the Eifel test,
the restore — was in the engine of commit 15 with its state; the parameters in *"size segments"*;
the per-segment reset in the PRR commit; and the receiver side — the D-SACK gate, the first
duplicate range, the duplicate SYN or FIN — in the commit of its name, mixed with AccECN option
emission, the full-close reset, a SYN-ACK for a retransmitted SYN, the callers of the renamed
callbacks and a reordering of the state struct. One commit, +195 / −4 across eight files. The
lookup of the first duplicate range was declared there and defined in the plumbing commit; it
comes with its declaration. `lastRcvdTSecr` stays where it is: the SYN-ACK timestamp check of
commit 20 reads it too, and nothing sets it before this commit — a dead check for five commits,
noted for the segment-size cut.

Verified the same way for all three, with complete sets: TLP's strip removes exactly the
`minRexmitTimeout` error and the feature commit adds none; F-RTO's strip removes exactly the
`resetLostBit` error and, with the definition carried, the feature commit adds none; undo's strip
changes nothing and its feature commit adds nothing, its symbols having been declared at 15 by the
state-field pass. In each case the feature commit and the original differ by exactly the
remainder's content.

**PRR — done 2026-09-15.** The pacing, its state and the exit deflation were in the engine of
commit 15; the parameter in *"size segments"*; the per-segment delivered-bytes mark in the commit
of its name, which otherwise held a keepalive line and two lines of RACK bookkeeping. One commit,
+139 / −1 across six files. `prrDeliveredMark` stays declared where it is: the AccECN decode of
commit 21 reads it too, before anything sets it — dead until PRR, noted for the AccECN cut.
Verified with complete sets: all four sets are identical, so PRR had no forward reference and the
cut neither removes nor adds an error.

**RACK — done 2026-09-15.** The detection, the timer callback and the loss rule were in the engine
of commit 15 with the state and the interface method; the parameter and its reads in *"size
segments"*; the timer, the queue's lost mark, send times, transmission counts, boundaries and
totals, and the segment-boundary cap in the commit of its name, which otherwise carried the STATUS
block, Fast Open and AccECN handshake work; the bookkeeping that stamps a retransmission in the
PRR commit; the bookkeeping that stamps an original transmission, records its boundary and clears
a SACKed region's lost mark in the plumbing commit; the engine's include in the connection in the
AccECN commit; and the two interface includes the algorithm header needs in the plumbing commit.
Until the plumbing commit, RACK read send times that nothing set and a `lost` flag that nothing
initialised. One commit, +538 / −6 across thirteen files, and everything above comes with it.
The algorithm interface's `getRecovery()`, which the timer needs, comes too, with
`getBytesInFlight()` and the fast-recovery ssthresh re-homed beside it; the flavours of commits 13
to 19 already use them, and the forward-reference pass moves the interface back to where it is
first needed. The queue's lost mark and totals are likewise used by the flavours' in-flight
estimate from commit 16 on; they stay with RACK, where they were written, for that pass to
re-home. The PRR remainder is down to the keepalive line and a comment.

Verified with complete sets: the strip removes exactly the nine RACK forward references at commit
20 and adds none; the feature commit, with the includes carried, removes 23 errors the flavours
had (the interface types unknown to them) and surfaces six latent ones — four flavours abstract
until the callback rename, and `markOutstandingLostOnRto`, a connection method the plumbing
commit declares; and against the original commit 21 it lacks the remainder's nineteen and has
those six plus `deriveLinuxCaState`, whose declaration is the remainder's.

Two findings for the audit report: RACK inert for twenty-four commits (unset times, an
uninitialised flag), and in the plumbing commit's enqueue path `region.lost` is assigned twice,
the second assignment overriding the `beforeEnd` inheritance of the first — dead code that
deserves a look.

**Segment sizing — done 2026-09-15.** Commit 20, *"size segments against the space options
actually leave"*, was the parameter concentrator of the series: 785 lines, of which the sizing
itself — the effective MSS net of the timestamp option, the advertised MSS as this side's own
limit, the address-family defaults, TCP_MAXSEG, the syncookie msstab, the two option flags and
the STATUS fields — is 124. The declaration of `calculateEffectiveMss()` was in the plumbing
commit, the `userMss` member in the socket-option commit, and the two option flags in a commit
named for segment-size negotiation that held nothing else; all three come with the feature, and
that last commit, empty, is dropped. The remainder — socket options, push and cork, the send-path
rewrite, the ECN and initial-window modes, the chronos, Fast Open and AccECN parameters and
options, and the state struct — is named for what it holds and feeds the last three cuts. The
flavours of commits 13 to 19 already count with `snd_effmss`, which nothing computed until here.

Verified with complete sets: the feature commit adds no error over commit 19, and the remainder
has exactly the original's errors minus the eight that the carried declarations resolve.

**PSH and cork — done 2026-09-15.** The most spread of all: the cork timer in the refactor; the
state, the parameter, the socket-option handlers, the enqueue-time boundaries and the send-path
holds in *"size segments"*; the four PSH rules of the send path and the second-segment pairing in
*"Accurate ECN"*; the boundary sets in the receive-buffer remainder; the flush in the keepalive
remainder; the cork-timer interface and the socket-option members in the TCP_NOTSENT_LOWAT
remainder; the clamp and the pruning in the commit of its name, which otherwise held one fix.
One commit, +328 / −4 across eight files, MSG_EOR record boundaries included; the fix stays as a
one-hunk fix commit. Verified with complete sets: all four sets at commit 43 are identical, and a
second pair at commit 30 shows the strip removing fifteen forward references and adding none.

**Fast Open — done 2026-09-15.** The parameters, the state, the cookie options and the listener's
acceptance were in *"size segments"*; SYN data on the wire, the deferred SYN, the handshake ACK
that covers SYN data and the blackhole trigger in the RACK remainder; the cookie cache and the
SYN-SENT paths in the TLP remainder, which is folded in and gone. One commit, +1036 / −65 across
nine files. `fastopenAccelerated` stays declared where it is: three flavours read it before this
commit. Verified: the strip removes four forward references at commit 32 and adds none, and the
feature commit equals the original.

**Accurate ECN — done 2026-09-15.** The mode, the parameters, the state and the handshake bits
were in *"size segments"*; the ACE decode, the handshake seed and the SYN's bits in the RACK
remainder; the option emission, the retransmitted-SYN check and the D-SACK fail rule in the undo
remainder; the classic-ECE gate in a fix commit of its own, folded in and gone; the mode constants
and the decode's declaration in *"TCP Fast Open"*. One commit, +884 / −104 across eight files.
Seven fields stay declared early because the recovery engine and DCTCP read them. Verified: the
strip removes two forward references (two signals) at the Fast Open commit and adds none, and the
feature commit equals the original. The nineteen signal errors the feature commit shows are the
original commit's own: it removes the connection's static signal members and their registrations,
which is the signals commit's business, and nothing includes `TcpSimsignals.h` in their place until
the plumbing commit.

**Clean-up after the cuts.** The keepalive idle stamp, left alone in a one-line commit, is folded
into the keepalive commit; that commit is now a comment change and named so. The six remainders
that still exist — the sizing commit's, RACK's, undo's, the receive buffer's, keepalive's and
TCP_NOTSENT_LOWAT's — are renamed for what they hold now; their bodies come with step 1d. The
branch is 75 commits, the head tree identical to the audited head plus the aliases, the
relocation and the release note.

**What the forward-reference pass owned**, gathered across the cuts: the callback rename (in
the reordering remainder) and `getRecovery()` with its two siblings (in RACK) back to the
interfaces commit and the refactor; the queue's lost mark, send times and totals (in RACK) back
to the recovery engines that read them; `markHeadLost()` (plumbing) to the RFC 6582 engine; the
static-signal removal (in AccECN) to the signals commit, with the includes it needs;
`enqueueSendCommandData()`'s declaration (plumbing) to the sizing remainder that defines it; the
`const` on `getLE()`/`getRE()` (undo remainder) with their definitions (plumbing); and the
`EV_INFO` in `sendData()` that names `effectiveWin` after the rewrite removed it. **Step 1j did
all of it, and the build found eight more.**

**Two lessons for the verification itself.** `comm` on `sort -u` output under a non-C locale
reports lines as differing that are identical, and the first comparison of this cut was wrong
because of it; every comparison now runs under `LC_ALL=C`. And a build script must write its
cache marker only after a successful build, or the next run touches the wrong files and compiles
nothing. And clang stops after twenty errors in one file, and `make` without `-k` stops at the
first failure, so the set from a plain build is incomplete in two ways — **the four earlier cuts
were compared on such sets.** Their static checks were exact and step 6 builds every commit, but
the comparisons were weaker than the paragraphs above claim. Every comparison build now runs
`make -k` with `CFLAGS_EXTRA=-ferror-limit=0`, and the *did you mean* suffix is stripped before
comparing, because it changes with the declared members.

The method, kept for the record: extract every block by signature with an assertion on each
match; build the feature commit as *the previous tree plus the feature*, never as *a later tree
minus other things* — the second way carried a `sendProbe()` rewrite along until the diff showed
23 deleted lines; and prove the cut with the complete error set of the commit it came from — `make -k`, no
error limit — not with its error count.


**1j. The forward-reference pass (F-10, second half). — done 2026-09-15.** Step 1i gives every
feature one commit. It does not give every commit its declarations: the original branch declared
late what it used early, and the re-cut preserved that. A flavour overrides a method the base class
does not declare for another twenty-five commits; a connection method is defined in one commit and
declared in another; the commit that only moves two files also deletes two functions from them. The
compiler is the only instrument that finds all of this, so this step builds every commit from the
signals commit to the plumbing commit and repairs what the build reports. Every build ran `make -k`
with `-ferror-limit=0`, so every error set is complete.

**The seven items the cuts had noticed are done.** The two ACK-callback renames --
`receivedDataAck` to `receivedAckForUnackedData` and `receivedAckForDataNotYetSent` to
`receivedAckForUnsentData` -- become a `name:` commit of their own before the interfaces commit,
where the flavours written into the commits after it already expect them; 38 lines change, and 38
change back, in 21 files. `getRecovery()` and the two interface includes go to the interfaces
commit. `getBytesInFlight()`, `calculateSsthreshForFastRecovery()` and `calculateSsthresh()` go to
the RFC 5681 commit, in the order the head has them, so that no later commit re-orders the header.
The queue's lost mark and `markHeadLost()` go to the RFC 6582 engine that calls it, with the two
initializers the mark needs. `supportsSackRecovery()` goes to the RFC 6675 engine. The three queue
byte totals go to the refactor, whose in-flight formula reads them. `segmentsAcked()` goes to the
refactor that overrides it. `enqueueSendCommandData()`'s declaration goes to the commit that
defines it, and its MSG_EOR doc comment to the commit that gives it that behavior. The `const` on
`getLE()`/`getRE()` goes to the commit that introduces them, declaration and definition together.
The `EV_INFO` that still named `effectiveWin` goes to the commit whose rewrite removed the
variable. The signals commit now also removes the 26 static signal members and their 25
registrations and adds the four includes their emitters need, which is what its message always
claimed it did. The commit that only renamed the algorithm callbacks is empty and gone.

**One signal does not move with the others.** `sndNxtSignal` stays a static member of
`TcpConnection` until the plumbing commit, because `TcpSimsignals` never declares it: the plumbing
commit renames the recorded statistic from `sndNxt` to `sndSeq`, and until then the connection
emits the old handle. **The rename of a recorded statistic is a user-visible change that no commit
message mentions and `WHATSNEW` does not carry** -- a finding for the report, and for step 3.

**The build found eight more forward references.** The `location:` commit deleted
`getBytesInFlight()` and `calculateSsthresh()` from the files it moved, so seven flavours were
abstract for two commits; the two functions stay in the move now, and the `name:` commit keeps the
half it always re-added. `receivedAckForAlreadyAckedData()` and `rttMeasurementComplete()` were
declared on `TcpAlgorithm` in the plumbing commit and overridden from the refactor on, so they move
to the refactor. `TcpAlgorithm::receivedDuplicateAck()` survived until the plumbing commit although
the refactor is what retires it -- the refactor replaces `DumbTcp`'s implementation with
`receivedAckForAlreadyAckedData()`, which left `DumbTcp` abstract for twenty-five commits. The
refactor now removes the pure virtual and replaces the connection's duplicate-ACK block with the
single call the algorithm answers, which is the move the plumbing commit performed far too late.
`markOutstandingLostOnRto()` was called by two flavours from the refactor on and declared in the
commit named for that behavior, so the two calls move to that commit. `deriveLinuxCaState()` was
defined in the sizing remainder and declared in the STATUS commit, so the definition moves to
STATUS, and STATUS gains the three includes its state reads need. The receive-buffer commit gains
the window-scale line that reads `getAcknowledgedDataLength()`, that method itself, and the
ownership socket option. The zero-copy commit gains the zero-copy record in the enqueue path, the
completion notification in the send path, the two send-buffer chrono calls, the receive-timestamp
socket option and its tag, and the three tag includes. The commit that existed only to supply "the
includes the connection sources need" is empty and gone: its last three pieces are the unused
`<climits>` in the event processor, which joins the TCP_NOTSENT_LOWAT commit, an RFC 2001 citation
that should have read RFC 5681, which joins the citation commit, and one include re-ordering.
**The series is 74 commits**, and the head tree is still the audited head plus the aliases, the
relocation and the release note.

**One commit changed place.** TcpCubic is written against the split architecture: it overrides
`receivedAckForAlreadyAckedData()`, calls `ensureRexmitTimerArmed()` and `getRecovery()`, and its
state message imports `TcpClassicAlgorithmBaseState`, none of which exist before the refactor. It
sat three commits earlier, where the message compiler could not even read its state message. It now
follows the refactor. The three commits it passed lose only the lines that were CUBIC's, and the
`fastopenAccelerated` state field it used to carry now arrives with the refactor that reads it.

**What the builds say.** Every commit from the signals commit to the head builds, with one
deliberate exception: the `location:` commit moves `TcpBaseAlg` and `TcpTahoeRenoFamily` without
renaming the classes in them, so the message compiler cannot resolve the imports of five state
messages. The seal owner allowed exactly this on 2026-09-14 -- "it's ok to move and rename
separately even if it doesn't build at both commits, bisect should be able to handle that" -- and
the `name:` commit after it builds clean.

**Three more findings for the report.** `TcpSimsignals.h` declares seven signals twice (`cwnd`,
`ssthresh`, `rtt`, `srtt`, `rttvar`, `rto`, `numRtos`). `TcpConnection::tcpConnectionAddedSignal`
was declared and never defined, in master as well as on the branch; the signals commit now removes
it. And the head keeps a doubled `region.lost` assignment in the queue's enqueue path, whose
`beforeEnd` inheritance the line after it overwrites.

**1f. Add the reproduction to the fix commits
([PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce)). — done 2026-09-15.**

The rule is new, written 2026-09-11. Sixteen commits in the series carry `.fix` -- the fourteen of
the branch, the `bytes == 0` repair the PSH cut left over, and the FIN clamp that step 1d took out
of a feature commit. **All sixteen already had a body, and the bodies are among the best in the
project**; what none of them said is how to see the defect happen. Each now carries a `Reproduce:`
paragraph that names a configuration and an observable: the parameter that selects the path, the
event that triggers it, and the error text, the assertion or the statistic that shows it.

Three of them take the second form the rule allows and name the test committed beside the repair
-- `tests/module/tcp_iw_2.test`, `tcp_ecn_reno_1.test` and `tcp_fastrexmit_1.test`. Four name the
oracle run instead, which is not in this repository, so those four also state the scenario in
words.

**Five of the sixteen can be sharpened by step 5.** The lost-mark split, the `nextSeg` underflow,
the classic ECN reaction, the uncounted duplicate ACKs and the missing deflation each move
fingerprint rows, and a row that moves *is* a scenario where the defect showed. Their reproductions
stand on the code today; step 5b replaces the scenario with the configuration of the row that
moves, which is stronger evidence and costs nothing once step 5 has done the analysis anyway.

**Two are worth a standalone regression test** against the rule's own test -- a defect on a crossed
path, or one a refactor could reintroduce. The `nextSeg` underflow is unsigned arithmetic that a
future edit to `setPipe` could bring back, and the missing `state->lossRecovery` is one line that
any rework of the recovery split could drop again. **The author decides; the plan only marks
them.**

**1g. Name this plan in every commit
([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)). — done 2026-09-15.**

Every commit this plan produces or rewrites carries, above its `Change:` trailer:

```
Plan: plan/pending/pr-1155-resolve-audit-findings.md
```

That is what lets a later reader find out why commit 15 became two commits, why the baselines are
spread across eleven commits instead of two, and who decided the seal. None of that fits in a
commit body and all of it is here. All 75 commits carry the line.

### Step 2 — Rebase onto master (F-6) — done 2026-09-29

The owner lifted the hold. The branch sits on `origin/master` `49e1fa0945`, 431 commits past the
old base (not the 99 this step first counted), as **73 commits**: the 75 of step 1 less the two that
held only fingerprint values. `origin/topic/tcp-new-audit-fixes` kept the old-base series until the
force-push, and `git range-diff 434658d729..<old> origin/master..<new>` compares the two: 67
commits are identical, six differ.

**How each conflict was resolved:**

| Commit | Conflict | Resolution |
| --- | --- | --- |
| lift the seal (1) | master turned the audit column of the row from a link into a path | the commit's note replaces the row, as before |
| restore the seal (5) | none -- but it restored the row in the old form, undoing master's change | found by a comparison with master after the rebase; the row now comes back in master's form (the history pass) |
| PPP RFC 1661 (3) | master renamed the serializer hooks of all 92 serializers (`9614b9913f`) | the header serializer keeps master's names, `serializeFields()` and `deserializeFields()`; the trailer serializer goes as before |
| PPP RFC 1661 (3) | none by git: master's new serializer suite (`7287f347aa`) filled a `PppTrailer` | its filler goes with the type, the `PppHeader` filler describes the RFC 1661 header, and `Ppp.ned` no longer names `~PppTrailer` |
| TFO and AccECN options (6) | none: only hunk headers moved with the hook rename | -- |
| the refactor (19) | master fixed RFC 6298 section 2.2 on its own (`f3064a83d0`), in the file the series moves | one fix: the series' structure, master's flag `rttMeasured`; the series' unused `rtt_measured` goes. The estimator part of the commit stays a refactor, because the starting value of `rttvar` it changes is read nowhere before the first sample overwrites it |
| the two fingerprint commits (46, 74) | both touch fingerprint files that master also changed | **dropped**: their values were measured on the old base; step 5 records the baselines in the commits that move them. This also removes the two baseline-only commits of F-3 |
| cover the new behavior (47) | 7 TCP module tests, which master re-recorded for the same RFC 6298 fix | the series' expected output, which already had that fix; the module run confirms it |
| the release note (73) | master added items 2 to 16 | the TCP note follows as item 17 |

**Checks after the rebase:**

- no serializer overrides or calls an old hook; four tests of `srtt > 0` mean "a sample was
  taken", which the flag keeps true, because `rttMeasurementComplete()` is the only writer of `srtt`;
- `check-series-builds.sh`: 72 of 73 commits build; commit 17, the exempt move, fails alone;
- `check-classification.sh` and `check-commits.sh` pass, `PR-SPLIT-BASELINE` included;
- a finding older than the rebase: `TcpSimsignals.h` declares `rtoSignal`, `rttSignal` and
  `rttvarSignal` twice each.

**The tests at the head, against `origin/master`.** On master every suite passes. At the head, 21
tests fail, and all of them because #1155 meets master's newer tests:

| Group | Tests | Cause |
| --- | --- | --- |
| the modern defaults | 13 TCP standards tests, 2 protocol self tests (`CaptureWithUnit`, `ReactiveInject`) | written for the old defaults; with `tests/module/lib/tcp-legacy.ini` included, the 13 pass |
| RFC or Linux | `Rfc6298FirstMeasurement` | the series computes the timeout as Linux does, SRTT + max(4·RTTVAR, minimum); RFC 6298 section 2.4 rounds the whole value up to 1 s |
| to analyze | `Rfc5681FastRetransmit`, `Rfc9293FlowControl` | fail under both sets of defaults |
| a leak in #1155 | module test `MIPv6_tcp_handover` | a `CLOSED` indication stays undisposed; master's newer check catches it |
| PPP and AccECN in the serializer suite | `serializer_fields`, `serializer_chunk_roundtrip` | the generated `rtp-rtcp.pcap` has the old PPP framing; the filler does not fill `TcpHeader::aeBit` |

**The owner's decisions, 2026-09-29:**

1. master's TCP standards tests are **rewritten for the modern defaults**, not pinned to the old
   ones;
2. the retransmission timeout **follows RFC 6298 by default**; a test that needs Linux's formula
   overrides it;
3. this plan lives on the branch;
4. **no serializer adaptation yet**: the PPP change may still be modified.

**2a. The timeout by RFC 6298.** The default computes RTO = max(SRTT + 4·RTTVAR, the minimum), the
Linux variance floor becomes a parameter, and the packetdrill configuration in `inet-gpl` selects
it, because the Linux scripts expect it.

**2b. The standards tests for the modern defaults.** The 13 TCP tests and the 2 self tests hold
under the new defaults; a test overrides a parameter only where its check needs it, and says why.
The TCP evidence documents follow.

**2c. The three differences.** `Rfc5681FastRetransmit`, `Rfc9293FlowControl` and the MIPv6 leak are
analyzed, and each one is repaired in the commit that caused it, or recorded.

**Still open from this step:** the force-push of the branch, after the owner's confirmation; then
the rebase of `topic/tcp-packetdrill-tests` onto the result; and the serializer adaptation,
deferred by decision 4.

### Step 3 — The release note (F-2) — done 2026-09-14

`WHATSNEW` is untouched today and `doc/src/` gains 15 lines, none of which mention a break. Four
kinds of break need naming:

1. **Removed signal and statistic** — `sndNxt`.
2. **Renamed types** — `TcpBaseAlg`, `TcpTahoeRenoFamily`; step 1b's alias makes this a deprecation
   rather than a break, and the note says when the alias goes.
3. **Removed public surface** — 25 functions removed outright, 22 more moved with their renamed
   class, and `PppTrailer` removed. The exact list is the *Breaking* section of
   `audit/pull-request/pr-1155-summary.md`; do not
   retype it, generate it.
4. **Changed defaults** — `tcpAlgorithmClass` `"TcpReno"` → `"TcpCubic"`; `sackSupport`,
   `timestampSupport`, `delayedAcksEnabled`, `limitedTransmitEnabled` `false` → `true`; `mss` `536`
   → `-1`; `advertisedWindow` `14 * this.mss` → `65535`. **This is what moves the 53 fingerprints**,
   so this entry and step 5 must agree.

A migration guide entry under `doc/src/migration-guide/` carries 2 and 3;
[RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate) asks for the repair
instructions, not only the announcement.

**Done when** a user who upgrades can find, for each of the four, what to change in their own code
or ini file. — **done.** Writing it corrected the finding twice: the default list was missing
`windowScalingSupport`, and `sndNxt` is renamed to `sndSeq` rather than removed. A migration guide
entry turned out not to be owed: `doc/src/migration-guide/` is a 3.x-to-4.x document, and the
release's own instructions belong in `WHATSNEW` beside the announcement, which is where the IEEE
802.11 entry puts them.

**Note for the project, not for this branch:** this same finding has now appeared in five pull
requests from two authors. Five occurrences say the obligation is not visible where it is incurred,
and the change summary already computes the exact list a `T3` check would need. That check is out
of scope here and belongs in its own plan.

### Step 4 — Unseal, edit, reseal (F-1) — done 2026-09-15

**The seal owner asked for this twice and explicitly**, so the branch does it literally: the seal
is lifted before the validated edits and restored after them. Two commits were added, and the
branch is now 66 commits:

| # | Commit | Seal on `common/packet/` |
| ---: | --- | --- |
| 1 | `seal: lift the common/packet/ seal for three validated edits` | **lifted** |
| 2 | `packet: keep the fill byte when splitting a BitCountChunk or ByteCountChunk` | lifted |
| 3 | `ppp: follow RFC 1661 rather than RFC 1331` | lifted |
| 4 | `pcap: record PPP traces as LINKTYPE_PPP rather than LINKTYPE_PPP_WITH_DIR` | lifted |
| 5 | `seal: restore the common/packet/ seal` | **restored, byte for byte** |

Both sealed-path edits sit inside the window and nothing else does. The row goes back exactly as
it was, against the same audit and the same accepted exceptions, so the head tree is unchanged by
the pair.

The row is removed rather than marked, because absence from the list *is* the unsealed state and
the registry keeps no open rows
([SR-DEFAULT-OPEN](../../doc/project/rule/sealing.md#sr-default-open)). A note stands in its place
during the window so a reader of those four commits sees why the seal is gone.

**Two things this does not do**, both stated in the commit messages rather than left to be
discovered:

- **It does not authorize the merge.**
  [SR-PR-APPROVAL](../../doc/project/rule/sealing.md#sr-pr-approval) binds that to a protected
  workflow and a required reviewer at the exact head, and
  `check-source-seals.sh --base origin/master` reads the **base** branch's seal list, so a
  branch-side lift is invisible to it by design: a branch cannot authorize itself.
- **It does not decide `AV-ORG-01` and `AV-ORG-02`.** Both are still `Open (decide)`, so the
  restored seal rests on two clusters the ledger calls invalid, which
  [SR-AUDIT-FIRST](../../doc/project/rule/sealing.md#sr-audit-first) forbids. True before this
  branch, true after it.

**The cost of doing it this way**, for the record: between commits 1 and 5 the path is unsealed
for everyone, not only for the three files. That is why the window is four commits long and no
longer.

### Step 5 — The baselines, every row explained (F-3)

**D-2 is decided: the hard way.** Every commit that moves a fingerprint carries the new values, and
every moved row is explained.

Eleven commits are in scope — nine that own a `fingerprint` and two that own a `?` and must be
settled either way:

| # | Commit | What it changes | Expect |
| --- | --- | --- | --- |
| 1 | `ed742203a8` | the fill byte of a split count chunk | `?` — settle it |
| 2 | `a705db8a75` | PPP for RFC 1661, `PppTrailer` removed | every row with a PPP link |
| 7 | `747ec0f435` | byte-align a truncated packet | `?` — settle it |
| 33 | `e24da2c9d4` | the modernized defaults | **the large one** — most of the 53 |
| 40 | `e04d92d9c3` | the CUBIC HyStart delay detector | rows that run CUBIC |
| 44 | `4190b1977e` | the lost mark across a SACK queue split | rows with SACK loss |
| 45 | `f2b5fd13b7` | the RFC 6675 `nextSeg` rule (2) underflow | rows with SACK loss |
| 49 | `37d121af2e` | the classic RFC 3168 ECN reaction | rows with ECN |
| 50 | `5d94c07e75` | duplicate ACK counting for every flavour | rows with loss |
| 51 | `7d52875f17` | non-SACK Reno cwnd deflate | rows with non-SACK Reno |
| 52 | `fea41a027a` | `TcpCubic` on `TcpClassicAlgorithmBase` | rows that run CUBIC |

**The method, per commit**, once the branch sits on the new master:

1. Run the fingerprint suite at that commit.
2. Take the rows that move, and only those. A row that moves here and was already moved by an
   earlier commit in the series belongs to the earlier one.
3. **Explain each row from the diff.** Which behavior in this commit changes the trajectory this
   row records. Group the rows that share an explanation — *"the 31 rows under `examples/inet/`
   all carry TCP traffic with the default algorithm, which this commit changes from `TcpReno` to
   `TcpCubic`"* is one explanation for 31 rows and it is complete.
4. **A row you cannot explain stops the step.** It is an unintended change until shown otherwise,
   and finding one is this step earning its cost.
5. Put the rows and the explanation in that commit.

**The expectation column above is a prediction, not a result.** A row that moves where the column
says it should not is the interesting case, and it is the reason to run all eleven rather than
assume the defaults commit owns everything.

**Reuse the text of `33e8b0d073`.** It is the best baseline provenance this audit has seen — the
tip built in a separate workspace, the suite run there, 53 rows named by family, and a record of
what it deliberately did not touch. That text becomes the explanation of commit 33 and the source
of the per-commit wording for the rest.

**A note on verification, from 2026-09-14.**

**The branch head builds: 1738 sources compiled, 0 errors.** So neither repair broke the final
tree, which is the claim that matters most.

**No intermediate commit has been built, and the attempt did not converge.** A per-commit build in
this worktree is defeated by `make` reporting nothing to do: it writes to `src/out`, not `out`,
and a target library left in `src/` from an earlier attempt makes the whole tree look current.
Three runs at commit 11 reported two compiler errors in `TcpSocket.cc` naming
`TcpSetPathMtuCommand` and `TcpSetRcvBufCommand` — **those errors are not real.** Commit 11's
`TcpSocket.cc` contains neither name, and both the use and the declaration of each arrive together
in a later commit. The errors came from a mixed tree that the build left behind, and they are
recorded here only so nobody re-discovers them as a finding.

So the two repairs rest on **static** evidence: every parameter and every state field is declared
at the commit that uses it, across all 63 commits, by
[check-ned-params.sh](../../doc/project/enforcement/check-ned-params.sh) and a matching pass over
the state messages.

**The build recipe, which now works.** From the repository **root**, never from `src/`, because
the root makefile generates `src/inet/features.h` and the sub-make does not:

```bash
export PATH=/home/levy/workspace/omnetpp/bin:$PATH OMNETPP_ROOT=/home/levy/workspace/omnetpp
export LD_LIBRARY_PATH=/home/levy/workspace/omnetpp/lib:$LD_LIBRARY_PATH
make MODE=release -j32
```

Three traps cost this session an hour. Objects go to **`src/out`**, not `out`. A `libINET.so`
left in `src/` by an earlier attempt makes the whole tree look current, so `make` exits 0 having
done nothing. And `git checkout` leaves the mtime of an unchanged file alone, so after moving to
an older commit the files that differ must be touched before `make` will rebuild them:

```bash
git diff --name-only HEAD <newer-ref> -- 'src/*' | xargs -r touch
```

With that, the head builds — **1693 sources, 0 errors** — and a per-commit build works, which is
what unblocked 1h3 above.

**Done when** every commit that moves a recorded expectation carries it with a row-level
explanation, `check-commits.sh` reports no `PR-SPLIT-BASELINE` violation, and
`check-classification.sh` reports no `CR-OBL-INERT` violation.

### Step 5b — The five reproductions that step 5 supplies

Commits 44, 45, 49, 50 and 51 owe a reproduction under
[PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce) and each moves
fingerprint rows. Step 5 identifies and explains those rows, and a row that moves is a scenario in
which the defect showed. Lift the configuration from the row analysis into the body.

**This is the one place where two obligations pay for each other**, and it is why the plan
separates these five from the nine in step 1f.

**Done when** each of the five names the configuration its own moved rows identify.

### Step 6 — Verify

**The series build ran on 2026-09-23**, ahead of the rebase and on the old base (step 2 is held):
`check-series-builds.sh` over all 75 commits, release mode, 12 jobs under `nice`. **73 of 75
build.** The two that fail:

- **Commit 17**, the move of the six `TcpBaseAlg` and `TcpTahoeRenoFamily` files. Expected, and its
  body says so: the moved files still declare the old types until commit 18 renames them.
  [PR-SERIES-BUILDS](../../doc/project/rule/pull-request.md#pr-series-builds) exempts a commit
  whose diff is only moves.
- **Commit 18**, the rename -- **a false failure, and a defect of the gate.** Commit 18 builds from
  the head's state and on the path that `git bisect skip` takes past 17 (16 then 18: status 0). It
  fails only after the failed build of 17: the message dependency files that commit 16 wrote
  (`*_m.h.d`) still name `TcpBaseAlgState.msg` and `TcpTahoeRenoFamilyState.msg`, the failed
  message compilation of 17 never rewrites them, and at 18 `make` runs `opp_msgtool` on two files
  that no longer exist. Removing the `*_m.h.d` files before the build makes 16, 17, 18 give 0, 2,
  0. The gate already deletes every generated `_m` file before each commit, so every message is
  compiled again anyway and removing its dependency file costs nothing. **The owner approved the
  repair, and it is in `check-series-builds.sh` since 2026-09-23**; over commits 16 to 18 the gate
  now reports ok, FAIL, ok. **The series therefore builds, except the one move that the rule
  exempts.**


```bash
export PATH=/home/levy/workspace/omnetpp/bin:$PATH        # REQUIRED — F-8
MB=$(git merge-base HEAD origin/master)
doc/project/enforcement/check-commits.sh         $MB..HEAD
doc/project/enforcement/check-classification.sh  $MB..HEAD
doc/project/enforcement/check-interfaces.sh --verbose
doc/project/enforcement/check-source-seals.sh --base origin/master
git merge-tree --write-tree HEAD origin/master
opp_summarize_changes --repo . --pr 1155 --usage -o pr-1155-summary.md
```

By hand, because no gate reaches them:

- every `.fix` commit names a configuration and an observable
  ([PR-MSG-REPRODUCE](../../doc/project/rule/pull-request.md#pr-msg-reproduce));
- every moved fingerprint row has an explanation, grouped where the explanation is shared
  ([TR-BASELINE-PROVENANCE](../../doc/project/rule/testing.md#tr-baseline-provenance));
- every commit carries `Plan: plan/pending/pr-1155-resolve-audit-findings.md`
  ([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)).

Then a fourth audit pass, which should turn F-1 to F-7 into `PASS`. **Move this file to
`plan/done/` when it does**, and the `Plan:` lines still resolve, because the path in a commit
message is read against the tree at that commit.

## 5. F-8 is separate, and it is not in this repository

`opp_summarize_changes` produced **210 added** where the truth is **547**, with an empty stderr and
exit status 0, because `opp_nedtool` and `opp_msgtool` were off the `PATH`. It removed every NED
and message fact — 337 of 547 — while its own preamble claimed those parsers had supplied them.

Two repairs in `opp_repl`:

1. **Fail loudly.** The tool checks that every parser it names is runnable, and stops if one is
   not.
2. **Record the provenance.** The generated preamble names the parser versions that actually ran,
   so a reader sees their absence.

**Both done 2026-09-23**, `opp_repl` commit `215d07b` on `main`, not pushed. The repair found a
second silent path: on a syntax error both parsers write the XML of the files they could read and
exit with status 1, and the extractor checked only that the XML existed, so the facts of a broken
file vanished and its types read as removed. Now:

| Situation | Before | After |
| --- | --- | --- |
| a parser is not on the `PATH` | a note at the end, exit 0 | stops before any git work, exit 2 |
| a parser exits non-zero | nothing, exit 0 | stops, names the file, exit 1 |
| `--allow-incomplete` (new) | — | writes the report; it opens with **This report is incomplete** and one line per gap |
| the preamble | asserts both parsers as sources | names the version and directory of each parser that ran |

On #1155: 547 added with the parsers, exit 2 without them, and 210 added under the incomplete
heading with `--allow-incomplete` -- the audit's two numbers, now told apart by the report itself.

The `PATH` export stays in every command line, as step 6 and
[review-a-pull-request.md](../../doc/project/guide/review-a-pull-request.md) have it: without it
the tool now stops instead of lying, but it still needs the parsers.

## 6. Effort, and where it goes

| Step | Size | What dominates |
| --- | --- | --- |
| 1a, 1b, 1c | small | mechanical, an hour each at most |
| 1d | medium | **the author's knowledge**, not typing; nobody else can write these twelve |
| 1e | small | the trailers already exist in the trial report |
| 1f | medium | nine reproductions now, five after step 5; the bodies exist and none names a configuration |
| 1g | small | one line per commit, added in the same pass |
| 2 | medium | two real source conflicts in `PppHeaderSerializer`, plus a build in both modes |
| 3 | medium | the generated *Breaking* list makes it writing rather than discovery |
| 4 | small | **unblocked** — the seal owner validated the three edits |
| 5 | **large** | eleven fingerprint runs, and then a row-level explanation for each; the suite is the clock and the analysis is the work |
| F-8 | small | two changes in `opp_repl` |

**Nothing gates the start any more.** Both decisions are made, and steps 1, 2 and 3 can begin
immediately. Step 5 is the clock: eleven fingerprint runs and eleven explanations.

## 7. Risks

| Risk | What happens | What reduces it |
| --- | --- | --- |
| the rebase loses the rename detection | the `git blame` history of three central TCP files ends at this branch | step 1a, done before step 2 |
| master advances again while this runs | step 2 and step 5 both go stale | do steps 2 to 5 in one stretch; the branch has already waited 99 commits |
| the twelve bodies get written as restatements | F-7 closes on paper and the history stays uninformative | the question table in step 1d; a body that answers none of it has not closed anything |
| a moved row gets explained by assertion | *"the defaults changed"* covers 53 rows and proves nothing; an unintended change rides through | step 5 point 4 — a row you cannot trace from the diff **stops the step** |
| the eleven runs find rows nobody predicted | step 5 grows, and the branch may hold a defect | that is the step working; the expectation column is a prediction and a miss is the finding |
| the reproductions get written from the code | a body that re-derives the bug from the diff is the body that is already there; the rule asks for a configuration and an observable | step 1f names what is missing per commit; five come from step 5's rows, which are evidence and not argument |
| the summary is regenerated without the parsers | the report states 210 where the truth is 547 | the `PATH` export in step 6, until F-8 lands |

## 8. Out of scope

- **The `T3` release-note check** that F-2's five occurrences argue for. It is a project change, not
  a branch change, and it needs its own plan.
- **`AR-EXT-MINIMAL-SURFACE` and `AR-EXT-VIRTUAL-IS-A-PROMISE`** — 28 uncalled and 95 unoverridden
  at the head. The audit records these as *questions to the author*, not findings, and answering
  them is review conversation rather than plan work.
- **The two unsanctioned `AV-ORG` clusters themselves.** D-1 settles what this branch does — unseal,
  edit, reseal — and leaves the clusters `Open (decide)`. Repairing them is the seal owner's work
  and it needs its own plan.
