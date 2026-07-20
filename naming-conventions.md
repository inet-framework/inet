# Naming Conventions

How things are named in the INET Framework. The goal is **guessability in both
directions**: given a concept, you can derive its name; given a name, you can tell what
*kind* of thing it is and what it does — without looking it up. A name is built from two
independent choices that compose: its **casing**, fixed by the *kind* of entity (a type,
a parameter, a package…), and its **role affixes**, a fixed vocabulary of prefixes and
suffixes that mark what part a component plays. Meaningful, scheme-following names take
priority over brevity.

The affix vocabulary is deliberately **redundancy-free**: each affix has one meaning, and
each role has one affix. Where two forms could express the same role, one is canonical and
the other is to be avoided — this is what makes `FooHeaderSerializer` the only name for the
serializer of a `FooHeader`. These conventions are the concrete form of the architectural
requirement [AR-QUAL-NAMING](architectural-requirements.md); both humans and tooling infer a
component's category and how to compose it from its name.

Where the current codebase deviates from a rule, this document states the rule
prescriptively and flags the deviation as *avoid* or *exception* — the rule is the target,
not merely a description of what exists.

## Casing by kind

Casing alone places a name in one of two families: **types** — things you instantiate,
extend, or send — are `PascalCase`; **everything you configure or call** — instances,
parameters, gates, members, methods — is `camelCase`.

| Kind | Casing | Example |
|---|---|---|
| NED types — `simple`, `module`, `network`, `channel`, `moduleinterface` | `PascalCase` | `EthernetSwitch`, `IRadio` |
| C++ classes and structs, MSG types | `PascalCase` | `Ipv4Header`, `MacAddress` |
| Submodule / instance names | `camelCase` | `radio`, `interfaceTable` |
| Parameters | `camelCase` | `sendInterval`, `bitrate` |
| Gates | `camelCase` + `In`/`Out` | `upperLayerIn`, `radioIn` |
| Signals and statistics | `camelCase` | `packetSent`, `endToEndDelay` |
| C++ methods | `camelCase`, verb-first | `handleMessage`, `getRouterId` |
| C++ member variables | `camelCase`, undecorated | `localPort`, `numRetries` |
| Enum *types* | `PascalCase` | `AccessCategory`, `ArpOpcode` |
| Enum *values*, named constants, macros | `ALL_CAPS_WITH_UNDERSCORE` | `INITSTAGE_LINK_LAYER` |
| Packages / directories | `lowercase`, singular | `linklayer`, `contract` |
| Namespaces | `lowercase` | `inet`, `inet::ieee80211` |
| Config sections (`.ini`) | `PascalCase` | `[Config WifiHandover]` |
| Files | the `PascalCase` type they define | `Ipv4Header.msg`, `Udp.cc` |

## Words and acronyms

- **An acronym is a word: capitalize only its first letter in a name.** Write
  `Ipv4Address`, not `IPv4Address`; `TcpConnection`, not `TCPConnection`; `Ieee80211Mac`,
  not `IEEE80211MAC`; likewise `UdpHeader`, `ArpPacket`, `SctpAssociation`,
  `Ospfv2HelloPacket`. This keeps word boundaries in a compound name unambiguous
  (`Ieee80211` + `Mac`). A few names keep their canonical real-world spelling as an
  established exception (`IPsec`).
- **No ad-hoc abbreviations.** Spell domain words out in type, parameter, signal, and
  member names (`address` not `addr`, `interface` not `iface`, `destination` not `dst`).
  The only sanctioned short forms are established domain terms that read unambiguously as
  their one expansion: `Mac`, `Phy`, `Nic`, `Fcs`, `Snir`, `Rng`, `Mtu`, and the protocol
  acronyms themselves (`Tcp`, `Udp`, `Ip`, `Ipv4`, `Ipv6`, `Arp`, `Ppp`, `Bgp`, `Ospf`, …).
