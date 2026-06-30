# Plan: Conformance-oriented WiFi Protocol Test Suite (up to & including WiFi 5)

## Context

The INET protocol test framework (`/home/levy/workspace/inet-protocoltest/tests/protocol/lib/`)
currently has only **3 WiFi tests** (802.11n ad-hoc, asserting a few Block-Ack/A-MSDU frame types).
The goal is a **faithful, comprehensive conformance test suite** for every WiFi generation **up to and
including WiFi 5 (802.11ac)**: 802.11-1997 legacy → b (WiFi 1) → a (WiFi 2) → g (WiFi 3) → n (WiFi 4)
→ ac (WiFi 5), plus the amendments folded into the standard along the way (802.11e QoS, 802.11h
spectrum management, 802.11i/RSN security, protected management frames, etc.).

**This suite is written against the standard, not against INET's current implementation.** Tests
assert the **required (mandatory)** and **optional** behaviors the IEEE 802.11 specification defines.
It is expected and acceptable that **some tests FAIL or ERROR** because INET's implementation is
incomplete, incorrect, or does not model a feature at all — those outcomes are the *point*: the suite
doubles as a conformance/gap report for INET's 802.11 stack.

## Test philosophy — spec-driven, failures are signal

- **Author to the spec.** Each test encodes a behavior mandated or permitted by 802.11, expressed as
  an observable frame exchange, a header/IE field, a timing relationship, or a PHY-mode property. We do
  **not** pre-shape tests to what INET happens to produce.
- **Three outcome classes**, recorded per test (the deliverable is this matrix, not "all green"):
  - **CONFORMS** — INET produces the spec-required behavior → test PASS.
  - **DEVIATES** — INET produces something, but not per spec → test FAIL.
  - **NOT-MODELED** — INET emits nothing / lacks the frame type/field/module → test FAIL on deadline
    or ERROR at setup.
- Each test row carries an **anticipated** outcome (C / X / ?) purely as triage so expected failures
  aren't mistaken for regressions. **The anticipation never changes how the test is written.**
- **[R]/[O]** tags mark each feature *required* vs *optional* per the standard.
- Where a feature is pure-PHY with **no MAC-observable artifact** (e.g. LDPC/STBC coding), the test
  targets its **observable proxy** — the Capabilities Information Element bit that advertises it, or
  the management/control frame that signals it. Truly unobservable items are listed as **documented
  coverage gaps** (no test), to stay honest about the framework's reach.

## Hard constraints (per user)

1. **Every new file lives under `tests/protocol/wifi/`** — a *sibling* of `lib/`, never inside it.
2. **No existing file is modified** without explicit approval (`lib/omnetpp.ini`, `lib/AUTHORING.md`,
   `lib/build.sh`, `lib/Makefile`, `ProtocolTests.cc`, the `WifiBlockAckDemo` suite — all untouched).
   The suite is therefore fully self-contained.

## Approach — self-contained suite, one binary, runtime selection

`tests/protocol/wifi/` is its own buildable unit with one subfolder per generation. Each test is its
own `.cc` registering a named program with `Define_ProtocolTest(wifi_<gen>_<feature>)`. A
**`wifi/build.sh`** runs `opp_makemake -f --deep -o wifitests` over the folder, producing a single
executable **`wifitests`** that links the framework (`-I../lib -L../lib -lprotocoltest`) and INET
(`-I<inet>/src -L<inet>/src -lINET`); `--deep` collects every generation subfolder, so all tests
compile into that one binary. Each `[Config]` sets `*.tester.testName = "wifi_<gen>_<feature>"` to
select the program. The framework's `ProtocolTester` module, registry and NED types come from `../lib`
(`libprotocoltest.so` for symbols, `-n ../lib` on the NED path for types) — so nothing in `lib/` is
edited; cross-library `Define_ProtocolTest` registration already works this way for the existing
`.test`/`runtest` builds.

```
tests/protocol/wifi/              ← new, sibling of tests/protocol/lib/
  package.ned                     package inet.protocoltest.wifi  (imports inet.protocoltest.*)
  build.sh                        opp_makemake -f --deep -o wifitests  -I../lib -L../lib -lprotocoltest  -I<inet>/src -L<inet>/src -lINET
  omnetpp.ini                     base + per-test [Config Wifi*] + Trace configs; `include`s each <gen>.ini
  README.md                       suite docs + live CONFORMS/DEVIATES/NOT-MODELED conformance matrix
  common/
    WifiTestSupport.h             frame-type/category constants, IE helpers, per-generation mode-cast
                                  predicates (single place INET mode/tag/frame headers are #included)
    WifiInfraNetwork.ned          AP + 2×STA infra net; opMode/band per-config
    WifiMuNetwork.ned             AP + 3×STA (for MU-MIMO / group-id / beamforming attempts)
    Wifi*.cc                      cross-generation tests (association FSM, beacon, retransmission, CW)
    wifi-common.ini
  legacy/  11b/  11a/  11g/  11n/  11ac/   one <gen>.ini + one .cc per feature row below
```

