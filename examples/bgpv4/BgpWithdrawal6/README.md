# BgpWithdrawal6

IPv6 / MP-BGP counterpart of `BgpWithdrawal`. It demonstrates **explicit BGP route
withdrawal propagation** carried in an `MP_UNREACH_NLRI` path attribute (RFC 4760) over
a linear, non-redundant chain:

`hostA - A - B - C - hostC`

Routers `A`, `B`, and `C` are in different ASes (eBGP between `A-B` and `B-C`), running
BGP over IPv6 (`addressFamily = "ipv6"`). Router `A` originates `2001:db8:1::/64` and
router `C` originates `2001:db8:3::/64`.

There is **no alternate path**, so this example reaches the IPv6 withdrawal-send branch
that `BgpDiamondFailover6` cannot (there a fallback path always survives).

The scenario timeline is:

- At `20s`, `hostC` starts pinging `hostA` at `2001:db8:1::100`.
- At `60s`, router `A` is shut down. Router `B` loses its only path to `2001:db8:1::/64`,
  finds no alternative in its Adj-RIB-In, and sends an explicit UPDATE carrying an
  **`MP_UNREACH_NLRI` (afi=2, safi=1)** withdrawal for `2001:db8:1::/64` to `C` over the
  still-up `B-C` session. `C` then removes the route.

What to look for in the logs (run with `--cmdenv-express-mode=false`):

```
B.bgp: Prefix 2001:db8:1::/64: no alternative path in Adj-RIB-In, sending explicit BGP withdrawal
B.bgp: Sending BGP Withdraw message to 2001:db8:23::3 ...   MP_UNREACH_NLRI: afi=2 safi=1 (1 prefixes)
```

Notes:

- This example intentionally uses shortened BGP timers (`connectRetryTime = 5s`,
  `holdTime = 6s`, `keepAliveTime = 2s`, `startDelay = 2s`) so the chain converges
  quickly in a short simulation.
- The IPv4 counterpart is `BgpWithdrawal`, where the withdrawal is carried in the legacy
  Withdrawn Routes field of the UPDATE message.
