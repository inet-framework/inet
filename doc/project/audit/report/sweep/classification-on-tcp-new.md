# Sweep report — the `CR-*` classification applied to `topic/tcp-new`

> **Kind:** report · **Status:** snapshot 2026-09-11 · **Seal:** none · **Owns:** — · **Stands on:** [rule/classification.md](../../../rule/classification.md), [rule/pull-request.md](../../../rule/pull-request.md)

- **Corpus:** `origin/topic/tcp-new`, 61 commits, merge base `434658d729`, 183 files,
  +15840 / −3685. The branch of [pull request #1155](../pull-request/pr-1155.md).
- **What this is:** every commit of the branch, rewritten as its author would write it under
  [classification.md](../../../rule/classification.md). **The branch is not changed.** This
  document exists to answer one question: does the classification earn its cost on real work?
- **Method:** the area, the depth for levels 0 to 3, the renames, the baselines and the body
  lengths come from the diff. The behavior claim, the intent and the group are my reading of the
  commits, which is what the author would state from knowledge.
- **Since the trial:** depth level 4 was renamed from `structure` to `refactor`, because the word
  carries the promise that the level makes and `structure` does not. The trailers below use the
  new name.

## The verdict first

**The classification pays for itself, and not where I expected.**

| What the trailer buys | Verdict |
| --- | --- |
| The subject shortening of [CR-TAG-SUBJECT](../../../rule/classification.md#cr-tag-subject) | **Almost worthless here.** See [finding 1](#f-1--the-subject-shortening-buys-almost-nothing-on-this-branch). |
| The depth field | **The strongest of the four.** 6 of 61 commits need no code review, and the trailer says so in one word. [Finding 2](#f-2--the-depth-field-is-the-strongest-of-the-four). |
| The obligation field | **Strong, and it exposes a defect the branch already has.** [Finding 3](#f-3--the-obligation-field-exposes-a-defect-the-branch-already-carries). |
| The group field | **Strong on this branch and weak on a short one.** [Finding 4](#f-4--the-group-is-strong-here-and-would-be-weak-on-a-short-branch). |
| The whole trailer as a review aid | It finds **five rule breaks that no gate finds today**. [What it caught](#what-the-classification-caught). |

The cost is one line per commit, and about half of the value arrives from the two fields the author
cannot get wrong by accident — depth and obligation.

## The distribution

The depth of the 61 commits, as the trailers below state it:

| Depth | Commits | Which |
| --- | --- | --- |
| `comment` | 4 | 8, 43, 57, 59 |
| `format` | 0 | — |
| `location` | 0 | — |
| `name` | 0 pure | commit 15 mixes it with `refactor` |
| `refactor` | 7 | 13, 15, 54, 55, 56, 58, 60 |
| `behavior` | 50 | the rest |

By area: **`src` 56, `tests` 3, `doc` 1, cross-area 1.**

Of the 50 behavior commits, **19 are `fix`** and 31 are deliberate. **26 add** something, **21
change** something, **3 remove** something.

## The messages

Every commit, in branch order, with the subject as the author would write it and the trailer as
[CR-TAG-FORM](../../../rule/classification.md#cr-tag-form) defines it. A ⚠ marks a commit whose
trailer exposes a rule break; the findings below name each one.

### Phase A — the prerequisites (1 to 8)

```
packet: keep the fill byte when splitting a BitCountChunk or ByteCountChunk

Change: src.common.packet.Chunk | behavior.change.fix | ?

ppp: follow RFC 1661 rather than RFC 1331

Change: src.linklayer.ppp.PppHeader | behavior.change | fingerprint whatsnew

pcap: record PPP traces as LINKTYPE_PPP rather than LINKTYPE_PPP_WITH_DIR

Change: src.common.packet.recorder.PcapWriter | behavior.change.fix | -

tcp: add the TCP Fast Open and AccECN header options, and the AE bit

Change: src.tcp.TcpHeader | behavior.add | - | tcp-modern-features

tcp: extend the socket contract with the commands, tags and status fields

Change: src.tcp.TcpSocket | behavior.add | whatsnew | tcp-modern-features

tcp: allow the receive-window and timestamp parameters to change at runtime

Change: src.tcp.Tcp | behavior.change | whatsnew | tcp-modern-features

ppp: byte-align a packet truncated by a mid-transmission disconnect

Change: src.linklayer.ppp.Ppp | behavior.change.fix | ?

doc: refresh the TCP RFC citations to the current documents                  ⚠

Change: doc examples | comment | -
```

Commits 1, 2, 3 and 7 carry **no group**, and that is the information. They are `packet`, `ppp` and
`pcap` work riding in a TCP branch. An empty fourth field beside 53 commits that carry one is a
[PR-SPLIT-DRIVEBY](../../../rule/pull-request.md#pr-split-driveby) question that a reviewer can
ask without reading a diff.

### Phase B — the algorithm split (9 to 15)

```
tcp: collect the TCP signals into one place                                  ⚠

Change: src.tcp.TcpSimsignals | behavior.add | - | tcp-algorithm-split

tcp: introduce the congestion-control and recovery interfaces                ⚠

Change: src.tcp.ITcpCongestionControl | behavior.add | - | tcp-algorithm-split

tcp: extract the RFC 5681 congestion control and fast recovery               ⚠

Change: src.tcp.Rfc5681CongestionControl | behavior.add | - | tcp-algorithm-split

tcp: extract the RFC 6582 (NewReno) recovery                                 ⚠

Change: src.tcp.Rfc6582Recovery | behavior.add | - | tcp-algorithm-split

tcp: move SACK loss recovery into Rfc6675Recovery

Change: src.tcp.Rfc6675Recovery | refactor | - | tcp-algorithm-split

tcp: add TcpCubic (RFC 9438) with HyStart

Change: src.tcp.TcpCubic | behavior.add | test | tcp-modern-features

tcp: move the classic flavours onto the split architecture                   ⚠

Change: src.tcp.flavours | name+refactor | whatsnew migration | tcp-algorithm-split
```

### Phase C — the modern features (16 to 32)

Every commit of this phase adds a mechanism that no parameter reaches yet. The parameters arrive in
commit 33. So the whole phase is inert, and its trailers say `-` seventeen times in a row.

```
tcp: size segments against the space options actually leave                  ⚠

Change: src.tcp.TcpConnection | behavior.change | test | tcp-modern-features

tcp: judge loss by transmission time (RFC 8985)                              ⚠

Change: src.tcp.Rfc8985Recovery | behavior.add | test | tcp-modern-features

tcp: learn how far the path reorders                                         ⚠

Change: src.tcp.TcpConnection | behavior.add | test | tcp-modern-features

tcp: pace the window down across recovery (RFC 6937)                         ⚠

Change: src.tcp.Rfc6937ProportionalRateReduction | behavior.add | test | tcp-modern-features

tcp: undo a reduction that turned out to be unnecessary                      ⚠

Change: src.tcp.TcpLossUndo | behavior.add | test | tcp-modern-features

tcp: recognise a premature retransmission timeout (RFC 5682)                 ⚠

Change: src.tcp.TcpConnection | behavior.add | test | tcp-modern-features

tcp: probe the tail rather than waiting out the timeout                      ⚠

Change: src.tcp.TcpConnection | behavior.add | test | tcp-modern-features

tcp: TCP Fast Open (RFC 7413)                                                ⚠

Change: src.tcp.TcpConnection | behavior.add | test whatsnew | tcp-modern-features

tcp: Accurate ECN                                                            ⚠

Change: src.tcp.TcpConnection | behavior.add | test whatsnew | tcp-modern-features

tcp: separate the receive buffer from the advertised window                  ⚠

Change: src.tcp.TcpReceiveQueue | behavior.change | test | tcp-modern-features

tcp: adapt the acknowledgement delay to the connection                       ⚠

Change: src.tcp.TcpConnection | behavior.add | test | tcp-modern-features

tcp: keepalive probing (RFC 1122 4.2.3.6)                                    ⚠

Change: src.tcp.TcpConnection | behavior.add | test whatsnew | tcp-modern-features

tcp: the socket options the new behavior exposes                             ⚠

Change: src.tcp.TcpSocket | behavior.add | whatsnew | tcp-modern-features

tcp: mark write boundaries with PSH                                          ⚠

Change: src.tcp.TcpSendQueue | behavior.add | test | tcp-modern-features

tcp: segment-size negotiation and path MTU handling                          ⚠

Change: src.tcp.TcpConnection | behavior.add | - | tcp-modern-features

tcp: retransmission, persist and handshake timing                            ⚠

Change: src.tcp.TcpAlgorithmBase | behavior.change | test | tcp-modern-features

tcp: connection plumbing the modern features share                           ⚠

Change: src.tcp.TcpConnection | refactor | - | tcp-modern-features
```

### Phase D — expose, default, record, follow (33 to 37)

```
tcp: expose the new behavior as parameters, and modernize the defaults       ⚠

Change: src.tcp.Tcp | behavior.add+change | fingerprint whatsnew migration | tcp-modern-defaults

tests: re-record the fingerprints the modernized defaults move               ⚠

Change: tests.fingerprint | behavior.change | - | tcp-modern-defaults

tests: cover the new behavior, and pin the old defaults where tests predate them  ⚠

Change: tests | behavior.add+change | - | tcp-modern-defaults

sctp: follow the TCP algorithm interface rename

Change: src.sctp | name | - | tcp-algorithm-split

tcpapp: let applications drive the new socket behavior

Change: src.applications.tcpapp | behavior.add | whatsnew | tcp-modern-features
```

### Phase E — the defects the review and the oracle surfaced (38 to 56)

```
tcp: three Fast Open fixes the oracle surfaced                               ⚠

Change: src.tcp.TcpConnection | behavior.change.fix | test | tcp-review-fixes

tcp: finish the Fast Open option-form fallback, and the server's pre-ACK window  ⚠

Change: src.tcp.TcpConnection | behavior.change.fix | test | tcp-review-fixes

tcp: match Linux's CUBIC HyStart delay detector

Change: src.tcp.TcpCubic | behavior.change.fix | fingerprint whatsnew | tcp-review-fixes

tcp: RFC 4821 packetized Path MTU discovery

Change: src.tcp.TcpConnection | behavior.add | test whatsnew | tcp-modern-features

tcp: model the receive buffer's memory limits, not just its byte count

Change: src.tcp.TcpReceiveQueue | behavior.add | test whatsnew | tcp-modern-features

tcp: document every RFC the module implements, and where it deviates

Change: src.tcp.Tcp | comment | -

tcp: keep the lost mark when a retransmission splits a SACK queue region

Change: src.tcp.TcpSendQueue | behavior.change.fix | fingerprint | tcp-review-fixes

tcp: clamp the snd_wnd-pipe underflow in RFC 6675 nextSeg rule (2)

Change: src.tcp.Rfc6675Recovery | behavior.change.fix | fingerprint | tcp-review-fixes

tcp: drop unusable SACK options instead of aborting the simulation

Change: src.tcp.TcpConnection | behavior.change.fix | - | tcp-review-fixes

tcp: serialize the experimental TCP Fast Open option (kind 254)

Change: src.tcp.TcpHeaderSerializer | behavior.add | test | tcp-review-fixes

tcp: compare increasedIWEnabled against initialWindow's real default

Change: src.tcp.TcpAlgorithmBase | behavior.change.fix | test | tcp-review-fixes

tcp: wire the classic RFC 3168 ECN reaction back into the ACK path

Change: src.tcp.TcpClassicAlgorithmBase | behavior.change.fix | test fingerprint | tcp-review-fixes

tcp: count duplicate ACKs for every flavour again, not just the classic ones

Change: src.tcp.TcpAlgorithmBase | behavior.change.fix | fingerprint | tcp-review-fixes

tcp: deflate cwnd when non-SACK Reno leaves fast recovery

Change: src.tcp.Rfc5681Recovery | behavior.change.fix | test fingerprint | tcp-review-fixes

tcp: put TcpCubic on TcpClassicAlgorithmBase so it gets the shared plumbing

Change: src.tcp.TcpCubic | behavior.change | fingerprint | tcp-review-fixes

tcpapp: let TcpServerSocketIo keep a half-closed connection open again

Change: src.applications.tcpapp.TcpServerSocketIo | behavior.change.fix | - | tcp-review-fixes

tcp: drop the unused PRR entry helper and explain the dupack design it hints at  ⚠

Change: src.tcp.Rfc6937ProportionalRateReduction | refactor | - | tcp-review-fixes

tcp: pick the fast-retransmit ssthresh by virtual, not by concrete-type sniffing

Change: src.tcp.TcpAlgorithmBase | refactor | - | tcp-review-fixes

tcp: stop rescanning the SACK scoreboard several times per ACK

Change: src.tcp.Rfc6675Recovery | refactor | - | tcp-review-fixes
```

### Phase F — the close (57 to 61)

```
tcp: document why DcTcp's ACK path is a fork, not a specialization

Change: src.tcp.DcTcp | comment | - | tcp-algorithm-split

tcp_lwip: record the sent-seqno statistic under the same name as tcp

Change: src.tcp_lwip.TcpLwip | name | whatsnew | tcp-algorithm-split

tcp: drop test-corpus provenance from code comments

Change: src.tcp | comment | -

tcp: fold DcTcp onto the shared ACK path, leaving only what is DCTCP's

Change: src.tcp.DcTcp | refactor | - | tcp-algorithm-split

tests: re-record the fingerprints this TCP workstream moves                  ⚠

Change: tests.fingerprint | behavior.change | - | tcp-review-fixes
```

## What the classification caught

Five rule breaks, none of which a gate finds today.

| Break | Where the trailer shows it |
| --- | --- |
| **17 commits carry no body at all** ([PR-MSG-BODY](../../../rule/pull-request.md#pr-msg-body)) | commits 16 to 32, every one of them `behavior` with a `test` obligation |
| **A rename rides inside a content change** ([CR-DEPTH-ONE](../../../rule/classification.md#cr-depth-one), [PR-SPLIT-MECHANICAL](../../../rule/pull-request.md#pr-split-mechanical)) | commit 15, the only `name+refactor` in the branch |
| **Two baselines are detached from their causes** ([PR-SPLIT-BASELINE](../../../rule/pull-request.md#pr-split-baseline)) | commits 34 and 61, the only two trailers whose area is `tests` and whose depth is `behavior` |
| **Two subjects carry an "and"** ([PR-SPLIT-ONE-CHANGE](../../../rule/pull-request.md#pr-split-one-change)) | commits 33 and 35, the only `behavior.add+change` trailers |
| **Tests arrive 20 commits after the behavior** ([TR-SHIP-WITH](../../../rule/testing.md#tr-ship-with)) | 17 `test` obligations in phase C, all discharged by commit 35 |

The last three are the point. **A `test` obligation that no commit in the same phase discharges is
a pattern that a reader sees in one column and that no diff shows.** Seventeen of them in a row is
not a judgment call.

## Findings

### F-1 — The subject shortening buys almost nothing on this branch

**Rule:** [CR-TAG-SUBJECT](../../../rule/classification.md#cr-tag-subject). **Finding.**

The rule says the subject need not repeat what the trailer states. Applied here, it permits
dropping the `tcp:` prefix from 47 subjects. **I kept it in every one, and I think that is the
right call.**

The reason is a hole in my own rule. `git log --oneline` shows the subject and nothing else. **The
trailer is invisible in the one view the subject exists to serve.** So "the trailer already says
it" is not an argument for dropping the area from the subject; it is an argument for dropping it
from everywhere a reader can see it.

What the relaxation is genuinely for is the **kind marker**, and this branch has none to drop. The
measurement behind the rule stands — 1.3% of subjects carry a kind marker — but the conclusion was
larger than the evidence.

**What would close it:** narrow `CR-TAG-SUBJECT` to the kind marker, and say plainly that the area
prefix stays, because `git log --oneline` cannot see the trailer. The freedom the rule grants is
real and it is small.

### F-2 — The depth field is the strongest of the four

**Rule:** [CR-DEPTH-SCALE](../../../rule/classification.md#cr-depth-scale). **Note.**

Four commits are `comment` and three are `refactor` with no observable change. **Seven of 61
commits need no behavioral review**, and one word in the trailer says which. Commit 43 alone is 326
insertions and 105 deletions in `Tcp.ned`, and every changed line is a comment.

This is also where the area-against-depth split earns its keep. Under the original eight-category
list, commit 43 would land in "documentation only", beside commit 8, which is a real `doc/` change.
They are not the same thing: one is `src` at depth 0 and the other is `doc`. A reviewer who skips
"documentation" skips a 431-line change to a NED file.

### F-3 — The obligation field exposes a defect the branch already carries

**Rule:** [CR-OBL-BASELINE](../../../rule/classification.md#cr-obl-baseline). **Blocking, and
already reported** as F-3 of [pr-1155.md](../pull-request/pr-1155.md).

Commit 61 says it re-records "the fingerprints this TCP workstream moves". Fifty-three fingerprint
rows change in it. **The branch cannot say which of the nineteen preceding fix commits moves which
row**, because they were batched. When one of those rows is later found wrong, `git bisect` lands
on commit 61, which is the one commit that explains nothing.

Writing the obligation field forces the question at the moment the author still knows the answer.
That is the whole mechanism, and on this branch it fires nineteen times.

### F-4 — The group is strong here and would be weak on a short branch

**Rule:** [CR-GROUP-LABEL](../../../rule/classification.md#cr-group-label). **Note.**

Four groups fit this branch: `tcp-algorithm-split`, `tcp-modern-features`, `tcp-modern-defaults`
and `tcp-review-fixes`. The single most useful thing the labels do is **reconnect commit 60 to
commits 9 to 15**, which are forty-five commits earlier. "Fold DcTcp onto the shared ACK path"
finishes the architecture split, and nothing except the label says so.

The second most useful thing is the **absence** of a label on commits 1, 2, 3 and 7.

But note what would happen on a five-commit branch: one group, equal to the pull request, and the
field carries nothing. **The group earns its line on a long branch and wastes one on a short
branch**, which is why the field is optional and why it must stay optional.

### F-5 — One commit crosses two areas, and the rule has no room for it

**Rule:** [CR-SCOPE-AREA](../../../rule/classification.md#cr-scope-area). **Finding against the
rule, not against the branch.**

Commit 8 refreshes RFC citations in two `.rst` files under `doc/` and in five `omnetpp.ini` files
under `examples/`. Every changed line is a comment. The commit is correct, and
[PR-SPLIT-MECHANICAL](../../../rule/pull-request.md#pr-split-mechanical) positively wants a
mechanical sweep in one commit. `CR-SCOPE-AREA` says one area, so the rule is wrong here.

**What would close it:** a clause on `CR-SCOPE-AREA` saying that a commit at depth `comment` or
`format` may list more than one area, because a mechanical sweep is defined by its rule and not by
its area. That is one sentence and it removes a false positive from every future citation sweep.

### F-6 — Four subjects say "extract" where the diff only adds

**Rules:** [CR-DEPTH-SCALE](../../../rule/classification.md#cr-depth-scale),
[PR-MSG-SUBJECT](../../../rule/pull-request.md#pr-msg-subject). **Finding.**

Commits 9, 10, 11 and 12 say "collect", "introduce" and "extract". Each one adds files and deletes
nothing:

| Commit | Insertions | Deletions | Files |
| --- | --- | --- | --- |
| 9 `f9c4974726` | 102 | 0 | 2 added |
| 10 `96b310f5f3` | 88 | 0 | 2 added |
| 11 `c816bf1fbe` | 321 | 0 | 4 added |
| 12 `3119cd72a2` | 216 | 0 | 2 added |

Nothing is extracted. New code appears beside the old, unreached, and commit 15 switches over. The
strategy is sound. **The subjects describe the intent of the phase and not the content of the
commit**, and the depth field is what makes the gap visible: a reader who sees `behavior.add` next
to the word "extract" asks the right question.

## What I would change in classification.md

**One change has been applied, and it did not come from this trial.** Depth level 4 is now
`refactor` and not `structure`: the word already means "no behavior change" to every reader, and
it is the word INET writes — 71 of master's last 3000 subjects carry it. `location` and `name`
stay as the two refactors that earn their own level.

Three more amendments, all small, all produced by this trial. **None is applied.**

1. **Narrow `CR-TAG-SUBJECT` to the kind marker** (F-1). The area prefix stays, because
   `git log --oneline` cannot see the trailer.
2. **Let depth `comment` and `format` list more than one area** in `CR-SCOPE-AREA` (F-5).
3. **Allow `name+refactor` and the other mixed depths to be written**, rather than forcing the
   author to pick the deeper one. Commit 15 is honest as `name+refactor` and misleading as either
   half. [CR-DEPTH-ONE](../../../rule/classification.md#cr-depth-one) still calls the mix a
   violation; the trailer should be able to state the violation rather than hide it.

## What this trial did not test

- **The cost to an author writing forwards.** I classified 61 finished commits with the whole
  branch in view. An author classifies one commit with none of it.
- **Whether a gate can parse the trailers.** No gate exists.
- **Whether the inert claims are true.** Seven commits claim no observable change. Only a run
  proves that, and [CR-OBL-INERT](../../../rule/classification.md#cr-obl-inert) is the check that
  would do it.
