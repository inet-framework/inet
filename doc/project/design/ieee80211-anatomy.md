# What the IEEE 802.11 model is made of

> **Kind:** design · **Status:** snapshot 2026-09-21 · **Seal:** none · **Owns:** — · **Stands on:** [node-anatomy.md](node-anatomy.md), [protocol-anatomy.md](protocol-anatomy.md), [../domain/ieee80211.md](../domain/ieee80211.md)

The parts of the IEEE 802.11 model, the responsibility of each part, and the means by which the
parts communicate: at run time, during initialization, and during the start, stop and crash
operations of a node. It is a survey: a map of the code at commit `4548adeb04` (2026-09-21) that a
reader can check against that code. It is not a specification of what the model must do. The
developer's guide chapter `doc/src/developers-guide/ch-80211.rst` states the design of the model,
and it replaces this document when that chapter is complete; until then the two are read together,
this one for the exact calls and stages, the chapter for the intended shape.

The scope is the two subtrees `src/inet/linklayer/ieee80211/` and
`src/inet/physicallayer/wireless/ieee80211/`, plus the generic wireless parts that the 802.11 radio
stands on. Two guide chapters exist for the model. The user's guide chapter
`doc/src/users-guide/ch-80211.rst` names the submodules of the interface, the parts of the MAC, the
radio variants, the management variants and the agent, and says what each one is for. The
developer's guide chapter `doc/src/developers-guide/ch-80211.rst` is a placeholder. The chapter
`doc/src/developers-guide/ch-physicallayer.rst` describes the generic wireless framework. This
document does not repeat those chapters. It adds what they do not say: the responsibility of each
part in terms of the code, the means by which the parts communicate, the services each part offers
and uses, the initialization stages, and the lifecycle operations.

**Path conventions.** `mac/` means `src/inet/linklayer/ieee80211/mac/`. `mgmt/`, `llc/`, `mib/` and
`portal/` are its sibling folders. `phy/` means
`src/inet/physicallayer/wireless/ieee80211/packetlevel/`, `mode/` means
`src/inet/physicallayer/wireless/ieee80211/mode/`, and `wireless/` means
`src/inet/physicallayer/wireless/common/`. A *module* is a NED module with a C++ class. An *object*
is a plain C++ object that a module creates with `new` and owns. Interface names start with `I`.

**2026-09-23 composition update:** this snapshot retains its historical shared
management-recovery description. The current implementation places that procedure
inside each EDCAF and binds it to the same EDCAF's CW. See the current
[developer guide](../../src/developers-guide/ch-80211.rst) for composition, dispatch
and configuration/signal migration.

## 1. The interface at a glance

`Ieee80211Interface` (`src/inet/linklayer/ieee80211/Ieee80211Interface.ned`) is a compound module
that extends `NetworkInterface`. It holds seven submodules and wires them. The figure shows the
submodules, the gates, and the kind of traffic on each connection.

```
                 upperLayerIn                      upperLayerOut
                      │                                 ▲
                      ▼                                 │
                ┌────────────┐                          │
                │ classifier │ egress only              │
                └─────┬──────┘                          │
                      │ packet + UserPriorityReq        │
                      ▼                                 │
  ┌───────┐     ┌──────────┐                       ┌────┴─────┐
  │ agent │◄───►│   mgmt   │                       │   llc    │
  └───────┘     └────┬─────┘                       └────┬─────┘
   primitives        │ mgmt frame bodies,               │ data packets
   (cMessage +       │ radio commands                   │ + MacAddressReq/Ind
   control info)     ▼                                  ▼
  ┌───────┐     ┌────────────────────────────────────────────┐
  │  mib  │◄═══►│                    mac                     │
  └───────┘     │   dcf │ hcf │ ds │ rx │ tx  (submodules)   │
   shared       └──────────────────────┬─────────────────────┘
   object          MAC frames +        │      ▲ signals: reception state,
                   radio commands      ▼      │ transmission state, signal part
                ┌────────────────────────────────────────────┐
                │                   radio                    │
                │  transmitter │ receiver │ antenna │ energy │
                └──────────────────────┬─────────────────────┘
                                       │ radioIn: WirelessSignal by sendDirect
                                       ▼
                             radioMedium (one per network)
```

| Slot | Default type | Replaceable | Responsibility |
| --- | --- | --- | --- |
| `mib` | `Ieee80211Mib` | no | Shared state: address, mode, BSS data, station table, HT state. |
| `classifier` | `OmittedIeee8021dQosClassifier` | yes | Sets the user priority tag on egress packets. Omitted by default. |
| `llc` | `Ieee80211LlcLpd` (`Ieee80211LlcEpd` when `opMode == "p"`) | yes | Encapsulates the payload in an LLC/SNAP or EtherType header. An access point uses `Ieee80211Portal` here. |
| `agent` | `Ieee80211AgentSta` | yes, or absent | Decides when to scan, authenticate and associate. |
| `mgmt` | `Ieee80211MgmtSta` | yes | The management plane: beacons, scan, authentication, association. Variants exist for AP, ad hoc, and simplified use. |
| `mac` | `Ieee80211Mac` | yes | Frame encapsulation, dispatch, channel access, acknowledgement, retry. |
| `radio` | `Ieee80211Radio` | yes | The physical layer: modes, channels, transmission and reception, error model. |

The connections are:

- `upperLayerIn → classifier → llc → mac` and `mac → llc → upperLayerOut`. The classifier is on the
  egress path only.
- `mgmt.macOut → mac.mgmtIn` and `mac.mgmtOut → mgmt.macIn`. Management traffic has its own gate pair
  on the MAC (`IIeee80211Mac.ned`).
- `agent.mgmtOut → mgmt.agentIn` and `mgmt.agentOut → agent.mgmtIn`, only when the agent exists.
- `mac.lowerLayerOut → radio.upperLayerIn` and `radio.upperLayerOut → mac.lowerLayerIn`.
- `radioIn → radio.radioIn`. The medium delivers signals to this gate with `sendDirect`.
- The MIB has no gates. Every module that needs it holds a pointer that it resolves from the
  `mibModule` parameter.

