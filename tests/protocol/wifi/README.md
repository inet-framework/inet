# WiFi Conformance Test Suite (up to & including WiFi 5)

A **spec-driven** conformance suite for INET's IEEE 802.11 stack, covering every WiFi
generation up to and including **WiFi 5 (802.11ac)**: 802.11-1997 legacy → b (WiFi 1) →
a (WiFi 2) → g (WiFi 3) → n (WiFi 4) → ac (WiFi 5), plus selected amendments (802.11e
QoS, 802.11h DFS, 802.11i RSN).

Tests are written against the **standard**, not against INET's implementation. A test
that fails or errors because INET is incomplete is a **finding**, not a suite defect —
the suite doubles as a conformance/gap report. Each test program asserts the
spec-required behavior; the `.test` wrapper records the *current* expected outcome so
the suite is also a stable CI gate.

## Layout & how it works

```
wifi/
  run-tests.sh        build + run everything via opp_test
  WifiTestSupport.h   C++ PHY-mode predicates (read the Ieee80211ModeReq tag)
  ned/                WifiInfraNetwork (AP + 2 STA) + package
  ini/                _base.ini + per-generation _legacy/_b/_a/_g/_n/_ac
  common/ legacy/ 11b/ 11a/ 11g/ 11n/ 11ac/   one <Name>.test per test
```

Each test is one **`.test`** file: its program lives in a `%file: <Name>.cc`
(a named `Define_ProtocolTest(...)`); `opp_test gen` extracts all of them and a single
`--deep` build links them into **one `wifitests` binary**, selected per test by the
`ProtocolTester.testName` parameter. `%contains` asserts the verdict.

```sh
. /home/levy/workspace/omnetpp/setenv -q
./run-tests.sh                       # all tests
./run-tests.sh 11n/N_BlockAck.test   # a subset
```

The PHY layer (VHT/HT/OFDM/DSSS mode, MCS, bandwidth, spatial streams) is carried on the
`Ieee80211ModeReq` tag as an opaque `IIeee80211Mode*` that the PacketFilter string engine
cannot follow; the predicates in `WifiTestSupport.h` cast it in C++ via
`EventPattern::match(lambda)`.

## Outcome semantics

