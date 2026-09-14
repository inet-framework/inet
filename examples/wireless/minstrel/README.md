# Minstrel HT and VHT

Select `MinstrelHtRateControl` in the MAC's `dcf.rateControl` or
`hcf.rateControl` slot and leave `rateSelection.dataFrameBitrate = -1bps`.
The supplied configurations send UDP traffic after infrastructure association:

```sh
inet --debug -u Cmdenv -f omnetpp.ini -c Ht -r 0
inet --debug -u Cmdenv -f omnetpp.ini -c Vht -r 0
```

HT uses the existing negotiated receive MCS mask, channel widths, BSS operating
width and guard interval support. VHT uses the existing `ac` PHY mode catalog,
the local antenna count and the configured common peer limits
`maxChannelWidth` and `maxNumSpatialStreams`. This checkout does not negotiate
VHT receive capabilities; configure these limits for every peer using the
controller. Only combinations present in the mode catalog can be selected.
The examples pair dimensional radios and a dimensional medium so overlapping
20/40/80 MHz transmissions are represented during width adaptation.

The controller keeps success estimates separately for each receiver and full PHY
mode. Its `datarateChanged`/`dataratePerStation` statistics report the preferred
throughput rate. The MAC's `datarateSelected` reports actual transmissions,
including probes and retries. Packet-owned retry plans make repeated duration
queries stable and isolate simultaneous QoS queues and fragments.

## Algorithm reference

