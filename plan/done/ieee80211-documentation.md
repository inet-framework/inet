# The IEEE 802.11 documentation: one catalog, three documents

Status: **done** — written 2026-09-21, implemented 2026-09-22 on the branch
`topic/ieee80211-documentation` in the worktree `inet-ieee80211-documentation`, from `master` at
`35014ca19d` (the eight commits of the group on top of `origin/master` `98117c3257`). The series is
one commit per file plus the plan commits; every commit carries the plan trailer and the group.

## 1. What this plan does

The 802.11 model has three documents now, written in one week from the same code study:

| Document | Kind | Reader | Lines |
| --- | --- | --- | --- |
| `doc/src/developers-guide/ch-80211.rst` | prescriptive design | a C++ developer who extends the model | 743 |
| `doc/src/users-guide/ch-80211.rst` | configuration and use | a simulation user, at the NED and ini level | 918 |
| `doc/project/design/ieee80211-anatomy.md` | survey of the code at one commit | a reviewer who checks the code | 692 |

The developer's guide is the one that must last, and it is the one that reads as a flat list. Its
sections mix three kinds of thing: the parts of the model, the patterns that repeat inside the
parts, and the properties that apply to every part. A reader cannot tell which is which.

This plan gives the three documents one skeleton, the **catalog** of §3. The developer's guide is
restructured on it. The user's guide follows it at the depth a user controls. The anatomy is marked
as the snapshot it is, and it is not restructured.

## 2. What exists now

**The developer's guide** (`ch-80211.rst`, 743 lines, three dot figures) has these sections, in
order: design goals; reference model; the programming interface; the network interface with the
means of communication; the MAC with the coordination function, the ownership of state and a
frame's journey; management and agent; the physical layer; initialization and lifecycle; extension
points; where the code differs. §7 maps each of them to the new structure.

**The user's guide** (`ch-80211.rst`, 918 lines) has: overview; nodes; operation modes; management
and agent parameters; MAC with nine subsections; physical layer; LLC, portal and classifier;
limitations; examples. It follows the catalog roughly already.

**The anatomy** (`ieee80211-anatomy.md`, 692 lines, status `draft`) describes the code at commit
`4548adeb04` with file and method names. The user calls it a survey: a snapshot at one moment,
which becomes unnecessary when the developer's guide is complete.

**The upstream model architecture** (`doc/project/design/ieee80211-model-architecture.md`, 94
lines, on master since 2026-09-18, by Miguel González López) prescribes four kinds of 802.11 state
(catalog, capability, control, status), their owners, their lifetime, and four kinds of
notification. The domain rules and the review guide cite it. It contradicts the developer's guide on
three points, listed under D-2 in §5.

## 3. The catalog

Three kinds of axis, each with one job.

### 3.1 Two axes divide the model into parts

**Layer** is the position in the stack: LLC, MAC, PHY, and the medium below them. It matches the
node anatomy and the reference model of the standard.

**Plane** is the kind of work. The *data plane* carries MSDUs, frame by frame, on a time scale of
microseconds to milliseconds. The *management plane* sets up and keeps the relation to a BSS: scan,
authentication, association, beacons, on a scale of seconds. The standard draws the same line
between the MAC data service and the MLME, PLME and SME. The code draws it too: management frames
have their own gate pair on the MAC.

Every module falls into one cell. The grid is the top-level catalog:

| Layer | Data plane | Management plane |
| --- | --- | --- |
| across layers | — | the agent (the SME); the MIB (shared state) |
| LLC | classifier; LLC or portal | — |
| MAC | codec and dispatcher; coordination function; tx, rx, ds | the management module (the MLME) |
| PHY | transmitter, receiver, error model, PHY header | radio configuration (the PLME); the catalog of modes, bands and channels |
| medium | radio medium | — |

### 3.2 Three patterns repeat inside every cell

They are not parts. They explain why parts come in pairs and in families, and they become table
columns at the second level.

- **Direction: originator and recipient.** Every layer has a transmit side and a receive side:
  encapsulate and decapsulate, the two data services, tx and rx, transmitter and receiver. In the
  management plane the station initiates and the access point responds. The source folders are
  named after this axis.
- **Decision and mechanism.** Each part decides or executes. The agent decides, the management
  module executes. A policy decides, a procedure executes. Rate control decides, rate selection
  applies. Almost every replaceable slot is on the decision side, so this axis tells a developer
  where to plug in.
- **Variant.** Parts come in families by generation or by fidelity: DCF and EDCA, the non-QoS and
  QoS class pairs, PHY families from DSSS to VHT, radios from unit disk to bit level, full and
  simplified management. This axis explains the parallel class families and the slots.

### 3.3 Four views apply to every part

They are properties of the whole model, not places in it, and each gets one section of its own.

| View | Question | Holds |
| --- | --- | --- |
| state | who owns what, for how long | the four state kinds and their owners; the ownership table |
| communication | how parts talk | the means, the peer lookup rule, the programming interface |
| time | what happens when | init stages, lifecycle operations, the time scales of the planes |
| observation | what the model tells the outside | signals and statistics |