- **American spelling** (`initialize`, `color`, `neighbor`).

## Packages and files

- **Package names are lowercase and singular**, and a package path always equals its
  directory path under `src/inet` (`inet.linklayer.ethernet.common` *is* the folder). The
  recurring functional subpackages carry fixed meanings: **`base`** holds shared base
  modules, **`contract`** holds the module interfaces, **`common`** holds cross-cutting
  infrastructure, **`configurator`** holds configurators. The protocol layers are
  `physicallayer`, `linklayer`, `networklayer`, `transportlayer`.
  *Exceptions:* two long-standing top-level packages are plural (`applications`,
  `networks`); a handful of deep packages use plurals (`…/tables`, `…/modes`) or
  underscores (`ospf_common`, `tcp_common`, `tcp_lwip`). Do not introduce new plural or
  underscored package names.
- **A file is named after the primary type it defines**, in that type's `PascalCase`:
  `Udp.ned` + `Udp.cc` + `Udp.h` define module `Udp`; `Ipv4Header.msg` defines
  `Ipv4Header`. Code generated from a `.msg` file carries the `_m` marker
  (`Ipv4Header_m.h`, `Ipv4Header_m.cc`) — the only underscore in an INET file name.

## Modules: role affixes

A module name reads as `[standard/vendor prefix] + Concept + [role suffix] + [variant]`.
The concept is a domain noun — very often itself a recognizable word such as `App`,
`Radio`, `Mobility`, `Mac`, `Receiver`, `Transmitter`, `Medium`, `Model`, `Cache`,
`Interface`, `Service`. The affixes around it mark its role and variation; read a module
name by stripping the affixes you recognize.

### Role suffixes — what the module *is*

Each suffix marks exactly one structural role, so the suffix alone tells you a module's
gate/behaviour shape:

| Suffix | Role | Example |
|---|---|---|
| `*Base` | abstract base module, extended by concrete variants | `MacProtocolBase`, `PacketFilterBase` |
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

### Role prefixes — how the module *varies*

| Prefix | Meaning | Example |
|---|---|---|
| `I*` | a module **interface** (`moduleinterface`), never a concrete module | `IRadio`, `IApp`, `IMobility` |
| `Ieee*` | a model of a specific IEEE standard | `Ieee80211ScalarRadio`, `Ieee802154NarrowbandMac` |
| `Ext*` | bridges to the external world / host OS | `ExtInterface`, `ExtLowerIpv4` |
| `Ep*` | energy/power domain (energy + power units) | `EpEnergyStorageBase` |
| `Cc*` | charge/current domain (charge + current units) | `CcBatteryPack` |
| `Omitted*` | the no-op passthrough standing in for an absent optional submodule | `OmittedPacketQueue`, `OmittedProtocolLayer` |
| `Compound*` | the compound-module variant of an otherwise simple concept | `CompoundPendingQueue` |

**Paired variants use a `Single*`/`Dual*` or `SingleRate*`/`DualRate*` prefix pair**, not a
suffix: `SimpleIeee8021qFilter` / `DualIeee8021qFilter`, `SingleRateThreeColorMeter` /
`DualRateThreeColorMeter`.

### The `6` variant marker

**Append `6` for the IPv6 variant of a node or module whose plain name is IPv4-default:**
`Router6`, `StandardHost6`, `WirelessHost6`, `HomeAgent6`. Protocol names themselves embed
their version rather than a marker (`Ipv4`/`Ipv6`, `Icmpv6`, `Ospfv2`/`Ospfv3`).

### Composition order

Prefixes and variants stack outward from the concept in one fixed order, so there is a single
legal spelling: standard/vendor prefix first, role suffix last, variant `6` at the very end.
`Ieee80211` + `Mgmt` + `Ap` → `Ieee80211MgmtAp`; `Multicast` + `Router` + `6` →
`MulticastRouter6`. Never reorder the words (not `MacIeee80211`).

