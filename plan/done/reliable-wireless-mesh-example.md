# Reliable wireless multi-hop example (protocol elements only)

## Goal

> **STATUS: DONE.** Implemented and validated. Two integration tests pass (`PeWirelessLink`
> single hop, `PeWirelessMultihop` 4-node line), full protocolelement suite is 30/30 green, and a
> runnable copy lives in `examples/protocolelement/reliablemesh/`. Broadcast acks were replaced
> with **unicast** acks (source-MAC pair) as the multi-hop cross-talk predicted here proved real.
> ~10 real bugs in the never-run Network91 stack were fixed along the way (see git log / memory).

A runnable, validated example network built **almost entirely from `protocolelement` modules** (no
real protocols — no Ipv4/Udp/Ieee80211/Radio): a multi-hop wireless topology where a packet is
generated at a source node and **forwarded hop-by-hop** to a destination several hops away over a
**shared wireless medium**, and **every hop is a reliable link** using stop-and-wait
acknowledgement, retransmission on loss, and **exponential backoff** between attempts.

It exercises, composed together, elements built/fixed in earlier steps:
- `Forwarding` (parameterized routes) + `SendWithHopLimit`/`ReceiveWithHopLimit` (TTL) — multi-hop.
- `Resending` + `SendWithAcknowledge`/`ReceiveWithAcknowledge` + `ExponentialBackoff` — reliable link.
- `SendToL3Address`/`ReceiveAtL3Address` — network addressing; `SendToMacAddress`/`ReceiveAtMacAddress`
  (+ new source-MAC pair) — link addressing / medium filtering.
- `SendWithProtocol`/`ReceiveWithProtocol` + `MessageDispatcher` — data/ack demux on the link.
- `PacketTransmitter`/`PacketReceiver` — the transmission boundary onto the medium.
- `NetworkInterface` + `InterfaceTable` + `@networkNode` — the node/interface framework.

## The simplified wireless medium

The one piece that is not a `protocolelement` is the medium itself — a PHY/channel abstraction,
explicitly in scope now. It is a **minimal shared broadcast medium**, NOT INET's real
`RadioMedium`/`Radio`:

- **`SimplifiedRadioMedium`** (a new C++ simple module, based on INET's `WireJunction` — a proven,
  protocol-agnostic shared bus that relays `Signal`s, honouring transmission duration/updates; I add
  range-limiting + loss). Nodes keep `PacketTransmitter`/`PacketReceiver`, connected to the medium by
  `DatarateChannel`s (the medium sets `MULTI` mode so concurrent transmissions are allowed):
  - `inout radio[numRadios]` — one gate per node radio.
  - A frame transmitted on `radio[i]` is delivered to every **in-range** `radio[j]` (j != i) after a
    propagation `delay`. The sender does not hear itself.
  - **Range/connectivity** is a parameter: a `string ranges` adjacency spec (e.g. `"0 1; 1 2; 2 3"`
    for a line), or `allInRange = true` for a fully-connected cell. Default: symmetric adjacency.
  - **Collisions (optional, `modelCollisions = default(false)`):** when two transmissions overlap in
    time at a common receiver, both are corrupted and dropped. Off by default so the conformance
    `.test` stays deterministic; on in the interactive example to show backoff **desynchronising
    contending senders** (the real reason exponential backoff exists). Carrier sense is *not*
    modelled (simplified) — contenders learn of the collision only via the missing ack, then back off.
  - **Loss lives here, not in the interface.** Loss is a channel phenomenon (attenuation,
    interference, fading), so the medium owns it — a frame is transmitted fine and the *medium* fails
    to deliver it, so no ack returns and the sender times out + retransmits. Modelled as a single
    `packetLossProbability` (global or per directed link): each in-range delivery is dropped with that
    probability. It is probabilistic but **reproducible** — opp_test runs with a fixed RNG seed, so
    the same frames are lost on every run. This replaces the send-side `OrdinalBasedDropper` at the
    physically correct place, and serves both the `.test` and the interactive demo.
  - Uses the existing streaming transceiver: nodes keep `PacketTransmitter`/`PacketReceiver`; the
    medium relays the `Signal` to in-range receivers (whole-frame relay is enough for the base
    version; overlap detection uses transmission start/end times).

Because the medium is a **broadcast** (every in-range node hears every frame), unicast now requires
**link-layer (MAC) filtering** — this is the main design change from a point-to-point link.

## Design decisions

- **Shared medium + one radio interface per node** (was: point-to-point interface per neighbour).
  Genuinely wireless: a transmission reaches all neighbours; MAC addressing selects the intended one.
- **MAC filtering for unicast:**
  - *Destination MAC.* `Forwarding` sets `NextHopAddressReq` (next-hop L3) + `InterfaceReq`. A small
    **static next-hop→MAC resolver** turns `NextHopAddressReq` into a `MacAddressReq` from a
    configured neighbour table (a trivial static "ARP"; a new tiny `PacketFlowBase` element).
    `SendToMacAddress` then stamps the destination MAC header; `ReceiveAtMacAddress` drops frames not
    addressed to this node (or broadcast).
  - *Acks are broadcast, matched by sequence number* (DECISION during implementation, revised from the
    source-MAC idea). `ReceiveWithAcknowledge` builds the ack as a fresh packet carrying only the
    sequence number; threading the data frame's source MAC onto it would couple that core element to
    MAC. Instead the ack goes out with a broadcast destination MAC (`SendToMacAddress` fallback
    `address`), every in-range node accepts it (`ReceiveAtMacAddress` accepts broadcast), and each
    node's `SendWithAcknowledge` acts only if it has a *pending* timer for that sequence number. In a
    line topology with sequential per-hop stop-and-wait this is unambiguous — at any instant only one
    node has an outstanding frame, and its ack reaches it; other hearers have no matching timer. The
    **source-MAC pair is therefore not built** (deferred); it would be the way to make acks unicast if
    a contending-flow scenario ever aliased sequence spaces.