### 3.4 One axis per cell at the second level

Each cell divides along the axis that matters for it.

The MAC data plane divides by **function**, which was the key idea of the 2015 design: "when may I
transmit" is separate from "what do I send".

| Function | Question | Originator | Recipient | From DCF to EDCA |
| --- | --- | --- | --- | --- |
| access | When may I transmit? | channel access, contention, TXOP | NAV and medium state in rx | one Dcaf becomes four Edcaf and a collision controller |
| exchange | What do I send and expect? | frame sequence, handler, RTS | ACK, CTS, block ack responses | DcfFs becomes HcfFs and TxOpFs |
| transformation | How does an MSDU become MPDUs? | sequence number, A-MSDU, fragmentation | duplicate removal, reordering, defragmentation, deaggregation | aggregation and reordering are added |
| reliability | Did it arrive, and what if not? | ack handler, recovery, block ack agreement | block ack record and agreement | per-category counters, block ack |
| rate and protection | How fast, and how long to reserve? | rate selection, rate control, duration field | response rate | QoS rate selection, single protection |

The management plane divides by **role** (station, access point, ad hoc) and by **procedure**
(discovery, join, upkeep). The PHY divides by **fidelity** (unit disk, scalar, dimensional, bit
level) and by **PHY family**. The LLC divides by **encapsulation** (LPD, EPD, portal).

### 3.5 How deep each document goes

| Level | Content | Developer's guide | User's guide | Anatomy |
| --- | --- | --- | --- | --- |
| 1 | the grid, the patterns, the views | yes | the grid only, as NED submodules | yes |
| 2 | one axis per cell; parts with their contract and slot | yes | the parts that have parameters or a typename | yes, with file names |
| 3 | one class per row; calls, callbacks, line references | no | no | yes |

The candidates that are not top-level axes, and why: *communication means* is a property of the
connections, so it is a view; *state kind* cuts across modules, and the MIB holds all four kinds,
so it is a view; *standard clauses* follow the history of the standard, not the code; *NED
containment* is the order of zoom, which the levels follow anyway.

## 4. The two guides: arc and outline

Legend for the outlines: **[F]** figure, **[T]** table, **[L]** list, **[I]** ini fragment,
**[P]** prose. Prose is at most a few short paragraphs per item; the tables and lists carry the
detail. The section names are the ones the documents will use.

### 4.1 The developer's guide

**The arc.** The reader first learns why the model has the shape it has, then gets the map, then
walks the map along the path of a frame, then sees the four properties that hold everywhere, then
learns where to plug in, and last learns where the code is not there yet. Each part assumes only
the parts before it. A reader who stops after part 1 can place any class of the model; one who
stops after part 2 can read any file; one who stops after part 3 can change one. The contracts
view in part 3 is the guard against interfaces that serve a technicality: it lists every interface
with the concept it stands for, and part 4 states the rule for a new one.

```
1. At a glance
   1.1 Purpose and reader                      [P] what the chapter is; the other two documents
   1.2 Design goals                            [P][L] the two goals; the three rules
   1.3 The map                                 [F] the grid: layers × planes, parts in the cells
                                               [T] the catalog: one row per cell, one sentence each
   1.4 Three patterns in every cell            [T] direction, decision, variant; one example per layer
   1.5 The model and the standard              [T] cell → clause of IEEE Std 802.11-2024
   1.6 The amendments and the code             [T] amendment letter → what it adds, name prefix in the
                                                   code, parts, status (modeled, partial, placeholder,
                                                   not modeled)

2. The parts, cell by cell
   Data plane, top down — the path of a frame
   2.1 The network interface                   [F] the wiring of the seven submodules
                                               [T] slot, contract, responsibility
                                               [P] the three paths that do not mix; the MIB pointer
   2.2 The LLC                                 [T] LPD, EPD, portal; the one-method contract
   2.3 The MAC                                 [F] codec and dispatcher, coordination function, tx, rx, ds
                                               [T] part, contract, responsibility
                                               [P] the coordination function as the unit of replacement
       2.3.1 Access                            [T] parts by direction and variant; the contention machine
       2.3.2 Exchange                          [F] the coordination function along the function axis
                                               [T] the frame sequences; the steps and combinators
       2.3.3 Transformation                    [T] originator and recipient steps in order; the policies
       2.3.4 Reliability                       [T] ack handler, recovery, block ack; per-variant counters
       2.3.5 Rate and protection               [T] rate selection, rate control, duration field
       2.3.6 A frame's journey                 [L] transmit in ten steps; receive in six
   2.4 The physical layer                      [T] what 802.11 adds to the generic radio: five items
                                               [T] radio variants by fidelity; PHY families
   2.5 The medium                              [P] registration, delivery, the filters
   Management plane
   2.6 The management module                   [T] variants by role; the procedures each one runs
                                               [P] what a variant must keep true in the MIB
   2.7 The agent                               [P] decides; speaks primitives only
   2.8 Radio configuration                     [P] the PLME: the command path through the MAC
   Across layers
   2.9 The MIB                                 [P] the one shared object; who writes which field (pointer to 3.1)

3. The views
   3.1 State                                   [T] the four kinds: catalog, capability, control, status
                                               [T] shared data: datum, kind, owner, writers, readers, lifetime
                                               [L] private data: one sentence per owner
   3.2 Communication                           [T] the means, and where each one is used
                                               [L] the peer lookup rule: the three allowed ways
                                               [T] the programming interface: tags, commands, primitives
   3.3 Time                                    [T] init stages: what a part may assume after each
                                               [T] lifecycle: part, stage, start, stop, crash
                                               [P] the two time scales of the planes
   3.4 Observation                             [T] signals that carry control, with their listener
                                               [L] signals that feed statistics only
   3.5 Contracts                               [T] the four kinds of interface: standard concept,
                                                   model decision, process boundary, callback
                                               [T] every C++ interface: kind, clause, implementing
                                                   family, callers, NED slot, purpose

4. Extension points
   4.1 Decisions to replace                    [T] policies, rate control, classifier, agent: contract and slot
   4.2 Variants to add                         [T] coordination function, channel access, frame sequence,
                                                   management variant, LLC, PHY mode, error model
   4.3 When a new interface is allowed         [L] the rule of D-6, with the mode set as the counter-example

5. Where the code differs from this design     [L] the known gaps, with a pointer to the anatomy
```

