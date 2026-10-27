# Plan: a PCAPng writer that follows the format

## Status

| Step | Commit | State |
| --- | --- | --- |
| P1 | the plan | done |
| P2 | snaplen limits | done, `10c4449080` |
| P3 | packets without a network interface | done, `fbceb2089d` |
| P4 | one interface description for each link type | done, `a7012aad80` |
| P5 | new interface numbers in each new file | done, `96a8e2ff51` |
| P6 | block-level write functions (refactor) | to do |
| P7 | the public block API with options | to do |
| P8 | move the plan to `plan/done/` | to do |

Branch `topic/pcapng-writer-format`, worktree `/home/levy/workspace/inet-pcapng-writer-format`, base `origin/master` `d48ae1fe10`. Local branch only, no pull request, no push until the user asks.

## Goal

PR #1130 (Radiotap capture for IEEE 802.11, Miguel González-López) needs some capabilities from the PCAPng writer. The PR adds them through a new pure virtual method `IPcapWriter::writePacketWithPrefix()`. The review of the PR found that this method is not a part of the PCAPng format. It also found real PCAPng defects that the PR repairs inside its commits, but does not name as repairs.

This plan gives `PcapngWriter` the parts of the PCAPng format that INET needs now. It also gives an API that lets a caller use the other options of the format. The later IEEE 802.11 capture work (Radiotap records, A-MPDU records, FCS indication) builds on this API. The section "How the rest of PR #1130 fits" at the end gives the connection.

This plan does not change `IPcapWriter`, `PcapWriter` or `PcapRecorder`.

## The format

The reference is draft-ietf-opsawg-pcapng-03. A PCAPng file is a sequence of blocks. A section starts with a Section Header Block (SHB). Interface Description Blocks (IDB) and Enhanced Packet Blocks (EPB) follow in the section. The rules that this plan uses:

- An IDB has one LinkType and one SnapLen. A SnapLen of zero means no limit.
- An IDB must come before the first block that refers to it. It can come after other EPBs.
- Interface IDs count from 0 in the order of the IDBs, and they are unique only inside their section.
- EPB Captured Packet Length is the length of the Packet Data field. The Original Packet Length must not be less than the captured length. The padding to 32 bits counts in neither length.
- `epb_flags`: bits 0-1 direction (00 not available, 01 inbound, 10 outbound), bits 2-4 reception type, bits 5-8 FCS length in octets (0 not available), bits 16-31 link-layer errors (bit 24 CRC error).
- `if_fcslen`: the FCS length of the interface. The `epb_flags` FCS length overrides it.
- `opt_comment` (code 1) is allowed in all blocks, and a block can have more than one.
- The option codes of different block types overlap. Code 2 is `shb_hardware` in an SHB, `if_name` in an IDB, and `epb_flags` in an EPB.

What the writer on `origin/master` writes:

| Block | Content | Defect |
| --- | --- | --- |
| SHB | byte-order magic in host order, version 1.0, section length -1, no options | none |
| IDB | link type, SnapLen 0, `if_name`, `if_description`, `if_MACaddr`, `if_IPv4addr`, `if_tsresol` | SnapLen is always 0 (P2). One IDB for each interface, also when the link type changes (P4) |
| EPB | interface ID, timestamp, lengths, data, `epb_flags` with the direction | no truncation, and original length = captured length (P2). Null interface crashes (P3). IDs continue in a new file (P5). Undefined direction throws (P7) |

The writer does not use these parts of the format: Simple Packet Block, Name Resolution Block, Interface Statistics Block, Decryption Secrets Block, custom blocks, and all options not in the table. After P7, a caller can add any option through the generic option list. The blocks stay out of scope until a user needs them.

## Source: the review of PR #1130

| PR commit | Finding | This plan |
| --- | --- | --- |
| `7150a80cac` snaplen | correct PCAPng | P2 takes it as it is |
| `22303f0861` prefix | one IDB for each pair of interface and link type: correct, repairs a real defect | P4 |
| `22303f0861` prefix | interface numbers start again in `open()`: correct, repairs a real defect | P5 |
| `22303f0861` prefix | null check before the dereference: correct, but the PR stops with an error | P3 writes the packet instead (D-6) |
| `22303f0861` prefix | `writePacketWithPrefix()`: not a format feature | not taken (D-7) |

## Decisions

