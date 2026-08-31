# Repository layout

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [decisions.md](decisions.md), [rule/architecture.md](../rule/architecture.md)

Where everything lives, and what keeps it there. [decisions.md](decisions.md) holds the choices this
layout carries out, and [rule/architecture.md](../rule/architecture.md) holds the rules that a change
to it must respect. This document is the inventory.

## The top level

| Path | What it holds |
| --- | --- |
| `src/inet/` | the framework: every model, in NED, `.msg` and C++ |
| `examples/` | one runnable network per feature area — 37 directories |
| `showcases/` | focused studies that demonstrate one effect, with a written page each |
| `tutorials/` | step-by-step teaching sequences |
| `tests/` | every test, in twelve categories |
| `doc/src/` | the published guides: user's, developer's, migration |
| `doc/project/` | this document set |
| `python/inet/` | the test drivers, the simulation runner, the chart and result tooling |
| `_scripts/` | maintenance and release scripts |
| `images/` | the icon set that `@display` draws from |
| `templates/` | the project and model templates the IDE offers |
| `releng/`, `bin/` | release engineering and the launchers |
| `plan/` | the design and implementation plans; a done plan is one step of the history |
| `.github/workflows/` | twelve test and build workflows |
| `.oppfeatures` | the feature descriptors — the single source of truth for the partition |

## The source tree

`src/inet/` is partitioned by protocol layer and by orthogonal domain, and the dependencies point one
way: **from a protocol toward the infrastructure, never back**
([AR-ORG-DOMAINS](../rule/architecture.md#ar-org-domains)).

**The layers**, in the order a packet crosses them:

| Package | Owns |
| --- | --- |
| `physicallayer/` | radios, transmitters, receivers, antennas, propagation, the medium, the analog models |
| `linklayer/` | Ethernet, IEEE 802.11, IEEE 802.15.4, PPP, and the shared link machinery |
| `networklayer/` | IPv4, IPv6, ICMP, ARP, and the network-layer contracts |
| `transportlayer/` | TCP, UDP, SCTP, QUIC, RTP |
| `routing/` | AODV, OSPF, BGP, RIP, PIM, EIGRP, and the routing tables |
| `applications/` | the traffic generators and application models |

**The orthogonal domains**, which combine freely with any network:

| Package | Owns |
| --- | --- |
| `mobility/` | how a node moves |
| `power/` | energy sources, consumers, storage |
| `clock/` | hardware clocks with drift, and everything that reads them |
| `environment/` | the physical environment: ground, obstacles, the object cache |
| `queueing/` | the push and pull datapath elements |
| `protocolelement/` | the atomic protocol operations that compose into a protocol |
| `emulation/` | the boundary to a real network and to real time |

**The infrastructure**, which the layers depend on and which depends on none of them:

| Package | Owns |
| --- | --- |
| `common/` | the packet and chunk API, tags, units, the dispatcher, the lifecycle, the result filters |
| `node/` | the pre-assembled node types: `StandardHost`, `Router`, `AccessPoint` |
| `networks/` | pre-assembled networks |
| `visualizer/` | every renderer, 2D and 3D — and nothing else depends on it |

`common/` is where the direction is under pressure, and where it is measured:
[audit/architecture-exceptions.md](../audit/architecture-exceptions.md) holds the couplings that
point the wrong way today, and [check-architecture.sh](../enforcement/check-architecture.sh) finds
them.

**A package is a directory is a NED package is a C++ namespace.** The four are one name
([NR-PKG](../rule/naming.md#nr-pkg)), which is what makes a component findable from its name alone.

## The four artifact kinds, side by side

A protocol is written in four languages, and each file kind sits beside the others in the same
directory:

| Kind | Declares | Named by |
| --- | --- | --- |
| `.ned` | the external interface: parameters, gates, signals, statistics, display | [NR-NED-TYPE](../rule/naming.md#nr-ned-type) |
| `.msg` | the packet content and the tags | [NR-MSG-TYPE](../rule/naming.md#nr-msg-type) |
| `.h` / `.cc` | the behavior | [NR-CPP-TYPE](../rule/naming.md#nr-cpp-type) |
| `*_m.h` / `*_m.cc` | generated from the `.msg` — never hand-edited | [NR-GEN](../rule/naming.md#nr-gen) |

[protocol-anatomy.md](protocol-anatomy.md) says what a complete protocol needs across all four.

## The three example trees

They are not three names for one thing. Each answers a different question, and a contribution
belongs in exactly one:

| Tree | Answers | Shape |
| --- | --- | --- |
| `examples/` | *Does this feature run, and how is it configured?* | one network per feature area, minimal, runnable |
| `showcases/` | *What effect does this produce, and what does the result look like?* | one scenario, measured, with a written page and charts |
| `tutorials/` | *How do I learn this, step by step?* | a numbered sequence, each step adding one idea |

An `examples/` directory that grows a written argument wants to be a showcase. A showcase that grows
a second and third step wants to be a tutorial.

## The test tree

Twelve categories under `tests/`. What each one is for, and which claim it can establish, is
[test-anatomy.md](test-anatomy.md); which one a change owes is
[rule/testing.md](../rule/testing.md).

```
unit  module  protocol  queueing  packet  networks
statistical  validation  fingerprint  speed  features  misc
```

## Where the tooling lives

| Path | Holds |
| --- | --- |
| `python/inet/test/` | the test drivers for every category |
| `python/inet/simulation/` | building, running and collecting a simulation |
| `python/inet/scave/` | reading result files and making charts |
| `python/inet/documentation/` | generating documentation from the sources |
| `doc/project/enforcement/` | the rule gates, and only the rule gates |
| `_scripts/` | maintenance sweeps, release steps, IDE helpers |

The boundary is stated in [enforcement/README.md](../enforcement/README.md): the enforcement folder
holds the direct transcription of a numbered rule, and heavy tooling stays in `python/` and
`_scripts/`.

## The build

| File | Says |
| --- | --- |
| `.oppfeatures` | which features exist, what each one covers, and what it depends on |
| `.oppfeaturestate` | which features are switched on in this working copy |
| `Makefile`, `src/Makefile`, `src/makefrag` | how it is built |
| `.nedfolders`, `.nedexclusions` | which directories the NED compiler reads |
| `/.clang-tidy` | the C++ gate for the naming and quality rules |

A feature descriptor is the single source of truth for the partition
([AR-BUILD-DECLARATIVE](../rule/architecture.md#ar-build-declarative)), and a feature identifier is
part of the published interface ([RR-FEATURE-STABLE](../rule/release.md#rr-feature-stable)).
