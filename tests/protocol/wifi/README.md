# WiFi Conformance Test Suite (up to & including WiFi 5)

A **spec-driven** conformance suite for INET's IEEE 802.11 stack, covering every WiFi
generation up to and including **WiFi 5 (802.11ac)**: 802.11-1997 legacy → b (WiFi 1) →
a (WiFi 2) → g (WiFi 3) → n (WiFi 4) → ac (WiFi 5), plus amendments (802.11d/e/h/i/k/w).

Tests are written against the **standard**, not against INET's implementation. A test
that fails because INET is incomplete is a **finding**, not a suite defect — the suite
doubles as a conformance/gap report. Each test program asserts the spec-required behavior;
the `.test` wrapper records the *current* expected outcome so the suite is also a stable
CI gate.

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
./run-tests.sh                       # all 74 tests
./run-tests.sh 11n/N_BlockAck.test   # a subset
```

PHY-layer mode (VHT/HT/OFDM/DSSS, MCS, bandwidth, spatial streams, guard interval, slot
time) rides on the `Ieee80211ModeReq` tag as an opaque `IIeee80211Mode*` that the
PacketFilter string engine cannot follow; the predicates in `WifiTestSupport.h` cast it in
C++ via `EventPattern::match(lambda)`. PHY rates are pinned with
`mac.*.rateSelection.dataFrameBitrate/Bandwidth/NumSpatialStreams`.

## Outcome semantics

Every test asserts the spec behavior, so `%contains` always expects the program to `PASS`;
the two outcomes differ only in whether INET actually does it:

- **CONFORMS** — INET produces the spec behavior; the program PASSes → opp_test **PASS**.
- **NOT-MODELED** — INET does not implement the feature, so the faithful assertion misses its
  deadline and the program FAILs. The `.test` carries an `%expected-failure:` directive, so
  opp_test reports it as **EXPECTEDFAIL** (yellow) — an honest, first-class "expected failure",
  not a disguised PASS, and it does not fail the run. If INET later implements the feature the
  program PASSes and the test flips EXPECTEDFAIL → PASS (a cue to move the row to CONFORMS). The
  `.cc` assertion stays honest to the standard.
- A **FAIL** (red) `opp_test` result therefore signals a real **regression** of a CONFORMS test.

**Today: 39 CONFORMS (PASS), 35 NOT-MODELED (EXPECTEDFAIL) across 74 tests — aggregate PASS.**

## Conformance matrix

`[R]` required, `[O]` optional. ✅ CONFORMS · ⛔ NOT-MODELED.

### common (cross-generation MAC + amendments)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `WifiBeacon` | AP beacon frame | R | ✅ |
| `WifiBeaconInterval` | Beacon periodicity | R | ✅ |
| `WifiProbe` | Active-scan ProbeReq/Resp | R | ✅ |
| `WifiAuthOpen` | Open-system authentication | R | ✅ |
| `WifiAssociation` | Association FSM (Beacon→Auth→Assoc) | R | ✅ |
| `WifiDataDelivery` | End-to-end data delivery | R | ✅ |
| `WifiAuthSharedKey` | WEP shared-key auth (4-frame challenge) | O | ⛔ open auth only |
| `WifiReassociation` | Reassociation Req/Resp (types 2/3) | R | ⛔ no roaming |
| `WifiDeauth` | Deauthentication (type 12) | R | ⛔ teardown is silent |
| `WifiRetransmission` | Retry bit on retransmission | R | ⛔* see caveat below |
| `WifiPmf` | Protected Management Frames (11w) | O | ⛔ |
| `WifiRsn4way` | RSN/WPA2 4-way EAPOL (11i) | O | ⛔ |
| `WifiQosAddts` | ADDTS/TSPEC admission (11e) | O | ⛔ |
| `WifiQosUapsd` | U-APSD power save (11e) | O | ⛔ |
| `WifiQosDelayedBa` | Delayed Block Ack (11e) | O | ⛔ immediate BA only |
| `WifiDfsChannelSwitch` | DFS Channel Switch Announcement (11h) | O | ⛔ |
| `WifiTpc` | TPC Report / Power Constraint (11h) | O | ⛔ |
| `WifiCountryIe` | Country element in Beacon (11d) | O | ⛔ |
| `WifiRrmMeasurement` | Measurement Request action (11k) | O | ⛔ |

`*` **`WifiRetransmission` caveat**: INET *does* model the Retry bit (BasicRecoveryProcedure),
but the loss-free shared test network never triggers a retransmission, and forcing
deterministic loss needs an interferer/PacketTap not in `WifiInfraNetwork`. So this is a
**test-coverage** gap, not an INET gap — listed ⛔ only because the assertion currently can't
be satisfied without changing the shared harness.

### legacy (802.11-1997)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Legacy_PhyMode` | DSSS 1 Mbps PHY | R | ✅ |
| `Legacy_DataAck` | DCF data + ACK | R | ✅ |
| `Legacy_RtsCts` | RTS/CTS exchange | O | ✅ |
| `Legacy_Fragmentation` | Fragmentation (`moreFragments`) | R | ✅ |
| `Legacy_Nav` | Non-zero NAV (duration field) | R | ✅ |
| `Legacy_Pcf` | PCF / CF-Poll | O | ⛔ "PCF is missing" (INET) |