- **`ReceiveAtMacAddress` needs a fix** (like `Forwarding` did): its `// KLUDGE` unconditionally
  re-dispatches to `sequenceNumber` and sets the NIC MAC as an init side effect. Change it to pop the
  MAC header and hand the frame to the next stage (protocol demux) without the hard-coded re-dispatch.
  Treat as a real bug fix, consistent with the project pattern.
- **Reliable-link element order `Resending -> ExponentialBackoff -> SendWithAcknowledge`.** Verified:
  `PacketDelayerBase` forwards the ack/timeout completion up to `Resending` and does NOT prematurely
  report completion (`handlePacketProcessed` only bumps counters), so the retry feedback loop stays
  intact; backoff before the ack sender means the ack timer starts *after* the backoff (correct
  802.11-like semantics).
- **Per-hop reliability**: ack/retry/backoff live inside the interface, so each hop is independently
  reliable; loss on one hop is recovered locally.
- **Backoff trigger:** frame loss is modelled in the medium (see above), NOT in the interface. The
  medium's `packetLossProbability` makes a link lossy to demonstrate retransmission+backoff — random
  but reproducible under the fixed test seed (`modelCollisions=false`); `modelCollisions=true` drives
  it via real collisions instead. The interface carries no dropper.
- **Validation**: self-contained `.test` files with a `ProtocolTester` asserting end-to-end delivery
  at the destination app sink (`on("dest.app.sink")` — verified: `on(path)` is a component-aligned
  prefix match on the network-relative path). Plus a runnable `examples/` copy (ned+ini).

## Compounds & new modules

New library-style modules to add first (small, each with a `.test`):
- `SimplifiedRadioMedium` (C++ simple module) — the shared medium.
- `NextHopMacResolver` (`PacketFlowBase`) — static `NextHopAddressReq` (L3) -> `MacAddressReq` (L2).
- `SendFromMacAddress` / `ReceiveFromMacAddress` (`SourceMacAddressHeader`) — source-MAC pair, mirror
  of the source-L3 pair; receiver records `MacAddressInd`.
- `ReceiveAtMacAddress` fix — de-kludge the re-dispatch/side-effect.

Example compounds (all from protocol elements):
- `SourceApp` / `SinkApp` (`like IApp`).
- `WirelessInterface extends NetworkInterface` (one radio per node):
  - egress: `upperLayerIn -> resending -> backoff -> macResolver -> sendToMac -> sendFromMac ->
    sendWithAck -> mux -> sendProtocol -> transmitter -> radio`  (no dropper — loss is in the medium)
  - ingress: `radio -> receiver -> receiveProtocol -> receiveAtMac(filter) -> receiveFromMac ->
    d1 -> {withAcknowledge: receiveWithAck -> upperLayerOut ; acknowledge: -> sendWithAck.ackIn}`
  - ack: `receiveWithAck.ackOut -> mux` (addressed back via the recorded source MAC)
  - (exact element order within egress to be settled during implementation — MAC stamping vs. ack
    numbering vs. backoff placement; see risks.)
- `ForwardingLayer` (Network91's ForwardingService, parameterized): `forwarding` + hop-limit + dispatcher.
- `WirelessNode` (`@networkNode`): `interfaceTable` + optional `app` + `forwardingLayer` + one
  `interface` (radio) + dispatcher; `radio` gate wired to the shared `SimplifiedRadioMedium`.

## Steps

1. [x] Add + unit-test the new modules: `SimplifiedRadioMedium`, `NextHopMacResolver`, source-MAC
   pair; fix `ReceiveAtMacAddress`. One `.test` each.
2. [x] Single reliable hop over the medium (2 nodes) — validate delivery, ack loop, MAC filtering.
3. [x] Induced loss (medium `packetLossProbability`) on the hop — assert retransmission + backoff, still delivered.
4. [x] Multi-hop line (4 nodes: source -> relay1 -> relay2 -> dest) over one shared medium with range
   limiting so each node hears only its neighbours; forwarding to the distant dest.
5. [x] Optional: enable `modelCollisions` in the interactive example; add a contending flow to show
   backoff resolving contention. (Not in the deterministic `.test`.)
6. [x] Runnable `examples/protocolelement/reliablemesh/` copy (ned+ini); docs + memory.

## Risks / watch-list

- Registration/dispatch through `MessageDispatcher` (data vs ack; InterfaceReq/MAC routing) —
  Network91's stack was never run end-to-end, so integration bugs are likely; debug iteratively.
- **Ack addressing on the shared medium** — threading the received frame's source MAC into the ack's
  destination MAC (via `ReceiveWithAcknowledge` tag carry-over) is the fiddliest part; may need a
  small change to how the ack packet is built.
- **Element ordering in the interface** — MAC stamping must happen on the data frame but the ack is
  generated later on the ingress side; getting a single mux + transmitter to carry both data and acks
  with correct addressing needs care. Transmitter contention at relays may need a queue before it.
- `SimplifiedRadioMedium` collision model is deliberately crude (overlap = drop, no capture/SNR, no
  carrier sense); keep it off for the conformance test.
- Hop-limit element resets per hop rather than being a true end-to-end TTL (fine for a loop-free line).

## Superseded

Earlier draft used **point-to-point per-neighbour links** (no medium, MAC dropped). Kept only as a
fallback if the shared-medium MAC path proves too costly; the draft
`tests/protocol/protocolelement/PeWirelessLink.test` (unvalidated) reflects that older approach and
will be reworked to the shared-medium design.
