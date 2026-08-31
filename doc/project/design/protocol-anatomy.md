# What a protocol model is made of

> **Kind:** design · **Status:** current · **Seal:** by section · **Owns:** — · **Stands on:** [packet-anatomy.md](packet-anatomy.md), [node-anatomy.md](node-anatomy.md), [rule/architecture.md](../rule/architecture.md)

Everything one protocol needs, across the four artifact kinds. This is the checklist behind
[guide/add-a-protocol.md](../guide/add-a-protocol.md), and the reason a new protocol is an
*extension* rather than a core edit ([D-EXTEND-BY-ATTACH](decisions.md#d-extend-by-attach)).

Take one protocol stem `Foo`. From it, every name follows without a lookup
([NR-NED-ROLE](../rule/naming.md#nr-ned-role)), and every part below has one place to live.

## The parts

| Part | Artifact | Why it is needed |
| --- | --- | --- |
| the module | `Foo.ned`, `Foo.h`, `Foo.cc` | the behavior, and its external interface |
| the interface | `IFoo.ned` | so the slot it fills can hold something else |
| the content | `FooHeader.msg` | the bytes, as typed fields |
| the wire boundary | `FooHeaderSerializer` | so the packet can leave and come back |
| the tooling | `FooProtocolDissector`, `FooProtocolPrinter` | so the packet can be described |
| the metadata | `FooReq`, `FooInd` | so a request travels down and an indication up |
| the identity | `Protocol::foo` | so a dispatcher can route to it |
| the state | `FooTable` | so what the protocol knows has an owner |
| the setup | `FooConfigurator` | so a network can be configured across nodes |
| the feature | `Foo`, `FooExamples`, `FooTests` in `.oppfeatures` | so it can be switched off |
| the evidence | a test in the matching category | so the claim it makes is established |
| the example | `examples/foo/` | so a user can run it |

Nine of the twelve are registrations into a mechanism that already exists. That is the point: the
cost of the next protocol falls because the mechanism was paid for once
([D-EXTEND-BY-ATTACH](decisions.md#d-extend-by-attach)).

## What must not appear

| Not this | Because |
| --- | --- |
| an edit to `common/` or to a dispatcher switch | [AR-EXT-NOCORE](../rule/architecture.md#ar-ext-nocore), [REJ-04](rejected-designs.md#rej-04) |
| a drawing call, or a reference to the visualizer | [AR-ORG-VIS-SPLIT](../rule/architecture.md#ar-org-vis-split), [REJ-03](rejected-designs.md#rej-03) |
| a zero-time message to a sibling submodule | [AR-COM-DIRECT](../rule/architecture.md#ar-com-direct), [REJ-01](rejected-designs.md#rej-01) |
| a path such as `^.^.interfaceTable` | [AR-MOD-NODEBASE](../rule/architecture.md#ar-mod-nodebase), [REJ-02](rejected-designs.md#rej-02) |
| metadata carried in the header instead of a tag | [AR-PKT-TAGS](../rule/architecture.md#ar-pkt-tags), [REJ-05](rejected-designs.md#rej-05) |
| a parameter hardcoded in C++ that belongs in NED | [AR-OBS-NED-TRUTH](../rule/architecture.md#ar-obs-ned-truth) |

## The behavior

A protocol's behavior is composed from small modules where it can be, rather than accumulated in one
([D-COMPOSE](decisions.md#d-compose)). Where the datapath is a chain — a queue, a shaper, a
classifier, a scheduler — it is built from the queueing contracts rather than from bespoke code
([D-QUEUEING](decisions.md#d-queueing)), which is what lets a TSN shaper sit in an Ethernet interface
that never heard of it.

Normative behavior cites the standard, the revision and the clause
([QR-CMT-STANDARD](../rule/quality.md#qr-cmt-standard)). A reader can then check the model against the
clause instead of against intuition, which is the only way a protocol model can be reviewed at all.

## What it emits

A protocol declares signals and statistics in NED, and emits them. It does not record, draw or
aggregate: an observer subscribes from outside ([D-OBSERVE](decisions.md#d-observe)). This is what
makes turning observation on free of any effect on the result
([R-VIS-NEUTRAL](../requirement/accepted-requirements.md#r-vis-neutral)).

## Fidelity

Where a concern is worth modeling at more than one level of detail, the levels fill the same
interface-typed slot and the study picks in configuration
([D-FIDELITY](decisions.md#d-fidelity)). A protocol that hardcodes its level of detail forces every
future study to accept that choice.