The suite's **own `omnetpp.ini`** holds all configs; each `<gen>.ini` defines a
`[Config Wifi<Gen>Scenario]` base (network, opMode/band/antennas, traffic) + per-test
`[Config Wifi<Gen>_<feature>]` + a `[Config Wifi<Gen>Trace]` authoring config.

## Confirmed framework + INET facts (from exploration)

- **Observability**: PHY mode is not a chunk field — it rides on the `Ieee80211ModeReq` tag as an
  opaque `const IIeee80211Mode *`. The string `PacketFilter` can't follow it; a **C++ lambda via
  `EventPattern::match(MatchPredicate)`** can (cast to the concrete mode class). MAC frames, headers,
  and Information Elements *are* dissectable by `packet("…")` filters and lambdas.
- **opMode strings** (exactly 7): `"a"`, `"b"`, `"g(mixed)"`, `"g(erp)"`, `"n(mixed-2.4Ghz)"`, `"ac"`,
  `"p"`. No `"n(mixed-5Ghz)"`; legacy 1997 = `"b"` + `bitrate=1Mbps` (→ `Ieee80211DsssMode`). Default `"g(mixed)"`.
- **Mode classes** (dynamic_cast targets) + getters via `getDataMode()`: `Ieee80211DsssMode` /
  `Ieee80211HrDsssMode` / `Ieee80211OfdmMode` / `Ieee80211ErpOfdmMode` (NSS 1, no MCS) and
  `Ieee80211HtMode` / `Ieee80211VhtMode` (add `getMcsIndex()`, NSS up to 4/8). Bands: `"2.4 GHz"`
  (b/g/n), `"5 GHz (20 MHz)"` (a), `"5 GHz (20/40/80/160 MHz)"` (ac).
- **Default mgmt does the full handshake** (`Ieee80211MgmtAp`/`Ieee80211MgmtSta`) for every opMode.
- **INET models** (so likely CONFORMS): DCF data+ACK, RTS/CTS, NAV, beacon/probe/auth(open)/assoc,
  fragmentation, EDCA/QoS, A-MPDU, A-MSDU, basic+compressed Block Ack, ADDBA/DELBA, AARF rate control,
  HT & VHT PHY modes. **INET likely does NOT model** (so likely NOT-MODELED → expected FAIL): MU-MIMO,
  SU/MU beamforming/NDP sounding, Group-ID management, Operating-Mode Notification, RTS bandwidth
  signaling, LDPC/STBC, multi-TID Block Ack (source says "TODO unimplemented"), TSPEC/ADDTS admission,
  PCF/HCCA, shared-key/RSN security, 802.11h DFS/TPC, protected management frames. (To verify per row.)

Frame `type` = combined type+subtype byte: `ST_ASSOCIATIONREQUEST=0`, `…RESPONSE=1`, `ST_PROBEREQUEST=4`,
`…RESPONSE=5`, `ST_BEACON=8`, `ST_DISASSOCIATION=10`, `ST_AUTHENTICATION=11`, `ST_DEAUTHENTICATION=12`,
`ST_ACTION=13`, `ST_BLOCKACK_REQ=24`, `ST_BLOCKACK=25`, `ST_PSPOLL=26`, `ST_RTS=27`, `ST_CTS=28`,
`ST_ACK=29`, `ST_DATA=32`, `ST_DATA_WITH_QOS=40`. Module paths: STA `sta1.wlan[0].mac`, AP `ap.wlan[0].mac`.

---

## Feature coverage (the test list) — [R]equired / [O]ptional, anticipated C/X/?