### 11b (WiFi 1)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `B_PhyMode` | HR/DSSS-CCK 11 Mbps PHY | R | ✅ |
| `B_Cck55Phy` | HR/DSSS-CCK 5.5 Mbps PHY | R | ✅ |
| `B_DataAck` | DCF data + ACK | R | ✅ |
| `B_RtsCts` | RTS/CTS | R | ✅ |
| `B_ShortPreamble` | HR/DSSS short preamble | O | ⛔ always long preamble |
| `B_Pbcc` | PBCC coding | O | ⛔ never selected |

### 11a (WiFi 2)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `A_PhyMode` | OFDM 5 GHz / 20 MHz PHY | R | ✅ |
| `A_5ghz` | OFDM 5 GHz, 20 MHz bandwidth | R | ✅ |
| `A_DataAck` | DCF data + ACK | R | ✅ |
| `A_OfdmRates` | OFDM 6 Mbps mandatory rate | R | ✅ |

### 11g (WiFi 3)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `G_PhyMode` | ERP-OFDM 2.4 GHz PHY | R | ✅ |
| `G_DataAck` | DCF data + ACK | R | ✅ |
| `G_MandatoryRates` | ERP-OFDM mandatory rate (6 Mbps) | R | ✅ |
| `G_ShortSlot` | Short slot time (9 µs) | R | ✅ |
| `G_Nav` | Non-zero NAV (duration field) | R | ✅ |
| `G_RtsCts` | RTS/CTS → data → ACK | O | ✅ |
| `G_ErpProtection` | ERP protection (CTS-to-self/RTS in mixed b/g) | R | ⛔ no protection logic |

### 11n (WiFi 4 — HT)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `N_HtPhy` | HT PHY mode | R | ✅ |
| `N_Mimo2ss` | HT 2 spatial streams (MIMO) | O | ✅ |
| `N_ShortGi` | HT short guard interval (400 ns) | O | ✅ |
| `N_QosEdcaAc` | EDCA QoS data (access category) | R | ✅ |
| `N_TxopBurst` | TXOP burst (approx.) | O | ✅ |
| `N_Amsdu` | A-MSDU aggregation | O | ✅ |
| `N_BlockAck` | HT Block Ack (ADDBA→BAR→BA) | R | ✅ |
| `N_CompressedBa` | Compressed Block Ack bitmap | R | ✅ |
| `N_RtsCts` | HCF RTS/CTS | R | ✅ |
| `N_Ampdu` | A-MPDU aggregation | R | ⛔ policy returns nullptr |
| `N_HtCapabilitiesIe` | HT Capabilities IE in AssocReq | R | ⛔ no IE in mgmt frame |
| `N_HtOperationIe` | HT Operation IE in Beacon | R | ⛔ no IE in mgmt frame |
| `N_Smps` | SM Power Save action frame | O | ⛔ |
| `N_Greenfield` | HT greenfield preamble | O | ⛔ always MIXED |
| `N_StbcCap` | HT STBC | O | ⛔ `getSTBC()` hardcoded 0 |
| `N_LdpcCap` | HT LDPC | O | ⛔ LDPC absent |
| `N_2040Coex` | 20/40 BSS Coexistence | O | ⛔ |
| `N_ReverseDirection` | Reverse Direction Grant (RDG) | O | ⛔ no HT Control field |