### 4.2 The user's guide

**The arc.** The reader first gets a working network with four ini lines, then learns which
choice comes first (the operation mode, because it decides which modules exist), then tunes the
data plane function by function in the same order as the developer's guide, then the radio, then
sees what the model records and what it does not do. The map is the same grid, shown as NED
submodules, and every section name matches a cell or a function of the developer's guide. Nothing
below the NED and ini level appears.

```
1. Overview
   1.1 What the model covers                   [L] PHY modes, access methods, operation modes, features
   1.2 A first network                         [I] four lines: medium, SSID, defaultSsid, opMode
   1.3 The interface and its submodules        [T] submodule, default type, role  (the grid as NED)
   1.4 The three interface parameters          [L] opMode, bitrate, address

2. Nodes and the medium
   2.1 Node types                              [L] WirelessHost, AdhocHost, AccessPoint
   2.2 Several interfaces per node             [P] numWlanInterfaces, mobility
   2.3 The radio medium                        [P] one per network; scalar or dimensional

3. Operation modes  (the management plane)
   3.1 The management variants                 [T] module, role, behaviour
   3.2 Infrastructure mode                     [L] the join in five steps
                                               [P] channels: AP fixed, station scans
                                               [I] two access points and a scanning station
   3.3 Simplified infrastructure mode          [P][I]
   3.4 Ad hoc mode                             [P][I]
   3.5 Management and agent parameters         [L] per module, with defaults

4. The MAC  (the data plane, function by function)
   4.1 The MAC parameters                      [L] qosStation, fcsMode, mtu, ...
   4.2 Where the settings live                 [T] setting → path under mac.dcf and under mac.hcf
   4.3 Channel access                          [P][L][I] DCF: contention window, DIFS, queue
   4.4 Quality of service                      [T] access categories, priorities, defaults
                                               [L] classifiers   [I] EDCA
   4.5 Frame exchange and protection           [P][I] RTS/CTS threshold
   4.6 Fragmentation and aggregation           [L][I] thresholds; A-MSDU policy; A-MPDU not available
   4.7 Acknowledgement and retries             [P] retry limits, timeouts
   4.8 Block acknowledgement                   [P][I] agreements, thresholds
   4.9 Bit rates and rate control              [L] rate selection parameters   [L][I] the three algorithms
   4.10 Statistics                             [L] by module

5. The physical layer
   5.1 Radio variants                          [T] radio, medium, signal representation
   5.2 PHY modes, bands and channels           [T] opMode → PHY, band, rates   [P] channel numbers   [I]
   5.3 Radio parameters                        [L] power, sensitivity, thresholds, error model, energy   [I]

6. LLC, portal and classifier                  [P]

7. Limitations                                 [L] what is not modeled; two behaviours that differ

8. Examples and showcases                      [L]
```

The user's guide keeps its current sections; the change is the order and the names inside part 4,
so that access, exchange, transformation, reliability and rate follow each other as in the
developer's guide. The current chapter has them as channel access, QoS, acknowledgement, RTS/CTS,
fragmentation, aggregation, block ack, rate.

## 5. Decisions

### D-1 — the axes — **decided 2026-09-21**

Layer and plane at the top; direction, decision and variant as patterns; function only inside the
MAC data plane. The user confirmed the axes on 2026-09-21.

### D-2 — the guide and the upstream model architecture — **decided 2026-09-21**

The upstream document is used **to the extent that the code implements it**. The guide adopts its
vocabulary of four state kinds — *catalog*, *capability*, *control*, *status* — and classifies the
data of the model elements by kind. It does not restate contracts that the code does not meet.