### common/ — cross-generation MAC (frames identical b→ac)
| Feature [R/O] | test | asserts | antic. |
|---|---|---|---|
| Beacon generation [R] | `wifi_beacon` | AP sends Beacon(8) carrying SSID/timestamp/beacon-interval IEs | C |
| Beacon periodicity [R] | `wifi_beacon_interval` | spacing ≈ beaconInterval (capture 2 beacons + lambda) | C |
| Active-scan probe [R] | `wifi_probe` | STA ProbeRequest(4) → AP ProbeResponse(5) | C |
| Open-System auth [R] | `wifi_auth_open` | Auth(11) seq=1 → seq=2 statusCode=0 | C |
| Shared-Key auth (WEP) [O] | `wifi_auth_sharedkey` | Auth 4-frame challenge exchange | X |
| Association [R] | `wifi_association` | AssocReq(0) → AssocResp(1) statusCode=0, capture AID | C |
| Reassociation [O] | `wifi_reassociation` | ReassocReq(2) → ReassocResp(3) | ? |
| Disassociation/Deauth [R] | `wifi_deauth` | Disassoc(10)/Deauth(12) on teardown | ? |
| Retransmission + retry bit [R] | `wifi_retransmission` | hidden-node collision → data with `retry==true` | C |
| Contention-window growth [R] | `wifi_contention_window` | `contentionWindowChanged` signal grows on failure | ? |
| Duplicate detection [R] | `wifi_duplicate_filtered` | retried dup not delivered twice upward | ? |
| Protected mgmt frames (11w) [O] | `wifi_pmf` | mgmt frames protected (RSN MFP) | X |

### legacy/ — 802.11-1997
| DSSS 1/2 Mbps PHY [R] | `wifi_legacy_dsss_phy` | tx mode = `Ieee80211DsssMode`, bitrate ∈ {1,2} Mbps | C |
| DCF data+ACK (SIFS) [R] | `wifi_legacy_data_ack` | Data(32) → ACK(29) after ~SIFS | C |
| DIFS+backoff before tx [R] | `wifi_legacy_difs_backoff` | gap ≥ DIFS before a contended frame (timestamp lambda) | ? |
| NAV via duration [R] | `wifi_legacy_nav` | Data/RTS `durationField > 0` | C |
| RTS/CTS [O] | `wifi_legacy_rts_cts` | RTS(27)→CTS(28)→Data→ACK | C |
| Fragmentation/reassembly [O] | `wifi_legacy_fragmentation` | `moreFragments` set, fragment numbers increment | C |
| PCF contention-free [O] | `wifi_legacy_pcf` | Beacon CF-Parameter-Set / CF-Poll frames | X |

### 11b/ — 802.11b (WiFi 1)
| HR/DSSS CCK 11 Mbps [R] | `wifi_11b_cck11_phy` | tx mode = `Ieee80211HrDsssMode`, 11 Mbps | C |
| CCK 5.5 Mbps [R] | `wifi_11b_cck55_phy` | mode `HrDsss`, 5.5 Mbps | C |
| Multi-rate fallback [R] | `wifi_11b_rate_fallback` | under loss, a lower-rate mode is selected across retries (lambda compares modes) | C |
| Short preamble [O] | `wifi_11b_short_preamble` | preamble = short (mode/PHY-header proxy) | ? |
| RTS/CTS [O] | `wifi_11b_rts_cts` | RTS→CTS→Data→ACK, RTS NAV>0 | C |
| PBCC [O] | `wifi_11b_pbcc` | (no INET modeling) | X |

### 11a/ — 802.11a (WiFi 2)
| OFDM mandatory 6/12/24 [R] | `wifi_11a_ofdm_mandatory` | mode = `Ieee80211OfdmMode` at 6/12/24 Mbps, 20 MHz, 5 GHz | C |
| OFDM full rate set 6–54 [O] | `wifi_11a_ofdm_rates` | sweep: each of the 8 OFDM rates realizable | C |
| 5 GHz band/channel [R] | `wifi_11a_5ghz` | tx mode bandwidth 20 MHz, 5 GHz center freq | C |

### 11g/ — 802.11g (WiFi 3)
| ERP-OFDM PHY [R] | `wifi_11g_erp_ofdm_phy` | mode = `Ieee80211ErpOfdmMode`, 2.4 GHz | C |
| ERP protection (mixed b/g) [R] | `wifi_11g_protection` | CTS-to-self or RTS/CTS precedes OFDM data when a b-STA is present | ? |
| Mandatory rate set (1/2/5.5/11/6/12/24) [R] | `wifi_11g_mandatory_rates` | rate sweep includes DSSS+OFDM | C |
| Short slot time [O] | `wifi_11g_short_slot` | inter-frame timing reflects 9 µs slot (timestamp lambda) | ? |

### 11e (QoS) — exercised on the 11n network (HCF); origin noted
| EDCA 4 access categories [R] | `wifi_qos_edca_ac` | VO/VI/BE/BK frames carry distinct UP/TID; higher AC wins access (timing) | C |
| TXOP bursting [O] | `wifi_qos_txop_burst` | multiple QoS data frames SIFS-separated within one TXOP | C |
| TSPEC admission (ADDTS/DELTS) [O] | `wifi_qos_addts` | ADDTS Request/Response action frames | X |
| U-APSD power save [O] | `wifi_qos_uapsd` | trigger-enabled QoS-Null + buffered delivery | X |
| Block Ack — delayed [O] | `wifi_qos_delayed_ba` | delayed BA variant (BAR/BA not in same TXOP) | ? |

