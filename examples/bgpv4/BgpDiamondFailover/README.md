# BgpDiamondFailover

This example demonstrates IPv4 BGP failover and failback in a pure-BGP diamond topology:

`hostA - A - B - C - hostC`

`hostA - A - D - C - hostC`

Routers `A`, `B`, `C`, and `D` are in different ASes. The preferred path initially goes through `A-B-C`.
The `A-B-C` branch also uses lower link delays than `A-D-C`, so the active path change is visible in the ping RTT results.

At `60s`, router `B` is shut down. After the shortened BGP hold timer expires, traffic switches to the alternate `A-D-C` path using the routes already stored in Adj-RIB-In.

At `100s`, router `B` is started again. Once the BGP sessions re-establish and exchange routes again, the preferred route returns to `A-B-C`.

Notes:

- This example intentionally uses modified BGP timers to make the failover and failback visible in a short simulation:
  - `connectRetryTime = 5s`
  - `holdTime = 6s`
  - `keepAliveTime = 2s`
  - `startDelay = 2s`
- The links on the alternate `A-D-C` branch use higher delay than the preferred `A-B-C` branch so failover and failback are easy to observe.
- The timers are shorter than typical BGP defaults and are meant only for demonstration.