What this means for the three points where the upstream text and the guide differ:

| Point | Upstream says | The code does | The guide says |
| --- | --- | --- | --- |
| mode set | a read-only provider; static dependencies are queried, not sent by a signal | the MAC publishes the set with `modesetChanged`; every part caches the pointer | the catalog is the mode set; the MAC is its owner and publishes it by signal |
| lifecycle | each owner defines its stop, crash and restart | only the MAC, the management module and the radio have hooks; the MAC's stop and crash are empty | the owners that have hooks, and what the MAC does not do yet, under "where the code differs" |
| MIB access | typed queries and transition operations; changes announced at the MIB commit boundary | parts hold a pointer to the MIB; management writes the fields; management emits the signals | the MIB fields with their writer and readers, as the code does it |

**The state view becomes a data classification.** For every piece of *shared* data — data that
more than one module reads or writes — the guide gives: the kind, the owner, the writers, the
readers, and the lifetime. Private data of one module gets one sentence per owner and no table,
because it belongs to that module alone. The table comes from the study of step 1b, not from
memory. The guide cites the upstream document as the source of the four kinds.

### D-3 — the anatomy is a survey snapshot — **decided 2026-09-21**

Its status becomes `snapshot 2026-09-21`, and one sentence at the top says that the developer's
guide replaces it when that guide is complete. It is not restructured and not trimmed.

### D-4 — the user's guide depth — **decided 2026-09-21**

The user's guide follows the catalog, but only at the level a user controls: NED submodules,
typenames and ini parameters. No C++ name appears in it.

### D-5 — where the work happens — **open**

The global rules say that a plan is implemented in a dedicated worktree. The eight commits of this
group are on `master` in `inet-master`, not pushed. Two options: a worktree `inet-topic-ieee80211-documentation`
on a branch `topic/ieee80211-documentation` from the current `master`, merged back when done; or
the work continues on `master` in `inet-master`, one commit per file, as the eight commits were made.
The plan takes the first option unless the review says otherwise.

### D-7 — an amendment map — **decided 2026-09-22**

No class in the tree carries an amendment letter; the code names the families (`Qos`, `Ht`,
`Vht`, `Erp`, `Dsss`, `Ofdm`), and an amendment cuts across cells. Part 1 of the developer's guide
gets one table, "The amendments and the code": one row per amendment (base, a, b, g, e, n, ac, p,
and one row for the amendments the model does not contain), with what it adds, the name prefix in
the code, the parts, and the status. Each attribution is checked against the standard text before
it goes in: block ack and TXOP are 11e, A-MSDU and A-MPDU are 11n, EPD comes from IEEE 802 and not
from 11p. The user's guide keeps its own PHY mode table and gets no amendment table.

### D-6 — what a C++ interface may stand for — **proposed 2026-09-22**

The tree has 53 C++ interfaces: 51 in `mac/contract/`, `IIeee80211Llc`, and `IIeee80211Mode` with
its band and channel companions. The guide names about 25 of them and gives no rule for a new one.
The user's concern: a developer adds an interface because one module needs information from
another, and the interface captures no concept of the model. No such interface exists today; the
risk is in the future.

Every interface of the tree stands for one of four things:

| Kind | Stands for | Members today |
| --- | --- | --- |
| standard concept | a thing the standard names | `ICoordinationFunction`, `IChannelAccess`, `IContention`, `IFrameSequence`, the two data services and their QoS variants, `IFragmentation`, `IDefragmentation`, `IReassembly`, the aggregation and deaggregation interfaces, `ISequenceNumberAssignment`, `IDuplicateRemoval`, `ITransmitLifetimeHandler`, `IRtsProcedure`, `ICtsProcedure`, `IRecipientAckProcedure`, the block ack handlers and procedures, `IAckHandler`, `IRecoveryProcedure`, `IRateSelection`, `IQosRateSelection`, `IIeee80211Mode`, `IIeee80211Llc`, `IDs` |
| model decision | a choice the standard leaves open, separated so that it can be replaced | every `I...Policy` (ack, RTS, CTS, fragmentation, MSDU and MPDU aggregation, block ack agreement), `IRateControl`, `IEdcaCollisionController`, the classifier |
| process boundary | the split of the 2015 design: channel access, frame exchange, transmission, reception | `ITx`, `IRx`, `IFrameSequenceHandler` |
| callback | the way a mechanism reports to its owner | the nested `ICallback` of `IChannelAccess`, `IContention`, `ITx` and `IFrameSequenceHandler`; `IProcedureCallback`; `IBlockAckAgreementHandlerCallback`; `IRecoveryProcedure::ICwCalculator` |

The fourth kind is the only technical one, and every member of it is a callback: it runs from the
mechanism to its owner, and it is nested in, or paired with, the service interface it serves.

**The rule for a new interface.** It must name a concept of the standard, or a decision of the
model with at least one alternative implementation in mind. A need for information from another
module is not a reason for an interface: the peer lookup rule and an existing contract, the shared
MIB, or a signal serve that need. A callback is nested in the interface it serves and never
stands alone. The counter-example in the guide: a provider interface for the mode set. The mode
set is a catalog datum with one owner, the MAC, and the existing means distribute it.