- **D-1** The format-level API is a set of public block methods in `PcapngWriter`. The `IPcapWriter` methods stay as a thin INET mapping in the same class. (User, 2026-10-27.)
- **D-2** A caller gives options as an ordered generic list (option code and value bytes), with typed helpers for the options that have a use now. (User, 2026-10-27.)
- **D-3** `src/inet/common/packet/` is sealed. The user permitted changes to the whole `src/inet/common/packet/recorder/` folder on this branch (2026-10-27). I tell the user before I change a file in that folder other than `PcapngWriter.h` and `PcapngWriter.cc`. This plan needs no other file. Each commit that changes a sealed file records the permission in its body.
- **D-4** P2 is PR commit `7150a80cac` with its author, Miguel González López. Its message gets a `Plan:` line and the group label.
- **D-5** P4 and P5 use the code of PR commit `22303f0861`, so their author is Miguel González López too. P3 behaves differently from the PR, so its author is the user.
- **D-6** For a packet without a network interface, the writer writes an IDB without interface options, one for each link type. Reasons: the format does not need `if_name`. The classic `PcapWriter` ignores the interface. `PacketFlowPcapFileRecorder` and `PcapFilePacketConsumer` pass `findContainingNicModule(this)`, which is null outside a network interface.
- **D-7** No prefix method. A caller gives the complete packet data. `PcapRecorder::tryConvertToLinkType()` already makes a new `Packet` for a record. A Radiotap adapter can do the same: a `BytesChunk` with the Radiotap header, then `peekDataAt()` of the MAC range. Chunks are shared, so the MAC bytes are not copied.
- **D-8** An undefined direction gives no direction bits ("not available"), and no exception. No INET module gives an undefined direction today: `PcapRecorder` maps each signal to inbound or outbound, and the NED `@enum` of the queueing recorders allows only `inbound` and `outbound`. Thus this change is a part of P7, and not a separate repair.
- **D-9** Each block type has its own option class, because the option codes overlap. With one class for all blocks, an `epb_flags` option in an IDB becomes an `if_name`.
- **D-10** The writer converts a `simtime_t` timestamp with the time precision of the interface. It writes `if_tsresol` itself, and it refuses an `if_tsresol` in the options of the caller.
- **D-11** The writer writes an empty option list as no options and no end-of-options marker. This keeps the SHB bytes of today, and the format permits it.

## Commits

All subjects use the area `recorder`. The trailer scope is `src.common.packet.recorder.PcapngWriter`, the group label is `pcapng-writer-format`. Unit tests run with `opp_run_unit_tests -m debug -f '<regex>'` against a debug library built from the same commit.

### P1 `plan: add: a PCAPng writer that follows the format`

This file. Trailer: `Change: plan | behavior.add | - | pcapng-writer-format`.

### P2 `recorder: fix: enforce finite PCAPng capture length limits`

PR commit `7150a80cac`, unchanged except for the message trailer lines (D-4).

- Scenario: `snaplen = 4`, a selected range of 6 bytes. Before: captured 6, original 6, IDB SnapLen 0. After: captured 4, original 6, IDB SnapLen 4. `snaplen = 0` stays unlimited.
- Files: `PcapngWriter.{h,cc}`, `tests/unit/PcapngWriterSnaplen_1.test`, `doc/whatsnew/incompatible/recorder-pcapng-capture-limits.txt`, `doc/src/migration-guide/development/recorder-pcapng-capture-limits.rst`.
- Verification: the unit test passes. tshark reads the file and shows `frame.len` 6 and `frame.cap_len` 4.

### P3 `recorder: fix: write PCAPng packets that have no network interface`

- Scenario: `PcapFilePacketConsumer` with `fileFormat = "pcapng"` outside a network interface. `findContainingNicModule()` gives null, and `PcapngWriter::writePacket()` calls `networkInterface->getId()` before its null check. The simulation crashes. After: the writer writes an IDB with the link type and `if_tsresol` only, and the packet refers to it (D-6).
- Files: `PcapngWriter.{h,cc}`, `tests/unit/PcapngWriterNoInterface_1.test`, `doc/whatsnew/fixes/recorder-pcapng-without-interface.txt`.
- Verification: the unit test writes two packets without an interface and one with an interface, and checks the IDBs and the interface IDs. tshark reads the file.

### P4 `recorder: fix: give each link type of an interface its own PCAPng interface`

- Scenario: one Ethernet interface records an `ethernetphy` packet (`LINKTYPE_ETHERNET_MPACKET`) and then an `ethernetmac` packet (`LINKTYPE_ETHERNET`). Before: the second EPB refers to the first IDB, so a reader decodes it with the wrong link type. After: the writer writes a second IDB, and the second EPB refers to it.
- Files: `PcapngWriter.{h,cc}`, `tests/unit/PcapngWriterLinkTypes_1.test`, `doc/whatsnew/fixes/recorder-pcapng-link-types.txt`.
- Verification: the unit test checks the two IDBs and the interface ID of each EPB. tshark shows the two encapsulations.

### P5 `recorder: fix: number the PCAPng interfaces again in each new file`

- Scenario: a program opens a `PcapngWriter`, writes a packet, closes it, and opens a second file with the same writer object. Before: the first packet of the second file refers to interface 0, but the second file has no IDB. After: the writer writes the IDB again in the second file. No INET module opens a writer two times, so this defect is not visible in a simulation.
- Files: `PcapngWriter.cc`, `tests/unit/PcapngWriterReopen_1.test`.
- Verification: the unit test checks the IDB in the second file. tshark reads the second file.

