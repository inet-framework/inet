# Implementation plan: Radiotap records for IEEE 802.11

Checkout and revision: the plan was written at `master` `7fda2e3987`. The branch starts at `origin/master` `5ac997f379`. The five commits between them change no file that the plan cites, so the line numbers in the links are correct for both. PR #1130 is in the local ref `refs/remotes/pr/1130`, head `c2c11be0f3`.

Plan revision: 1, 2026-10-27.

Approval: the maintainer approved revision 1 on 2026-10-28, in the conversation that wrote the plan. The approval covers C1 to C14, the seal permissions of section 8, decision O-1 (b) and decision O-2 (do C4).

Implementation: in the worktree `/home/levy/workspace/inet-ieee80211-radiotap-capture`, on the local branch `topic/ieee80211-radiotap-capture`. There is no pull request and no push until the maintainer asks.

## Status

| Commit | Subject | State |
| --- | --- | --- |
| C1 | the plan | done, in the commit that adds this file |
| C2 | Radiotap header chunk and serializer | done, `2306aaae90` |
| C3 | the receiver's copy of radio frames | done, `340be81284` |
| C4 | CRC errors in PCAPng records (O-2) | done, `f9933f4950` |
| C5 | record C2 to C4 in the plan | done, in the commit that records it |
| C6 | the used mode, channel and power as tags | to do |
| C7 | VHT queries of the mode contract | to do |
| C8 | helpers first, with the direction | to do |
| C9 | the Radiotap helper | to do |
| C10 | record C6 to C9 in the plan | to do |
| C11 | the showcase | to do |
| C12 | the User's Guide | to do |
| C13 | record C11 and C12 in the plan | to do |
| C14 | move the plan to `plan/done/` | to do |

## 1. Problem and intended behavior

The showcase [pcaprecording](../../showcases/general/pcaprecording/omnetpp.ini) writes the Wi-Fi traffic of `host1` to `host1.80211.pcap`. The records have link type 105, so each record holds only the 802.11 MAC frame. Wireshark thus shows no data rate, no MCS, no channel, no signal power and no transmit power. Also, a frame that `host1` could not decode looks like a good frame in the file. A developer who examines block ack, rate selection or TXOP limits needs these values for each frame.

After this plan, the user sets one parameter on a recorder:

```ini
*.host1.pcapRecorder[0].helpers = "inet::physicallayer::Ieee80211RadiotapPcapRecorderHelper"
```

The recorder then writes each 802.11 frame as a Radiotap record (link type 127). Linux writes the same format in monitor mode, and Wireshark decodes each field. A record shows:

- the PHY mode: the legacy data rate, or the HT or VHT MCS, the spatial streams, the bandwidth and the guard interval;
- the channel frequency;
- the transmit power of a sent frame, or the signal power of a received frame;
- the FCS flags, with "bad FCS" on a frame that the receiver could not decode.

Separately, the recorder writes the receiver's copy of each received radio frame, for all radios. The `dumpBadFrames` parameter then works for radios too.

PR #1130 of Miguel González López has the same goal. Its PCAPng repairs are on `master` already ([pcapng-writer-format.md](../done/pcapng-writer-format.md)). This plan keeps the goal and the field encoding of the PR, but it uses a different structure. Section 4 gives the reasons.

## 2. Current implementation and evidence

All facts in this section come from the source at the revision above. I did not run a scenario for them.