- **CONFORMS** — INET produces the spec behavior; the program PASSes; `%contains` expects `PASS`.
- **NOT-MODELED** — INET does not implement the feature; the program FAILs on its deadline;
  `%contains` expects `FAIL` (an *expected failure*, like the repo's `ViolationDetected.test`).
- A red `opp_test` result therefore means a **change**: a CONFORMS test regressed, or a
  NOT-MODELED feature started working (update the matrix). Today: **27 CONFORMS, 11 NOT-MODELED,
  0 DEVIATES — aggregate PASS.**

## Conformance matrix

`[R]` = required by the standard, `[O]` = optional.

| Test | Gen | Feature | R/O | Outcome |
|------|-----|---------|-----|---------|
| `Legacy_PhyMode` | legacy | DSSS 1 Mbps PHY | R | CONFORMS |
| `Legacy_DataAck` | legacy | DCF data + ACK | R | CONFORMS |
| `B_PhyMode` | 11b | HR/DSSS-CCK 11 Mbps PHY | R | CONFORMS |
| `B_DataAck` | 11b | DCF data + ACK | R | CONFORMS |
| `A_PhyMode` | 11a | OFDM 5 GHz / 20 MHz PHY | R | CONFORMS |
| `A_DataAck` | 11a | DCF data + ACK | R | CONFORMS |
| `G_PhyMode` | 11g | ERP-OFDM 2.4 GHz PHY | R | CONFORMS |
| `G_DataAck` | 11g | DCF data + ACK | R | CONFORMS |
| `G_Nav` | 11g | Non-zero NAV (duration field) | R | CONFORMS |
| `G_RtsCts` | 11g | RTS/CTS → data → ACK | O | CONFORMS |
| `N_HtPhy` | 11n | HT PHY mode | R | CONFORMS |
| `N_Mimo2ss` | 11n | HT 2 spatial streams (MIMO) | O | CONFORMS |
| `N_Amsdu` | 11n | A-MSDU aggregation | O | CONFORMS |
| `N_BlockAck` | 11n | HT Block Ack (ADDBA → BAR → BA) | R | CONFORMS |
| `N_CompressedBa` | 11n | Compressed Block Ack bitmap | R | CONFORMS |
| `N_RtsCts` | 11n | HCF RTS/CTS | R | CONFORMS |
| `N_Ampdu` | 11n | A-MPDU aggregation | R | **NOT-MODELED** |
| `N_Smps` | 11n | SM Power Save action frame | O | **NOT-MODELED** |
| `Ac_PhyMode` | 11ac | VHT 80 MHz PHY mode | R | CONFORMS |
| `Ac_160mhz` | 11ac | VHT 160 MHz channel | O | CONFORMS |
| `Ac_256qam` | 11ac | 256-QAM (VHT MCS 8/9) | O | CONFORMS |
| `Ac_4ss` | 11ac | VHT 4 spatial streams (INET max) | O | CONFORMS |
| `Ac_BlockAck` | 11ac | Block Ack session | R | CONFORMS |
| `Ac_Ampdu` | 11ac | A-MPDU (mandatory in 11ac) | R | **NOT-MODELED** |
| `Ac_OpModeNotification` | 11ac | VHT Operating Mode Notification | O | **NOT-MODELED** |
| `Ac_GroupIdMgmt` | 11ac | VHT Group ID Management (MU) | O | **NOT-MODELED** |
| `Ac_MuMimo` | 11ac | MU-MIMO downlink | O | **NOT-MODELED** |
| `Ac_Beamforming` | 11ac | VHT beamforming sounding (NDP) | O | **NOT-MODELED** |
| `Ac_MultiTidBa` | 11ac | Multi-TID Block Ack | O | **NOT-MODELED** |
| `WifiBeacon` | common | AP beacon frame | R | CONFORMS |
| `WifiBeaconInterval` | common | Beacon periodicity | R | CONFORMS |
| `WifiProbe` | common | Active-scan ProbeReq/Resp | R | CONFORMS |
| `WifiAuthOpen` | common | Open-system authentication | R | CONFORMS |
| `WifiAssociation` | common | Association FSM (Beacon→Auth→Assoc) | R | CONFORMS |
| `WifiDataDelivery` | common | End-to-end data delivery | R | CONFORMS |
| `WifiQosAddts` | common (11e) | ADDTS/TSPEC admission | O | **NOT-MODELED** |
| `WifiDfsChannelSwitch` | common (11h) | DFS Channel Switch Announcement | O | **NOT-MODELED** |
| `WifiRsn4way` | common (11i) | RSN/WPA2 4-way (EAPOL) | O | **NOT-MODELED** |

## Notable findings (INET 802.11 gaps surfaced by the suite)

- **A-MPDU is not implemented** (`N_Ampdu`, `Ac_Ampdu`). INET defines
  `Ieee80211MpduSubframeHeader` and an aggregation-policy module, but
  `BasicMpduAggregationPolicy::computeAggregateFrames()` returns `nullptr`, so no A-MPDU
  is ever emitted. This is a *required* 802.11ac feature (all VHT data is meant to be
  carried in A-MPDUs) — the most significant gap here. (A-MSDU *is* modeled, `N_Amsdu`.)
- **Spatial streams cap at 4.** 802.11ac optionally allows up to 8; INET's VHT MCS tables
  and antenna model top out at 4 SS. `Ac_4ss` verifies the realistic maximum; 5–8 SS is a gap.
- **Multi-TID Block Ack** is explicitly `// TODO unimplemented` in INET (`Ac_MultiTidBa`).
- **VHT MU features** — MU-MIMO, Group ID Management, beamforming sounding (NDP
  Announcement / compressed beamforming report), Operating Mode Notification — are not
  modeled (`Ac_MuMimo`, `Ac_GroupIdMgmt`, `Ac_Beamforming`, `Ac_OpModeNotification`).
- **Amendments**: SM Power Save (`N_Smps`), 802.11e ADDTS/TSPEC (`WifiQosAddts`), 802.11h
  DFS/Channel Switch (`WifiDfsChannelSwitch`), 802.11i RSN/WPA2 EAPOL (`WifiRsn4way`) are
  not present at the MAC level.

## Adding a test

Copy an existing `.test` (e.g. `11ac/Ac_PhyMode.test`). Put the program in `%file: <Name>.cc`
(unique basename), select it via `*.tester.testName` in the `%inifile`, include the right
`ini/_<gen>.ini` + `ini/_base.ini`, and set `%contains` to the expected verdict. For a
PHY assertion, use the predicates in `WifiTestSupport.h`; for a frame-sequence assertion,
match `ieee80211mac.type == <n>` (see the type table in the test comments). To author
against the real frame trace, temporarily add `*.tester.logEvents = true` and read
`work/<Name>/test.out`.
