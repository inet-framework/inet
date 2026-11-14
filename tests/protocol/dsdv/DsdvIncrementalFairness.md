# Fair sequence-only incremental updates

`DsdvIncrementalFairness.test` checks the section 4 fairness behavior of Perkins/Bhagwat 1994 through real UDP delivery and wireless reception. The router runs the production DSDV module. A fixture application supplies controlled updates and a separate silent neighbor decodes advertisements received over the radio.

```mermaid
flowchart LR
    I[UDP injector] <-->|100 m| D[DSDV router]
    D -->|100 m| R[Silent UDP receiver]
```

The 48-byte payload budget fits two version-1 entries. After the first periodic self-sequence advance, four destinations establish an overflow full-dump baseline. Three receive sequence-only changes. A fourth changes metric, and stale withdrawals for it create further corrective send opportunities. Its required entry consumes the first slot; the optional entries must occupy the remaining slot in the order 4, 5, 6. The parent repeatedly selects destination 4, so the third-send assertion exposes the gap without calling table helpers.

A newly reachable fifth destination consumes the spare slot. The receiver checks that this packet contains only the two required entries. A further new destination forces a fragmented full dump. All four fragments must arrive, after which new sequence-only changes and another metric change must select destination 4 first again, proving reset of the rotation cursor. Every observed datagram must fit the payload budget and contain no duplicate destination.

The fixture uses General/run 0/seed 0, ordinary local ARP, fixed radio positions and no supplied connected or multihop routes. Injector and receiver are out of each other's range. Periodic timing is observed rather than assumed to start at phase zero. Neighbor and destination timeouts exceed the whole scenario.

Run from the repository root after a fresh debug build:

```sh
opp_run_protocol_tests --load . -p inet -m debug -w '(^|/)dsdv$' -f 'DsdvIncrementalFairness\.test$'
```

The assertions establish fairness for a stable three-entry optional set and the tested send opportunities. They do not require extra broadcasts to drain optional changes, fairness without spare capacity, or delivery under arbitrary loss. Execution status belongs in the DSDV results record; this explanation alone does not claim a passing run.
