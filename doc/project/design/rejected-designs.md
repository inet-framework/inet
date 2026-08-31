# Rejected designs

> **Kind:** decision · **Status:** current · **Seal:** by decision · **Owns:** `REJ-*` · **Stands on:** [decisions.md](decisions.md), [architecture.md](../rule/architecture.md)

The designs that were considered and turned down, and the reason for each. The document exists
because a question of the form *why does INET not do X?* comes back every year, in a review, an issue
or a mailing-list thread. Without a citable answer it is argued again from the start each time.

A `REJ-*` entry is not a rule and it does not forbid anything. It is a record: **this option was
weighed against the decision that beat it, and here is why it lost.** Cite one in a review when
someone re-proposes the option — `REJ-01` is shorter and fairer than repeating the argument.

Each entry seals independently. A sealed entry is settled; reopening it needs the same explicit
permission as changing it. An entry that new evidence overturns is not deleted: its status becomes
`reopened`, with the reason, so the history stays readable.

## How to read an entry

1. **The option**, stated as its advocate would state it, at its strongest.
2. **What it would buy.** An option with no advantage was never a real option.
3. **Why it lost**, against the decision that beat it.
4. *Beaten by* — the `D-*` decision, and the `AR-*` rule that keeps the choice in force.

## Index

Every rejected design in document order.

**The rejected designs**

