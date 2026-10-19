# Approved TXOP admission fingerprints

Twenty-one CSV rows change. Ten source changes belong to C4, and five belong to C5. Twenty rows also need inherited graphical updates. The first commit owns those graphical updates. C4 and C5 contain their additional changes.

The user authorized fingerprint updates on 2026-10-19. The source and test definitions remain identical to the original published series. This task changes expectations and evidence only.

## Cause and commit owner

The unchanged dependency is `d841dc77db7d0b141bf31e5e215a0de9bb281957`. C3 is `d98c98fcfb`, C4 is `c419a07251`, and the original final source is `67bacef61c`. These identifiers name the source revisions used for the comparisons. The baseline amendments preserve those source trees.

The dependency and C3 produce identical values in all twenty-five selected rows. Their event, communication, and packet-data expectations pass. Twenty graphical expectations differ from the stored values. The earlier cause of that graphical drift is unknown. The separate first commit records those measured dependency values.

C4 changes ten rows from C3. C5 changes five rows from C4. The table names every final CSV update. An inherited entry means that the first commit changes only `tyf`. A later source commit can change that same row again.

| Directory | Configuration | Commit owners | Reason |
| --- | --- | --- | --- |
| `showcases/wireless/aggregation/` | `NoAggregation; run 0` | inherited | Only inherited graphical drift changes. |
| `showcases/wireless/aggregation/` | `Aggregation; run 0` | inherited, C4 | C4 excludes A-MSDUs without HT capabilities. |
| `showcases/wireless/aggregation/` | `VoicePriorityAggregation; run 0` | inherited, C4 | C4 excludes A-MSDUs without HT capabilities. |
| `showcases/wireless/blockack/` | `Fragmentation; run 0` | C5 | C5 retains the Basic fragment group within a zero-limit TXOP. |
| `showcases/wireless/fragmentation/` | `DCFfrag; run 0` | inherited | Only inherited graphical drift changes. |
| `showcases/wireless/fragmentation/` | `HCFfrag; run 0` | inherited, C4 | C4 defers the next MSDU when its exchange cannot fit. |
| `showcases/wireless/fragmentation/` | `HCFfragblockack; run 0` | inherited, C5 | C5 applies prepared Basic admission and correct fragment reservations before BAR access. |
| `showcases/wireless/qos/` | `NonQos; run 0` | inherited | Only inherited graphical drift changes. |
| `showcases/wireless/qos/` | `Qos; run 0` | inherited, C4 | C4 excludes A-MSDUs without HT capabilities. |
| `showcases/wireless/txop/` | `General; run 0` | inherited, C5 | C5 applies prepared Basic admission and excludes non-HT A-MSDUs. |
| `examples/adhoc/qos/` | `MacQos; run 0` | inherited, C4 | C4 applies prepared admission and excludes non-HT A-MSDUs. |
| `examples/adhoc/qos/` | `MacQos; run 1` | inherited, C5 | C5 applies prepared Basic admission, legal frame forms, and the 2016 recovery contract. |
| `examples/adhoc/qos/` | `Fragmentation` | inherited | Only inherited graphical drift changes. |
| `examples/adhoc/qos/` | `MsduAggregation` | inherited, C4 | C4 excludes A-MSDUs without HT capabilities. |
| `examples/wireless/mactest/` | `MacEdca; run 0` | inherited, C4 | C4 sends the next fragment after SIFS within a zero-limit TXOP. |
| `examples/wireless/mactest/` | `MacDcf; run 0` | inherited | Only inherited graphical drift changes. |
| `examples/wireless/qos/` | `MacNonQos; run 0` | inherited | Only inherited graphical drift changes. |
| `examples/wireless/qos/` | `MacQos; run 0` | inherited, C4 | C4 applies prepared admission and excludes non-HT A-MSDUs. |
| `examples/wireless/qos/` | `MacQosWithoutAggregation; run 0` | inherited, C4 | C4 derives Retry from local ACK state. |
| `examples/wireless/qos/` | `MacQosWithRtsCts; run 0` | inherited, C4 | C4 excludes non-HT A-MSDUs before the protected exchange. |
| `examples/wireless/qos/` | `MacQosWithBlockAck; run 0` | inherited, C5 | C5 applies prepared Basic admission and excludes non-HT A-MSDUs. |