The study of step 1c classifies every interface and confirms the table above against the code.
The policies get one sentence in the guide that says they are a concept of this model, not of the
standard.

## 6. Steps

Each step ends with a commit series that the four gates accept: `check-commits.sh`,
`check-classification.sh`, `check-seals.sh` and `check-links.sh`. Commits carry the group
`ieee80211-documentation` and the trailer `Plan: plan/pending/ieee80211-documentation.md`.

### Step 1 — the catalog as text and figure — review checkpoint A

- [x] Write the catalog section for the developer's guide: the grid, the three patterns, the four
      views, the second-level axes, in about 120 lines. *Done 2026-09-22 as part 1, "At a glance", 150 lines.*
- [x] Draw the grid as a dot figure, `figures/ieee80211_catalog.dot`: layers as rows, planes as
      columns, one node per cell with its parts. Width at most 6.5 inches. *Done: 5.9 inches, as one HTML table node.*
- [x] Redraw the coordination function figure along the function axis, five rows, so that the
      figure and the table of §3.4 say the same thing. *Done: five dashed bands in a two-column grid, 6.6 inches wide; only the edges between bands are drawn.*
- [x] The user reviews the catalog before step 2 starts. *Reviewed 2026-09-22.*

### Step 1b — the shared-data study

- [x] List every piece of shared data of the model: every MIB field; the mode set; the radio's
      channel, band and mode; the pending queue and the in-progress frames; the ack status; the
      contention window; the NAV; the radio state as the MAC sees it; the peer HT state; the block
      ack agreements; the TXOP; the interface entry; the local HT capabilities. A datum is shared
      when a module other than its owner reads or writes it.
- [x] For each datum, find in the code: its kind (catalog, capability, control, status), its
      owner, every writer, every reader, and its lifetime (immutable; survives a stop; cleared by
      its owner; per relationship). Cite the file for each fact. A grep for `mib->` and for the
      pointer members of each module finds the readers and writers.
- [x] Mark the data whose treatment in the code departs from the upstream document, for the
      "where the code differs" section.
- [x] The result is a table in the plan's decision log first, then the state view of step 2. *Done 2026-09-22; §9.*

### Step 1c — the interface study

- [x] List every C++ interface of the two 802.11 subtrees, with: its kind per D-6; the clause of
      the standard when it is a standard concept; the classes that implement it, as a family
      (non-QoS and QoS, DCF and EDCA); the parts that call it; the NED slot that takes it, if any;
      and its purpose in one line. *Done 2026-09-22; the condensed table is in §9.*
- [x] Mark every interface with one implementation and no slot. It stays if it names a concept;
      it is a candidate for the "where the code differs" list if it does not. *Done: 22, all stay.*
- [x] The result is a table in the plan's decision log first, then the contracts view of step 2.

### Step 2 — restructure the developer's guide

- [x] Reorder the chapter into the five parts of §4.1: at a glance; the parts; the views;
      extension; where the code differs.
- [x] Rewrite each cell section along its second-level axis, with the three patterns as table
      columns. The text of the current sections moves; it is not rewritten where it is right.
- [x] Write the state view per D-2: the four kinds in one paragraph each, then the shared-data
      table from step 1b (kind, owner, writers, readers, lifetime), then one sentence per owner
      for its private data.
- [x] Regroup the extension points by the decision and variant axes.
- [x] Write the contracts view from the table of step 1c, and the rule of D-6 as the last
      extension section.
- [x] Write the amendment table of D-7 as section 1.6 of part 1, and check every attribution
      against the standard text. *Checked against the amendment list of the 2016 front matter.*
- [x] Keep every `:doc:` reference with an explicit title, and keep every table free of long
      identifiers, as the PDF review of 2026-09-21 required.
- [x] Build both guides as PDF with the Makefile and read the chapter once in the PDF. *Done for the developer's guide: no overfull line, no undefined reference; the new pages were read.*

### Step 3 — align the user's guide

- [x] Reorder its sections onto the outline of §4.2: the MAC part follows the function axis.
- [x] Name the cells and the functions with the same words as the developer's guide, so that a
      reader who moves between the guides finds the same map.
- [x] Keep the depth at NED and ini. Remove nothing that a user configures.
- [x] Build the PDF and read the chapter once. *No overfull line above one point after one column change; no undefined reference.*

### Step 4 — mark the anatomy

- [x] Change the header status to `snapshot 2026-09-21` and add the retirement sentence per D-3.
- [x] Add a cross-reference from the anatomy's introduction to the developer's guide chapter.

### Step 5 — close

- [x] Run the four gates on the whole series. *Commit, classification and seal gates pass; the link gate reports only the 16 pre-existing broken links of the upstream wifi results file.*
- [x] Update this plan with the decisions made during the work (§9) and move it to `plan/done/`.

## 7. Where the current developer's guide sections go