| Rule | Statement |
| --- | --- |
| [REJ-01](#rej-01) | A zero-time message for same-instant coordination inside a node |
| [REJ-02](#rej-02) | Finding a service by its path in the module tree |
| [REJ-03](#rej-03) | Drawing from inside the model |
| [REJ-04](#rej-04) | A central switch that knows every protocol |
| [REJ-05](#rej-05) | Carrying local metadata in the packet content |
| [REJ-06](#rej-06) | A flat byte buffer for packet content |
| [REJ-07](#rej-07) | Documentation prose that restates the NED declaration |
| [REJ-08](#rej-08) | Deep inheritance as the way to add behavior |
| [REJ-09](#rej-09) | Fingerprints as the only regression test |
| [REJ-10](#rej-10) | One build with everything in it |

## The rejected designs

### REJ-01

**A zero-time message for same-instant coordination inside a node**

**The option.** Two submodules of one node that must agree at one instant exchange a message with
zero delay, instead of calling each other. Everything is then a message, the module boundary stays
anonymous, and the interaction shows up in the event log for free.

**What it would buy.** One interaction mechanism instead of two, no compile-time coupling between
sibling submodules, and a uniform trace.

**Why it lost.** An event stops meaning what it means. The event log, the sequence chart and the
fingerprint all rest on the reading that an event is *something that happened in the model*; when
half the events are procedure calls in disguise, the trajectory describes the implementation instead
of the behavior, and the fingerprint's signal quality falls with it. The zero-delay message also
hides the causal order inside one instant, which is exactly the order a debugging session needs.

*Beaten by* [D-DIRECT](decisions.md#d-direct),
kept in force by [AR-COM-DIRECT](../rule/architecture.md#ar-com-direct).

### REJ-02

**Finding a service by its path in the module tree**

**The option.** A module that needs the interface table, the routing table or a clock reaches it by a
path such as `^.^.interfaceTable`, or by an absolute path from the network.

**What it would buy.** No registration, no lookup mechanism, and a reference that a reader can follow
by eye in the NED file.

**Why it lost.** The path is the node's structure written into its parts. Every node layout that
differs from the one the author had in mind breaks the reference, so a module cannot be reused in a
node it was not written for — which is the whole point of a composable node. It also makes the
structure impossible to change: moving a submodule one level edits every path that reached across it.

*Beaten by* [D-REGISTRY](decisions.md#d-registry),
kept in force by [AR-MOD-NODEBASE](../rule/architecture.md#ar-mod-nodebase).

### REJ-03

**Drawing from inside the model**

**The option.** A mobility model, a radio or a protocol draws its own figure on the canvas, because
it is the code that knows what to show and when.

**What it would buy.** The shortest path from a state change to a picture, and no signal to declare.

**Why it lost.** Observation stops being free. Once the model owns the drawing, the drawing runs
whenever the model runs, and a study that wants no visualization pays for one anyway — or the model
grows a flag, and now the model has two behaviors. It also puts a rendering dependency inside the
protocol layer, which is the coupling that
[audit/architecture-exceptions.md](../audit/architecture-exceptions.md) still tracks as `AV-VIS-01`.

*Beaten by* [D-OBSERVE](decisions.md#d-observe),
kept in force by [AR-ORG-VIS-SPLIT](../rule/architecture.md#ar-org-vis-split).

### REJ-04

**A central switch that knows every protocol**

**The option.** Dispatch, dissection, printing and build selection are each a switch or a table in
the core, listing every protocol INET supports. Adding a protocol means adding a case.

**What it would buy.** One place to read the whole list, and a compile error when a protocol forgets
to register.

**Why it lost.** It makes every new protocol a core edit. The core then has a reason to change for
every model in the tree, which is the opposite of what a core is for, and two protocols in flight
collide in the same switch. It also makes an out-of-tree protocol impossible: nothing outside the
repository can add a case.

*Beaten by* [D-EXTEND-BY-ATTACH](decisions.md#d-extend-by-attach),
kept in force by [AR-EXT-NOCORE](../rule/architecture.md#ar-ext-nocore).

### REJ-05

**Carrying local metadata in the packet content**

**The option.** The interface a packet came in on, the socket that owns it, and the flow it belongs
to are fields of the header, or of an extra chunk that the serializer skips.

**What it would buy.** One container to reason about instead of two, and metadata that survives a
copy without any tag machinery.

**Why it lost.** The wire form stops being the wire form. Either the extra fields serialize, and the
capture no longer matches a real one, or they do not, and the packet has two contents that a reader
must keep apart in their head. Tags carry the same information and are *defined* to stop at the
transmission boundary, which is a rule a serializer test can check.

*Beaten by* [D-TAGS](decisions.md#d-tags),
kept in force by [AR-PKT-TAGS](../rule/architecture.md#ar-pkt-tags).

### REJ-06

**A flat byte buffer for packet content**

**The option.** A packet is a byte array. A header is a struct laid over an offset. Parsing is a
cast, and there is no chunk algebra to learn.

**What it would buy.** A representation every C programmer already knows, no allocation for the
chunk tree, and serialization for free because the bytes are already the wire form.

**Why it lost.** A model that has not decided its bytes cannot be represented. Much of a simulation
runs above the wire — a header whose fields are set but whose encoding is irrelevant, a payload that
is only a length, a chunk shared by a hundred receivers — and a byte buffer forces all of it to be
encoded, copied and re-parsed. The typed form also lets the dissector, the printer and the filter
work on fields, which is what makes the analysis tooling truthful about what the model represented.

*Beaten by* [D-CHUNKS](decisions.md#d-chunks),
kept in force by [AR-PKT-CHUNKS](../rule/architecture.md#ar-pkt-chunks).

### REJ-07

**Documentation prose that restates the NED declaration**

**The option.** The guide lists a module's parameters, gates, signals and statistics in prose, so a
reader gets everything on one page.

**What it would buy.** A page that reads on its own, with no jump into generated reference material.

**Why it lost.** Two lists drift, and the prose one is always the stale one — it has no compiler and
no test. A reader who trusts it then configures a parameter that no longer exists. The generated
reference is derived from the declaration itself and cannot go stale.

*Beaten by* [D-NED-TRUTH](decisions.md#d-ned-truth),
kept in force by [AR-OBS-NED-TRUTH](../rule/architecture.md#ar-obs-ned-truth).

### REJ-08

**Deep inheritance as the way to add behavior**

**The option.** A new variant of a protocol is a subclass of the existing one, overriding what it
needs. A shared concern moves up into a base class.

**What it would buy.** No new modules, no NED wiring, and code reuse that the compiler checks.

**Why it lost.** The chain becomes the architecture. After three levels no reader can say what a
method does without reading all three, a change in the base reaches every descendant, and two
variants that need different halves of the chain cannot both be built. Composition puts each concern
in its own module, where it can be replaced, omitted or tested alone. A `*Base` class is still right
for genuine shared machinery; the rejection is of inheritance as the *default* way to vary behavior.

*Beaten by* [D-COMPOSE](decisions.md#d-compose),
kept in force by [AR-MOD-COMPOSITION](../rule/architecture.md#ar-mod-composition).

### REJ-09

**Fingerprints as the only regression test**

**The option.** A trajectory fingerprint per configuration is the test suite. It is cheap to run,
covers everything at once, and catches any behavior change anywhere.

**What it would buy.** Broad coverage for almost no authoring cost, and one number to compare.

**Why it lost.** A fingerprint proves that behavior changed, never that behavior is correct. A model
that has been wrong since the day it was written has a perfectly stable fingerprint, and a
legitimate improvement and a regression are the same event to it. It is a wide net, not a claim.
Every change therefore also ships the test category that matches its claim — a unit test for a
computation, a validation test for a standard-defined behavior — and the fingerprint stays as the
net beneath them.

*Beaten by* [D-FINGERPRINT](decisions.md#d-fingerprint),
kept in force by [AR-QUAL-TESTS](../rule/architecture.md#ar-qual-tests).

### REJ-10

**One build with everything in it**

**The option.** No feature partition. Every protocol compiles into every build, and a user who does
not need a model simply does not instantiate it.

**What it would buy.** No feature descriptors, no dependency graph, no build matrix, and no
combination of flags that anyone has to test.

**Why it lost.** A model with a heavy or platform-bound dependency then binds the whole framework to
it, and a user on a platform that cannot satisfy it gets no INET at all. The partition is also what
proves the dependency direction is real: a feature that cannot be switched off is a feature whose
neighbours depend on it, and the feature-off build is the only check that finds that.

*Beaten by* [D-FEATURES](decisions.md#d-features),
kept in force by [AR-EXT-FEATURES](../rule/architecture.md#ar-ext-features).

## Adding an entry

1. State the option at its strongest, in the words its advocate would use. An option written to lose
   teaches nobody anything, and it will be re-proposed by someone who spots the straw man.
2. Say what it would buy. If it buys nothing, it was not a real alternative and it does not belong
   here.
3. Give the reason it lost, against the decision that beat it, and name that decision.
4. Take the next free number. An identifier is permanent and is never reused.
