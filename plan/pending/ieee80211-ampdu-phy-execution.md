# Partial HT aggregate reception

The common PHY receiver owns header and DATA error decisions. It computes the error probability of each region over that region's time interval and caches failed regions in the reception decision. The IEEE 802.11 receiver validates HT-SIG and reports whether the header decoded, whether it signalled aggregation, and whether the declared PSDU is present.

The default packet corruption mode gives aggregate DATA one outcome. Chunk mode preserves failures on the affected chunks of the received PSDU. For example, a failed payload in the middle MPDU can coexist with correct first and final MPDUs. A PSDU serialized as a single raw chunk has one DATA region in chunk mode. The MAC owns delimiter recovery, FCS checks, and receipt decisions.

## Verification

The PHY stack was tested on upstream master `60160028cb` with the part decisions from #1452 and the region decisions from #1443. The release build of the partial-reception stage passed the following checks.

| Fixture | Observation |
| --- | --- |
| `PartErrorDecisions_Ieee80211Radio_1` and `ContentErrors_Ieee80211Radio_1` | Header failure, DATA failure, and the configured form of corruption reach the receiver through the common decision path. |
| `RegionErrorRates_2` | The part-rate matrix exercises the region model, including NIST and Yans at HT rates. |
| `Ieee80211AmpduMacBoundary_1` and `WifiHtAmpduPartialReception` | The aggregate indication and local chunk errors reach Rx. The later standalone CTS sets NAV. |
| `WifiHtAmpduWire` and `WifiVhtHeaderIdentity` | HT-SIG survives wire conversion, and VHT transmission selects the VHT header. |
| `Ieee80211AmpduPhyCompatibility_1` | All 576 ordinary reception cases complete. |

An additional comparison ran the same ordinary-reception fixture on the integrated #1443 parent and the complete A-MPDU stack. All 576 rows matched byte for byte, including duration, indications, RNG draw counts and the next random value. The SHA-256 of each sequence of control rows was `384d2fb75fcf399c002b01ff587bad9c3b351d279d3e3f09a3679581bc71fb05`.