| Current section | New place |
| --- | --- |
| design goals | part 1, at a glance |
| reference model | part 1, at a glance, as the mapping of the grid to the standard |
| the programming interface | part 3, the communication view |
| the network interface | part 2, level 1: the grid as NED wiring |
| means of communication | part 3, the communication view |
| the MAC: the parts table | part 2, the MAC data plane cell, level 1 of the cell |
| the coordination function | part 2, the MAC data plane cell, divided by function |
| ownership of state | part 3, the state view, expanded into the shared-data table per D-2 |
| a frame's journey | part 2, the end of the MAC data plane cell |
| management and agent | part 2, the management plane cells |
| the physical layer | part 2, the PHY cells |
| initialization and lifecycle | part 3, the time view |
| extension points | part 4, regrouped by decision and variant |
| where the code differs | part 5, unchanged, plus the data whose treatment departs from the upstream document |

## 8. Verification

- The docutils parse of each chapter is clean, and the Sphinx build of both guides gives no
  warning for the two chapters.
- The PDF chapters have no overfull line and no raw reference.
- Every `:ned:` name in both chapters is in `nedtags.xml`.
- The four gates pass on the series.
- The three documents use the same cell names; a grep for each cell name finds it in all three.
- Every MIB field and every datum that a `mib->` or a cross-module pointer touches in the two
  802.11 subtrees has a row in the shared-data table.
- Every `I*.h` under `mac/contract/`, `llc/` and `physicallayer/wireless/ieee80211/mode/` has a
  row in the contracts table.

## 9. Decision log

To be filled during the implementation: the facts found and the choices made, with the date.

### 2026-09-22 — step 1 done; the interface study (step 1c) done

**Step 1.** Part 1 of the developer's guide, "At a glance", is written (150 lines): design goals,
the map with the grid figure and the catalog table, the three patterns, the mapping to the
standard. The grid figure is one HTML table node in dot, 5.9 inches wide. The coordination function
figure has five dashed bands along the function axis in a two-column grid and only the edges between bands; 6.6 inches
wide, legible at page width. Both guides build; the chapter has no overfull line in the PDF.

**Step 1c, the interface study.** Corrections to D-6: `mac/contract/` holds 47 headers, not 51;
with `IIeee80211Llc`, the four interfaces of `IIeee80211Mode.h` and `IIeee80211Band` there are 53
top-level interfaces, and 58 with the five nested ones (four `ICallback`, one `ICwCalculator`).
The kinds of D-6 hold, with one disagreement: `IEdcaCollisionController` stands for a rule of the
standard (the higher category wins an internal collision), so it is a standard concept with a slot
for experiments, not a model decision. `IContention` is the same case. The guide will say "almost
every slot is on the decision side" and name these two as the exceptions.

The condensed table; the full table with methods and purposes is the source for the contracts view.

| Interface | Kind | Implementations | Called through the interface by | NED slot |
| --- | --- | --- | --- | --- |
| `ICoordinationFunction` | standard | `Dcf`, `Hcf`, `Pcf`, `Mcf` | **nobody**: the MAC holds `Dcf*` and `Hcf*` | `dcf` (`IDcf`), `hcf` (`IHcf`) |
| `IChannelAccess` + `::ICallback` | standard + callback | `Dcaf`, `Edcaf`, `Hcca`; callback: `Dcf`, `Hcf` | `Pcf` (member); callback: `Dcaf`, `Edcaf` | none |
| `IContention` + `::ICallback` | standard + callback | `Contention`; callback: `Dcaf`, `Edcaf` | `Rx`, `Dcaf`, `Edcaf`; callback: `Contention` | `contention` in `Dcaf`, `Edcaf` |
| `IEdcaCollisionController` | standard (see above) | `EdcaCollisionController` | `Edcaf` | `collisionController` in `Edca`, as NED `ICollisionController` |
| `IFrameSequence` | standard | 4 combinators, 11 primitives | `FrameSequenceHandler`, the combinators | none |
| `IFrameSequenceHandler` + `::ICallback` | boundary + callback | `FrameSequenceHandler`; callback: `Dcf`, `Hcf` | `Dcf`, `Hcf`; callback: the handler | none |
| `ITx` + `::ICallback` | boundary + callback | `Tx`; callback: `Dcf`, `Hcf` | `Dcf`, `Hcf`; callback: `Tx` | `tx` |
| `IRx` | boundary | `Rx` | `Tx`, `Dcf`, `Hcf`, `CtsPolicy`, `QosCtsPolicy` | `rx` |
| `IDs` | standard | `Ds` | nobody through the type; NED gates | `ds` |
| `IOriginatorMacDataService` | standard | non-QoS and QoS | `Dcf`, `Hcf`, `InProgressFrames` | none |
| `IRecipientMacDataService`, `IRecipientQosMacDataService` | standard | one each | `Dcf`; `Hcf` | none |
| `IFragmentation`, `IDefragmentation`, `IReassembly` | standard | one each | the data services; `IDefragmentation`: nobody | none |
| `IMsduAggregation`, `IMpduAggregation`, `IMsduDeaggregation`, `IMpduDeaggregation` | standard | one each | the QoS data services | none |
| `ISequenceNumberAssignment`, `IDuplicateRemoval` | standard | non-QoS and QoS | the data services | none |
| `ITransmitLifetimeHandler` | standard | DCF and EDCA | `Dcf` (never created) | none |
| `IRtsProcedure`, `ICtsProcedure`, `IRecipientAckProcedure` | standard | one each | `Dcf`, `Hcf`, the context | none |
| block ack handlers and procedures (4) | standard | one each | `Hcf`, the context | none |
| `IAckHandler` | standard | non-QoS and QoS | `InProgressFrames` | none |
| `IRecoveryProcedure` + `::ICwCalculator` | standard + callback | non-QoS and QoS; calculator: `Dcaf`, `Edcaf` | nobody through the type (only signals); calculator: the two procedures | none |
| `IRateSelection`, `IQosRateSelection` | standard | one each | `Dcf` and the non-QoS policies; `Hcf` and the QoS policies | `IRateSelection.ned` exists, but the slot is a fixed type |
| `IRateControl` | decision | `RateControlBase` (AARF, ARF, Onoe) | `Dcf`, `Hcf`, the rate selections | `rateControl` |
| the policies: ack (4), RTS, CTS, fragmentation, MSDU and MPDU aggregation, block ack agreement (2) | decision | one or a non-QoS and QoS pair each | `Dcf`, `Hcf`, the context, the procedures | one slot each |
| `IProcedureCallback`, `IBlockAckAgreementHandlerCallback` | callback | `Dcf`, `Hcf`; `Hcf` | the procedures and handlers, as a parameter | none |
| `IIeee80211Llc` | standard | LPD, EPD, portal | nobody through the type; NED gates | `llc` |
| `IIeee80211Mode` and its three submodes; `IIeee80211Band` | standard | one base each, seven PHY families | the mode set, the transmitter, the receiver, rate selection, the management | none |