## Interfaces, networks, and channels

- **A module interface is the concept prefixed with `I`** and lives in a `contract` package:
  `IApp`, `IMacProtocol`, `IRadio`, `INetworkInterface`, `IPacketQueue`, `IClock`. The `I` is
  the sole marker of substitutability — a `like`-typed submodule always names an `I*` type,
  and every `moduleinterface` starts with `I` (no exceptions).
- **Networks** are `PascalCase`, named for what they model; reusable bases carry `*Base`
  (`NetworkBase`, `WiredNetworkBase`, `WirelessNetworkBase`).
- **Channels** are `PascalCase`, named for their characteristics (`Eth100M`, and
  `DatarateChannel` extensions).

## Gates

- **Direction is a suffix, `In` or `Out`; a bidirectional gate is an `inout` and takes
  neither.** `upperLayerIn` and `upperLayerOut` are the two halves of one logical port.
- **The stem names the peer the gate faces.** Toward the adjacent layers:
  `upperLayerIn`/`upperLayerOut`, `lowerLayerIn`/`lowerLayerOut`. Toward a concrete peer:
  `socketIn`/`socketOut`, `appIn`/`appOut`, `transportIn`/`transportOut`, `ipIn`/`ipOut`,
  `queueIn`/`queueOut`, `mgmtIn`/`mgmtOut`, and the receive-only `radioIn`. A plain
  single-purpose port may be a bare `in`/`out`.
- **A gate vector adds `[]`, and its stem still carries the direction:** `upperLayerOut[]`,
  `generatorIn[]`. *Exception:* the historical link-frame vector `ethg[]` (Ethernet) predates
  this rule; prefer `<stem>In[]`/`<stem>Out[]` for new gate vectors.

## Parameters

- **Parameters are `camelCase`**, named for the quantity they set: `bitrate`, `sendInterval`,
  `packetLength`, `startTime`.
- **A reference to another module is a `*Module` string parameter** holding a path:
  `interfaceTableModule`, `routingTableModule`, `clockModule`, `macTableModule`. The `Module`
  suffix says "this is a path to a module," distinguishing the reference from the referenced
  module's own data. This is the single most consistent parameter convention in INET.
- **A dynamic-type selection parameter is a `*Type` `typename` parameter** used to pick a
  submodule's type (`queueType`, `schedulerType`).
- **Booleans are `camelCase` and read as a state**, most often a bare adjective or verb
  (`forwarding`, `active`, `promiscuous`, `padding`), sometimes `enable*` or `is*`. Prefer a
  bare state word; reserve `enable*` for a switch that turns an otherwise-present behaviour on
  or off.
- **Every physical quantity declares `@unit`** (`@unit(s)`, `@unit(bps)`, `@unit(m)`,
  `@unit(B)`, `@unit(W)`, `@unit(dBm)`, …) — a parameter's unit is part of its contract, never
  a bare number.

## Signals and statistics

- **Signal and statistic names are `camelCase`, undecorated** in their NED `@signal[...]` /
  `@statistic[...]` bracket: `packetSent`, `packetReceived`, `queueLength`, `endToEndDelay`.
- **An event signal is `<subject><PastParticiple>` — it reports something that happened**:
  `packetSent`, `packetReceived`, `packetDropped`, `packetPushed`, `packetPulled`,
  `interfaceCreated`, `interfaceDeleted`, `linkBroken`. The recognized change verbs form a
  family: **Created / Deleted / Added / Removed / Changed / Sent / Received / Pushed / Pulled /
  Started / Ended / Broken**.
- **Direction toward a peer is a `To<Peer>` / `From<Peer>` suffix**:
  `packetSentToLower`, `packetReceivedFromUpper`, `packetSentToPeer`.
- **A state-change signal is `<subject>Changed`** (`carrierSenseChanged`,
  `transmissionStateChanged`, `stateChanged`).
