# BgpWithdrawal

This example demonstrates **explicit BGP route withdrawal propagation** in a linear,
non-redundant pure-BGP chain:

`hostA - A - B - C - hostC`

Routers `A`, `B`, and `C` are in different ASes (eBGP between `A-B` and `B-C`). Router
`A` originates `10.0.1.0/24` and router `C` originates `10.0.3.0/24`. The hosts use
default routes toward their adjacent routers.

Unlike `BgpDiamondFailover`, there is **no alternate path**. This is what makes the
example interesting: it exercises the explicit withdrawal-send path that the diamond
topology can never reach (there, best-path reselection always finds a surviving
alternative and re-advertises instead of withdrawing).

The scenario timeline is:

- At `20s`, `hostC` starts pinging `hostA` at `10.0.1.100`.
- At `60s`, router `A` is shut down. Router `B` loses its only path to `10.0.1.0/24`,
  finds no alternative in its Adj-RIB-In, and sends an **explicit BGP UPDATE with a
  Withdrawn Routes field** for `10.0.1.0/24` to `C`. The `B-C` session itself never
  fails, so this signaling is the only way `C` learns the prefix is gone. `C` then
  removes the route instead of black-holing traffic toward the dead router.

What to look for in the logs (run with `--cmdenv-express-mode=false`):

```
B.bgp: BGP session ... to peer 10.0.12.1 lost: ... re-running best-path selection
B.bgp: Prefix 10.0.1.0/24: no alternative path in Adj-RIB-In, sending explicit BGP withdrawal
B.bgp: Sending BGP Withdraw message to 10.0.23.3 ...
C.bgp: delete route BGP 10.0.1.0/24 ...
```

The `Restart` config (`-c Restart`) extends the scenario: it also restarts `A` at `100s`,
so the session to `B` re-establishes (exercising the RFC 4271 §6.8 connection-collision
handling on reconnect), `10.0.1.0/24` is re-advertised `A-B-C`, and the ping recovers.

Notes:

- This example intentionally uses shortened BGP timers (`connectRetryTime = 5s`,
  `holdTime = 6s`, `keepAliveTime = 2s`, `startDelay = 2s`) so the chain converges
  quickly in a short simulation.
- The IPv6 / MP-BGP counterpart is `BgpWithdrawal6`, where the withdrawal is carried in
  an `MP_UNREACH_NLRI` path attribute (RFC 4760).