The [value ledger](values.json) preserves every original, dependency, C3, C4, and final value. Thus, it also identifies graphical values that change twice. C4 never receives a C5 value.

## Concrete evidence

The aggregation traces show the first changed frame form. VoicePriorityAggregation first replaces a 516-byte A-MSDU with one 192-byte MPDU. The QoS examples show the same change without HT capabilities. The existing [three-case report](../execution-20261019/c5-fingerprint-causality.md) contains the original Aggregation trace.

In HCFfrag, the next MSDU starts at 0.00345597088 seconds before C4. It starts at 0.00379597088 seconds after C4. Complete admission defers that MSDU after the current fragment group.

In MacEdca, FTP-7-frag1 starts at 2.335881047819 seconds on the dependency. It starts at 2.335461047819 seconds after C4. The zero-limit sequence sends the next fragment after SIFS instead of new contention.

MacQosWithoutAggregation exposes a distinct packet-data change. Video-16 reaches the AP after an uplink retry. Before C4, the AP retains that Retry bit for its first downlink transmission. C4 derives Retry from this hop's ACK state and clears it. The paired captures first differ at capture timestamp 1.019021 seconds, sequence 16, TID 5. Their event and communication fingerprints remain unchanged.

The Basic traces separate C5 from C4. HCFfragblockack first moves its BAR from 0.004589367968 to 0.004773367968 seconds. Prepared Basic admission and fragment reservations change that access. The original Basic Fragmentation trace shows the next fragment after SIFS within a zero-limit TXOP. General and MacQosWithBlockAck replace non-HT A-MSDUs with ordinary MPDUs.

The [trace excerpts](trace-excerpts.json) retain the first changed transmissions. The [trace commands](trace-commands.json) record the configuration, run, time limit, and status. The capture commands explicitly compute FCS and checksums.

## Commands and results

All fingerprint comparisons use debug libraries with OMNeT++ 6.4.0aipre2. The canonical command runs from `tests/fingerprint`:

```sh
./fingerprinttest -d -t 8 -m '((showcases/wireless/(aggregation|blockack|fragmentation|qos|txop)|examples/(adhoc/qos|wireless/(qos|mactest)))/)'
```

The selector executes twenty-five rows. The CSV defines each simulation limit and run. MacQos in the ad hoc example includes runs zero and one. Configurations without an explicit run select run zero.

The corrected dependency command selects an explicit launcher for the dependency library. The first launcher attempt resolved the current library through `PATH`; this record excludes that attempt. C3 and C4 each receive a debug build before their comparisons. The final source receives a debug rebuild after those comparisons. The [command record](commands.json) includes library hashes and exit statuses.

| Check | Result |
| --- | --- |
| Dependency versus original expectations | Twenty graphical differences; all other selected ingredients pass. |
| Dependency versus approved graphical expectations | All twenty-five rows pass; status zero. |
| C3 versus dependency | All twenty-five rows match. |
| C4 versus C3 | Ten rows change. |
| Final source versus C4 | Five rows change. |
| C4 versus its approved expectations | All ten rows pass; status zero. |
| Final source versus approved CSV files | All twenty-five rows pass; status zero. |

The [dependency output](base.txt), [C3 output](c3.txt), [C4 output](c4.txt), and [original final output](head-before-update.txt) preserve the comparisons before updates. These comparison runs return status one because they deliberately use earlier expectations. The [dependency check](base-approved.txt), [C4 check](c4-approved.txt), and [final check](final.txt) pass against their approved expectations.

The fifteen changed non-graphical rows match the release CI failures on the original PR head. Graphical results describe this pinned local environment. They do not establish graphical stability across toolchains. The [publication record](../../../pending/ieee80211-txop-admission-publication.md) retains the broader source tests and global gate limits.
