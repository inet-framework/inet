# IEEE 802.11 mesh — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `WIFI-F-*` · **Stands on:** [Standards map](standards.md), [Mesh catalog](../../standard/ieee80211/catalog.md)

These features describe the selected mesh mechanisms independently of a simulation implementation. Every listed check is core to its feature. Optionality describes the standard's feature selection: once a profile selects a feature, its conditional requirements apply. Execution and implementation support belong in the [model ledger](../../model/wifi/hwmp-coverage.md).

| Feature | Level within the selection | Catalog statements | Core checks |
| --- | --- | --- | --- |
| WIFI-F-MESH-WIRE | mandatory | MESH-WIRE | [wire](checks.md#wifi-check-wire) |
| WIFI-F-MESH-PEERING | mandatory | MESH-DEPENDENCIES, MESH-PROFILE, MESH-BASIC-SETS, MESH-PEERING, MESH-PEERING-FSM | [peering](checks.md#wifi-check-peering), [peering failure](checks.md#wifi-check-peering-failure) |
| WIFI-F-MESH-TIMING | mandatory for neighbor-offset synchronization | MESH-TSF | [beacon timing](checks.md#wifi-check-beacon-timing) |
| WIFI-F-MESH-FORWARDING | mandatory | MESH-FORWARD, MESH-FORWARDING-STATE, MESH-FORWARDING-LIFETIMES, MESH-GROUP, MESH-NO-PATH | [unicast](checks.md#wifi-check-forwarding), [group](checks.md#wifi-check-group-forwarding), [limits](checks.md#wifi-check-discovery-limits) |
| WIFI-F-MESH-AIRTIME | mandatory for the selected metric | MESH-AIRTIME, MESH-METRIC-REPORT | [airtime](checks.md#wifi-check-airtime) |
| WIFI-F-HWMP-DISCOVERY | mandatory | MESH-SEQUENCE, MESH-PATH-UPDATE, MESH-DISCOVERY, MESH-PREQ-CASES, MESH-PREP-CASES | [discovery](checks.md#wifi-check-discovery), [selection](checks.md#wifi-check-path-selection), [limits](checks.md#wifi-check-discovery-limits) |
| WIFI-F-HWMP-MAINTENANCE | mandatory | MESH-PERR, MESH-PATH-UPDATE | [path error](checks.md#wifi-check-path-error), [rediscovery](checks.md#wifi-check-rediscovery) |
| WIFI-F-HWMP-ROOT-PREQ | optional, selected | MESH-ROOTS, MESH-ROOT-PREQ, MESH-DISCOVERY | [proactive PREQ](checks.md#wifi-check-root-preq), [coexistence](checks.md#wifi-check-root-coexistence) |
| WIFI-F-HWMP-ROOT-RANN | optional, selected | MESH-ROOTS, MESH-RANN-PROPAGATION, MESH-DISCOVERY | [RANN](checks.md#wifi-check-rann), [coexistence](checks.md#wifi-check-root-coexistence) |
| WIFI-F-MESH-HT | optional, selected | MESH-BASIC-SETS, MESH-HT-VHT-CAPABILITIES | [HT negotiation](checks.md#wifi-check-ht-negotiation), [high throughput data](checks.md#wifi-check-high-throughput-data) |
| WIFI-F-MESH-VHT | optional, selected | MESH-BASIC-SETS, MESH-HT-VHT-CAPABILITIES, MESH-AMPDU | [VHT negotiation](checks.md#wifi-check-vht-negotiation), [high throughput data](checks.md#wifi-check-high-throughput-data) |
| WIFI-F-MESH-AMSDU | optional, selected | MESH-AMSDU, MESH-AMSDU-IDENTITY, MESH-AIRTIME | [A-MSDU](checks.md#wifi-check-amsdu), [aggregation limits](checks.md#wifi-check-aggregation-limits), [airtime](checks.md#wifi-check-airtime) |
| WIFI-F-MESH-AMPDU | optional, selected | MESH-AMPDU, MESH-AGGREGATION-LIMITS, MESH-BLOCK-ACK-RECOVERY, MESH-AIRTIME | [A-MPDU/Block Ack](checks.md#wifi-check-ampdu-block-ack), [aggregation limits](checks.md#wifi-check-aggregation-limits), [airtime](checks.md#wifi-check-airtime) |
| WIFI-F-MESH-POWER-MODES | optional, selected | MESH-POWER-MODES, MESH-TIM | [modes](checks.md#wifi-check-power-modes), [TIM/DTIM](checks.md#wifi-check-tim-dtim) |
| WIFI-F-MESH-POWER-DOZE | optional, selected | MESH-AWAKE-WINDOW, MESH-WAKE-DOZE | [awake windows](checks.md#wifi-check-awake-window), [light sleep](checks.md#wifi-check-light-sleep), [deep sleep](checks.md#wifi-check-deep-sleep) |
| WIFI-F-MESH-MPSP | optional, selected | MESH-MPSP, MESH-WAKE-DOZE, MESH-AMPDU | [service periods](checks.md#wifi-check-service-period), [recovery](checks.md#wifi-check-power-recovery), [HT/VHT sleep](checks.md#wifi-check-power-ht-vht) |

Catalog identifiers in the table have the common prefix `IEEE80211-2024-`; for example, MESH-PERR refers to [IEEE80211-2024-MESH-PERR](../../standard/ieee80211/catalog.md#ieee80211-2024-mesh-perr). The catalog is the authoritative source of each law and its source locator.