The implementation uses **Linux v4.19 Minstrel-HT**, an explicitly pinned version
of the HT/VHT algorithm. The [Linux Wireless Minstrel overview](https://wireless.docs.kernel.org/en/latest/en/developers/documentation/mac80211/ratecontrol/minstrel.html)
explains measured-throughput selection and probing; the HT implementation is
the precise reference for the rules below. Minstrel is a rate adaptation policy,
not a normative IEEE 802.11 algorithm.

| Reference rule | INET implementation | Direct test |
| --- | --- | --- |
| [`minstrel_calc_rate_stats`, `minstrel_ewma`](https://github.com/torvalds/linux/blob/v4.19/net/mac80211/rc80211_minstrel.c): first sample initializes Q12 probability; later samples use 75% historical weight; empty intervals retain it | `updateStatistics` | `MinstrelHtRateControlReference_1`: exact values 3072, 2688, 2016, 2536; negative-delta truncation and idle interval |
| [`minstrel_ht_get_tp_avg`](https://github.com/torvalds/linux/blob/v4.19/net/mac80211/rc80211_minstrel_ht.c): zero throughput below 10%, cap success probability at 90%, rank expected delivery per airtime | `computeThroughput`, `rankRates` | `Reference_1`: Q12 threshold boundaries, throughput beating nominal bitrate |
| `minstrel_ht_set_best_prob_rate`, `minstrel_ht_prob_rate_reduce_streams`: probability fallback, throughput preference above 75%, prefer usable lower-NSS groups | `rankRates` | `Reference_1`: probability/throughput choices; multi-group catalog coverage |
| `minstrel_get_sample_rate`: group sampling, skip primary/probability rates and probabilities above 95%; restrict slow samples until 20 skipped intervals, at most three per update | `selectSample` | `Reference_1`: shuffled permutations, 95% boundary, wait, aging and slow-sample budget |
| `minstrel_ht_update_rates`, [`rate_control_fill_sta_table`](https://github.com/torvalds/linux/blob/v4.19/net/mac80211/rate.c): primary, second throughput, probability fallback; a one-attempt probe replaces the primary slot | `getRateForFrame` and `MinstrelRateControlTag` | `Reference_1`: repeated queries, stage progression and independent packets; HT/VHT exchange tests observe on-air fallback |
| `minstrel_calc_retransmit`, `minstrel_ht_set_rate`: bounded airtime retry stages; two attempts below 20% probability | `computeRetryCount` | `Reference_1`: 20% boundary and independently calculated contention/airtime totals |
| `minstrel_ht_tx_status`: attribute attempts and successes to the transmitted rates; downgrade failing throughput groups after more than 30 attempts with under 20% success | `frameTransmitted`, `downgradeRate` | `Reference_1`: actual-mode attribution, completion and timeout feedback; HT/VHT exchange tests use real ACK/timeout paths |
| Supported-rate filtering before selection | PHY catalog, local antenna limit, existing HT peer selection | `MinstrelHtRateControlEligibility_1`: sparse MCS, width/GI, singleton peer and changed capabilities; `HtExchange_1`: actual association |

The PHY catalog and HT peer-selection machinery are shared with `../inet` at
the implementation starting point; their source trees matched and required no
port or duplicate duration tables.

## Model differences and limits

- This implementation handles **individual normal-ACK data attempts**. The
  existing HCF does not report aggregate Block Ack transmission status to rate
  control. Unicast Block Ack/no-ACK data is rejected explicitly. Aggregation
  length is one; Minstrel does not start aggregation sessions or set A-MSDU sizes.
- Linux's integer table of amortized 1200-byte payload durations is replaced by
  INET mode durations for the configurable reference MPDU length, including the
  actual mode preamble and an ACK estimate at the fastest mandatory legacy rate.
  Throughput scores use floating-point packets/second; probability updates and
  thresholds retain Linux's Q12 arithmetic. ACK rate overrides are not modeled
  in this estimate. Throughput numbers are therefore not bit-for-bit Linux output.
- Sampling uses ten simulator-seeded permutations **of supported rates in each
  group**, instead of Linux's shared table with unsupported entries skipped.
  The group order follows width, GI and NSS; ties use deterministic catalog order.
  Initial sampling has four opportunities; subsequent opportunities are separated
  by 18 new packets, the normal-ACK value of `16 + 2 * avg_ampdu_len`.
- INET reports each software attempt rather than a completed hardware retry
  series. It retains each packet's plan during retries. MAC recovery owns the
  total retry limit: it can truncate the plan, and the probability fallback mode
  is retained until MAC recovery terminates the packet. RTS failures and internal
  collisions are not data-rate attempts. Fallback stages do not force RTS/CTS;
  protection remains under the existing MAC policy. SMPS and CCK fallback are
  outside this model.
- HT/VHT rates come from the existing mode catalog, including any available
  160 MHz or additional-NSS entries when configuration and antenna limits allow
  them. This generalizes the reference version's smaller group catalog, without
  synthesizing absent combinations. The exchange tests cover HT 20 MHz and VHT
  up to 80 MHz; they do not establish all catalog combinations or hardware parity.
  The current packet-level HT PHY advertises only 20 MHz. HT 40 MHz group
  eligibility is tested with an explicit MIB fixture, not an on-air HT40 claim.

## Validation commands

Run from the repository root, with the current debug library:

```sh
make MODE=debug -j4
inet_run_module_tests -m debug -f 'MinstrelHtRateControl.*\.test'
inet_run_module_tests -m debug -f '(AarfRateControlRetryFeedback|OnoeRateControlRetryFeedback|OnoeRateControlInterleavedFeedback|Ieee80211HtAntennaRateControl)_1\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mac
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mac/ratecontrol
```

The existing rate-control fingerprints can be checked from `tests/fingerprint`:

```sh
./fingerprinttest -d -m 'wireless/ratecontrol' -f 'tplx' -f '~tNl' -f '~tND'
```

The reference and eligibility tests establish computations and API behavior.
The exchange tests establish production-path integration, not statistical
throughput superiority. They pin seed 0, force first data attempts to fail until
200 ms, then clear the link. They assert successful delivery, probes, fallback
transmissions and upward adaptation through both DCF and HCF. Their output
includes the observed counts for inspection.

Validation on 2026-09-10 with OMNeT++ 6.4.0aipre2 and a fresh Clang debug build:
all four Minstrel module tests, all four listed existing module regressions,
all three selected fingerprints, and the scoped architecture/naming checks passed.
Existing fingerprint baselines were unchanged. The exchange tests recorded:

| Mode / MAC | Successful data attempts | Failed data attempts | Probe attempts | Fallback attempts |
| --- | ---: | ---: | ---: | ---: |
| HT / DCF | 434 | 26 | 25 | 7 |
| HT / HCF | 433 | 25 | 25 | 6 |
| VHT / DCF | 434 | 41 | 26 | 18 |
| VHT / HCF | 433 | 39 | 26 | 20 |

Both supplied 3-second, seed-0 examples delivered 2900 UDP packets to the sink.
The preferred-rate vector started at 6.5 Mbit/s and ended at 130 Mbit/s for HT,
and at 173.33 Mbit/s for VHT. These are preferred PHY rates, not application
throughput measurements. Scalars and vectors were exported using `opp_scavetool`;
one run per configuration establishes functional behavior without a statistical
performance comparison.

## Rate-control extension API

`IRateControl::getRateForFrame(Packet *)` is the packet-aware query used by both
selectors. `RateControlBase` forwards it to the existing address query, preserving
AARF and Onoe behavior. An external implementation deriving directly from
`IRateControl` must implement this query or derive from `RateControlBase`.
Overrides of the selectors' protected `computeDataOrMgmtFrameMode` method must
accept the new leading `Packet *` argument. No changes to packet wire content or
PHY mode definitions are required.