**Findings for the guide.**

- Four interfaces are called by nobody through their C++ type: `ICoordinationFunction`, `IDs`,
  `IIeee80211Llc`, `IRecoveryProcedure`. The first matters: the MAC casts its slots to the
  concrete `Dcf` and `Hcf`, so a coordination function of another class cannot be plugged in
  without a change to the MAC. This goes to "where the code differs".
- 22 interfaces have one implementation and no slot. All of them name a mechanism of the standard
  (procedures, handlers, aggregation, reassembly, the two data services of the recipient side, the
  QoS rate selection) or a PHY concept (`IIeee80211Mode`, `IIeee80211Band`). They stay; the rule
  of D-6 allows an interface that names a concept even with one implementation.
- Four NED and C++ names disagree: `ICollisionController.ned` against `IEdcaCollisionController.h`;
  `IDcf.ned` and `IHcf.ned` have no C++ counterpart, the C++ contract is `ICoordinationFunction`;
  `IOriginatorQosAckPolicy.ned` against `IOriginatorQoSAckPolicy.h`. The guide names them in the
  contracts view; a rename is a code change outside this plan.
- `IRateSelection.ned` exists but no slot uses it: `Dcf` and `Hcf` wire the rate selection as a
  fixed type.

### 2026-09-22 — the shared-data study (step 1b) done

The full report, with file and line citations, is in `audit/ieee80211-documentation/shared-data.md`
at the repository root (not in git). The condensed table follows; it is the source of the state
view of step 2.