- **In C++ the registered signal is the name plus a `Signal` suffix.** The
  `registerSignal("packetSent")` string is exactly the signal name; the `simsignal_t` constant
  that holds it is `packetSentSignal`. The `Signal` suffix appears only on the C++ identifier,
  never inside the registered name.

## Packets, headers, and tags

Modern INET represents a protocol data unit as a generic `Packet` carrying typed **chunks**;
the message types declared in `.msg` files are those chunks and the metadata that rides with
them.

- **An on-wire header chunk is `<Protocol>Header`**; a trailer is `<Protocol>Trailer`; a frame
  check sequence is `<Protocol>Fcs` (`Ipv4Header`, `EthernetMacHeader`, `PppTrailer`,
  `EthernetFcs`).
- **A self-contained protocol message is `<Protocol>Packet`; a link-layer frame is
  `<Protocol>Frame`.** The two are role-distinct, not synonyms: `Frame` is an L2 PDU
  (`Ieee80211BeaconFrame`, `EthernetPauseFrame`), `Packet` an L3-and-above or generic PDU
  (`ArpPacket`, `Ospfv2HelloPacket`). *Avoid* the bare, meaningless suffixes `Msg` / `Message`.
- **A packet-independent protocol request is `<Concept>Command`** (`Ipv4SocketCloseCommand`,
  `ConfigureRadioCommand`).
- **Metadata attached to a packet is a tag, suffixed by direction:**
  - **`*Tag`** — plain packet or region metadata (`PacketProtocolTag`, `CreationTimeTag`,
    `FlowTag`, `DirectionTag`).
  - **`*Req`** — a **request**, flowing from a higher layer to a lower one (`DispatchProtocolReq`,
    `InterfaceReq`, `SocketReq`, `DscpReq`).
  - **`*Ind`** — an **indication**, flowing from a lower layer to a higher one
    (`DispatchProtocolInd`, `SocketInd`, `EcnInd`). Every `*Req` has a matching `*Ind`; use
    `Req`/`Ind`, not `Request`/`Indication`, for tags.

## C++ classes, methods, and members

- **Class-name suffixes mark the class's role**, mirroring the module and message suffixes
  above (`*Base`, `*Table`, `*Filter`, `*Classifier`, `*Configurator`, `*Inserter`,
  `*Checker`, `*Visualizer`, `*Receiver`, `*Cache`, …) plus a few C++-only roles:

  | Suffix | Role | Example |
  |---|---|---|
  | `*Chunk` | a packet-content representation | `SliceChunk`, `FieldsChunk` |
  | `*Serializer` | serializes/deserializes a chunk to bytes | `Ipv4HeaderSerializer` |
  | `*ProtocolDissector` | dissects a protocol's packets | `Ipv4ProtocolDissector` |
  | `*ProtocolPrinter` | renders a protocol's packets as text | `Ipv4ProtocolPrinter` |
  | `*Signal` | a physical-layer signal (analog phenomenon) | `WirelessSignal`, `EthernetSignal` |
  | `*Function` | a class wrapping a mathematical function | `ConstantFunction`, `AntennaGainFunction` |
  | `*Handler`, `*Mode`, `*Modulation`, `*Figure` | C++-only helper roles | `TcpAlgorithm`, `Ieee80211OfdmMode` |

  (The `*Impl` "hidden implementation" suffix is **not** used in INET — do not introduce it.)
- **Methods are `camelCase` and verb-first**; the subject is the object, not the name.
  Getters and setters are strictly `getX` / `setX`; predicates are `isX` / `hasX`
  (`isForwardingEnabled`, `hasCarrier`). The OMNeT++ lifecycle handlers keep their kernel
  names — `initialize`, `handleMessage`, `handleMessageWhenUp`, `handleSelfMessage`,
  `handleStartOperation`, `receiveSignal`, `refreshDisplay`, `finish` — and dispatched
  handlers follow `handle<Event>` (`handleUpperPacket`, `handleAckFrame`, `handleClockEvent`).