### 11ac (WiFi 5 — VHT)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Ac_PhyMode` | VHT 80 MHz PHY mode | R | ✅ |
| `Ac_160mhz` | VHT 160 MHz channel | O | ✅ |
| `Ac_256qam` | 256-QAM (VHT MCS 8/9) | O | ✅ |
| `Ac_4ss` | VHT 4 spatial streams (INET max) | O | ✅ |
| `Ac_BlockAck` | Block Ack session | R | ✅ |
| `Ac_Ampdu` | A-MPDU (mandatory in 11ac) | R | ⛔ policy returns nullptr |
| `Ac_VhtIe` | VHT Capabilities/Operation IE | R | ⛔ no IE in mgmt frame |
| `Ac_OpModeNotification` | VHT Operating Mode Notification | O | ⛔ |
| `Ac_GroupIdMgmt` | VHT Group ID Management (MU) | O | ⛔ |
| `Ac_MuMimo` | MU-MIMO downlink | O | ⛔ |
| `Ac_Beamforming` | VHT beamforming sounding (NDP) | O | ⛔ |
| `Ac_MultiTidBa` | Multi-TID Block Ack | O | ⛔ `// TODO` in INET |
| `Ac_80p80` | VHT 80+80 MHz non-contiguous | O | ⛔ |
| `Ac_LdpcStbcCap` | VHT LDPC + STBC | O | ⛔ STBC hardcoded 0, no LDPC |

## Notable findings (INET 802.11 gaps surfaced by the suite)

- **A-MPDU is not implemented** (`N_Ampdu`, `Ac_Ampdu`): `Ieee80211MpduSubframeHeader` and an
  aggregation-policy module exist, but `BasicMpduAggregationPolicy::computeAggregateFrames()`
  returns `nullptr`, so no A-MPDU is ever emitted — a *required* 802.11ac feature (all VHT data
  is meant to ride in A-MPDUs). The most significant gap. (A-MSDU *is* modeled.)
- **No HT/VHT management Information Elements** (`N_HtCapabilitiesIe`, `N_HtOperationIe`,
  `Ac_VhtIe`): Beacon/AssociationRequest carry no HT/VHT Capabilities or Operation elements, so
  capability negotiation is not modeled — notable for real-device interoperability.
- **ERP protection absent** (`G_ErpProtection`): `g(mixed)` never emits CTS-to-self/RTS before
  ERP-OFDM data when legacy STAs are present.
- **STBC hardcoded off** in both HT and VHT signal modes (`getSTBC()` ≡ 0); **LDPC absent**.
- **Spatial streams cap at 4** (spec allows 8); `Ac_4ss` verifies INET's realistic maximum.
- **Multi-TID Block Ack** is `// TODO unimplemented` in INET.
- **Not modeled**: PCF, PBCC, short preamble, greenfield, RDG, 80+80 MHz, 20/40 coexistence,
  reassociation/roaming, deauth-on-teardown, shared-key/WEP & RSN/WPA2, PMF (11w), U-APSD &
  delayed BA, ADDTS/TSPEC (11e), DFS & TPC (11h), Country IE (11d), RRM measurement (11k).

## Adding a test

Copy an existing `.test` (e.g. `11ac/Ac_PhyMode.test`). Put the program in `%file: <Name>.cc`
(unique basename), select it via `*.tester.testName`, include the right `ini/_<gen>.ini` +
`ini/_base.ini`, set `%contains` to the expected verdict (`PASS` for CONFORMS, `FAIL` for a
faithful NOT-MODELED assertion). Use `WifiTestSupport.h` predicates for PHY assertions or
`ieee80211mac.type == <n>` for frame sequences. To author against the real trace, temporarily
add `*.tester.logEvents = true` and read `work/<Name>/test.out`.
