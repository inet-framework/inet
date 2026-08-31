# Naming rules

> **Kind:** rule · **Status:** current · **Seal:** by rule · **Owns:** `NR-*` · **Stands on:** [architecture.md](architecture.md)

How everything in INET is named. The goal is **guessability in both directions**: given a concept you
can derive its name, and given a name you can tell what *kind* of thing it is and what it does,
without looking it up.

A name is built from two choices that compose. Its **casing** is fixed by the kind of entity — a
type, a parameter, a package. Its **role affixes** come from a fixed vocabulary that marks the part a
component plays. Meaningful, scheme-following names beat short ones.

The affix vocabulary is **free of redundancy**: each affix has one meaning, and each role has one
affix. Where two forms could express one role, one is canonical and the other is to be avoided. That
is what makes `FooHeaderSerializer` the only name for the serializer of a `FooHeader`.

These rules are the concrete form of [AR-QUAL-NAMING](architecture.md#ar-qual-naming). Both a human
and a tool infer a component's category, and how to compose it, from its name.

A rule has a stable identifier `NR-<AREA>`, and the identifier is the heading, so a citation links to
the rule: [NR-NED-GATE](naming.md#nr-ned-gate). Cite the identifier in a review or in a ledger row
rather than repeating the rule.

**The rules here are the target.** They are stated for the ideal, not for the current state of every
file. Where the code departs from them, the departure is recorded in
[audit/naming-exceptions.md](../audit/naming-exceptions.md) — as an `NS-*` sanctioned exception or an
`NV-*` rename candidate. Do not weaken a rule here to match the code.

What this document does *not* name: a branch, a commit or a pull request, which are
[pull-request.md](pull-request.md); and a document, a report or a plan, which are
[documentation.md](documentation.md).

## Index

Every naming rule in document order. The identifier links to the rule; the statement is its lead sentence.

**General**

| Rule | Statement |
| --- | --- |
| [NR-CASE](#nr-case) | Casing alone places a name in one of a few families. |
| [NR-WORD](#nr-word) | An acronym is a word, and a name is spelled out rather than abbreviated. |

**NED names**

| Rule | Statement |
| --- | --- |
| [NR-PKG](#nr-pkg) | A package, a directory and a namespace share one lowercase run-together name; a file is named for the type it defines. |
| [NR-NED-ROLE](#nr-ned-role) | A module carries an affix from a fixed vocabulary, and the affix says what part the module plays. |
| [NR-NED-TYPE](#nr-ned-type) | A module interface is `I<Concept>`; a network and a channel are `PascalCase` like any other NED type. |
| [NR-NED-GATE](#nr-ned-gate) | A gate is `<stem>In` or `<stem>Out`, and the stem names the peer it faces. |
| [NR-NED-PARAM](#nr-ned-param) | A parameter is `camelCase`, carries its `@unit` when it is a physical quantity, and means one thing. |
| [NR-NED-PROP](#nr-ned-prop) | A property is `@lowercase`, and it is spelled the same wherever it appears. |
| [NR-NED-SIGNAL](#nr-ned-signal) | A signal is `<subject><PastParticiple>` in NED and `<name>Signal` in C++. |

**Message (`.msg`) names**

| Rule | Statement |
| --- | --- |
| [NR-MSG-TYPE](#nr-msg-type) | A message type is named for the role it plays on the wire, or for the metadata it carries beside it. |
| [NR-MSG-FIELD](#nr-msg-field) | A field is `camelCase` and spelled out, and it carries its unit. |

**C++ names**

| Rule | Statement |
| --- | --- |
| [NR-CPP-TYPE](#nr-cpp-type) | A C++ type is `PascalCase` and matches the NED type it implements. |
| [NR-CPP-NAME](#nr-cpp-name) | A function, a member and a local are `camelCase`, and the verb says what the function does. |
| [NR-CPP-REG](#nr-cpp-reg) | A registration macro is `Register_<Thing>`, and the identifier it registers follows the domain's own spelling. |
| [NR-CPP-TIMER](#nr-cpp-timer) | A self-message member ends in `Timer` or `Msg`, and its name string matches the member. |

**Configuration, build and assets**

| Rule | Statement |
| --- | --- |
| [NR-INI](#nr-ini) | A config section is `[Config <PascalCase>]` and an iteration variable is `${camelCase}`. |
| [NR-FEATURE](#nr-feature) | A feature id is the `PascalCase` protocol stem, with `Examples` and `Tests` companions. |
| [NR-DIR](#nr-dir) | A directory is lowercase and run-together, singular, and it matches the NED package it holds. |
| [NR-TEST](#nr-test) | A test file is named for what it tests, and a fingerprint tag is a lowercase descriptive word. |
| [NR-ASSET](#nr-asset) | An icon file is lowercase and run-together, and its path is the icon name. |
| [NR-GEN](#nr-gen) | A generated file carries the name of its source, with the generator's suffix, and it is never hand-edited. |
| [NR-TOOL](#nr-tool) | A Python module is lowercase and run-together; a script is a lowercase hyphenated verb phrase that says what it does. |
| [NR-CI](#nr-ci) | A workflow file is named for the job it runs, lowercase and hyphenated, and it matches the test category it drives. |

## General

### NR-CASE

**Casing alone places a name in one of a few families.**

Casing alone places a name in one of a few families. In short: **types** — things you
instantiate, extend, or send — are `PascalCase`; **everything you configure or call** —
instances, parameters, gates, members, methods, locals — is `camelCase`; **packages and
namespaces** are `lowercase`; **enum values, constants, and macros** are `ALL_CAPS`.

| Kind | Casing | Example |
|---|---|---|
| NED types — `simple`, `module`, `network`, `channel`, `moduleinterface` | `PascalCase` | `EthernetSwitch`, `IRadio` |
| C++ classes, structs, MSG types | `PascalCase` | `Ipv4Header`, `MacAddress` |
| C++ type aliases (`typedef` / `using`) | `PascalCase` (`_t` only for scalar kernel-style) | `RouteVector`, `simtime_raw_t` |
| C++ template parameters | single uppercase letter; `PascalCase` if disambiguating | `T`, `R`, `Value` |
| C++ enum *types* | `PascalCase` | `AccessCategory`, `ArpOpcode` |
| Enum *values*, named constants, macros | `ALL_CAPS_WITH_UNDERSCORE` | `INITSTAGE_LINK_LAYER` |
| Submodule / instance names | `camelCase` | `radio`, `interfaceTable` |
| NED / MSG parameters and fields | `camelCase` | `sendInterval`, `sequenceNo` |
| Gates | `camelCase` + `In`/`Out` | `upperLayerIn`, `radioIn` |
| Signals and statistics | `camelCase` | `packetSent`, `endToEndDelay` |
| NED properties (`@…`) | `lowercase` camelCase | `@display`, `@lifecycleSupport` |
| C++ methods, free functions, arguments, locals, members | `camelCase`, verb-first for actions | `handleMessage`, `localPort` |
| Registration macros | `Define_*` / `Register_*` (capitalized words, underscores) | `Define_Module`, `Register_Serializer` |
| Packages / directories / namespaces | `lowercase`, run-together | `linklayer`, `inet::tcp` |
| Config sections (`.ini`) | `PascalCase` (hyphen-joined for combinations) | `[Config Rstp-LargeNet]` |
| Config iteration variables (`${…}`) | `camelCase` | `${advertisedWindowKiB=…}` |
| Feature ids (`.oppfeatures`) | `PascalCase` (acronym-as-word) | `Aodv`, `Bgpv4` |
| Icon paths | `lowercase`, run-together | `block/checker` |
| Files | the `PascalCase` type they define | `Ipv4Header.msg`, `Udp.cc` |

*Enforced at T3 — [`.clang-tidy`](../../../.clang-tidy) for the C++ half; agent review for the rest.*

### NR-WORD

**An acronym is a word, and a name is spelled out rather than abbreviated.**

- **An acronym is a word: capitalize only its first letter in a name.** Write
  `Ipv4Address`, not `IPv4Address`; `TcpConnection`, not `TCPConnection`; `Ieee80211Mac`,
  not `IEEE80211MAC`; likewise `UdpHeader`, `ArpPacket`, `SctpAssociation`,
  `Ospfv2HelloPacket`. This keeps word boundaries in a compound name unambiguous
  (`Ieee80211` + `Mac`). Established canonical real-world spellings are the only exception.
- **No ad-hoc abbreviations.** Spell domain words out in type, parameter, signal, field,
  and member names (`address` not `addr`, `interface` not `iface`, `destination` not
  `dst`). The only sanctioned short forms are established domain terms that read
  unambiguously as their one expansion: `Mac`, `Phy`, `Nic`, `Fcs`, `Snir`, `Rng`, `Mtu`,
  and the protocol acronyms themselves (`Tcp`, `Udp`, `Ip`, `Ipv4`, `Ipv6`, `Arp`, `Ppp`,
  `Bgp`, `Ospf`, …).
- **American spelling** (`initialize`, `color`, `neighbor`).

---

## NED names

*Enforced at T4 — agent review.*

### NR-PKG

**A package, a directory and a namespace share one lowercase run-together name; a file is named for the type it defines.**

- **Package names are lowercase, run-together, and singular**, and a package path always
  equals its directory path under `src/inet` (`inet.linklayer.ethernet.common` *is* the
  folder). The recurring functional subpackages carry fixed meanings: **`base`** holds
  shared base modules, **`contract`** holds the module interfaces, **`common`** holds
  cross-cutting infrastructure, **`configurator`** holds configurators. The protocol layers
  are `physicallayer`, `linklayer`, `networklayer`, `transportlayer`. Do not introduce new
  plural or underscored package names.
- **A file is named after the primary type it defines**, in that type's `PascalCase`:
  `Udp.ned` + `Udp.cc` + `Udp.h` define module `Udp`; `Ipv4Header.msg` defines
  `Ipv4Header`. Code generated from a `.msg` file carries the `_m` marker
  (`Ipv4Header_m.h`, `Ipv4Header_m.cc`) — the only underscore in an INET source file name.

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

### NR-NED-ROLE

**A module carries an affix from a fixed vocabulary, and the affix says what part the module plays.**

A module name reads as `[standard/vendor prefix] + Concept + [role suffix] + [variant]`.
The concept is a domain noun — very often itself a recognizable word such as `App`,
`Radio`, `Mobility`, `Mac`, `Receiver`, `Transmitter`, `Medium`, `Model`, `Cache`,
`Interface`, `Service`. The affixes around it mark its role and variation; read a module
name by stripping the affixes you recognize.

#### Role suffixes — what the module *is*

Each suffix marks exactly one structural role, so the suffix alone tells you a module's
gate/behaviour shape:

| Suffix | Role | Example |
|---|---|---|
| `*Base` | abstract base module, extended by concrete variants | `MacProtocolBase` |
| `*Layer` | compound protocol layer with upper/lower gates | `Ieee8022LlcLayer` |
| `*Inserter` | adds a protocol header (in → out) | `Ieee8021qTagEpdHeaderInserter` |
| `*Checker` | verifies and removes a protocol header (in → out) | `Ieee8022LlcChecker` |
| `*Processor` | generic command/packet processing (in → out) | `MacRelayUnitProcessor` |
| `*Table` | state shared across modules of a node | `MacForwardingTable`, `Ipv4RoutingTable` |
| `*Configurator` | configures a scope larger than one module | `Ipv4NetworkConfigurator` |
| `*Visualizer` | renders model state | `InterfaceTableVisualizer` |
| `*Host` / `*Router` / `*Switch` | network node types | `StandardHost`, `EthernetSwitch` |

**Queueing elements** take the suffix of their queueing role, so any packet-processing
element announces how it composes into a chain: `*Source`, `*Sink`, `*Queue`, `*Server`,
`*Buffer`, `*Filter`, `*Classifier`, `*Scheduler`, `*Gate`, `*Meter`, `*Shaper`, `*Marker`,
`*Policy`. Because the suffix names the push/pull role, a `PacketQueue` and a `PacketFilter`
are known to chain without reading their NED.

**Visualizers split by rendering back-end:** a `*CanvasVisualizer` draws on the 2D canvas, a
`*OsgVisualizer` in the 3D OSG scene, and the plain `*Visualizer` is the back-end-independent
base/integrator.

#### Role prefixes — how the module *varies*

| Prefix | Meaning | Example |
|---|---|---|
| `I*` | a module **interface** (`moduleinterface`), never a concrete module | `IRadio`, `IApp` |
| `Ieee*` | a model of a specific IEEE standard | `Ieee80211ScalarRadio`, `Ieee802154NarrowbandMac` |
| `Ext*` | bridges to the external world / host OS | `ExtInterface`, `ExtLowerIpv4` |
| `Ep*` | energy/power domain (energy + power units) | `EpEnergyStorageBase` |
| `Cc*` | charge/current domain (charge + current units) | `CcBatteryPack` |
| `Omitted*` | the no-op passthrough standing in for an absent optional submodule | `OmittedPacketQueue` |
| `Compound*` | the compound-module variant of an otherwise simple concept | `CompoundPendingQueue` |

**Paired variants use a `Single*`/`Dual*` or `SingleRate*`/`DualRate*` prefix pair**, not a
suffix: `SimpleIeee8021qFilter` / `DualIeee8021qFilter`, `SingleRateThreeColorMeter` /
`DualRateThreeColorMeter`.

#### The `6` variant marker and composition order

**Append `6` for the IPv6 variant of a node or module whose plain name is IPv4-default:**
`Router6`, `StandardHost6`, `WirelessHost6`, `HomeAgent6`. Protocol names themselves embed
their version (`Ipv4`/`Ipv6`, `Icmpv6`, `Ospfv2`/`Ospfv3`). Prefixes and variants stack
outward from the concept in one fixed order — standard/vendor prefix first, role suffix
last, variant `6` at the very end: `Ieee80211` + `Mgmt` + `Ap` → `Ieee80211MgmtAp`;
`Multicast` + `Router` + `6` → `MulticastRouter6`. Never reorder the words.

*Enforced at T4 — agent review; a wrong role suffix is the most common finding.*

### NR-NED-TYPE

**A module interface is `I<Concept>`; a network and a channel are `PascalCase` like any other NED type.**

- **A module interface is the concept prefixed with `I`** and lives in a `contract` package:
  `IApp`, `IMacProtocol`, `IRadio`, `INetworkInterface`, `IPacketQueue`, `IClock`. The `I` is
  the sole marker of substitutability — a `like`-typed submodule always names an `I*` type,
  and every `moduleinterface` starts with `I` (no exceptions).
- **Networks** are `PascalCase`, named for what they model; reusable bases carry `*Base`
  (`NetworkBase`, `WiredNetworkBase`, `WirelessNetworkBase`).
- **Channels** are `PascalCase`, named for their characteristics (`Eth100M`, and
  `DatarateChannel` extensions).

*Enforced at T4 — agent review.*

### NR-NED-GATE

**A gate is `<stem>In` or `<stem>Out`, and the stem names the peer it faces.**

- **Direction is a suffix, `In` or `Out`; a bidirectional gate is an `inout` and takes
  neither.** `upperLayerIn` and `upperLayerOut` are the two halves of one logical port.
- **The stem names the peer the gate faces.** Toward the adjacent layers:
  `upperLayerIn`/`upperLayerOut`, `lowerLayerIn`/`lowerLayerOut`. Toward a concrete peer:
  `socketIn`/`socketOut`, `appIn`/`appOut`, `transportIn`/`transportOut`, `ipIn`/`ipOut`,
  `queueIn`/`queueOut`, `mgmtIn`/`mgmtOut`, and the receive-only `radioIn`. A plain
  single-purpose port may be a bare `in`/`out`.
- **A gate vector adds `[]`, and its stem still carries the direction:** `upperLayerOut[]`,
  `generatorIn[]`. Prefer `<stem>In[]`/`<stem>Out[]` for new gate vectors.

*Enforced at T4 — agent review.*

### NR-NED-PARAM

**A parameter is `camelCase`, carries its `@unit` when it is a physical quantity, and means one thing.**

- **Parameters are `camelCase`**, named for the quantity they set: `bitrate`, `sendInterval`,
  `packetLength`, `startTime`.
- **A reference to another module is a `*Module` string parameter** holding a path:
  `interfaceTableModule`, `routingTableModule`, `clockModule`. The `Module` suffix says "this
  is a path to a module." This is the single most consistent parameter convention in INET.
- **A dynamic-type selection parameter is a `*Type` `typename` parameter** (`queueType`,
  `schedulerType`).
- **Booleans are `camelCase` and read as a state**, most often a bare adjective or verb
  (`forwarding`, `active`, `promiscuous`), sometimes `enable*` or `is*`. Prefer a bare state
  word; reserve `enable*` for a switch that turns an otherwise-present behaviour on or off.
- **Every physical quantity declares `@unit`** (`@unit(s)`, `@unit(bps)`, `@unit(m)`, …) — a
  parameter's unit is part of its contract, never a bare number.

*Enforced at T1 — `@unit` is checked by the NED compiler; T4 for the single-meaning rule.*

### NR-NED-PROP

**A property is `@lowercase`, and it is spelled the same wherever it appears.**

NED and message **properties are lowercase, camelCase, prefixed with `@`**, and take their
arguments either indexed in `[]` (one per named instance) or by value in `()`:
`@signal[packetSent](type=inet::Packet)`, `@class(Ipv4)`, `@display("i=block/network")`.
The OMNeT++-builtin properties (`@display`, `@class`, `@unit`, `@signal`, `@statistic`,
`@labels`, `@figure`, `@namespace`, `@mutable`, `@loose`, `@directIn`, `@enum`,
`@messageKinds`, `@defaultStatistic`, `@statisticTemplate`) are used as-is. INET adds a few
of its own, in the same lowercase style, each marking a capability the framework's tooling
looks for: **`@networkNode`** (a compound module is a node), **`@networkInterface`** (a NIC),
**`@lifecycleSupport`** (handles start/shutdown/crash), **`@application`** (an app submodule),
and **`@omittedTypename(OmittedFoo)`** (names the no-op type for an optional submodule).

*Enforced at T1 — the NED compiler rejects an unknown property.*

### NR-NED-SIGNAL

**A signal is `<subject><PastParticiple>` in NED and `<name>Signal` in C++.**

- **Signal and statistic names are `camelCase`, undecorated** in their NED `@signal[...]` /
  `@statistic[...]` bracket: `packetSent`, `packetReceived`, `queueLength`, `endToEndDelay`.
- **An event signal is `<subject><PastParticiple>` — it reports something that happened**:
  `packetSent`, `packetReceived`, `packetDropped`, `packetPushed`, `packetPulled`,
  `interfaceCreated`, `interfaceDeleted`, `linkBroken`. The recognized change verbs form a
  family: **Created / Deleted / Added / Removed / Changed / Sent / Received / Pushed / Pulled /
  Started / Ended / Broken**.
- **Direction toward a peer is a `To<Peer>` / `From<Peer>` suffix** (`packetSentToLower`,
  `packetReceivedFromUpper`); **a state-change signal is `<subject>Changed`**
  (`carrierSenseChanged`, `stateChanged`).
- **The C++ constant that holds the signal is the name plus a `Signal` suffix**:
  `registerSignal("packetSent")` ↔ `simsignal_t packetSentSignal`. The suffix appears only on
  the C++ identifier, never inside the registered string.

---

## Message (`.msg`) names

Modern INET represents a protocol data unit as a generic `Packet` carrying typed **chunks**;
the message types declared in `.msg` files are those chunks and the metadata that rides with
them.

*Enforced at T4 — agent review.*

### NR-MSG-TYPE

**A message type is named for the role it plays on the wire, or for the metadata it carries beside it.**

- **An on-wire header chunk is `<Protocol>Header`**; a trailer is `<Protocol>Trailer`; a frame
  check sequence is `<Protocol>Fcs` (`Ipv4Header`, `EthernetMacHeader`, `PppTrailer`,
  `EthernetFcs`).
- **A self-contained protocol message is `<Protocol>Packet`; a link-layer frame is
  `<Protocol>Frame`.** The two are role-distinct, not synonyms: `Frame` is an L2 PDU
  (`Ieee80211BeaconFrame`, `EthernetPauseFrame`), `Packet` an L3-and-above or generic PDU
  (`ArpPacket`, `Ospfv2HelloPacket`). Use these role-bearing suffixes, never a bare
  `Msg` / `Message`.
- **A packet-independent protocol request is `<Concept>Command`** (`Ipv4SocketCloseCommand`,
  `ConfigureRadioCommand`).
- **Metadata attached to a packet is a tag, suffixed by direction:**
  - **`*Tag`** — plain packet or region metadata (`PacketProtocolTag`, `CreationTimeTag`,
    `FlowTag`).
  - **`*Req`** — a **request**, flowing from a higher layer to a lower one (`DispatchProtocolReq`,
    `SocketReq`, `DscpReq`).
  - **`*Ind`** — an **indication**, flowing from a lower layer to a higher one
    (`DispatchProtocolInd`, `SocketInd`, `EcnInd`). Every `*Req` has a matching `*Ind`; use
    `Req`/`Ind`, not `Request`/`Indication`, for tags.

*Enforced at T4 — agent review.*

### NR-MSG-FIELD

**A field is `camelCase` and spelled out, and it carries its unit.**

- **Fields are `camelCase`, undecorated** — no Hungarian prefixes, no leading underscore:
  `sequenceNo`, `srcPort`, `destPort`, `headerLength`, `moreFragments`. Boolean fields usually
  read as a domain-specific state or flag rather than `is*`/`has*` (`moreFragments`,
  `dontFragment`, `synBit`, `ackBit`). Fields freely mix C++ primitives (`uint16_t`, `bool`)
  with INET value types (`Ipv4Address`, `MacAddress`, `B` for a byte quantity). Spell field
  names out (`srcAddress`/`destAddress`, not `src`/`dest`).
- **Each `.msg` opens with a `namespace` statement** (`namespace inet;`, or a protocol
  sub-namespace `namespace inet::tcp;`), never an `@namespace(...)` property (that property form
  is reserved for `package.ned`).
- **Message/field properties are lowercase `@` properties.** The role-bearing ones:
  **`@existingClass`** (this MSG type wraps a hand-written C++ class rather than generating one),
  **`@packetData`** (a field is part of the on-wire content), **`@enum(FooType)`** (an integer
  field ranges over a named enum), **`@owned`** (the object owns a pointed-to member), and the
  descriptor/accessor set `@getter`/`@setter`/`@sizeGetter`/`@byValue`/`@toString`/`@fromString`.

---

## C++ names

*Enforced at T4 — agent review.*

### NR-CPP-TYPE

**A C++ type is `PascalCase` and matches the NED type it implements.**

- **Classes, structs, and MSG-generated types are `PascalCase`.** Class-name suffixes mark the
  class's role, mirroring the module and message suffixes above (`*Base`, `*Table`, `*Filter`,
  `*Classifier`, `*Configurator`, `*Inserter`, `*Checker`, `*Visualizer`, `*Receiver`, …) plus
  a few C++-only roles:

  | Suffix | Role | Example |
  |---|---|---|
  | `*Chunk` | a packet-content representation | `SliceChunk`, `FieldsChunk` |
  | `*Serializer` | serializes/deserializes a chunk to bytes | `Ipv4HeaderSerializer` |
  | `*ProtocolDissector` | dissects a protocol's packets | `Ipv4ProtocolDissector` |
  | `*ProtocolPrinter` | renders a protocol's packets as text | `Ipv4ProtocolPrinter` |
  | `*Signal` | a physical-layer signal (analog phenomenon) | `WirelessSignal`, `EthernetSignal` |
  | `*Function` | a class wrapping a mathematical function | `ConstantFunction`, `AntennaGainFunction` |
  | `*Handler`, `*Mode`, `*Modulation`, `*Figure` | C++-only helper roles | `Ieee80211OfdmMode` |

  (The `*Impl` "hidden implementation" suffix is **not** used in INET — do not introduce it.)
- **Type aliases (`typedef` / `using`) are `PascalCase` like the types they abbreviate**,
  typically naming a collection: `InterfaceVector`, `RouteVector`, `TcpAppConnMap`,
  `MulticastGroupList`; alias templates too (`template<typename T> using Ptr = …`). The only
  lowercase aliases are `_t`-suffixed scalar aliases mirroring kernel style (`simtime_raw_t`)
  and the SI-symbol unit typedefs in `common/Units.h` (`bps`, `nW`).
- **Template parameters are single uppercase letters** — `T` by default, `R`/`D` for
  result/domain in the math library — using a descriptive `PascalCase` name (`Value`, `Unit`)
  only where the role needs disambiguating.
- **Enum *types* are `PascalCase`; enum *values* are `ALL_CAPS_WITH_UNDERSCORE`**
  (`enum State { IDLE, TRANSMITTING, BACKOFF }`). Protocol FSM tokens follow a
  `PROTOCOL_CATEGORY_NAME` macro form (`TCP_S_*` states, `TCP_E_*` events, `TCP_C_*` commands,
  `TCP_I_*` indications).
- **Namespaces are lowercase and match the NED subpackage**: everything is in `inet`, with a
  per-protocol sub-namespace where the protocol is substantial (`inet::tcp`, `inet::ieee80211`,
  `inet::physicallayer`, `inet::queueing`, `inet::visualizer`, `inet::ospfv2`).
- **Exceptions:** the idiomatic error is `throw cRuntimeError("…")`, not a custom hierarchy. On
  the rare occasion a dedicated exception type is warranted, name it `<Condition>Exception`
  (`ConnectionClosedException`); reserve `cTerminationException` for ending a run cleanly.

*Enforced at T3 — [`.clang-tidy`](../../../.clang-tidy).*

### NR-CPP-NAME

**A function, a member and a local are `camelCase`, and the verb says what the function does.**

- **Methods and free functions are `camelCase` and verb-first**; the subject is the object, not
  the name (`sendAck`, `retransmitData`, `findContainingNode`). Getters and setters are strictly
  `getX` / `setX`; predicates are `isX` / `hasX` (`isForwardingEnabled`, `hasCarrier`). The
  OMNeT++ lifecycle handlers keep their kernel names — `initialize`, `handleMessage`,
  `handleMessageWhenUp`, `handleSelfMessage`, `handleStartOperation`, `receiveSignal`,
  `refreshDisplay`, `finish` — and dispatched handlers follow `handle<Event>`
  (`handleUpperPacket`, `handleAckFrame`). Free functions live at namespace scope, marked
  `INET_API`.
- **Method arguments and local variables are `camelCase`, full words** (`servicePrimitive`,
  `localPort`, `relativeHeaderLength`). Short pointer nouns are acceptable for the obvious
  subject (`packet`, `msg`). Loop counters are `i`, iterators `it`/`iter`, and the range-`for`
  element binding is `elem`; prefer `auto` for iterators and casts.
- **Member variables are plain `camelCase`, with no `m_`, leading, or trailing underscore**
  (`socketId`, `localAddr`, `numRetries`, `sendQueue`); a boolean member may take an `is`
  prefix (`isNodeUp`).
- **Named constants are `ALL_CAPS_WITH_UNDERSCORE`** regardless of type
  (`INITSTAGE_LINK_LAYER`, `UDP_MAX_MESSAGE_SIZE`, `CFM_CCM_MULTICAST_ADDRESS`). The one
  sanctioned exception is a **named-instance constant** — a `static const` object that behaves
  like a registered enumerated value — which takes `camelCase` (`Protocol::ipv4`,
  `dsssHeaderMode1Mbps`). Keep new scalar constants ALL_CAPS.

*Enforced at T3 — [`.clang-tidy`](../../../.clang-tidy).*

### NR-CPP-REG

**A registration macro is `Register_<Thing>`, and the identifier it registers follows the domain's own spelling.**

- **Registration macros are named `Define_<Thing>` / `Register_<Thing>`** — each word
  capitalized, underscore-separated — and you use far more than you define. The common ones and
  the shape of their arguments: `Define_Module(Udp)` and `Define_Channel(Foo)` take the type
  identifier; `Register_Class(TcpNewReno)` / `Register_Abstract_Class(NetworkHeaderBase)` take a
  class; `Register_Serializer(Ipv4Header, Ipv4HeaderSerializer)` pairs a chunk with its
  serializer; `Register_Protocol_Dissector(&Protocol::ppp, PppProtocolDissector)` and
  `Register_Protocol_Printer(...)` key on a protocol; `Register_Enum`, `Define_InitStage`, and
  the `Register_Packet_*_Function` family round it out. (Note: the dissector/printer macros are
  `Register_Protocol_*`, *not* `Register_Packet_*`.)
- **A registered string name is cased for its kind.** Result filters and recorders take a
  `camelCase` string (`Register_ResultFilter("liveThroughput", LiveThroughputFilter)`), figures a
  lowercase one (`Register_Figure("gauge", GaugeFigure)`).
- **Registered protocol identifiers** live in `Protocol` as `camelCase` static members —
  `Protocol::ipv4`, `Protocol::ethernetMac`, `Protocol::ieee80211DsssPhy` — each constructed with
  a **lowercase, collapsed** string id (`"ethernetmac"`, `"ieee80211dsssphy"`) and a
  natural-caps human-readable name (`"Ethernet MAC"`, `"IEEE 802.11 DSSS PHY"`). They are listed
  alphabetically, split into a standard-protocols block and an INET-specific block.

*Enforced at T1 — the macro will not compile without its argument; T4 for the identifier spelling.*

### NR-CPP-TIMER

**A self-message member ends in `Timer` or `Msg`, and its name string matches the member.**

A self-message or timer is held in a `camelCase` member named for its purpose, ending in
`*Timer` (`txTimer`, `endTxTimer`, `retransmitTimer`) or `*Msg`. **Give the `cMessage` a
descriptive `camelCase` name string, ideally identical to the member** (`new cMessage("sendTimer")`).

---

## Configuration, build and assets

*Enforced at T4 — agent review.*

### NR-INI

**A config section is `[Config <PascalCase>]` and an iteration variable is `${camelCase}`.**

- **Config sections are `[Config PascalCase]`**, named for the scenario; a combination of
  configs joins their names with hyphens (`[Config Rstp-LargeNet]` `extends = LargeNet, Rstp`).
  A base config not meant to be run directly is marked `abstract = true`, and **every config
  carries a `description = "…"`**.
- **Parameter-study iteration variables `${name=…}` are `camelCase`** — a single lowercase word
  for simple sweeps (`${distance=10..550 step 2}m`, `${bitrate=6,9,…}Mbps`), camelCase for
  multi-word ones (`${advertisedWindowKiB=…}`).
- INET does not use `experiment-label` / `measurement-label` / `replication-label`; scenario
  identity comes from the config name and iteration variables.

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

### NR-FEATURE

**A feature id is the `PascalCase` protocol stem, with `Examples` and `Tests` companions.**

- **A feature `id` is `PascalCase` with acronyms as words** (`Aodv`, `Bgpv4`, `Ospfv2`, `Mrp`),
  paired with a free-text, spaced human-readable `name` (`"AODV"`, `"BGPv4 routing"`). Its
  `nedPackages` are lowercase dotted paths and its `compileFlags` define `-DINET_WITH_<UPPER_ID>`.
  Auxiliary features derived from a base feature append a role suffix:
  `<Feature>Examples`, `<Feature>Showcases`, `<Feature>Tests`, `<Feature>Tutorial`. New
  feature ids follow the `Aodv`/`Bgpv4` form.

*Enforced at T3 — `inet_featuretool` validates the descriptor.*

### NR-DIR

**A directory is lowercase and run-together, singular, and it matches the NED package it holds.**

- **Example, showcase, tutorial, and test directories are lowercase, run-together words**,
  usually the protocol or feature they exercise (`examples/aodv`, `examples/ethernet`,
  `showcases/tsn/framepreemption`, `tutorials/queueing`). The `tests/` subtree is partitioned by
  test *kind*: `fingerprint`, `module`, `unit`, `statistical`, `validation`, `packet`, `speed`.
  Prefer lowercase, run-together names for new scenario folders.

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

### NR-TEST

**A test file is named for what it tests, and a fingerprint tag is a lowercase descriptive word.**

- **A fingerprint test is identified by its working directory plus its run command**, not a
  separate name: `<dir>, -f <inifile> -c <ConfigName> -r <run#>, <timelimit>, <fingerprint>,
  <result>, <tags>`, one per line in a per-area CSV (`ethernet.csv`, `examples.csv`). The
  trailing **tags** are lowercase, space-separated model-area words used for filtering
  (`wireless adhoc Ipv4`, `routing`).
- **`tests/module/*.test` and `tests/unit/*.test` files are `<Subject>[_<n>].test`**
  (`AntennaOrientation_1.test`, `Clock_SettableLinear_1.test`) — `PascalCase` subject, optional
  numeric variant.

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

### NR-ASSET

**An icon file is lowercase and run-together, and its path is the icon name.**

A trailing `_vs`, `_s`, `_l` or `_vl` is the **OMNeT++ icon size suffix** and is not an underscore in
the name: `router_l.png` is the large variant of `router`. It is the one sanctioned underscore in an
asset name.

- **Icons under `images/` are lowercase, run-together file names** grouped in category folders
  (`block/`, `misc/`, `background/`, `maps/`, `3d/`), referenced from NED as
  `@display("i=<category>/<name>")` with no extension (`i=block/checker`, `i=misc/cloud`). A
  size variant appends a code — `_vs` / `_s` / `_l` / `_vl` — and numeric variants a digit
  (`truck2_vs`). Generic `device/`, `abstract/`, and `status/` icons come from the OMNeT++ core
  set and are referenced, not shipped, by INET.

---

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

### NR-GEN

**A generated file carries the name of its source, with the generator's suffix, and it is never
hand-edited.**

`Foo.msg` produces `Foo_m.h` and `Foo_m.cc`; the pair sits beside its source and takes its name from
it. The same holds for anything else a tool writes: the name says which file to edit instead. A
generated file follows the seal of its source — see
[SR-SEAL-GENERATED](sealing.md#sr-seal-generated).

*Enforced at T1 — the message compiler names the pair.*

### NR-TOOL

**A Python module is lowercase and run-together; a script is a lowercase hyphenated verb phrase that
says what it does.**

`python/inet/simulation/`, `python/inet/scave/` for the modules; `check-ned-file-names`,
`removetrailingspaces` for the scripts under `_scripts/`. A script that checks something starts with
`check-`, and one that changes files in place says so in its name. The historical run-together script
names (`removetrailingspaces`, `namespaceize.pl`) are the departures, not the rule.

*Enforced at T4 — agent review.*

### NR-CI

**A workflow file is named for the job it runs, lowercase and hyphenated, and it matches the test
category it drives.**

`fingerprint-tests.yml`, `module-tests.yml`, `statistical-tests.yml`, `build-linux.yml`. A workflow
that runs the test category `<x>` is `<x>-tests.yml`, so the failing check name in a pull request
says which suite broke without opening it. A rule gate is `check-<rule family>.yml`.

*Enforced at T3 — [check-naming.sh](../enforcement/check-naming.sh).*

## Auditing and exceptions

The rules above describe the target, not the current state of every file. Real deviations —
both permanent, sanctioned exceptions (`IPsec`, the `applications`/`networks` packages, the SI
unit typedefs) and violations still awaiting a rename (`Msg`/`Message` types, underscored
packages, `rtp` leading-underscore members, and so on) — are tracked in
[naming-exceptions.md](../audit/naming-exceptions.md), which is the rename backlog. When you audit a file
or area against this document, record what you find there (with a suggested fix and a status)
rather than fixing it silently or weakening a rule here to match the code.

## Deriving a protocol's whole vocabulary

The payoff of the scheme: from one protocol stem `Foo` you can write down every name without
looking anything up.

| Artifact | Name |
|---|---|
| package / directory / namespace | `foo` / `inet::foo` |
| protocol module / interface | `Foo` / `IFoo` |
| header chunk, serializer | `FooHeader`, `FooHeaderSerializer` |
| dissector, printer | `FooProtocolDissector`, `FooProtocolPrinter` |
| header inserter / checker modules | `FooHeaderInserter` / `FooHeaderChecker` |
| state table, configurator | `FooTable` / `FooRoutingTable`, `FooConfigurator` |
| request / indication tag | `FooReq` / `FooInd` |
| a "packet sent" signal (NED / C++) | `packetSent` / `packetSentSignal` |
| registered protocol identifier | `Protocol::foo` (`"foo"`, `"Foo"`) |
| module registration | `Define_Module(Foo);` |
| build feature id | `Foo` (+ `FooExamples`, `FooTests`) |
| example / test directory | `examples/foo`, `tests/…/foo` |
| IPv6 node variant | `FooRouter6` |
| files | `Foo.ned`, `Foo.cc`, `Foo.h`, `FooHeader.msg` (+ generated `FooHeader_m.h/.cc`) |

## Quick reference

| Shape | Meaning | Example |
|---|---|---|
| `PascalCase` | a type (NED type, C++ class/alias, MSG type) | `EthernetSwitch` |
| `camelCase` | instance, parameter, gate, field, signal, method, member, local | `interfaceTable` |
| `lowercase` (run-together) | package / directory / namespace / icon path | `linklayer` |
| `ALL_CAPS_WITH_UNDERSCORE` | enum value, constant, macro | `INITSTAGE_LINK_LAYER` |
| `I<Concept>` | module interface (`moduleinterface`) | `IRadio` |
| `Ieee<Std><Concept>` / `Ext<Concept>` | standard model / OS bridge | `Ieee80211Mac`, `ExtInterface` |
| `<Concept>Base` / `Table` / `Configurator` | base / node state / cross-scope configurator | `Ipv4RoutingTable` |
| `<Concept>Inserter` / `Checker` | add / verify-and-remove a header | `FooHeaderInserter` |
| `<Role>` queueing suffix | push/pull processing element | `PacketFilter`, `PacketScheduler` |
| `<Concept>CanvasVisualizer` / `OsgVisualizer` | 2D / 3D renderer | `MobilityCanvasVisualizer` |
| `<Concept>6` | IPv6 node/module variant | `Router6` |
| `<Protocol>Header` / `Trailer` / `Fcs` | on-wire chunk | `Ipv4Header` |
| `<Protocol>Packet` / `<Protocol>Frame` | L3+ PDU / L2 PDU | `ArpPacket` / `Ieee80211BeaconFrame` |
| `<Concept>Command` | packet-independent protocol request | `ConfigureRadioCommand` |
| `<Concept>Tag` / `Req` / `Ind` | metadata / request (down) / indication (up) | `SocketReq` / `SocketInd` |
| `<Chunk>Serializer` / `<Protocol>ProtocolDissector` / `ProtocolPrinter` | packet tooling | `Ipv4HeaderSerializer` |
| `<stem>In` / `<stem>Out` | gate facing a peer | `upperLayerIn` |
| `<name>Module` | parameter holding a module path | `interfaceTableModule` |
| `@<lowercase>` | NED/MSG property | `@lifecycleSupport`, `@display` |
| `<subject><PastParticiple>` / `<subject>Changed` | event / state-change signal | `packetSent`, `stateChanged` |
| `get<X>` / `set<X>` / `is<X>` / `has<X>` | accessor / predicate | `getRouterId`, `hasCarrier` |
| `handle<Event>` | dispatched event handler | `handleUpperPacket` |
| `<Verb>Vector` / `<K><V>Map` | C++ collection type alias | `RouteVector`, `TcpAppConnMap` |
| `<Name>Signal` | C++ `simsignal_t` for signal `<name>` | `packetSentSignal` |
| `Define_Module` / `Register_<Thing>` | registration macro | `Register_Serializer` |
| `Protocol::<name>` | registered protocol identifier | `Protocol::ethernetMac` |
| `*Timer` / `*Msg` | self-message / timer member | `retransmitTimer` |
| `[Config <PascalCase>]` / `${<camelCase>}` | ini config section / iteration variable | `[Config Handover]` |
| `<Type>_m.h` / `_m.cc` | generated code from `<Type>.msg` | `Ipv4Header_m.h` |