- **Member variables are plain `camelCase`, with no `m_`, leading, or trailing underscore**
  (`socketId`, `localAddr`, `numRetries`, `sendQueue`). A boolean member may take an `is`
  prefix (`isNodeUp`). *Exception:* the `rtp` subsystem uses a legacy leading-underscore style;
  do not copy it.
- **Enum *types* are `PascalCase`; enum *values* and named constants are
  `ALL_CAPS_WITH_UNDERSCORE`** (`enum State { IDLE, TRANSMITTING, BACKOFF }`,
  `INITSTAGE_LINK_LAYER`). Protocol FSM constants follow a `PROTOCOL_CATEGORY_NAME` macro form
  (`TCP_S_*` states, `TCP_E_*` events, `TCP_C_*` commands, `TCP_I_*` indications). *Avoid* the
  competing `PascalCase` static-constant style seen in a few headers; keep constants ALL_CAPS.
- **Namespaces are lowercase and match the NED subpackage**: everything is in `inet`, with a
  per-protocol sub-namespace where the protocol is substantial (`inet::tcp`, `inet::ieee80211`,
  `inet::physicallayer`, `inet::queueing`, `inet::visualizer`, `inet::ospfv2`).

## Deriving a protocol's whole vocabulary

The payoff of the scheme: from one protocol stem `Foo` you can write down every name without
looking anything up.

| Artifact | Name |
|---|---|
| package / directory | `foo` (under the owning layer) |
| C++ namespace | `inet::foo` |
| protocol module | `Foo` |
| module interface (if pluggable) | `IFoo` |
| header chunk (`.msg`) | `FooHeader` |
| header serializer | `FooHeaderSerializer` |
| protocol dissector | `FooProtocolDissector` |
| protocol printer | `FooProtocolPrinter` |
| header inserter / checker modules | `FooHeaderInserter` / `FooHeaderChecker` |
| state table (if any) | `FooTable` / `FooRoutingTable` |
| network configurator (if any) | `FooConfigurator` |
| a request / indication tag | `FooReq` / `FooInd` |
| a "packet sent" signal (NED / C++) | `packetSent` / `packetSentSignal` |
| IPv6 node variant | `FooRouter6` |
| files | `Foo.ned`, `Foo.cc`, `Foo.h`, `FooHeader.msg` (+ generated `FooHeader_m.h/.cc`) |

## Quick reference

| Shape | Meaning | Example |
|---|---|---|
| `PascalCase` | a type (NED type, C++ class, MSG type) | `EthernetSwitch` |
| `camelCase` | an instance, parameter, gate, signal, method, member | `interfaceTable` |
| `lowercase` (singular) | package / directory / namespace | `linklayer` |
| `ALL_CAPS_WITH_UNDERSCORE` | enum value, constant, macro | `INITSTAGE_LINK_LAYER` |
| `I<Concept>` | module interface (`moduleinterface`) | `IRadio` |
| `Ieee<Std><Concept>` | model of a specific IEEE standard | `Ieee80211Mac` |
| `Ext<Concept>` | bridge to the host OS / external world | `ExtInterface` |
| `<Concept>Base` | abstract base module/class | `MacProtocolBase` |
| `<Concept>Table` / `Configurator` | shared node state / cross-scope configurator | `Ipv4RoutingTable` |
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
| `<subject><PastParticiple>` / `<subject>Changed` | event / state-change signal | `packetSent`, `stateChanged` |
| `get<X>` / `set<X>` / `is<X>` / `has<X>` | accessor / predicate | `getRouterId`, `hasCarrier` |
| `handle<Event>` | dispatched event handler | `handleUpperPacket` |
| `<Name>Signal` | C++ `simsignal_t` for signal `<name>` | `packetSentSignal` |
| `<Type>_m.h` / `_m.cc` | generated code from `<Type>.msg` | `Ipv4Header_m.h` |