Three parameters of the interface fan out: `opMode` sets `**.opMode` (the radio's mode set) and
`mac.modeSet`; `bitrate` sets `**.bitrate` and the data bitrate of the rate selection; `address`
becomes the MAC address. The parameters `*.macModule`, `*.mibModule`, `*.interfaceTableModule` and
`*.energySourceModule` give the submodules the paths for their direct calls.

## 2. The nine means of communication

The model uses nine distinct mechanisms. The table names each one, shows what it looks like in the
code, and says where the 802.11 model uses it. The sections below refer to them by name.

| Means | In the code | Used for |
| --- | --- | --- |
| **Packet on a gate** | `send(packet, "lowerLayerOut")`; local metadata travels in tags ([D-TAGS](decisions.md#d-tags)) | LLC ↔ MAC, mgmt → MAC, MAC ↔ radio |
| **Command message on a gate** | a `Request` message with kind `RADIO_C_CONFIGURE` and a `ConfigureRadioCommand` control info | mgmt → MAC → radio (channel change), MAC → radio (radio mode) |
| **Primitive message on a gate** | a bare `cMessage` with an `Ieee80211PrimRequest` or `Ieee80211PrimConfirm` control info (`mgmt/Ieee80211Primitives.msg`) | agent ↔ mgmt |
| **Direct call through a contract interface** | `rx->lowerFrameReceived(packet)`; the pointer comes from a submodule, from a parameter path such as `par("rxModule")`, from the gate neighbour, or from the interface module that contains the caller ([D-DIRECT](decisions.md#d-direct)) | everywhere inside the MAC and the radio |
| **Callback interface** | the caller passes `this` and gets a call later, for example `requestChannel(this)` or `transmitFrame(packet, header, ifs, this)` | channel grant, transmission complete, frame sequence events, control responses |
| **Signal** | `emit` and `subscribe` ([D-NOTIFY](decisions.md#d-notify), [D-OBSERVE](decisions.md#d-observe)) | radio state to the MAC, mode set to the MAC parts, channel change to the AP management, association events to the node, statistics |
| **Shared object** | a pointer to a module or object that several parts read and write | the MIB, the mode set, the pending queue and the in-progress frames of a coordination function |
| **Self-message timer** | `scheduleAt` on a `cMessage` owned by the module | every wait: backoff, IFS, ACK timeout, NAV, beacon, scan, block ack inactivity |
| **NED parameter assignment** | a compound module sets a parameter of its submodules, for example `*.rxModule = "^.rx"` | module paths and shared values, set once at build time |

Two conventions matter for the reader. First, **a direct call crosses module boundaries freely**,
and the caller finds the callee in one of four ways: `getSubmodule("rx")`,
`getModuleByPath(par("rxModule"))` with a default that the parent NED sets,
`gate("lowerLayerOut")->getNextGate()->getOwnerModule()`, or
`getContainingNicModule(this)->getSubmodule("mac")`. Second, **a callback is a direct call in the
other direction**: the module that offers a service also declares a nested `ICallback` interface,
and the client implements it. `Dcf` and `Hcf` each implement five such callback interfaces.

## 3. The parts and their responsibilities

### 3.1 The upper edge: classifier, LLC, network interface

| Part | Files | Responsibility |
| --- | --- | --- |
| `NetworkInterface` | `src/inet/networklayer/common/NetworkInterface.{ned,cc}` | The interface entry. Generates the MAC address when `address` is `"auto"`, adds itself to the interface table, holds the up/down state and the carrier flag. |
| `IIeee8021dQosClassifier` | `src/inet/linklayer/common/` | Maps a packet to a user priority 0..7 and adds a `UserPriorityReq` tag. `QosClassifier` maps by IP protocol and port. The default `OmittedIeee8021dQosClassifier` is a pass-through. |
| `Ieee80211LlcLpd` | `llc/Ieee80211LlcLpd.*`, base `src/inet/linklayer/ieee8022/Ieee8022Llc.*` | LLC/SNAP encapsulation (802.2 length/type-independent). Adds only `getProtocol()`, which returns `ieee8022llc`. |
| `Ieee80211LlcEpd` | `llc/Ieee80211LlcEpd.*` | EtherType encapsulation (`Ieee802EpdHeader`). Chosen for `opMode == "p"`. |
| `Ieee80211Portal` | `portal/Ieee80211Portal.*` | Converts Ethernet frames to LLC/SNAP frames and back. Replaces the LLC in `AccessPoint.ned`. |

The LLC variants share the NED interface `IIeee80211Llc` and the C++ interface of the same name,
which has one method, `getProtocol()`. The MAC calls it to tag the payload protocol of a received
frame. The tag `LlcProtocolTag` records which LLC produced a frame, because nothing on the wire
says so.

### 3.2 The management plane: management modules, agent, MIB

| Part | Files | Responsibility |
| --- | --- | --- |
| `Ieee80211MgmtBase` | `mgmt/Ieee80211MgmtBase.*` | Common base (an `OperationalBase`). Dispatches timers, frames and commands; builds the Supported Rates element from the mode set; sends frame bodies down to the MAC. |
| `Ieee80211MgmtSta` | `mgmt/Ieee80211MgmtSta.*` | The station MLME: scan (active or passive), authentication, association, reassociation, beacon timeout. Writes the association state into the MIB. |
| `Ieee80211MgmtAp` | `mgmt/Ieee80211MgmtAp.*`, base `Ieee80211MgmtApBase.*` | The access point MLME: beacons, probe responses, authentication, association, the station table in the MIB, AID allocation. |
| `Ieee80211MgmtAdhoc` | `mgmt/Ieee80211MgmtAdhoc.*` | Sets the MIB mode to `INDEPENDENT` and does nothing else. No beacons. |
| `Ieee80211MgmtStaSimplified`, `Ieee80211MgmtApSimplified` | `mgmt/*Simplified.*` | No management frames at all. The simplified station writes itself as associated into its own MIB and into the AP's MIB, which it finds through the address resolver. The simplified AP sends nothing and drops every management frame. |
| `Ieee80211AgentSta` | `mgmt/Ieee80211AgentSta.*` | Drives the station management with primitives: scan, then authenticate with the best BSS by received power, then associate. Reacts to `l2BeaconLost` with a new scan. |
| `Ieee80211Mib` | `mib/Ieee80211Mib.*` | A plain shared state module with no gates and no behaviour of its own. |

The MIB fields and their owners:

| Field | Meaning | Written by | Read by |
| --- | --- | --- | --- |
| `address` | own MAC address | `Ieee80211Mac` (from the network interface) | MAC, `Ds`, management, `Rx` (a cached copy) |
| `mode` | `INFRASTRUCTURE`, `INDEPENDENT` or `MESH` | the management module at init | MAC (address roles), `Ds` |
| `qos` | QoS station | `Ieee80211Mac` (from `qosStation`) | MAC (chooses `dcf` or `hcf`) |
| `bssData.ssid`, `bssData.bssid` | the BSS | management | MAC (address 3), `Ds` |
| `bssStationData.stationType`, `isAssociated` | `ACCESS_POINT` or `STATION`; association state of a station | management | MAC (drops data when not associated), `Ds`, agent |
| `bssAccessPointData.stations` | the AP's station table with the status of each station | `Ieee80211MgmtAp`, the simplified station | MAC (drops data to unknown stations), `Ds` (forwards) |
| `associationIds` and reservations | committed and reserved AIDs | the MIB itself, on request of `Ieee80211MgmtAp` | `Ieee80211MgmtAp` |
| HT capabilities, HT operation, primary channel, peer HT state | the HT-related part of the MIB, deliberately partial | `Ieee80211Mac` (local capabilities), `Ieee80211MgmtApBase` (primary channel from the radio), management (peer state) | management, rate selection |

### 3.3 The MAC: `Ieee80211Mac` and its five submodules

`Ieee80211Mac` (`mac/Ieee80211Mac.{ned,cc}`) extends `MacProtocolBase`, which extends
`LayeredProtocolBase` and `OperationalBase`. It is a compound module with a simple-module class:
the C++ class dispatches, and the submodules do the work.

| Part | Files | Responsibility |
| --- | --- | --- |
| `Ieee80211Mac` | `mac/Ieee80211Mac.*` | Splits messages into upper, lower, management and command. Encapsulates a data packet into a MAC frame with addresses from the MIB. Decapsulates a received frame into a packet with indication tags. Builds the MAC header for a management body. Routes every frame to `dcf` or `hcf` by `mib->qos`. Relays the radio state signals to `rx` and `tx`. Controls the radio mode. Forwards radio commands from management, with a delay while the medium is busy. Registers the interface entry (address, MTU). Publishes the mode set. |
| `Rx` | `mac/Rx.*` | Checks the FCS of each received frame. Keeps the NAV timer. Computes *medium free* from the reception state, the transmission state and the NAV, and tells every registered contention when it changes. Reports a corrupt frame to the contentions. |
| `Tx` | `mac/Tx.*` | Transmits one frame after an inter-frame space: fills the transmitter address and the FCS, waits the IFS on a timer, hands the frame to the MAC. When the radio reports the end of the transmission, calls back the requester and extends the NAV in `Rx` by the duration field. |
| `Ds` | `mac/Ds.*` | The distribution service. A station passes a received data frame up when associated and when the frame came from its BSSID. An access point passes the frame up, forwards it to another station through the MAC, or does both for a group address, as the station table dictates. |
| `Dcf` | `mac/coordinationfunction/Dcf.*` | The distributed coordination function, for a non-QoS station. See §3.4. |
| `Hcf` | `mac/coordinationfunction/Hcf.*` | The hybrid coordination function, for a QoS station. Only EDCA exists in the code. See §3.5. |
| `Pcf`, `Mcf` | `mac/coordinationfunction/{Pcf,Mcf}.*` | Placeholders. Every entry point throws. The MAC declares members for them but never creates them. |

The five submodules exist as `dcf`, `hcf` (only when `qosStation`), `ds`, `rx` and `tx`. The MAC
sets `*.rxModule = "^.rx"` and `*.txModule = "^.tx"` so that every part below can find `Rx` and
`Tx` by path.

Both coordination functions share one skeleton. Each one:

- offers `ICoordinationFunction` to the MAC: `processUpperFrame`, `processLowerFrame`,
  `corruptedFrameReceived`;
- owns one `FrameSequenceHandler` object, which runs one frame exchange at a time;
- owns the RTS, CTS and recipient ACK procedure objects, which build control frames;
- implements the callbacks `IChannelAccess::ICallback` (channel granted), `ITx::ICallback`
  (transmission complete), `IFrameSequenceHandler::ICallback` (transmit this, a step succeeded or
  failed, the sequence finished) and `IProcedureCallback` (send this control response, process
  this management frame);
- holds one timer, `startRxTimer`, for the response timeout of a receive step.

### 3.4 Inside `Dcf`

`Dcf` has one channel access, one queue pair, one frame exchange at a time, and one set of
policies. Every part below is a submodule of `Dcf` unless marked *object*.

| Part | Type | Responsibility |
| --- | --- | --- |
| `channelAccess` | `Dcaf` (fixed) | Owns the pending queue, the in-progress frames and the contention. Computes DIFS and EIFS from the mode set. Grants the channel to `Dcf` and keeps the contention window (`incrementCw`, `resetCw`). |
| `channelAccess.pendingQueue` | `like IPacketQueue`, default `PendingQueue` | Holds packets from the upper layer. A `DropTailQueue` whose comparator puts management frames first. |
| `channelAccess.inProgressFrames` | `InProgressFrames` (fixed) | Holds the frames that are under transmission. Pulls the next frame from the pending queue through the originator data service, on demand. |
| `channelAccess.contention` | `like IContention`, default `Contention` | The backoff state machine: `IDLE`, `DEFER`, `IFS_AND_BACKOFF`. Learns the medium state from `Rx`. Reports the grant through two self-messages with scheduling priority 1000. |
| `originatorMacDataService` | `OriginatorMacDataService` (fixed) | Transmit-side data plane: assigns the sequence number, fragments when the policy asks for it. |
| `recipientMacDataService` | `RecipientMacDataService` (fixed) | Receive-side data plane: duplicate removal, reassembly. It does not process control frames. |
| `rateSelection` | `RateSelection` (fixed) | Picks the mode for every transmitted frame and for every response frame. Asks the rate control when present. Downgrades to a mode the peer supports, from the MIB's peer HT state. |
| `rateControl` | `like IRateControl`, absent by default | Adaptive rate control: `AarfRateControl`, `ArfRateControl`, `OnoeRateControl`. Gets success and failure feedback from `Dcf`. |
| `recoveryProcedure` | `NonQosRecoveryProcedure` (fixed) | Counts short and long retries per frame and per station. Decides when the retry limit is reached. Doubles or resets the contention window through `Dcaf`. |
| `originatorProtectionMechanism` | fixed | Computes the Duration/ID field of RTS, data and management frames. |
| `ackHandler` | `AckHandler` (fixed) | Records the acknowledgement state of every in-progress frame. `InProgressFrames` asks it which frame may be transmitted. |
| `originatorAckPolicy`, `recipientAckPolicy` | `like I...AckPolicy` | Whether a frame needs an ACK, the ACK timeout, the ACK duration field. |
| `rtsPolicy`, `ctsPolicy` | `like IRtsPolicy`, `like ICtsPolicy` | The RTS threshold and CTS timeout; whether to answer an RTS (only when the medium is free), the CTS duration field. |
| `FrameSequenceHandler` | *object* | Runs a frame sequence step by step. Asks the sequence for the next step, transmits or waits, and reports each outcome to `Dcf`. |
| `DcfFs` and the primitive sequences | *objects*, one per exchange | The grammar of a DCF exchange: `[RTS CTS] (fragment ACK)* last-fragment ACK`, or a group-addressed frame with no response. Built from the combinators `SequentialFs`, `AlternativesFs`, `RepeatingFs`, `OptionalFs`. |
| `FrameSequenceContext` | *object*, one per exchange | Everything one exchange needs: own address, mode set, the in-progress frames, the RTS procedure and policies, the ACK policy. |
| `RtsProcedure`, `CtsProcedure`, `RecipientAckProcedure` | *objects* | Build an RTS, a CTS in reply to an RTS, an ACK in reply to a frame. The reply procedures ask their policy and call back `Dcf`. |
| `StationRetryCounters` | *object* | The station-wide short and long retry counters. |

Data flow inside `Dcf`: the pending queue holds packets. `InProgressFrames` pulls one packet
through the data service when it has no eligible frame. The frame sequence asks `InProgressFrames`
for the frame to transmit. `Dcf` drops a frame from `InProgressFrames` when it is acknowledged or
when the retry limit is reached.

### 3.5 Inside `Hcf`

`Hcf` repeats the `Dcf` skeleton, but with one channel access per access category, and with
aggregation and block acknowledgement added. The four access categories are `AC_BK`, `AC_BE`,
`AC_VI` and `AC_VO`, in this order of index and priority.

| Part | Type | Responsibility |
| --- | --- | --- |
| `edca` | `Edca` (fixed) | Holds the `edcaf[0..3]` array. Maps a TID to an access category. Finds the channel owner and the internally collided EDCAFs. |
| `edca.edcaf[ac]` | `Edcaf` (count `numEdcafs`, default 4) | One channel access per access category. Computes AIFS, CW min and CW max from the AC and the mode set. Owns its own pending queue, in-progress frames, contention, `QosAckHandler`, `QosRecoveryProcedure` and `TxopProcedure`. Asks the collision controller before it grants. |
| `edca.collisionController` | `like ICollisionController`, default `EdcaCollisionController` | Records the expected transmission start of each EDCAF. An internal collision exists when a higher-priority EDCAF has the same start time. |
| `edca.mgmtAndNonQoSRecoveryProcedure` | `NonQosRecoveryProcedure` (fixed) | Retry counters for management and non-QoS frames, shared by all access categories. Its CW calculator is `edcaf[1]`, the best-effort EDCAF. |
| `edcaf[ac].txopProcedure` | `TxopProcedure` (fixed) | The transmit opportunity: start time, limit from a per-AC table (0 for `AC_BK` and `AC_BE`), time left. |
| `hcca` | `Hcca` (fixed) | A stub. Never owns the channel; every request throws. |
| `originatorMacDataService` | `OriginatorQosMacDataService` (fixed), one for all ACs | Transmit-side data plane: A-MSDU aggregation (policy), sequence number per (receiver, TID), fragmentation (policy). A-MPDU aggregation is present but not wired. |
| `recipientMacDataService` | `RecipientQosMacDataService` (fixed) | Receive-side data plane: duplicate removal per (transmitter, TID), block ack reordering, reassembly, A-MSDU deaggregation. |
| `singleProtectionMechanism` | fixed | Duration/ID for RTS, data, management and BlockAckReq frames under single protection. Multiple protection throws. |
| `rateSelection`, `rateControl` | `QosRateSelection` (fixed), `like IRateControl` | As in `Dcf`, plus a mode for BlockAck responses and a TXOP-aware rule for control frames. |
| `originatorAckPolicy`, `recipientAckPolicy`, `rtsPolicy`, `ctsPolicy` | QoS variants | As in `Dcf`, plus the block ack policy decision, the BlockAckReq trigger (after `blockAckReqThreshold` outstanding frames) and the block ack timeouts. |
| `originatorBlockAckAgreementPolicy`, `recipientBlockAckAgreementPolicy` | `like I...Policy`, only when `isBlockAckSupported` | When to request an agreement, whether to accept one, buffer size, timeout. |
| Block ack handlers and procedures | *objects*, only when both policies exist | The agreement tables per (peer, TID); the ADDBA, DELBA, BlockAckReq and BlockAck frame builders; the per-MPDU bitmap record on the recipient side. |
| `HcfFs`, `TxOpFs` | *objects*, one per exchange | The grammar of an EDCA exchange: a group-addressed burst, or a repeat of TXOP rounds. Each round is one of `[RTS CTS] data ACK`, `[RTS CTS] data` under block ack, `BlockAckReq BlockAck`, or `[RTS CTS] management ACK`. `HtTxOpFs`, `McfFs` and `PcfFs` exist but are empty and unused. |
| `inactivityTimer` | timer | The shared block ack inactivity timer; the handlers ask `Hcf` to schedule it through `IBlockAckAgreementHandlerCallback`. |

The context of an exchange carries a `QoSContext` with the in-progress frames and the TXOP
procedure of the access category that won the channel. One `FrameSequenceHandler` serves all four
access categories, so a second grant during a sequence is an error.

### 3.6 The physical layer

| Part | Files | Responsibility |
| --- | --- | --- |
| `Ieee80211Radio` | `phy/Ieee80211Radio.*`, bases `wireless/base/packetlevel/FlatRadioBase`, `NarrowbandRadioBase`, `wireless/radio/packetlevel/Radio` | The radio module: radio mode, transmission and reception timers, the PHY header, the 802.11 configure command, band and channel, the FCS mode of the PHY header. Emits the state signals. |
| `Ieee80211ScalarRadio`, `Ieee80211DimensionalRadio`, `Ieee80211UnitDiskRadio` | `phy/*.ned` | NED-only variants that set the analog representation. `Ieee80211OfdmRadio` (`bitlevel/`) swaps in the layered OFDM transmitter and receiver. |
| `Ieee80211Transmitter` | `phy/Ieee80211Transmitter.*` | Picks the mode (from the `Ieee80211ModeReq` tag, else the bitrate request, else its own mode) and the channel, computes the duration from the mode, and creates an `Ieee80211Transmission` that carries mode and channel. |
| `Ieee80211Receiver` | `phy/Ieee80211Receiver.*` | Admits a transmission only when its mode is in this receiver's mode set. Adds `Ieee80211ModeInd` and `Ieee80211ChannelInd`. The SNIR threshold, the error model, and the other indications come from the generic bases. |
| Error models | `phy/errormodel/` | `Ieee80211NistErrorModel` and `Ieee80211YansErrorModel` compute the success rate of the header and the data part per modulation and code. `Ieee80211BerTableErrorModel` reads a table. A failed packet is delivered with the bit-error flag set; the MAC drops it. |
| `Ieee80211ModeSet`, the modes | `mode/` | Static tables, one per `opMode` string (`a`, `b`, `g(mixed)`, `g(erp)`, `p`, `n(mixed-2.4Ghz)`, `ac`). A mode gives bitrates, durations, and the timing constants. The MAC takes slot time, SIFS, CW min and CW max from the `referenceMode` of the set, not from the mode of a frame. |
| Bands and channels | `mode/Ieee80211Band.*`, `Ieee80211Channel.*` | A channel is a (band, index) pair. The band maps the index to a center frequency. |
| `Ieee80211RadioMedium` | `phy/Ieee80211RadioMedium.ned`, base `wireless/medium/RadioMedium` | Adds no code. Sets four default values (background noise, center frequency 2.4 GHz for the limit cache, minimum reception and interference power). |
| `Ieee80211PhyHeader` | `phy/Ieee80211PhyHeader.msg` | One PHY header chunk per PHY family. The radio prepends and pops it. |
| Energy consumers | `phy/Ieee80211StateBased{Ep,Cc}EnergyConsumer.ned` | Power or current per radio state. They listen to the radio's state signals. |

## 4. Communication paths

### 4.1 Upper layer, LLC, MAC

**Egress.** A packet enters on `upperLayerIn` with a `MacAddressReq` (set by the network layer)
and a `PacketProtocolTag`. The classifier may add a `UserPriorityReq`. The LLC inserts its header
and sets `PacketProtocolTag` to `ieee8022llc` or `ieee802epd`. The MAC then:

1. drops the packet when the station is not associated, or when the AP has no association for the
   destination (`Ieee80211Mac::handleUpperPacket`);
2. records the payload protocol in an `LlcProtocolTag`, builds an `Ieee80211DataHeader` with the
   addresses that the MIB mode and station type dictate, sets the QoS subtype and the TID when a
   `UserPriorityReq` is present, and appends the `Ieee80211MacTrailer` (`encapsulate`);
3. hands the frame to `dcf` or `hcf` (`processUpperFrame`).

**Ingress.** The MAC pops the header and the trailer, and sets `PacketProtocolTag` (from
`llc->getProtocol()`), `MacAddressInd`, `InterfaceInd`, and `UserPriorityInd` for a QoS frame
(`decapsulate`). A management frame goes out on `mgmtOut`. A data frame goes to `Ds`, which calls
`mac->sendUp` and the packet leaves on `upperLayerOut` towards the LLC, which pops its header and
sets the protocol tags for the network layer.

No module under `src/inet/linklayer/ieee80211/` calls `registerService` or `registerProtocol`. The
inherited registration in `Ieee8022Llc` is behind a parameter that defaults to false. The LLC
delivers upward by the protocol registrations that come down from the network layer.

### 4.2 Agent, management, MAC, MIB, radio

**Agent to management.** Six request primitives: scan, authenticate, deauthenticate, associate,
reassociate, disassociate. Four confirm primitives with a result code: scan, authenticate,
associate, reassociate. Each travels as a `cMessage` with the primitive as control info, on the
`agent`/`mgmt` gate pair.

**Management to MAC.** The management module builds the **body** of a management frame and sends
it on `macOut` with three tags: `MacAddressReq` (the receiver), `Ieee80211SubtypeReq` (the frame
subtype) and `PacketProtocolTag = ieee80211Mgmt`. The MAC builds the `Ieee80211MgmtHeader`
(subtype, receiver, address 3 = BSSID on an AP) and the trailer, and `Tx` fills the transmitter
address last. Received management frames come back on `macIn` with the header already popped. The
management module reads the header back by a peek of 24 bytes behind the front offset.

**Management to radio, through the MAC.** A station changes channel with an
`Ieee80211ConfigureRadioCommand` that carries only the channel number. It goes out on `macOut`.
The MAC forwards it to the radio at once when `rx->isMediumFree()`, otherwise it stores the message
as `pendingRadioConfigMsg` and merges a later command into it. The stored command leaves when the
current frame sequence finishes, or when the contention enters `IDLE` or `DEFER`.

**Radio to management.** `Ieee80211Radio` emits `radioChannelChanged` with the band in the details
object. `Ieee80211MgmtApBase` subscribes to it on the radio and writes the primary channel into the
MIB. `Ieee80211MgmtSta` subscribes to `receptionStateChanged` on the host during an active scan,
to detect a busy channel.

**MAC to management.** `Ieee80211MgmtAp` subscribes to `frameTransmissionOutcome` on the MAC. An
association response is marked with an `Ieee80211MgmtTransactionTag`; the AP reserves the AID when
it builds the response and commits it only when the outcome says *acknowledged*.

**Management to MIB.** The management module is the writer of the mode, the station type, the BSS
data, the association flag, the station table and the peer HT state (§3.2). The MAC and `Ds` read
them on every frame.

**Management to the node.** `Ieee80211MgmtSta` emits `l2Associated` and `l2BeaconLost`.
`Ieee80211AgentSta` emits `l2Disassociated`, `l2AssociatedNewAp`, `l2AssociatedOldAp`.
`Ieee80211MgmtAp` emits `l2ApAssociated`, `l2ApDisassociated`. The listeners outside the interface
are `Ieee80211VisualizerBase`, `DhcpClient` and `Ipv6NeighbourDiscovery` (on `l2Associated`), and
`Pmipv6` (on the AP signals). No module writes to the `NetworkInterface` from the management plane.

### 4.3 MAC and radio

**Down.** `Tx` calls `mac->sendDownFrame`. The MAC switches the radio to transmitter mode with a
`ConfigureRadioCommand` on the same gate, sets `PacketProtocolTag = ieee80211Mac`, and sends the
frame. The coordination function has already set `Ieee80211ModeReq` (from the rate selection). The
radio's `encapsulate` prepends the PHY header and the transmitter creates the transmission.

**Up.** The radio decapsulates the PHY header, sets `PacketProtocolTag = ieee80211Mac`, and
delivers the packet with the indications `Ieee80211ModeInd`, `Ieee80211ChannelInd`,
`SignalPowerInd`, `SnirInd`, `SignalTimeInd` and `ErrorRateInd`. A packet that failed the SNIR
threshold or the error model arrives with the bit-error flag set. `Rx` treats a bit error, an
incorrect chunk and a bad FCS in the same way: it emits `packetDropped` with reason
`INCORRECTLY_RECEIVED`, tells the contentions (they switch to EIFS), and the MAC tells the
coordination function (`corruptedFrameReceived`).

**Radio state to the MAC.** The MAC subscribes on the radio to `receptionStateChanged`,
`transmissionStateChanged` and `receivedSignalPartChanged`, and relays them to `Rx`, which
recomputes *medium free*. On the transition `TRANSMITTING → IDLE` the MAC calls
`tx->radioTransmissionFinished()` and switches the radio back to receiver mode. The subscription to
`radioModeChanged` has no handler.

**Direct calls to the radio.** At link-layer initialization the MAC calls `radio->setRadioMode`
for the initial mode, and reads the antenna, transmitter and receiver to compute the HT
capabilities. The MAC finds the radio as the neighbour of its `lowerLayerOut` gate.

**The mode set.** The MAC reads `modeSet` and calls `Ieee80211ModeSet::getModeSet`. It publishes
the pointer with `modesetChanged` on the interface module. Every part that needs timing or modes
subscribes to that signal at `INITSTAGE_LOCAL`: all `ModeSetListener` subclasses (`Dcf`, `Hcf`, the
policies, the protection mechanisms, the rate controls), `Dcaf`, `Edcaf`, the rate selections and
`Ieee80211MgmtBase`. The radio's transmitter and receiver build their own mode set from `opMode`.
The interface wires both from one parameter.

### 4.4 Inside the MAC

The call chains, in the order of a transmission:

| From | To | Means | What |
| --- | --- | --- | --- |
| `Ieee80211Mac` | `Dcf`/`Hcf` | direct, `ICoordinationFunction` | `processUpperFrame`, `processLowerFrame`, `corruptedFrameReceived` |
| `Dcf`/`Hcf` | `Dcaf`/`Edcaf` | direct, `IChannelAccess` | `requestChannel(this)`, `releaseChannel` |
| `Dcaf`/`Edcaf` | `Contention` | direct, `IContention` | `startContention(cw, ifs, eifs, slot, this)` |
| `Rx` | `Contention` | direct, `IContention` | `mediumStateChanged`, `corruptedFrameReceived`; `Rx` learned the contention from `registerContention` |
| `Contention` | `Dcaf`/`Edcaf` | callback, `IContention::ICallback` | `channelAccessGranted`, `expectedChannelAccess` |
| `Edcaf` | `EdcaCollisionController` | direct, `IEdcaCollisionController` | `expectedChannelAccess(edcaf, time)`, `isInternalCollision(edcaf)` |
| `Dcaf`/`Edcaf` | `Dcf`/`Hcf` | callback, `IChannelAccess::ICallback` | `channelGranted(channelAccess)` |
| `Dcf`/`Hcf` | `FrameSequenceHandler` | direct | `startFrameSequence(sequence, context, this)`, `processResponse`, `transmissionComplete`, `handleStartRxTimeout` |
| `FrameSequenceHandler` | `Dcf`/`Hcf` | callback, `IFrameSequenceHandler::ICallback` | `transmitFrame`, `scheduleStartRxTimer`, `originatorProcessTransmittedFrame`, `originatorProcessReceivedFrame`, `originatorProcessFailedFrame`, `originatorProcessRtsProtectionFailed`, `frameSequenceFinished` |
| frame sequences | `InProgressFrames` | shared object, through the context | `getFrameToTransmit`, `hasInProgressFrames` |
| `InProgressFrames` | data service, `AckHandler` | direct, by parameter path | `extractFramesToTransmit(pendingQueue)`, `frameGotInProgress`, `isEligibleToTransmit` |
| `Dcf`/`Hcf` | `RateSelection` | direct, `IRateSelection` | `computeMode`, `computeResponse...Mode` |
| `Dcf`/`Hcf` | `Tx` | direct, `ITx` | `transmitFrame(packet, header, ifs, this)` |
| `Tx` | `Ieee80211Mac` | direct | `sendDownFrame`, `getAddress`, `getFcsMode` |
| `Ieee80211Mac` | `Tx` | direct | `radioTransmissionFinished` (from the radio signal) |
| `Tx` | `Dcf`/`Hcf` | callback, `ITx::ICallback` | `transmissionComplete(packet, header)` |
| `Tx` | `Rx` | direct, `IRx` | `frameTransmitted(duration)` sets or extends the NAV |
| `Dcf`/`Hcf` | recovery, rate control, ack handler | direct | on ACK: reset counters and CW, report success, mark acknowledged, drop the frame; on failure: count the retry, double the CW, report failure, set the retry bit or drop |
| `RecipientAckProcedure`, `CtsProcedure`, block ack procedures | `Dcf`/`Hcf` | callback, `IProcedureCallback` | `transmitControlResponseFrame(response, received)` |
| block ack handlers | `Hcf` | callbacks | `processMgmtFrame` (ADDBA, DELBA), `scheduleInactivityTimer` |
| `Dcf`/`Hcf` | `Ieee80211Mac` | direct | `sendUpFrame`, `sendDownPendingRadioConfigMsg`, `getAddress` |
| `Ds` | `Ieee80211Mac` | direct | `sendUp`, `processUpperFrame` (a forwarded frame re-enters the MAC) |
| pending queue | `Dcf`/`Hcf` | signal `packetDropped` | a queue overflow; re-emitted as `frameTransmissionOutcome` for a management frame |

### 4.5 Radio, medium, energy consumer

The radio registers itself with the medium at `INITSTAGE_PHYSICAL_LAYER` with `medium->addRadio`.
It finds the medium through the `radioMediumModule` parameter. To transmit, the radio calls
`medium->transmitPacket`, and the medium sends a `WirelessSignal` by `sendDirect` to the `radioIn`
gate of every potential receiver. During a reception the radio asks the medium
`isReceptionAttempted`, `isReceptionSuccessful`, `getReceptionDecision` and `receivePacket`, and the
medium asks the receiver's `computeIsReception...` methods. The medium subscribes to the radio's
signals only when its filter parameters are on; by default a channel change does not reach the
medium's cached listening objects.

The energy consumer is a submodule of the radio. It subscribes to the five radio state signals on
its parent and registers with the energy source at `INITSTAGE_POWER`.

### 4.6 Signals that carry control, and signals that only feed statistics

Some signals change the behaviour of a listener. The reader must know these:

| Signal | Emitter | Listener that acts | Effect |
| --- | --- | --- | --- |
| `receptionStateChanged`, `transmissionStateChanged`, `receivedSignalPartChanged` | radio | `Ieee80211Mac` → `Rx`, `Tx` | medium free, NAV, transmission complete |
| `modesetChanged` | `Ieee80211Mac` | every mode set listener | timing constants, mode tables |
| `radioChannelChanged` | `Ieee80211Radio` | `Ieee80211MgmtApBase` | primary channel in the MIB |
| `frameTransmissionOutcome` | `Dcf`/`Hcf` | `Ieee80211MgmtAp` | commit of the AID after an acknowledged response |
| `packetDropped` on a pending queue | `PendingQueue` | `Dcf`/`Hcf` | outcome report for a management frame |
| `l2BeaconLost` | `Ieee80211MgmtSta` | `Ieee80211AgentSta` | new scan |
| `l2Associated`, `l2ApAssociated`, `l2ApDisassociated` | management | `DhcpClient`, `Ipv6NeighbourDiscovery`, `Pmipv6`, visualizers | node-level reactions |
| the five radio state signals | radio | energy consumers | power state |

All other signals of the model feed `@statistic` declarations only: `packetSentToPeer`,
`packetReceivedFromPeer`, `linkBroken`, `frameSequenceStarted/Finished`, `datarateSelected`,
`channelOwnershipChanged`, `backoffStarted/Stopped`, `contentionWindowChanged`, `navChanged`,
`txopStarted/Ended`, `edcaCollisionDetected`, `blockAckAgreementAdded/Deleted`,
`packetFragmented/Defragmented/Aggregated/Deaggregated`, and the generic `packetSentToUpper` and
`packetReceivedFromLower` family.

## 5. Services offered and used

*Offers* lists the C++ interfaces a part implements, or the NED interface when the part has no C++
contract. *Uses* lists the interfaces of other parts that it calls.

| Part | Offers | Uses |
| --- | --- | --- |
| `Ieee80211Interface` | `IWirelessInterface` (NED) | `IInterfaceTable` (adds the interface entry) |
| classifier | `IIeee8021dQosClassifier` (NED) | — |
| `Ieee80211LlcLpd` | `IIeee80211Llc`, `IIeee8022Llc`, protocol registration listener | — |
| `Ieee80211LlcEpd`, `Ieee80211Portal` | `IIeee80211Llc` | — |
| `Ieee80211AgentSta` | `IIeee80211Agent` (NED) | the MIB, `IInterfaceTable`, the management primitives |
| management modules | `IIeee80211Mgmt` (NED), `ILifecycle` | the MIB, `IInterfaceTable`, the MAC gates, the radio (AP: channel signal), the MAC (AP: outcome signal) |
| `Ieee80211Mib` | none, a shared object | — |
| `Ieee80211Mac` | `IIeee80211Mac` (NED), `ILifecycle`; concrete methods for `Tx`, `Ds`, `Dcf`, `Hcf` | `IIeee80211Llc`, `IRadio`, `IRx`, `ITx`, `IDs`, `ICoordinationFunction` (as the concrete `Dcf` and `Hcf`) |
| `Rx` | `IRx` | `IContention` (registered by `Dcaf`/`Edcaf`), `Ieee80211Mac` (the address) |
| `Tx` | `ITx` | `Ieee80211Mac`, `IRx`, `ITx::ICallback` |
| `Ds` | `IDs` | `Ieee80211Mac`, the MIB |
| `Dcf` | `ICoordinationFunction`, `IChannelAccess::ICallback`, `ITx::ICallback`, `IFrameSequenceHandler::ICallback`, `IProcedureCallback` | `IChannelAccess`, `ITx`, `IRx`, `IRateSelection`, `IRateControl`, the two data services, the four policies, `IRecoveryProcedure`, `IFrameSequenceHandler`, the procedures, `Ieee80211Mac` |
| `Hcf` | the five above plus `IBlockAckAgreementHandlerCallback` | `Edca`, `Edcaf`, `Hcca`, `ITx`, `IRx`, `IQosRateSelection`, the QoS data services and policies, the block ack handlers and procedures, `Ieee80211Mac` |
| `Dcaf` | `IChannelAccess`, `IContention::ICallback`, `IRecoveryProcedure::ICwCalculator` | `IContention`, `IRx`, `IPacketQueue`, `InProgressFrames` |
| `Edcaf` | the same three | the same, plus `IEdcaCollisionController`, `QosAckHandler`, `QosRecoveryProcedure`, `TxopProcedure` |
| `Contention` | `IContention` | `IContention::ICallback`, `Ieee80211Mac` (flushes a held radio command) |
| `FrameSequenceHandler` | `IFrameSequenceHandler` | `IFrameSequence`, `IFrameSequenceHandler::ICallback`, `InProgressFrames` |
| data services | `IOriginatorMacDataService`, `IRecipientMacDataService` or `IRecipientQosMacDataService` | the aggregation, fragmentation, sequence number, duplicate removal and reassembly objects, through their `I...` interfaces |
| rate selection | `IRateSelection` or `IQosRateSelection` | `IRateControl`, the MIB (peer HT state), the mode set |
| `Ieee80211Radio` | `IRadio`, `IPhysicalLayer`, `ILifecycle` | `IRadioMedium`, `IAntenna`, `ITransmitter`, `IReceiver` |
| `Ieee80211Transmitter` | `ITransmitter` | the mode set, the bands, the analog model |
| `Ieee80211Receiver` | `IReceiver` | `IErrorModel`, the mode set, the medium's analog model |
| `Ieee80211RadioMedium` | `IRadioMedium` | `IPropagation`, `IPathLoss`, `IObstacleLoss`, `IMediumAnalogModel`, `IBackgroundNoise`, the caches |
| error models | `IErrorModel` | the mode objects |
| energy consumers | `IEpEnergyConsumer` or `ICcEnergyConsumer` | the energy source, the radio signals |

## 6. Initialization

INET numbers its initialization stages at run time from the dependencies in
`src/inet/common/InitStages.cc`. The order that matters here is `LOCAL` (0), `POWER` (7),
`PHYSICAL_LAYER` (8), `NETWORK_INTERFACE_CONFIGURATION` (10), `LINK_LAYER` (13), `LAST` (23). A
compound module runs its own `initialize(stage)` before its submodules run theirs. A module that
does not override `numInitStages()` runs only stage 0.

| Stage | What happens in the 802.11 interface |
| --- | --- |
| `LOCAL` | **Interface:** gates, interface table reference, initial up state and carrier. **MAC:** the mode set from `modeSet`, the FCS mode, the MIB reference, `mib->qos`. **`Rx`, `Tx`:** timers; `Tx` finds the MAC and `Rx`. **`Ds`, MIB:** references and parameters (one stage only). **`Dcf`, `Dcaf`, policies:** subscribe to `modesetChanged`; policies find the rate selection. **`Hcf`, `Edcaf`:** find all their parts and create their objects and timers. **Management:** the MIB and interface table, the mode set subscription, the MIB mode and station type (`isAssociated = false` on a station); the AP subscribes to `radioChannelChanged` on the radio and to `frameTransmissionOutcome` on the MAC, and creates the beacon timer. **Agent:** schedules its `startUp` timer and subscribes to `l2BeaconLost`. **Radio:** timers, antenna, transmitter, receiver, the medium reference; the transmitter and receiver build their mode set from `opMode` and their band and channel. **Medium:** its submodules and filters. **Energy consumer:** parameters, subscription to the radio signals. |
| `POWER` | The energy consumer registers with the energy source. |
| `PHYSICAL_LAYER` | **Radio:** `medium->addRadio(this)`, the initial radio mode, the mode switch times. `Ieee80211Radio` applies `channelNumber` and emits `radioChannelChanged`; the AP management writes the primary channel into the MIB. The radio's operational state is set here and its `handleStartOperation` runs. |
| `NETWORK_INTERFACE_CONFIGURATION` | **Interface:** generates the address when `"auto"`, adds itself to the interface table. **MAC:** `registerInterface` copies the address into `mib->address` and sets the MTU and the capability flags. **`Rx`:** caches the address. **Management:** its operational state is set here (`isInitializeStage`), so `start()` runs: the AP schedules its first beacon after a random delay. |
| `LINK_LAYER` | **MAC:** finds the LLC and the radio as gate neighbours, `ds`, `rx`, `tx`, `dcf`, `hcf` as submodules; subscribes to the radio signals; computes the HT capabilities into the MIB; emits `modesetChanged`; sets the initial radio mode. Its operational state is set here. **`Dcf`:** finds all its parts and creates its objects and timer. **`Dcaf`, `Edcaf`:** `rx->registerContention(contention)`, the timing constants. **`Edca`:** builds the EDCAF array. **Rate selection:** the rate control, the fixed modes. **AP management:** `bssid = mib->address`. **Simplified station:** writes its association. **Agent:** caches the interface. **`Ieee8022Llc`:** sockets and registration. |
| `LAST` | **`Dcf`, `Hcf`:** subscribe to `packetDropped` on the pending queues, because the queues resolve only at `LINK_LAYER`. **Recovery procedures:** find the RTS policy and the CW calculator, read the retry limits, emit the initial CW. **`Contention`:** the `initialChannelBusy` state. **AP management:** checks the 40 MHz HT configuration. **Simplified station:** writes its association a second time. |

The order that a reader should keep in mind:

1. **The address flows in one stage.** The interface generates it, the MAC copies it into the MIB,
   and `Rx` caches it, all at `NETWORK_INTERFACE_CONFIGURATION`, in parent-before-child order.
2. **The radio publishes its channel before the MAC publishes the mode set.** The AP management
   needs both, and completes the HT operation at `LAST`.
3. **The management modules start before the MAC is wired.** Their `start()` runs at stage 10,
   the MAC finds its peers at stage 13. This works because the beacon timer fires at a later
   simulation time.
4. **The agent schedules its first action at `LOCAL`.** Its `startUp` timer fires at
   `startingTime`, or at a random time within `maxChannelTime` when `startingTime` is negative.
5. **`Dcf` and `Hcf` resolve their parts at different stages** (`LINK_LAYER` and `LOCAL`). Both
   work, because submodules exist from construction. Both defer the queue subscription to `LAST`.
6. **The first start is part of initialization.** `OperationalMixin::initialize` calls
   `handleStartOperation(nullptr)` at the module's `isInitializeStage` when the node has no
   `status` submodule or its state is `UP`. See §7.

## 7. Lifecycle operations

**The mechanism.** `LifecycleController` runs a `ModuleStartOperation`, `ModuleStopOperation` or
`ModuleCrashOperation` on a node. For each stage of the operation it walks the module tree and calls
`handleOperationStage` on every module that implements `ILifecycle`. A module that extends
`OperationalMixin` (`OperationalBase`, and so `LayeredProtocolBase`, `MacProtocolBase`,
`PhysicalLayerBase`, `Ieee80211MgmtBase`) answers only at its own stage and calls its
`handleStartOperation`, `handleStopOperation` or `handleCrashOperation`. The start stages run
bottom-up (`PHYSICAL_LAYER` before `LINK_LAYER`), the stop stages top-down (`LINK_LAYER` before
`PHYSICAL_LAYER`), and a crash has one stage. While a module is `NOT_OPERATING`, a packet from the
radio is deleted, a self-message throws, and any other message throws unless it arrives in the
same instant as the state change.

| Part | Stage | Start | Stop | Crash |
| --- | --- | --- | --- | --- |
| `NetworkInterface` | `LINK_LAYER` | state `UP`, carrier on | state `DOWN`, carrier off | same as stop |
| `Ieee80211Mac` | `LINK_LAYER` | re-applies the initial radio mode; does nothing when called from `initialize` | **empty**, marked `FIXME`; does not call `MacProtocolBase` | **empty**, marked `FIXME` |
| `Rx`, `Tx`, `Ds`, `Dcf`, `Hcf` and every part inside them | — | none | none | none |
| management modules | `PHYSICAL_LAYER` | `start()`: the AP schedules its beacon; the simplified station re-associates (not when called from `initialize`) | `stop()`: the station ends a scan, cancels every authentication and association timer and the beacon timeout, clears the association and the peer HT state; the AP cancels the beacon timer and clears its station list, the AIDs and the station table; both emit no signal | same as stop |
| `Ieee80211AgentSta` | — | none (`TODO add ILifecycle`) | none | none |
| `Ieee80211LlcLpd` (`Ieee8022Llc`) | `LINK_LAYER` | clears sockets | clears sockets | clears sockets |
| LLC EPD, portal, classifier, MIB | — | none | none | none |
| radio | `PHYSICAL_LAYER` | re-applies the initial radio mode | cancels the switch timer, "aborts" the transmission (the signal already in flight continues to propagate), sets mode `OFF` | same as stop |
| radio medium | — | none | none | none |

**What the MAC sees when the radio stops.** The radio emits `radioModeChanged` (`OFF`), then
`receptionStateChanged` and `transmissionStateChanged` with the `UNDEFINED` values, then the part
signals with `NONE`. `Rx` recomputes *medium free*. The transition `TRANSMITTING → IDLE` does not
happen, so `Tx` gets no `radioTransmissionFinished` for a transmission that a crash cut short.

**What does not happen on a MAC stop.** Nothing inside the MAC resets. The pending queues and the
in-progress frames keep their packets. The acknowledgement states, the retry counters, the
contention window, the block ack agreements and the reorder buffers keep their values. A frame sequence
in progress keeps its context. The timers `startRxTimer`, `inactivityTimer`, the NAV timer, the
IFS timer and the contention timers stay scheduled, and they fire as usual, because the submodules
do not know the MAC's state. The interface's own state goes `DOWN` through `NetworkInterface`, not
through the MAC.

**What restarts a station after a start operation.** Only the management module and the
interface act. The agent has no lifecycle hooks. Its only trigger for a new scan is `l2BeaconLost`,
which the station management emits when 3.5 beacon intervals pass without a beacon. After a stop
that cleared the association, no beacon timeout is armed, so nothing restarts the association
sequence.

The operations `InterfaceUpOperation` and `InterfaceDownOperation` exist in
`src/inet/common/lifecycle/InterfaceOperations.h`. No module handles them.

## 8. Four flows in words

### 8.1 A unicast data frame under DCF

1. The network layer sends a packet to `upperLayerIn`. The classifier passes it. The LLC inserts
   its header and sends it to `mac.upperLayerIn`.
2. `Ieee80211Mac::handleUpperPacket` drops it when the station is not associated. `encapsulate`
   builds the data header with the receiver address from the MIB (the BSSID on a station) and
   appends the trailer. `processUpperFrame` calls `dcf->processUpperFrame`.
3. `Dcf` enqueues the frame in the pending queue and calls `channelAccess->requestChannel(this)`.
4. `Dcaf` starts the contention with the current CW, DIFS, EIFS and slot time. `Contention` waits
   until `Rx` reports a free medium, then waits DIFS (or EIFS after a corrupt frame) plus the
   backoff slots on a timer. At expiry it calls back `Dcaf`, which calls back `Dcf::channelGranted`.
5. `Dcf` builds a `FrameSequenceContext` and starts a `DcfFs` in the `FrameSequenceHandler`.
6. The first transmit step asks `InProgressFrames` for a frame. `InProgressFrames` pulls a packet
   from the pending queue through `OriginatorMacDataService`, which assigns the sequence number
   and fragments the packet when needed, and registers the frame with `AckHandler`.
7. The handler calls `Dcf::transmitFrame`. `RateSelection` picks the mode and sets
   `Ieee80211ModeReq`. `OriginatorProtectionMechanism` computes the duration field. `Dcf` calls
   `tx->transmitFrame(packet, header, ifs, this)`.
8. `Tx` fills the transmitter address and the FCS, waits the IFS, and calls `mac->sendDownFrame`.
   The MAC switches the radio to transmitter mode with a command and sends the frame down.
9. The radio prepends the PHY header and hands the packet to the medium. At the end of the
   transmission the radio emits `transmissionStateChanged`; the MAC calls
   `tx->radioTransmissionFinished`, which calls back `Dcf::transmissionComplete`; the MAC switches
   the radio back to receiver mode. The handler moves to the receive step, and `Dcf` schedules
   `startRxTimer` with the ACK timeout.
10. The ACK arrives on `mac.lowerLayerIn`. `Rx` checks the FCS. `dcf->processLowerFrame` cancels
    the timer and gives the frame to the handler. The step is accepted, and
    `Dcf::originatorProcessReceivedFrame` marks the frame acknowledged, resets the retry counters
    and the CW, reports success to the rate control, and drops the frame from `InProgressFrames`.
    The sequence finishes: `Dcf` releases the channel, requests it again when the queue is not
    empty, and flushes a held radio command.

On a timeout the handler aborts the sequence and calls `originatorProcessFailedFrame`. The
recovery procedure counts the retry and doubles the CW. Below the limit `Dcf` sets the retry bit
and the frame stays in `InProgressFrames`; at the limit `Dcf` drops it and emits `packetDropped`
and `linkBroken`. Either way the sequence finishes and a new contention starts.

### 8.2 A received frame

1. The medium delivers a `WirelessSignal` to `radio.radioIn`. The radio starts a reception timer.
   At the end, the medium's decision (SNIR threshold, then the error model) says whether the
   packet is correct. The radio pops the PHY header, sets the indication tags and the bit-error
   flag when needed, and sends the packet to `mac.lowerLayerIn`.
2. `Ieee80211Mac::handleLowerPacket` calls `rx->lowerFrameReceived`. A corrupt frame is dropped
   here; the contentions switch to EIFS and the coordination function learns of it. A correct frame
   that is not for this station sets or extends the NAV.
3. `dcf->processLowerFrame` gives the frame to the sequence in progress when one exists and the frame is
   for this station. Otherwise `recipientProcessReceivedFrame` runs: `RecipientAckProcedure`
   builds an ACK (or `CtsProcedure` a CTS in reply to an RTS), which `Dcf` sends through `Tx`
   after SIFS; `RecipientMacDataService` removes duplicates and reassembles fragments and returns
   the complete frames.
4. `Dcf::sendUp` calls `mac->sendUpFrame`. The MAC decapsulates the frame. A management frame goes
   out on `mgmtOut`. A data frame goes to `Ds`, which passes it up (or, on an AP, forwards it into
   the BSS through `mac->processUpperFrame`). The LLC pops its header and the packet leaves on
   `upperLayerOut`.

### 8.3 A station joins a BSS

1. The agent's `startUp` timer fires. The agent sends a scan request primitive to the management
   module.
2. `Ieee80211MgmtSta` disassociates if needed, then visits each channel: it sends an
   `Ieee80211ConfigureRadioCommand` through the MAC to the radio, sends a probe request (active
   scan) or waits (passive scan), and records every beacon and probe response.
3. After the last channel the management module sends a scan confirm with the list of BSSs. The
   agent picks the BSS with the highest received power whose SSID matches, and sends an
   authenticate request.
4. The management module changes channel, sends the authentication frames with a timeout per
   step, and confirms. The agent then sends an associate request.
5. The management module sends the association request. On the response it writes the SSID, the
   BSSID, `isAssociated = true` and the peer HT state into the MIB, emits `l2Associated`, and arms
   the beacon timeout.
6. On the AP, `Ieee80211MgmtAp` answers the probe, the authentication and the association. It
   reserves an AID when it builds the response and commits it, marks the station `ASSOCIATED` and
   emits `l2ApAssociated` when `frameTransmissionOutcome` reports the acknowledged response. From
   then on `Ds` forwards frames to and from that station.
7. When 3.5 beacon intervals pass without a beacon, the station management emits `l2BeaconLost`.
   The agent emits `l2Disassociated` and sends a new scan request. The association flag in the MIB
   stays true until that scan request arrives.

### 8.4 A QoS station: EDCA and block acknowledgement

1. The classifier sets `UserPriorityReq`. The MAC sets the QoS subtype and the TID. `Hcf` maps
   the TID to an access category and enqueues the frame in that EDCAF's pending queue. A
   management frame always goes to `AC_VO`.
2. Each EDCAF contends on its own, with AIFS and a CW per access category. When two EDCAFs would
   transmit at the same instant, the collision controller lets the higher category win; `Hcf`
   treats the loss as a transmission failure of the loser and re-requests the channel.
3. On a grant `Hcf` starts the TXOP of the winner and an `HcfFs`. Each round is one of: data with
   ACK, data under block ack policy without a response, a BlockAckReq with its BlockAck, or a
   management frame with ACK. The TXOP limit bounds the rounds; it is 0 for `AC_BK` and `AC_BE`.
4. After the first eligible QoS data frame, the originator agreement handler builds an ADDBA
   request, which `Hcf` sends as a management frame. The peer's recipient handler answers with an
   ADDBA response. From then on the ack policy may be `BLOCK_ACK`. After `blockAckReqThreshold`
   outstanding frames the originator sends a BlockAckReq, and the recipient answers with a bitmap
   from its `BlockAckRecord`. The recipient data service reorders frames per agreement before it
   passes them up.

## 9. Points to check

The study found the items below in the code as it is. They are facts to check, not judgments.
Each one changes what a reader may assume from the tables above.

1. **The MAC's stop and crash hooks are empty**, marked `FIXME`, and skip `MacProtocolBase`. No
   part inside the MAC resets on a stop (§7).
2. **The agent has no lifecycle support** (`TODO add ILifecycle` in `mgmt/Ieee80211AgentSta.h`).
3. **The radio returns to receiver mode from `Ieee80211Mac::receiveSignal`**, marked `FIXME this
   is in a very wrong place`. The coordination function does not control the radio mode.
4. **A channel change from management can wait without bound** while the medium is busy
   (`Ieee80211Mac::handleUpperCommand`, `TODO waiting potentially indefinitely`).
5. **Placeholders and dead code:** `Pcf` and `Mcf` throw; `Hcca` is a stub; `HtTxOpFs`, `McfFs`
   and `PcfFs` are empty and unused; the two block ack handler `.ned` files declare modules that
   cannot be instantiated.
6. **Features that the NED comments advertise but the code does not run:** A-MPDU aggregation
   (the call is commented out), MSDU lifetime (the handlers are never created), self-CTS (the
   predicate returns false).
7. **Block ack timers:** the handlers pass an absolute expiration time, `Hcf` treats it as a
   delay; the recipient DELBA lookup uses the receiver address where the table is keyed by the
   originator; the default timeout `0s` disables the timer, so both defects are latent.
8. **Two mode sets:** the MAC builds one from `mac.modeSet`, the transmitter and receiver build
   one from `opMode`. The interface wires both from `opMode`; a direct override of `mac.modeSet`
   desynchronizes them.
9. **Timing comes from the reference mode of the set.** For `g(mixed)` the reference is the
   1 Mbps DSSS mode, so slot time and SIFS are 20 µs and 10 µs whatever mode a frame uses.
10. **`Ieee80211RadioMedium` sets the limit cache center frequency to 2.4 GHz.** A 5 GHz network
    that uses it without an override gets range limits for 2.4 GHz.
11. **A corrupt packet reaches the MAC.** The error model delivers it with the bit-error flag,
    `Rx` drops it, and both `Rx` and the MAC report the corruption, on two separate paths.
12. **The management module recovers the popped MAC header by a peek of 24 bytes**, a constant
    that must match the header the MAC popped.
13. **The simplified AP management never fills the station table.** The MAC and `Ds` then drop
    downlink frames and pass uplink frames up; the simplified station compensates by a direct
    write into the AP's MIB.
14. **Beacon loss does not clear the association.** `l2BeaconLost` leaves `isAssociated` true; the
    agent's new scan request clears it. No disassociation frame is sent.
15. **TXOP predicates are stubs.** `isFinalFragment`, `isTxopInitiator` and `isTxopTerminator`
    return false, which changes the duration fields and the control-frame rate rule.
16. **Placement questions that the code raises itself:** `Dcf.ned` proposes to merge `Dcaf` into
    `Dcf`; `Edcaf.ned` and `QosAckHandler.h` propose to move the ack handler and the recovery
    procedure up to `Hcf`.
17. **Inert subscriptions and unused declarations:** the MAC subscribes to `radioModeChanged` but
    has no handler; `frameSequenceAborted` is declared and never emitted; `Ieee80211SubtypeInd`,
    `IRateControl::frameReceived`, `RateSelection::frameTransmitted` and
    `Contention::revokeBackoffOptimization` have no user.
18. **No protocol registration.** Nothing in the 802.11 tree registers a protocol or a service; the
    LLC's inherited registration is off by default.

## 10. How this document was made

Five code studies, one per area (the MAC skeleton, the inside of `Dcf`, the inside of `Hcf`, the
management plane with the LLC and the MIB, and the physical layer), each read the files of its area
and reported the parts, the calls, the signals, the initialization stages and the lifecycle hooks
with file and line citations. This document condenses those reports. The line numbers were dropped
on purpose; the file names and the method names stay, so that a reader can open the file and check
the statement. Where the code carries a `TODO`, `FIXME` or `KLUDGE` that changes the picture, §9
says so.