### P6 `recorder: refactor: write PCAPng blocks through block-level functions`

- Purpose: P7 needs one function for each block type and one encoder for the option list. This commit makes them, protected, and the INET mapping uses them. The output does not change.
- Files: `PcapngWriter.{h,cc}`, `tests/unit/PcapngWriterBytes_1.test`.
- Verification: the new test compares a complete file (SHB, two IDBs, three EPBs) with a fixed byte array. The test passes at P5 and at P6.

### P7 `recorder: add: PCAPng block API with section, interface and packet options`

- Purpose: a caller can write the blocks of the format directly, and give the options of each block (D-1, D-2, D-9, D-10). The INET mapping uses the same API, so a caller can mix both in one file.
- API:
  - `PcapngOptions`: the generic list. `addOption(code, bytes)`, `addString`, `addUint8`, `addUint32`, `addUint64`, `addComment`.
  - `PcapngSectionOptions`, `PcapngInterfaceOptions` (`addName`, `addDescription`, `addIpv4Address`, `addMacAddress`, `addFcsLength`), `PcapngPacketOptions` (`addFlags`).
  - `PcapngPacketFlags`: direction, reception type, FCS length, link-layer errors.
  - Enums of the option codes of each block type.
  - `open(filename)`, `writeSectionHeaderBlock(options)`, `writeInterfaceDescriptionBlock(linkType, snaplen, timePrecision, options)` gives the interface ID, `writeEnhancedPacketBlock(interfaceId, time, data, dataLength, originalLength, options)`.
- Checks: a block before the first SHB, an unknown interface ID, an original length less than the data length, an option value longer than 65535 bytes, an `if_tsresol` in the options, and an FCS length above 15 octets each cause an error.
- Behavior change: an undefined direction gives no `epb_flags` option instead of an exception (D-8).
- Files: `PcapngWriter.{h,cc}`, `tests/unit/PcapngWriterBlocks_1.test`, `doc/whatsnew/compatible/recorder-pcapng-block-api.txt`.
- Verification: the unit test checks the bytes of each option and each error. tshark shows the comments, the flags, the FCS length (`if_fcslen` and the `epb_flags` FCS length on an Ethernet frame) and a second section. The bytes test of P6 still passes.

### P8 `plan: change: move the PCAPng writer plan to done`

`git mv` to `plan/done/`, as its own commit (PR-SPLIT-MOVE).

## Gates

Before each commit: `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`, and `check-source-seals.sh --base origin/master` (it reports the sealed folder, D-3). After P7: a release build, `check-naming.sh --base origin/master`, `check-pull-request.sh --base origin/master`, and `whatsnew.py check`.

## How the rest of PR #1130 fits

- The Radiotap adapter makes the complete record as a `Packet` and calls `writePacket()` with `LINKTYPE_IEEE802_11_RADIOTAP` (D-7). `IPcapWriter` keeps its present method set, so custom writers do not break.
- P4 lets bare 802.11 records and Radiotap records of one interface stand in one file, each with its own IDB.
- Radiotap has its own FCS flag in the header. For bare 802.11 and Ethernet records, PCAPng gives `if_fcslen` and the `epb_flags` FCS length, and `epb_flags` bit 24 can mark the frames that `dumpBadFrames` records. `PcapRecorder` has no route to these options yet. The choice of the route (an `IPcapWriter` change, or a direct use of the block API) is the first decision of the next plan.
- An A-MPDU gives one EPB for each MPDU. The Radiotap A-MPDU status field connects them. The writer needs nothing more for this.

## Evidence

Environment: OMNeT++ `omnetpp-6.x`, debug build of the worktree, `opp_run_unit_tests -m debug -f 'PcapngWriter'`. tshark and capinfos 4.x from `/usr/bin` read the files that a temporary, untracked probe test wrote to `/var/tmp/claude-pcapng-files/`.

- **P2:** `PcapngWriterSnaplen_1.test` PASS. tshark: `frame.len` 6, `frame.cap_len` 4. capinfos: the interface "Capture length = 4".
- **P3:** at the parent, the probe stopped with "Segmentation fault (Address not mapped to object [0x48])". After the repair, `PcapngWriterNoInterface_1.test` PASS. tshark reads the packet without an interface on interface 0 and the other packet on interface 1. capinfos shows no name for interface 0.
- **P4:** at the parent, tshark gave both packets encapsulation 198 and decoded the MAC frame as `fpp:eth`. After the repair, `PcapngWriterLinkTypes_1.test` PASS, and tshark shows interface 0 with encapsulation 198 and interface 1 with encapsulation 1 (`eth`).
- **P5:** at the parent, tshark reported the second file as damaged: "interface index 0 is not less than section interface count 0". My first probe met the same defect without intent, because it opened two files with one writer. After the repair, `PcapngWriterReopen_1.test` PASS, and tshark reads the second file.
- Gates after P5: `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD` PASS. P5 has the obligation `test` only, without a release note, because no simulation can show the defect. The gate accepts this.