### 11n/ — 802.11n / HT (WiFi 4)
| HT MCS0–7, 20 MHz, 1 stream [R] | `wifi_11n_ht_mcs_basic` | mode = `Ieee80211HtMode`, mcs 0–7, NSS 1, 20 MHz | C |
| 40 MHz channel [O] | `wifi_11n_40mhz` | `HtMode` bandwidth 40 MHz | C |
| Short Guard Interval [O] | `wifi_11n_short_gi` | `getGuardIntervalType()==SHORT` | ? |
| MCS 8–31 (2–4 streams) [O] | `wifi_11n_mimo_streams` | `HtMode` NSS ≥ 2, mcs ≥ 8 | C |
| A-MPDU [R] | `wifi_11n_ampdu` | ≥2 `Ieee80211MpduSubframeHeader` in a PSDU | C |
| A-MSDU [O] | `wifi_11n_amsdu` | `aMsduPresent==true`, ≥2 `Ieee80211MsduSubframeHeader` | C |
| HT-immediate Block Ack [R] | `wifi_11n_block_ack_full` | ADDBA→Data(BA policy)→BAR(24)→BA(25) | C |
| Compressed Block Ack [R] | `wifi_11n_compressed_ba` | `Ieee80211CompressedBlockAck.compressedBitmap==1` | C |
| HT Capabilities IE [R] | `wifi_11n_ht_capabilities_ie` | Beacon/AssocReq carry HT Capabilities element | ? |
| HT Operation IE [R] | `wifi_11n_ht_operation_ie` | Beacon carries HT Operation element | ? |
| RD (reverse direction) [O] | `wifi_11n_reverse_direction` | RDG grant + reverse data in one TXOP | X |
| STBC [O] | `wifi_11n_stbc_cap` | HT Capabilities STBC bit advertised (proxy) | X |
| LDPC [O] | `wifi_11n_ldpc_cap` | HT Capabilities LDPC bit advertised (proxy) | X |
| Greenfield preamble [O] | `wifi_11n_greenfield` | greenfield preamble mode (proxy) | X |
| SM power save [O] | `wifi_11n_smps` | SM Power Save action frame | X |
| 20/40 BSS coexistence [O] | `wifi_11n_2040_coex` | 20/40 BSS Coexistence mgmt frame / intolerant bit | X |

### 11ac/ — 802.11ac / VHT (WiFi 5)
| VHT 80 MHz, MCS0–7 [R] | `wifi_11ac_vht_80mhz` | mode = `Ieee80211VhtMode`, 80 MHz, mcs 0–7 | C |
| 160 MHz [O] | `wifi_11ac_160mhz` | `VhtMode` bandwidth 160 MHz | C |
| 80+80 MHz [O] | `wifi_11ac_80p80` | non-contiguous 160 (80+80) | X |
| 256-QAM (MCS 8/9) [O] | `wifi_11ac_256qam` | `VhtMode` mcs ∈ {8,9} | ? |
| Up to 8 spatial streams [O] | `wifi_11ac_8ss` | `VhtMode` NSS up to 8 | C |
| A-MPDU mandatory (all data) [R] | `wifi_11ac_ampdu_mandatory` | every data MPDU carried in an A-MPDU | ? |
| Compressed Block Ack mandatory [R] | `wifi_11ac_compressed_ba` | `Ieee80211CompressedBlockAck` | C |
| VHT Capabilities / Operation IE [R] | `wifi_11ac_vht_ie` | Beacon/AssocReq carry VHT Capabilities + Operation elements | X? |
| Operating Mode Notification [O] | `wifi_11ac_opmode_notification` | VHT-action Operating Mode Notification frame | X |
| RTS/CTS bandwidth signaling [R] | `wifi_11ac_dynamic_bw_rts` | RTS carries bandwidth indication; CTS echoes granted BW | X |
| Group ID Management (MU) [O] | `wifi_11ac_group_id_mgmt` | VHT-action Group ID Management frame | X |
| MU-MIMO downlink [O] | `wifi_11ac_mu_mimo` | per-STA MU PPDU after group setup (proxy: group-id + VHT-SIG) | X |
| SU/MU beamforming sounding [O] | `wifi_11ac_beamforming` | VHT NDP Announcement → NDP → VHT Compressed Beamforming report | X |
| Multi-TID Block Ack [O] | `wifi_11ac_multi_tid_ba` | `Ieee80211MultiTidBlockAck` (INET: "TODO unimplemented") | X |
| LDPC / STBC capability [O] | `wifi_11ac_ldpc_stbc_cap` | VHT Capabilities LDPC/STBC bits (proxy) | X |

