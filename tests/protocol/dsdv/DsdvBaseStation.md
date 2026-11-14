# Base-station default reachability

`DsdvBaseStation.test` exercises the optional version-2 profile with real UDP broadcasts, ordinary local ARP and bidirectional Ping traffic across a wired uplink. The base station has one ad hoc interface and one Ethernet interface. The external network's return route is explicitly configured; the fixture adds and removes the base station's wired default through the routing-table API. No multihop host routes are supplied.

```mermaid
flowchart LR
    P[Controlled replay sender] --> M[Mobile]
    M <-->|Wireless| R[Relay]
    R <-->|Wireless| B[Base station]
    B <-->|Ethernet| W[Wired router]
    W <--> E[External host]
```

Wireless neighbors are 100 m apart with a 110 m range. All wireless addresses share one `/24`, whose connected route remains installed. DSDV `/32` host routes override that prefix, while an unknown internal destination selects the connected prefix rather than the external default. Mobile and relay defaults must be DSDV-owned, with metrics two and one and the correct wireless next hops. The base station keeps its foreign wired default. The observer also requires external-destination ICMP traffic at both the wireless relay's and wired router's IPv4 modules, connecting application replies to the intended intermediate forwarding path.

The base station starts without a wired default. After three periodic dumps, the mobile must have a host route to the base station but no default and no external Ping replies. Adding the wired default enables external access in the same physical topology. Once replies arrive, the fixture deletes that route again. Within 50 ms the mobile must receive an incremental capability-false advertisement through the relay, remove its default, and retain its finite host route to the base station. A separate sender then replays an older capability-true route. It must not reinstate the default. A version-1 packet and a wrong-origin capability packet must both be rejected by the production receive path. Restoring the wired default requires a newer origin sequence and restores the mobile default.

Further actions replace the mobile default with a foreign manual route, remove that route to restore DSDV's projection, take the base station's wired interface down/up, and move the relay out of radio range and back. The fixture mobility module changes position and emits the ordinary mobility notification, so the radio medium and MAC handle the resulting delivery failure normally. This physical disconnection exercises real MAC failure feedback, and application replies must resume after wireless recovery.

Finally, the fixture stops and restarts the base station's DSDV controller through lifecycle operations. Its owned routes disappear while its wired default survives, then a newer origin sequence and the mobile default reappear. At least one Ping reply must arrive more than 100 ms after controller restart. This tests the controller lifecycle with the rest of the node running; it does not claim a complete Ethernet-node reboot test.

Run General/run 0/seed 0 from the repository root after a fresh debug build:

```sh
opp_run_protocol_tests --load . -p inet -m debug -w '(^|/)dsdv$' -f 'DsdvBaseStation\.test$'
```

The parent lacks version-2 configuration/fields and rejects the extra active interface, so it cannot execute this gateway scenario. That is source-grounded missing capability, not a claimed observed forwarding regression. Version-1 host-only controls establish compatibility separately. Exact wire bytes, payload boundaries, mixed-profile domain convergence and route-batch reentrancy require their associated unit/module cases. The topology supports one configured base station and explicit return routing, not automatic gateway election or remote Internet-health detection. Execution status belongs in the DSDV results record.
