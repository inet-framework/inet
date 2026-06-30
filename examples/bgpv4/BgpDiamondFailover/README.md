# BgpDiamondFailover

This example demonstrates IPv4 BGP failover, failback, and a second failure in a pure-BGP diamond topology:

`hostA - A - B - C - hostC`

`hostA - A - D - C - hostC`

Routers `A`, `B`, `C`, and `D` are in different ASes. Router `A` advertises the `10.0.1.0/24` network, router `C` advertises the `10.0.3.0/24` network, and the hosts use default routes toward their adjacent routers. The preferred BGP path initially goes through `A-B-C`.

The `A-B-C` branch uses `50us` links and the alternate `A-D-C` branch uses `300us` links, so the active path change is visible in the ping RTT results.

The scenario timeline is:

- At `20s`, `hostA` starts pinging `hostC` at `10.0.3.100`.
- At `60s`, router `B` is shut down. After the shortened BGP hold timer expires, traffic switches to the alternate `A-D-C` path using the routes already stored in Adj-RIB-In.
- At `100s`, router `B` is started again. Once the BGP sessions re-establish and exchange routes again, the preferred route returns to `A-B-C`.
- At `140s`, router `B` crashes. This exercises failover again after the network has already failed back to the preferred branch.

Notes:

- This example intentionally uses modified BGP timers to make the failover and failback visible in a short simulation:
  - `connectRetryTime = 5s`
  - `holdTime = 6s`
  - `keepAliveTime = 2s`
  - `startDelay = 2s`
- The links on the alternate `A-D-C` branch use higher delay than the preferred `A-B-C` branch so failover and failback are easy to observe.
- The timers are shorter than typical BGP defaults and are meant only for demonstration.
