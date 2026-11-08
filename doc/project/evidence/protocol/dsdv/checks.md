# DSDV English checks and hand-worked oracles

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features](features.md)

These procedures precede implementation. Metrics below include the receiving hop.

* **choice:** Install destination D via B at (10,2). Receive (8,1): ignore.
  Receive via C (12,4): select C. Receive via B (12,2): select B. Receive
  via C (12,2): retain B. Receive via B (12,3): ignore without refresh.
* **arithmetic:** Reject finite wire metrics above signed-int maximum, and
  reject adding a hop at that maximum. Accept maximum minus one. Reject
  odd finite and even infinite records. Stop before unsigned sequence exhaustion.
* **expiry:** Refresh neighbor B while omitting D; D must still expire at its
  own deadline. Expire B and invalidate every selected route through B once.
  A fully delivered refresh exactly at expiry wins over the expiry callback.
* **correction:** With finite (12,2), receive infinity (11). Submit (12,2)
  immediately even when it equals the full-dump baseline or was settling.
* **recovery:** Invalidate (12,2) to (13,infinity). Ignore delayed finite 12.
  Accept finite 14. On self withdrawal 15, originate self (16,0).
* **incremental:** Compare to the last full dump, not the last incremental.
  Required metric changes precede optional sequence changes. A corrective
  obligation survives baseline equality. Duplicates appear only once.
* **payload:** With capacity K, K required entries fit one incremental;
  K+1 require a split full dump. Optional sequence-only changes never force
  that escalation. Count all headers and IPv4/UDP overhead against MTU.
* **settling:** With cap 1 and initial estimate .5, accept a newer route
  at t=.1, then improve at .2. Its original deadline 1.1 cannot move later.
  A full dump at .3 must carry the preceding advertised route. On closing
  that sample use .75*(.2-.1)+.25*.5=.2. Invalidation discards a sample.
* **deadlines:** New reachability and withdrawals are immediate. Stable
  metric/successor changes after the cap are immediate. Other changes wait
  at most one cap while the interface remains usable. Sequence-only updates
  create no trigger and appear in a subsequent periodic dump.
* **periodic:** Advance the self sequence by two per update, not per fragment.
  Lose simultaneous startup dumps; later randomized periodic dumps converge.
* **encoding:** Check independent network-order bytes for a fixed header and
  entry. Reject truncated, extra, duplicate, invalid-version, invalid-address,
  invalid-kind and invalid-parity datagrams atomically.
* **lifecycle:** Stop, crash and interface down cancel all timers and withdraw
  only owned routes. Interface down is silent; interface up and restart retain
  floors, advance self by two and repeat all withdrawals in the startup dump.
* **forwarding:** Use a disconnected node, two neighbors and a four-node line
  with static addresses, local ARP, forwarding and no supplied multihop routes.
  Require owned host routes and intermediate traversal. Under identical
  conditions with the routing protocol disabled, endpoint delivery must fail.
* **graph:** After each controlled receive/expiry on line, diamond and cycle
  fixtures, follow every usable successor and reject a repeated node. Under
  loss-free static operation compare distances to independent shortest paths.

Paper §4 Tables 1–2: MH4 forwards to MH1 via MH2 with metric 2 and advertises
metric 2; self MH4 has metric 0. Tables 3–4: after MH1 moves, MH4 selects MH6
with metric 3 and newer MH1 sequence 516 (previously 406). MH1's metric change
is required in the incremental; other unchanged-metric sequence updates fill
remaining space. This profile puts sender identity in every datagram header.