**F1. Inbound records are the packet of the transmitter.** The default inbound signal is `receptionEnded` ([PcapRecorder.ned:48](../../src/inet/common/packet/recorder/PcapRecorder.ned#L48)). The radio emits it with the `IReception` ([Radio.cc:530](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc#L530)). For an `IReception`, the recorder writes `reception->getTransmission()->getPacket()` ([PcapRecorder.cc:195-196](../../src/inet/common/packet/recorder/PcapRecorder.cc#L195-L196)). That packet has no bit error flag and no `Ind` tags. Thus the check `dumpBadFrames || !packet->hasBitError()` ([PcapRecorder.cc:230](../../src/inet/common/packet/recorder/PcapRecorder.cc#L230)) never removes a radio frame.

**F2. The receiver's copy exists.** For each attempted reception, `Radio::endReception()` gets the receiver's copy from the medium, removes the PHY header and sends the copy up ([Radio.cc:518-529](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc#L518-L529)). `ReceiverBase::computeReceivedPacket()` removes the tags of the copy, and it sets the bit error flag if the reception failed ([ReceiverBase.cc:133-140](../../src/inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.cc#L133-L140)). The receiver then adds `SignalPowerInd`, `SnirInd`, `SignalTimeInd` and `ErrorRateInd` ([ReceiverBase.cc:95-112](../../src/inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.cc#L95-L112)), and the 802.11 receiver adds `Ieee80211ModeInd` and `Ieee80211ChannelInd` ([Ieee80211Receiver.cc:67-68](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.cc#L67-L68)). The radio emits `receptionEnded` in the same branch, after `sendUp()`. Thus the signal occurs for the same receptions as the copy.

**F3. The sent packet does not carry all used values.** The default outbound signal is `transmissionEnded`. The recorder writes `transmission->getPacket()` ([PcapRecorder.cc:193-194](../../src/inet/common/packet/recorder/PcapRecorder.cc#L193-L194)), which is the PHY frame. The dissector finds the MAC frame in it and gives its offsets.
- The MAC sets only `Ieee80211ModeReq` ([RateSelection.cc:206](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelection.cc#L206), [Hcf.cc:1777](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L1777), [Hcf.cc:1927](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L1927)).
- `Ieee80211Transmitter::createTransmission()` takes the mode, the channel and the power from the `Req` tags, or else from the radio defaults ([Ieee80211Transmitter.cc:138-159](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.cc#L138-L159)).
- Only the transmitters read `Ieee80211ChannelReq` and `SignalPowerReq` ([Ieee80211Transmitter.cc:68](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.cc#L68), [FlatTransmitterBase.cc:68](../../src/inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.cc#L68)).
- `Ieee80211Radio::encapsulate()` already calls `computeTransmissionMode()` ([Ieee80211Radio.cc:251-254](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.cc#L251-L254)).
- `computeTransmissionMode()` and `computeTransmissionChannel()` are public ([Ieee80211Transmitter.h:38-39](../../src/inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h#L38-L39)). `computeTransmissionPower()` is protected ([FlatTransmitterBase.h:33](../../src/inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h#L33)).
- The MAC gives a copy of the frame to the radio ([Tx.cc:80](../../src/inet/linklayer/ieee80211/mac/Tx.cc#L80)).

**F4. A helper cannot change a built-in link type.** `PcapRecorder::IHelper` has three functions ([PcapRecorder.h:27-39](../../src/inet/common/packet/recorder/PcapRecorder.h#L27-L39)). `writePacket()` asks for the link type, and it converts the packet if the link type does not match. It writes the converted packet, emits it with `packetRecorded`, and then deletes it ([PcapRecorder.cc:201-222](../../src/inet/common/packet/recorder/PcapRecorder.cc#L201-L222)). The built-in table maps `ieee80211Mac` to `LINKTYPE_IEEE802_11` before the recorder asks the helpers ([PcapRecorder.cc:279-328](../../src/inet/common/packet/recorder/PcapRecorder.cc#L279-L328)). The only helper in INET is `AckingMacToEthernetPcapRecorderHelper` ([AckingMacHeaderSerializer.cc:53-90](../../src/inet/linklayer/acking/AckingMacHeaderSerializer.cc#L53-L90)). Only [examples/wireless/nic/omnetpp.ini:13](../../examples/wireless/nic/omnetpp.ini#L13) uses it. No module in INET listens to `packetRecorded`.

**F5. Each record goes through the serializers.** `PcapngWriter::writePacket()` gets the bytes with `peekDataAt<BytesChunk>()` ([PcapngWriter.cc:306](../../src/inet/common/packet/recorder/PcapngWriter.cc#L306)). Thus each chunk of a record goes through its registered serializer. The 802.11 trailer serializer throws if the FCS is not computed ([Ieee80211MacHeaderSerializer.cc:712-715](../../src/inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.cc#L712-L715)). Thus each recorded 802.11 frame ends with a correct FCS.

**F6. The mode contract has HT queries but no VHT queries.** `IIeee80211Mode` gives `getModulationClass()`, `getLegacyPreambleType()`, `getHtMcsIndex()` and `isHtShortGuardInterval()` ([IIeee80211Mode.h:58-72](../../src/inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h#L58-L72)). `Ieee80211ModeBase` gives the defaults for the other modes ([Ieee80211ModeBase.h:30](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeBase.h#L30)). The VHT data mode has the MCS index and the guard interval type, but the mode contract does not show them ([Ieee80211VhtMode.h:49-52](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h#L49-L52)). Commit `df141e02d4` added the earlier queries, with a migration guide section.

**F7. INET has no A-MPDU.** `BasicMpduAggregationPolicy` gives no aggregate ([BasicMpduAggregationPolicy.cc:22](../../src/inet/linklayer/ieee80211/mac/aggregation/BasicMpduAggregationPolicy.cc#L22)), and the call of the aggregation is a comment ([OriginatorQosMacDataService.cc:168](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc#L168)). Step W4 of [wifi-protocol-follow-ups.md](wifi-protocol-follow-ups.md) plans it.

**F8. The import reads Radiotap.** `PcapReader` removes the Radiotap header of a link type 127 record, and it sets `FcsInd` from the Flags field ([PcapReader.cc:146-156](../../src/inet/common/packet/recorder/PcapReader.cc#L146-L156)).

**F9. Seals.** `common/packet/` is sealed, with all files below it ([seal-list.md](../../doc/project/audit/seal-list.md)). This includes `PcapRecorder.{h,cc,ned}` and `PcapngWriter.{h,cc}`. The radio, the 802.11 PHY, the mode classes and `common/Protocol.{h,cc}` are not sealed.

**F10. Relays remove tags.** `MacRelayUnitBase` removes the tags of a packet that it forwards ([MacRelayUnitBase.cc:61-62](../../src/inet/linklayer/base/MacRelayUnitBase.cc#L61-L62)). I did not check the other modules that forward packets.

**F11. The error models use the minimum SNIR by default.** `ErrorModelBase` takes the minimum or the mean SNIR, and the default of `snirMode` is `min` ([ErrorModelBase.cc:41-51](../../src/inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.cc#L41-L51), [ErrorModelBase.ned:20](../../src/inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.ned#L20)).

**The structure of PR #1130.** The PR adds these parts on top of its PCAPng repairs:
- three global registries with macros, for adapters, protocol resolvers and observation adapters (`PcapCaptureAdapterRegistry.{h,cc}`, `IPcapCaptureAdapter.h`);
- `WirelessPcapCaptureObservationAdapter` in `physicallayer/wireless/common/pcap/`, which turns radio objects into an observation;
- `Ieee80211RadiotapPcapCaptureAdapter` in `linklayer/ieee80211/pcap/` (526 lines), which makes the Radiotap bytes;
- temporary members in `PcapRecorder` that carry the observation from one hook to the next, and the NED flag `enableProtocolSpecificCaptureAdapters`.

The adapter reads the mode from the transmission or from tags, with `dynamic_cast` to concrete mode classes. It reads the power from the analog model. It also parses A-MPDUs, which INET does not make (F7).

## 3. Scope and acceptance criteria

| Id | Criterion | Commits |
| --- | --- | --- |
| A1 | With the Radiotap helper, each 802.11 record has link type 127. tshark shows the fields of the table in D-9, with the values of the simulation. | C9 |
| A2 | Without the helper, each 802.11 record keeps link type 105 and the same MAC bytes. | C8, C9 |
| A3 | Inbound radio records are the receiver's copy. `dumpBadFrames = false` removes the receptions that failed. | C3 |
| A4 | The fingerprints do not change, because no commit changes the simulation behavior. | all |
| A5 | Ethernet, PPP and AckingMac records do not change. | C3, C8 |

Optional work: C4 marks frames with bit errors as CRC errors in PCAPng records (decision O-2).

Follow-up work, not in this plan:
- A-MPDU records, after W4. Each MPDU becomes one record with the Radiotap A-MPDU status field. The helper then gives more than one packet, so the helper API changes again.
- An import that turns the Radiotap fields into tags. It needs a hook in `PcapReader`, because `common/` must not use the 802.11 mode set.
- A Radiotap dissector and printer. They become useful with the import (section 4.2).
- Radiotap as the default record format for 802.11 (D-8).
- HE fields. INET has no HE PHY.

Preserved conditions: the PCAPng block API, the `IPcapWriter` interface, and the rule of the PCAP writer of one link type in each file.

## 4. Proposed design and reasons

### 4.1 The record path

Inbound:
1. `Radio::endReception()` emits `receptionEnded` with the `IReception` and, as the details, the receiver's copy (C3).
2. `PcapRecorder::receiveSignal()` takes the copy from the details and calls `recordPacket()`.
3. `writePacket()` asks the helpers first (C8). The Radiotap helper gives link type 127. Its `tryConvertToLinkType()` makes a converted copy: the Radiotap header, then the MAC frame (C9).
4. The PCAPng or PCAP writer writes the converted copy. Then the recorder deletes it.

Outbound:
1. `Ieee80211Radio::encapsulate()` puts the mode, the channel and the power that the transmitter will use on the packet, as `Req` tags (C6).
2. The radio emits `transmissionEnded`. The recorder takes `transmission->getPacket()`, and the dissector gives the offsets of the MAC frame.
3. The helper removes the PHY header and the PHY tail, and it puts the Radiotap header in front of the MAC frame.

### 4.2 The Radiotap header is a temporary chunk

The Radiotap header exists only in the converted copy that the recorder writes and then deletes ([PcapRecorder.cc:220-221](../../src/inet/common/packet/recorder/PcapRecorder.cc#L220-L221)). The simulated packet never carries it, and it never goes to the medium.

`IHelper::tryConvertToLinkType()` gives a `Packet`, so the header must be a chunk in any case. A `FieldsChunk` with a serializer is better than bytes that the helper writes:
- it keeps two tasks apart: the helper selects the values, and the serializer writes the byte layout and the alignment;
- a test can check each task alone;
- it gives the deserializer that the round-trip test and a later import need.

The plan adds no dissector and no printer. No module listens to `packetRecorded` (F4), and the recorder deletes the converted copy at once. Packets that keep the header for a longer time appear only with an import (section 3).

### 4.3 Decisions

**D-1. Radiotap, link type 127.** Radiotap is the usual per-frame PHY header of 802.11 captures. Wireshark decodes each of its fields, and `PcapReader` already reads it (F8). PCAPng options on link type 105 carry only text, so Wireshark cannot show the values as columns. PPI (link type 192) can carry the same data, but few tools write it for 802.11.

**D-2. A header chunk with a serializer.** Section 4.2 gives the reasons. The PR writes the bytes in the adapter.

**D-3. The helper reads only tags.** A received frame gives the `Ind` tags of the receiver's copy (F2). A sent frame gives the `Req` tags that the radio completes (C6). The PR reads the transmission and the analog model, which has three problems:
- the power depends on the type of the analog model, scalar or dimensional, so the PR takes the minimum power over the band;
- the converter must know the concrete PHY classes;
- the inbound record stays the packet of the transmitter (F1).

**D-4. The receiver's copy goes with `receptionEnded`, as the details.** The recorder already listens to that signal, and the details parameter of an OMNeT++ signal can carry such data. The `emit` moves before `sendUp()`, so the radio still owns the packet when the listeners see it. Other options:
- `receivingSignalNames = "packetSentToUpper"` changes no code. But the default stays wrong, and other modules in the NIC emit the same signal.
- A new signal is not necessary, because a present signal can carry the data ([AR-OBS-SIGNALS](../../doc/project/rule/architecture.md#ar-obs-signals)).

**D-5. Reuse `PcapRecorder::IHelper`** ([AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse)), in place of the three registries of the PR. The helper mechanism needs two changes:
- The recorder asks the helpers before it uses its built-in table. Then a helper can select another link type for `ieee80211Mac` (F4).
- `tryConvertToLinkType()` gets the direction. The helper must select the `Ind` tags of a received frame or the `Req` tags of a sent frame. A test for the presence of a tag works only if each module that forwards a packet removes its tags, and F10 shows this for `MacRelayUnitBase` only. The recorder knows the direction from the signal.

The direction parameter is an incompatible change of the helper API, so C8 adds a migration guide section. Another option is a separate capture module that uses the PCAPng block API. It needs no recorder change. But it copies the subscription and filter code of `PcapRecorder`, and it writes Wi-Fi and Ethernet records into different files.

**D-6. VHT queries on the mode contract.** `getVhtMcsIndex()` and `isVhtShortGuardInterval()` mirror the HT queries (F6). Then the helper needs no `dynamic_cast` to concrete mode classes. [IIeee80211Mode.h:66-68](../../src/inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h#L66-L68) states this rule for HT.

**D-7. The new folder `src/inet/physicallayer/wireless/ieee80211/radiotap/`.** The Radiotap fields are PHY data, and the helper reads PHY tags and the mode contract. The folder is in the NED package `inet.physicallayer.wireless.ieee80211`, so the feature `Ieee80211` includes it. A new file in `common/packet/` would be sealed, and `common/` must not use the 802.11 PHY. The PR puts its adapter in `linklayer/ieee80211/pcap/`.

**D-8. Radiotap is opt-in.** The default stays link type 105 (A2). A later decision can make Radiotap the default.

**D-9. The value of each field.** The mode values come from the `Ind` tags for a received frame and from the `Req` tags for a sent frame.

| Radiotap field (bit) | Received frame | Sent frame |
| --- | --- | --- |
| TSFT (0) | The start time from `SignalTimeInd`, in µs. INET has no TSF timer, so the simulation time stands in for it. | Not written |
| Flags (1) | 0x10 FCS at end if `FcsInd` says so, or else if the MAC frame ends with an `Ieee80211MacTrailer`. INET writes the trailer with a computed FCS only (F5). 0x40 bad FCS if `hasBitError()`. 0x02 if `getLegacyPreambleType()` is `SHORT`. | 0x10 and 0x02 as for a received frame |
| Rate (2) | The net bit rate of the data mode in 500 kb/s units, for the modulation classes `DSSS_HRDSSS`, `OFDM` and `ERP_OFDM` | Same |
| Channel (3) | `Ieee80211ChannelInd`: the center frequency in MHz. Flags: 2 GHz or 5 GHz from the frequency, CCK for `DSSS_HRDSSS`, OFDM for the other classes. | `Ieee80211ChannelReq` |
| Antenna signal (5) | `SignalPowerInd` in dBm, rounded and limited to -128..127 | Not written |
| Antenna noise (6) | The antenna signal minus the minimum SNIR of `SnirInd`, in dB: the noise plus the interference (O-1) | Not written |
| TX power (10) | Not written | `SignalPowerReq` in dBm |
| MCS (19) | For `HT`: `getHtMcsIndex()`, 20 MHz or 40 MHz from the data mode, `isHtShortGuardInterval()` | Same |
| VHT (21) | For `VHT`: `getVhtMcsIndex()`, `getNumberOfSpatialStreams()`, the bandwidth code for 20, 40, 80 or 160 MHz, `isVhtShortGuardInterval()` | Same |

The field encoding follows the adapter of PR #1130. The test values of `PcapRecorderRadiotapHtVht_1.test` and `PcapRecorderRadiotapRoundTrip_1.test` of the PR can be expected values for C2 and C9. The messages of C2 and C9 name this source.

### 4.4 Additions

| Addition | Responsibility | Consumer | Reuse option | Reason |
| --- | --- | --- | --- | --- |
| `Ieee80211RadiotapHeader` and its serializer | The Radiotap fields and their byte layout | The helper; the writers through `peekDataAt()` | A `BytesChunk` that the helper makes | D-2 |
| `Protocol::ieee80211Radiotap` | The `PacketProtocolTag` of the converted copy | The helper | `Protocol::ieee80211Mac` | The copy does not start with a MAC header |
| `Ieee80211RadiotapPcapRecorderHelper` | The conversion of an 802.11 record to link type 127 | `PcapRecorder` | The registries of the PR | D-5 |
| The details of `receptionEnded` | The receiver's copy | `PcapRecorder` | `packetSentToUpper` | D-4 |
| The direction of `tryConvertToLinkType()` | The direction of the record | The helper | A test for a tag | D-5 |
| `getVhtMcsIndex()`, `isVhtShortGuardInterval()` | The VHT MCS index and guard interval | The helper | `dynamic_cast` | D-6 |

C6 adds no class and no tag type. It sets tag types that exist.

## 5. Affected components and compatibility

| Commit | Artifacts | Compatibility | Release note and migration |
| --- | --- | --- | --- |
| C2 | New `.msg` file and serializer | Compatible | None, because no user can see it alone |
| C3 | `Radio.cc`, `PcapRecorder.cc` | Inbound radio records change (a repair) | `doc/whatsnew/fixes/recorder-wireless-receiver-copy.txt` |
| C4 | `PcapngWriter.cc` | New flag bits in PCAPng records | `doc/whatsnew/compatible/recorder-pcapng-crc-errors.txt` |
| C6 | `Ieee80211Radio.cc`, `FlatTransmitterBase.h`, tag comments | Compatible. A sent packet carries more tags. | None |
| C7 | `IIeee80211Mode.h`, `Ieee80211ModeBase.h`, `Ieee80211VhtMode.{h,cc}` | A mode class that implements `IIeee80211Mode` without `Ieee80211ModeBase` must add two functions | `doc/whatsnew/incompatible/ieee80211-vht-mode-queries.txt`, `doc/src/migration-guide/development/ieee80211-vht-mode-queries.rst` |
| C8 | `PcapRecorder.{h,cc}`, `AckingMacHeaderSerializer.{h,cc}` | Each helper must add the direction parameter | `doc/whatsnew/incompatible/recorder-helper-direction.txt`, `doc/src/migration-guide/development/recorder-helper-direction.rst` |
| C9 | New helper, `Protocol.{h,cc}`, `PcapRecorder.ned` (comment only) | Compatible and opt-in | `doc/whatsnew/compatible/ieee80211-radiotap-capture.txt` |
| C11, C12 | Showcase, User's Guide | Documentation | None |

## 6. Future commits and dependencies

| Commit and proposed subject | Dependencies |
| --- | --- |
| C1 `plan: add: Radiotap records for IEEE 802.11` | none |
| C2 `ieee80211: add: Radiotap header chunk and serializer` | C1 |
| C3 `recorder: fix: record the receiver's copy of radio frames` | C1 |
| C4 `recorder: add: mark frames with bit errors as CRC errors in PCAPng` (optional) | C3 |
| C5 `plan: change: record steps C2 to C4` | C2, C3, C4 |
| C6 `ieee80211: change: keep the used mode, channel and power as tags on the sent packet` | C1 |
| C7 `ieee80211: add: VHT MCS index and guard interval queries for PHY modes` | C1 |
| C8 `recorder: change: ask the helpers first and give them the direction` | C1 |
| C9 `ieee80211: add: Radiotap records through a PCAP recorder helper` | C2, C3, C6, C7, C8 |
| C10 `plan: change: record steps C6 to C9` | C9 |
| C11 `showcases: add: Radiotap records in the PCAP recording showcase` | C9 |
| C12 `doc: change: describe Radiotap records and the receiver's copy in the User's Guide` | C3, C9 |
| C13 `plan: change: record steps C11 and C12` | C11, C12 |
| C14 `plan: location: move the Radiotap capture plan to done` | C13 |

No commit repairs a protocol bug or adds a protocol capability. The commits change how INET records frames. Thus the protocol tests and reviewer documents of [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) do not apply. Each commit names its own tests.

### C1: `plan: add: Radiotap records for IEEE 802.11`

This file, in the new worktree.

### C2: `ieee80211: add: Radiotap header chunk and serializer`

Files: new `src/inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapHeader.msg` and `Ieee80211RadiotapHeaderSerializer.{h,cc}`, and a new unit test.

The change:
- The `.msg` file declares the field bits and the header class (shape below).
- A function `computeIeee80211RadiotapHeaderLength()` calculates the length from the present bits and the alignment rules. The code that makes a header sets the chunk length from this function.
- The serializer writes `it_version`, `it_pad`, `it_len` and `it_present`, and then each present field in bit order. Each field starts at a multiple of its alignment, with zero bytes before it. All values are little-endian (`MemoryOutputStream::writeUint16Le()` and the other `Le` functions).
- The deserializer reads in the same order, and it skips extended present words (bit 31). A table gives the alignment and the size of the fields 0 to 27 of the Radiotap namespace. The deserializer skips a field that has a size in the table but no place in the chunk, for example RX flags, and it reads the fields after it. At a field without a size, for example bit 18, it stops. In both cases, it goes to `it_len` and marks the chunk as improperly represented. An unchanged chunk then serializes to its original bytes, because `FieldsChunkSerializer` keeps the bytes of a deserialized chunk.

```
enum Ieee80211RadiotapField   // bit numbers in it_present
{
    RADIOTAP_TSFT = 0;  RADIOTAP_FLAGS = 1;  RADIOTAP_RATE = 2;  RADIOTAP_CHANNEL = 3;
    RADIOTAP_DBM_ANTSIGNAL = 5;  RADIOTAP_DBM_ANTNOISE = 6;  RADIOTAP_DBM_TX_POWER = 10;
    RADIOTAP_MCS = 19;  RADIOTAP_AMPDU_STATUS = 20;  RADIOTAP_VHT = 21;
}

class Ieee80211RadiotapHeader extends FieldsChunk
{
    chunkLength = B(8);
    uint32_t present = 0;        // one bit for each field below
    uint64_t tsft;               // microseconds
    uint8_t flags;               // 0x02 short preamble, 0x10 FCS at end, 0x40 bad FCS
    uint8_t rate;                // 500 kb/s units
    uint16_t channelFrequency;   // MHz
    uint16_t channelFlags;
    int8_t antennaSignal;        // dBm
    int8_t antennaNoise;         // dBm
    int8_t txPower;              // dBm
    uint8_t mcsKnown;  uint8_t mcsFlags;  uint8_t mcsIndex;
    uint32_t ampduReference;  uint16_t ampduFlags;  uint8_t ampduDelimiterCrc;
    uint16_t vhtKnown;  uint8_t vhtFlags;  uint8_t vhtBandwidth;  uint8_t vhtMcsNss[4];
    uint8_t vhtCoding;  uint8_t vhtGroupId;  uint16_t vhtPartialAid;
}
```

The A-MPDU fields are in the header now, because the serializer test can cover the complete field set at one time. The helper does not set them until the A-MPDU follow-up.

Behavior against the parent: no change, because nothing uses the chunk yet.

Test: new `tests/unit/Ieee80211RadiotapHeaderSerializer_1.test`.
- A legacy header with Flags, Rate, Channel and antenna signal gives fixed bytes.
- An HT header gives fixed bytes.
- A VHT header gives fixed bytes, with the alignment bytes before the VHT field.
- A header with TSFT gives fixed bytes, with TSFT on an 8-byte boundary.
- The deserializer turns each byte array back into the same fields.
- A byte array with an unknown field bit gives a chunk with the length `it_len` that is improperly represented.

The fixed bytes come from the field definitions on radiotap.org and from the tests of the PR. One tshark decode of a probe file checks the bytes, and the plan records the result.

### C3: `recorder: fix: record the receiver's copy of radio frames`

Files: `Radio.cc`, `PcapRecorder.cc` (sealed), the release note, and a new module test.

```cpp
// Radio::endReception()
        decapsulate(macFrame);
        emit(receptionEndedSignal, check_and_cast<const cObject *>(reception), macFrame);
        sendUp(macFrame);
```

```cpp
// PcapRecorder::receiveSignal()
        else if (auto reception = dynamic_cast<const physicallayer::IReception *>(obj)) {
            // a radio gives the packet that it received as the details
            auto packet = dynamic_cast<const Packet *>(details);
            recordPacket(packet != nullptr ? packet : reception->getTransmission()->getPacket(), direction, source);
        }
```

Behavior against the parent:
- Inbound records of all packet-level radios hold the receiver's copy.
- For packet-level receivers, the bytes stay the same. Only the bit error flag and the tags change.
- With `dumpBadFrames = false`, the receptions that failed are not in the file any more.
- For bit-level receivers, the record holds the decoded bits (risk R-5).
- Ethernet records do not change. The Ethernet PHY emits `receptionEnded` with a `Signal` and no details.
- The simulation does not change. The order of two listener calls in one event changes (risk R-1).

Test: new `tests/module/PcapRecorderReceiverCopy_1.test`.
- Three hosts with UnitDisk radios, AckingMac without acknowledgements, and `ignoreInterference = false`. The scenario comes from `Interference_IdealRadio_Collision_SS_1.test`. Both senders ping the receiver at 0 s, so the first ping arrives with bit errors. The first sender pings again at 1 s, without interference.
- Two recorders at the network level listen to the receiving host, with `dumpBadFrames` true and false. They use the AckingMac helper. The node type fixes its recorders as `PcapRecorder`, so a NED subtype in the test adds a statistic that counts `packetRecorded`.
- The test checks the two scalars: 2 frames and 1 frame. Against C2, both scalars are 2, so the test fails. After C3, the test passes.

### C4 (optional): `recorder: add: mark frames with bit errors as CRC errors in PCAPng`

Files: `PcapngWriter.cc` (sealed), the release note, and a test case.

`PcapngWriter::writePacket()` sets `PCAPNG_ERROR_CRC` in the link-layer errors of `epb_flags` if `packet->hasBitError()`. A packet with no direction and no bit error still gets no `epb_flags` option. Wireshark then shows "CRC error" for the record, for each link type.

Test: a new case in `tests/unit/PcapngWriterBytes_1.test`. An inbound packet with a bit error gives `epb_flags` 0x01000001.

The writer change does not need C3. But radio frames with bit errors reach the writer only after C3.

### C5: `plan: change: record steps C2 to C4`

The status table, the hashes, and the facts that the implementation found.

### C6: `ieee80211: change: keep the used mode, channel and power as tags on the sent packet`

Files: `Ieee80211Radio.cc`, `FlatTransmitterBase.h` (`computeTransmissionPower()` becomes public), and the comments in `Ieee80211Tag.msg` and `SignalTag.msg`.

```cpp
// Ieee80211Radio::encapsulate()
    auto mode = ieee80211Transmitter->computeTransmissionMode(packet);
    // keep the values that the transmitter uses on the packet: recorders read them
    packet->addTagIfAbsent<Ieee80211ModeReq>()->setMode(mode);
    packet->addTagIfAbsent<Ieee80211ChannelReq>()->setChannel(ieee80211Transmitter->computeTransmissionChannel(packet));
    packet->addTagIfAbsent<SignalPowerReq>()->setPower(ieee80211Transmitter->computeTransmissionPower(packet));
```

The tag comments say this: after the radio, the `Req` tags show the values that the radio used.

Why the simulation does not change:
- Each `compute` function gives the value of the `Req` tag if the tag exists (F3). `createTransmission()` calls the same functions after `encapsulate()`, so it gets the same values.
- If the MAC already set a `Req` tag, `addTagIfAbsent()` gives that tag, and the code sets the same value again.
- The radio's packet is a copy (F3). The frame that the MAC keeps for a retransmission gets no new tags.
- Only the transmitters read these tags (F3).
- The channel pointer is the pointer that `Ieee80211Transmission` keeps. Its lifetime does not change.

Test: the fingerprint tests of the 802.11 configurations must give the same values (risk R-3). The plan adds no separate test, because a direct check needs a C++ listener in the test. The module test of C9 checks the tags through the sent records. Until C9, this part is not verified.

### C7: `ieee80211: add: VHT MCS index and guard interval queries for PHY modes`

Files: `IIeee80211Mode.h`, `Ieee80211ModeBase.h`, `Ieee80211VhtMode.{h,cc}`, the release note, the migration guide section, and a new unit test.

```cpp
// IIeee80211Mode.h
    // Returns the VHT MCS index of this mode, or -1 for modes of other PHY generations.
    virtual int getVhtMcsIndex() const = 0;
    // Returns whether this mode uses the 400 ns VHT guard interval.
    // Modes of other PHY generations return false.
    virtual bool isVhtShortGuardInterval() const = 0;
```

`Ieee80211ModeBase` returns -1 and false. `Ieee80211VhtMode` returns the MCS index of its data mode, and it compares the guard interval type of its data mode with the short type. The commit follows `df141e02d4`.

Test: new `tests/unit/Ieee80211VhtModeQueries_1.test`. A VHT mode with MCS 8 and the short guard interval gives 8 and true. An HT mode and a legacy mode give -1 and false.

### C8: `recorder: change: ask the helpers first and give them the direction`

Files: `PcapRecorder.{h,cc}` (sealed), `AckingMacHeaderSerializer.{h,cc}`, the release note, and the migration guide section.

The change:
- In `protocolToLinkType()` and `matchesLinkType()`, the loop over the helpers comes before the built-in table.
- `IHelper::tryConvertToLinkType()` and `PcapRecorder::tryConvertToLinkType()` get the parameter `Direction direction`. `writePacket()` gives its direction.
- `AckingMacToEthernetPcapRecorderHelper` gets the new parameter.

Behavior against the parent: no change in INET. The only helper answers only for `ackingMac` (F4).

Test:
- The configuration of [examples/wireless/nic](../../examples/wireless/nic/omnetpp.ini) with the AckingMac helper gives the same PCAP file before and after C8 (`cmp`).
- The fingerprint tests give the same values.
- The tests of C9 check the new order of the helpers.

### C9: `ieee80211: add: Radiotap records through a PCAP recorder helper`

Files: `Protocol.{h,cc}` (`Protocol::ieee80211Radiotap`), new `radiotap/Ieee80211RadiotapPcapRecorderHelper.{h,cc}`, the comment of the `helpers` parameter in `PcapRecorder.ned` (sealed), the release note, and two new tests.

```cpp
class INET_API Ieee80211RadiotapPcapRecorderHelper : public cObject, public PcapRecorder::IHelper
{
  public:
    // ieee80211Mac gives LINKTYPE_IEEE802_11_RADIOTAP
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const override;
    // false: each frame needs the conversion
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const override;
    virtual Packet *tryConvertToLinkType(const Packet *packet, b frontOffset, b backOffset,
            PcapLinkType pcapLinkType, const Protocol *protocol, Direction direction) const override;

  protected:
    virtual Ptr<Ieee80211RadiotapHeader> createHeader(const Packet *packet, Direction direction) const;
};

Packet *Ieee80211RadiotapPcapRecorderHelper::tryConvertToLinkType(...) const
{
    auto newPacket = packet->dup();
    newPacket->eraseAtFront(frontOffset);   // the PHY header of a sent frame
    newPacket->eraseAtBack(backOffset);     // the PHY tail and padding
    newPacket->insertAtFront(createHeader(packet, direction));
    newPacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Radiotap);
    return newPacket;
}
```

`createHeader()` fills the fields of D-9.

Behavior against the parent: no change, unless a recorder lists the helper.

Tests:
- New `tests/unit/Ieee80211RadiotapPcapRecorderHelper_1.test`, with packets that the test makes from a MAC frame and tags:
  - a received legacy OFDM frame at 54 Mb/s and -61 dBm on channel 6 gives rate 108, 2437 MHz and flags 0x10;
  - the same frame with a bit error gives flags 0x50;
  - a received HT frame with MCS 7, 40 MHz and the short guard interval gives the MCS field;
  - a received VHT frame with MCS 8, 2 spatial streams and 80 MHz gives the VHT field;
  - a sent frame with `Req` tags, a PHY header and a PHY tail gives a copy without PHY bytes, with TX power and without antenna signal;
  - another protocol gives `nullptr`.
- New `tests/module/PcapRecorderRadiotap_1.test`. Two Wi-Fi hosts use `fcsMode = "computed"`. One host has two recorders, one with the helper and one without. After the run, the test reads both files with `PcapReader`. The files must have the same number of records and the same MAC bytes, and the Radiotap file must give `FcsInd` with `hasFcs` true. This test checks A2 and a part of A1.
- A manual check with tshark on the file of the module test:

  ```
  tshark -r <file> -T fields -e radiotap.datarate -e radiotap.mcs.index -e radiotap.channel.freq \
      -e radiotap.dbm_antsignal -e radiotap.dbm_tx_power -e radiotap.flags.badfcs
  ```

  The plan records the output.

### C10: `plan: change: record steps C6 to C9`

The status table, the hashes, the tshark output, and the facts that the implementation found.

### C11: `showcases: add: Radiotap records in the PCAP recording showcase`

Files: [showcases/general/pcaprecording/omnetpp.ini](../../showcases/general/pcaprecording/omnetpp.ini) and the showcase page in `doc/`.

The commit adds a configuration `Radiotap` that extends `PcapRecording`. In it, `host1` writes a Radiotap file. The present configuration does not change. The page gets a section with a Wireshark screenshot that shows the rate, channel and signal columns. If the fingerprint store needs an entry for the new configuration, the commit adds it ([TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)).

Test: run the configuration, check the file with tshark, and build the documentation.

### C12: `doc: change: describe Radiotap records and the receiver's copy in the User's Guide`

File: [ch-collecting-results.rst](../../doc/src/users-guide/ch-collecting-results.rst), in the section on PCAP records.

The text tells:
- that inbound radio records are the receiver's copy, and how `dumpBadFrames` works for them;
- how to select Radiotap, and which fields a record has;
- that a PCAP file holds only one link type. A host with Ethernet and Wi-Fi interfaces needs PCAPng or two recorders.

Test: build the documentation.

### C13: `plan: change: record steps C11 and C12`

The status table and the hashes.

### C14: `plan: location: move the Radiotap capture plan to done`

The move only.

## 7. Verification methods and expected results

Build and run in the worktree:

```
cd /home/levy/workspace/omnetpp-6.x && source setenv -q
cd <worktree> && source setenv -q && export INET_ROOT=$PWD
make MODE=debug -j$(nproc)
opp_run_unit_tests -m debug -f '<regex>'
opp_run_module_tests -m debug -f '<regex>'
opp_run_fingerprint_tests -f '<regex>'
```

The unit and module tests run in debug mode, because they compare `EV` output. The test files of sections C2, C3, C7 and C9 do not exist yet.

Gates for each commit ([run-the-gates.md](../../doc/project/guide/run-the-gates.md)): `check-pull-request.sh --base master`, `check-source-seals.sh --base master`, `check-architecture.sh` and `check-naming.sh` on the changed folders, `check-interfaces.sh`, and `whatsnew.py check`. `check-architecture.sh` and `check-naming.sh` find old violations on `master`, so only new findings count. `check-cpp.sh` cannot run, because clang-tidy is not installed.

Coverage limits:
- The automatic tests do not show that Wireshark decodes each field as the plan expects. The manual tshark check of C9 covers this one time, for the frames of the module test.
- The tests do not cover bit-level receivers (risk R-5) or A-MPDU.
- C6 has no direct test before C9.

Observed results: see the section "Evidence".

## 8. Risks, unresolved decisions, and approval scope

Decisions that the maintainer made on 2026-10-28:
- **O-1. Antenna noise: (b).** The helper writes the signal power minus the minimum SNIR, in dB. That value is the noise plus the interference, and the default error models use the minimum SNIR (F11). Wireshark then shows an SNR column. The User's Guide tells that the value includes the interference. The other option was (a): do not write the field.
- **O-2. Do C4.** Wireshark then marks bad frames for each link type, also for link type 105 and Ethernet.

Risks:
- **R-1. The order of the listeners (C3).** In `Radio::endReception()`, the listeners of `receptionEnded` now run before the listeners of `packetSentToUpper`, in the same event. The listeners of `receptionEnded` in INET are `Gptp` (Ethernet NICs), `ResidenceTimeMeasurer` (if configured) and `PcapRecorder`. The fingerprint tests check that nothing changes.
- **R-2. What the `Req` tags show (C6).** After the radio, the `Req` tags show the values that the radio used. Only the transmitters read them (F3). A module outside INET that reads them sees more tags.
- **R-3. Fingerprints (C6).** I assume that tags are not a part of a fingerprint. The fingerprint tests check this assumption.
- **R-4. Incompatible APIs (C7, C8).** Code outside INET that implements `IIeee80211Mode` directly, or that implements `PcapRecorder::IHelper`, must change. The migration guide sections tell how.
- **R-5. Bit-level receivers (C3).** The receiver's copy of a bit-level receiver holds the decoded bits. After C3, both configurations of [examples/wireless/layered80211](../../examples/wireless/layered80211/omnetpp.ini) write records that tshark decodes as correct 802.11 frames (see "Evidence"). The risk did not occur there.

Seal permissions that the plan needs. The maintainer granted all of them on 2026-10-28, for this branch:

| Sealed file | Commits |
| --- | --- |
| `src/inet/common/packet/recorder/PcapRecorder.cc` | C3, C8 |
| `src/inet/common/packet/recorder/PcapRecorder.h` | C8 |
| `src/inet/common/packet/recorder/PcapRecorder.ned` | C9 (comment only) |
| `src/inet/common/packet/recorder/PcapngWriter.cc` | C4 (optional) |

Approval scope: revision 1, C1 to C14, with C4 and with the field of O-1 (b).

## Evidence

**Build environment.** On 2026-10-28, the tools of `omnetpp-6.x` were coverage builds, for example `opp_msgtool`, but the coverage libraries were not in `lib/`. The build therefore used `LD_LIBRARY_PATH` with links from `liboppcommon_coverage.so` and the other coverage names to the release libraries, and `LLVM_PROFILE_FILE` in `/var/tmp`. This affects only the tools, not the INET libraries.

**C2.** `Ieee80211RadiotapHeaderSerializer_1.test` passes. tshark 4.6.4 decoded the fixed bytes of the test, each in front of an 802.11 ACK frame with a correct FCS:

| Case | Length | tshark shows |
| --- | --- | --- |
| legacy | 16 | 54 Mb/s, 2437 MHz, OFDM, 2 GHz, -61 dBm signal, -95 dBm noise, FCS at end |
| HT | 12 | MCS 7, 40 MHz, short guard interval, 150 Mb/s |
| VHT | 22 | MCS 8, 2 spatial streams, 80 MHz, short guard interval |
| TSFT | 18 | MAC time 72623859790382856 (0x0102030405060708), TX power 20 dBm |
| A-MPDU | 20 | reference 287454020 (0x11223344), last subframe |
| RX flags | 15 | RX flags 0x1234, and the MCS field after it |

tshark also noted that the MCS field did not declare the HT format, STBC and the extension streams as known, and that the VHT field did not declare STBC as known. C2 therefore also defines these bits. INET has no STBC and no extension streams, so C9 can declare them as known with the value 0.

**C3.** Against C2, `PcapRecorderReceiverCopy_1.test` fails with 2 frames in both recorders. After C3, the recorders write 2 frames and 1 frame. 774 module tests with the filter `Pcap|pcap|Interference|IdealRadio|tun-echo|UDP_dscp|ieee80211|Ieee80211|wireless|Wireless|apsk|Apsk|802154` pass in debug mode. Both configurations of `examples/wireless/layered80211`, with a recorder and 1 s of simulation time, write 4 records each. tshark decodes them as two data frames of 120 bytes and two ACK frames.

**Fingerprints.** The full fingerprint suite in release mode gave the same results at C2 and at C3: 1792 tests, 1768 PASS, 2 expected and 22 unexpected ERROR. All 24 errors come from optional features that this build does not have: lwIP, VoIP and Z3.

**C4.** `PcapngWriterBytes_1.test` and the other five writer tests pass. The collision scenario of `PcapRecorderReceiverCopy_1.test`, with PCAPng files, gives this tshark output: the first file shows "CRC error" for the frame that collided at 0 s and not for the frame at 1 s, and the second file holds only the frame at 1 s.