### Cross-cutting amendments (5 GHz / security) — anticipated mostly NOT-MODELED
| 11h DFS channel switch [O/R-5GHz] | `wifi_dfs_channel_switch` | Channel Switch Announcement frame/IE | X |
| 11h TPC [O/R-5GHz] | `wifi_tpc` | Power Constraint IE / TPC Report | X |
| 11i RSN (WPA2) [O] | `wifi_rsn_4way` | RSN IE in beacon + EAPOL 4-way handshake | X |
| 11d country IE [O] | `wifi_country_ie` | Country element in Beacon | X |
| 11k radio measurement [O] | `wifi_rrm_measurement` | Measurement Request/Report action frames | X |

A handful of intentional-PASS sanity checks (e.g. `wifi_association`, `wifi_11ac_vht_80mhz`) anchor the
suite so a *green* baseline is distinguishable from breakage; everything else is judged against its
anticipated column.

## PHY-mode lambda pattern (`common/WifiTestSupport.h`)

```cpp
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h" // + Ht/Ofdm/ErpOfdm/HrDsss/Dsss
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
using namespace inet::physicallayer;
inline const IIeee80211Mode *txMode(const PacketEvent& e) {
    auto r = e.packet->findTag<Ieee80211ModeReq>();  return r ? r->getMode() : nullptr; }
inline bool isVht(const PacketEvent& e, Hz bw, int minSs) {
    auto m = dynamic_cast<const Ieee80211VhtMode*>(txMode(e));
    return m && m->getDataMode()->getBandwidth()==bw && m->getDataMode()->getNumberOfSpatialStreams()>=minSs; }
```
Used as `.match([](const MatchContext& c){ return isVht(c.event, MHz(80), 2); }).describe("…")`, with a
cheap `.packet("ieee80211mac.type == 40")` prefilter. One small predicate per PHY family (Dsss/HrDsss/
Ofdm/ErpOfdm/Ht/Vht); `getMcsIndex()` only on Ht/Vht. IE-bit/action-frame proxies use `packet("…")`
filters or lambdas over the dissected management frame.

## Files

All new, all under **`tests/protocol/wifi/`** (tree above): `package.ned`, `build.sh`, `omnetpp.ini`,
`README.md` (holds the live conformance matrix), `common/` (shared header, two networks, cross-gen
tests, ini), and one subfolder per generation (one `.cc` per feature row + `<gen>.ini`). **No existing
file modified** — the suite references `lib/` (headers + `libprotocoltest.so` + NED path) but never
edits it; the existing protocol-test suite is unaffected by construction. (The only conceivable `lib/`
edit — a one-line cross-reference in `lib/AUTHORING.md` — is approval-gated and excluded here.)

On completion, move the plan `plan/pending/ → plan/done/` per the repo convention.

## Items to confirm during implementation (does not gate authoring)

- Exact A-MPDU policy param path (`…originatorMacDataService.mpduAggregationPolicy.*`).
- That `Ieee80211ModeReq` is present at the `packetSentToLower` point (verify via a Trace config).
- Whether INET dissects HT/VHT Capabilities/Operation IEs and which action-frame categories exist
  (decides whether several [R] IE/action tests land as DEVIATES vs NOT-MODELED — but they're authored
  to spec regardless).
- Recovery-procedure module path emitting `contentionWindowChanged`; 11n 40 MHz channel setup;
  `units` namespace for `MHz(...)`/`Mbps(...)`; app `startTime` after association settles.

## Verification — produce a conformance report, not all-green

1. **Build**: `lib/build.sh` (unmodified) → `libprotocoltest.so`; then `wifi/build.sh` → `wifitests`.
2. **Author** each test against its `Wifi<Gen>Trace` config (`logEvents=true`), writing assertions to
   the **standard**; capture the real INET behavior only to label the *anticipated* column, never to
   weaken the assertion.
3. **Run all** `[Config Wifi*]` via `./wifitests`; tabulate **actual** vs **anticipated** outcome into
   the `README.md` matrix (CONFORMS / DEVIATES / NOT-MODELED). The deliverable is that matrix; FAIL/
   ERROR rows are findings, not defects in the suite.
4. **Sanity**: confirm the intentional-PASS anchors pass and that the existing `lib/` suite is
   untouched (re-run it once). Optionally a `.test` under `wifi/` pins one anchor end-to-end.