| Datum | Kind | Owner | Writers | Readers | Lifetime |
| --- | --- | --- | --- | --- | --- |
| MIB: address, mode, qos, stationType | control | MIB | the MAC (address, qos); the management module (mode, stationType) | MAC, `Ds`, `Rx`, `Tx`, the coordination functions, management | set at init |
| MIB: bssData (ssid, bssid) | control on an AP, status on a station | MIB | management | MAC, `Ds`, the network configurator | AP: init; station: per relationship |
| MIB: isAssociated | status | MIB | station management | `Ds`, MAC, agent, station management | per relationship |
| MIB: station table | status | MIB | AP management; the simplified station writes the AP's MIB | `Ds`, MAC, AP management | per relationship |
| MIB: association identifiers and reservations | status, control | MIB, through its methods | AP management through reserve, commit, release | MIB | per relationship; a reservation per response exchange |
| MIB: local HT capabilities | capability | MIB | the MIB, on request of the MAC at init | management | set at init |
| MIB: HT operation, primary channel | control, status | MIB, private | the MIB, on the radio's channel signal and at init | AP management, management base | owner, on a channel change |
| MIB: peer HT states | status, with a derived compatibility cache | MIB, private | the MIB, on request of management | rate selection | per relationship |
| the mode set catalog | catalog | a static table | none | MAC, radio | immutable |
| the MAC's mode set and every cached copy | catalog | MAC; each listener | MAC at init; listeners from one signal | every listener | set at init |
| supported rate elements | capability | management base | from the mode set signal | the management variants | set at init |
| transmitter and receiver: mode set, mode, band, channel | catalog, control | transmitter, receiver | the radio, on a configure command, through `const_cast` | transmitter, receiver, MAC at init | on command |
| tuned channel and band | status | radio | radio | AP management, by signal | on change |
| radio mode | status | radio | radio | MAC reads it directly | on change |
| Rx: reception state, transmission state, signal part | status | `Rx` | `Rx`, from the radio signals relayed by the MAC | `Rx`, coordination functions | per frame |
| Rx: medium free | status | `Rx` | `Rx` | MAC, CTS policies; pushed to every contention | per frame |
| NAV | status | `Rx` | `Rx`, on a received frame and on `Tx`'s request | `Rx` | per frame |
| pending queues | status | the queue module of each channel access | the coordination function enqueues; in-progress frames dequeue | coordination function | per frame |
| in-progress frames | status | `InProgressFrames` | its own methods, called by the coordination function, the handler | coordination function, frame sequences | per frame |
| frame sequence context | status | the handler | built by the coordination function | the frame sequences | per exchange |
| ack status maps | status | the ack handlers | own methods, called by the coordination function and in-progress frames | the same | per frame |
| contention window | status | channel access | own methods, on request of the recovery procedure | recovery procedure, coordination function | on outcome |
| station retry counters | status | `Dcf`; each `Edcaf` | the non-QoS recovery procedure, on an object it does not own | the same | on outcome |
| per-frame retry counters | status | the recovery procedures | own methods | coordination function through accessors | per frame |
| block ack agreements | status | the two handlers | the handlers, driven by `Hcf` | ack policy, frame sequences, recipient data service, procedures | per relationship |
| block ack record | status | the recipient agreement | block ack reordering writes; the recipient procedure reads | same | per frame |
| TXOP: start, limit | status, control | TXOP procedure | `Hcf` starts and ends it | frame sequences, rate selection, protection | per exchange |
| interface entry: address, MTU, state | control, status | `NetworkInterface` | address: nobody in the tree; MTU: MAC; state: the MAC base class | MAC; rate selection reads peers' interfaces | set at init; state on lifecycle |

**Departures from the upstream document that the guide will list.** The MIB has a public half
with direct writes by the MAC and the management modules and a private half with typed operations;
no MIB change is announced at a commit boundary, the association signals come from management
after the write. The simplified station writes the MIB of another node. The AID reservation is a
pending transaction in the MIB. The mode set is distributed by one signal inside the link-layer
stage, and the rate selection uses it in the same stage, so readiness depends on sibling order. The
radio writes its transmitter and receiver through `const_cast`. `Rx` and the MAC keep two copies of
the transmission state, and `Rx` keeps an unrefreshed copy of the address. The frame sequence
context hands out live handles. The recovery procedure announces the contention window change of a
module it does not own. The shared management recovery procedure of a QoS station changes the
contention window of the best-effort category whatever the calling category. The block ack record
is written by the reordering and read by the procedure, not by its holder. Four data are dead:
the last transmitted mode maps, the generation counter of the peer HT state, `numSentBaPolicyFrames`
and `isAddbaResponseSent`.

### 2026-09-22 — step 2 done

The developer's guide is restructured: 1610 lines, five parts, four figures, in the worktree
`/home/levy/workspace/inet-ieee80211-documentation`. Facts and choices:

- The amendment attributions were checked against the amendment list in the front matter of
  IEEE Std 802.11-2016 (the 2024 front matter does not list them in a form that a text search
  finds): 11e is the QoS amendment, 11n higher throughput, 11p vehicular access, 11ac VHT, 11s
  mesh, 11i security, 11r fast BSS transition, 11k measurement, 11w protected management frames.
  The a, b and g amendments predate that list and were taken as known.
- The shared-data table has 29 rows and six columns; it is the densest table of the chapter and
  prints at the limit of readability. The "Kind" column got 14 % of the width and two short cells.
- Long interface names never sit in a narrow column: a name goes into the responsibility cell, or
  a row is split. Nine overfull lines were removed this way.
- The contracts table groups families into one row each (29 rows for 58 interfaces); the full
  list stays in `audit/ieee80211-documentation/interfaces.md`.
- "Where the code differs" grew from eight to thirteen items with the study findings: the concrete
  `Dcf` and `Hcf` pointers of the MAC, the readiness of the mode set, the shared recovery procedure
  of a QoS station, the duplicated caches, and the unused state.

### 2026-09-22 — steps 3 and 4 done

The user's guide keeps every section and every parameter; the MAC part changed order and names
only: access, quality of service, frame exchange and protection, fragmentation and aggregation
(one section from two), acknowledgement and retries, block acknowledgement, bit rates and rate
control, statistics. One sentence names the five functions and points to the developer's guide.
The anatomy header says `snapshot 2026-09-21`, and its introduction says that the developer's guide
replaces it when complete. The seal gate passes; the link gate reports the 16 pre-existing broken
links of the upstream wifi results file and none of this series.
