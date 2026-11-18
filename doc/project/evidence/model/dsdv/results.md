# DSDV fingerprint results

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks](../../protocol/dsdv/checks.md)

**13 PASS, 4 FAIL.** All 17 simulations complete. Results use debug mode,
Cmdenv, run/seed 0 and OMNeT++ 6.4.0aipre2. Visual tests use Fake-GUI refresh
probabilities of 0.001 and three interpolation steps, with the zero-step loop guard.

## [examples/dsdv/omnetpp.ini](../../../../../examples/dsdv/omnetpp.ini)

| Configuration | Limit | Ingredients | Expected | Calculated | Result |
| --- | --- | --- | --- | --- | --- |
| Mobile | 32s | `tplx` | `2bf5-fcd0` | `2bf5-fcd0` | PASS |

## [examples/manetrouting/dsdv/omnetpp.ini](../../../../../examples/manetrouting/dsdv/omnetpp.ini)

| Configuration | Limit | Ingredients | Expected | Calculated | Result |
| --- | --- | --- | --- | --- | --- |
| IPv4 | 10s | `tplx` | `fe44-d006` | `fe44-d006` | PASS |
| IPv4 | 10s | `~tNl` | `052c-d3e9` | `052c-d3e9` | PASS |
| IPv4 | 10s | `~tND` | `6975-ddb0` | `6975-ddb0` | PASS |
| IPv4 | 10s | `tyf` | `4aa9-4bbb` | `5ece-ee9f` | FAIL |
| MultiIPv4 | 10s | `tplx` | `9d2b-c13d` | `9d2b-c13d` | PASS |
| MultiIPv4 | 10s | `~tNl` | `be75-f184` | `be75-f184` | PASS |
| MultiIPv4 | 10s | `~tND` | `a9c7-4579` | `a9c7-4579` | PASS |
| MultiIPv4 | 10s | `tyf` | `c4d3-2514` | `44f0-3d2e` | FAIL |
| DynamicIPv4 | 22s | `tplx` | `ea2a-cd36` | `ea2a-cd36` | PASS |
| DynamicIPv4 | 22s | `~tNl` | `ec39-cd04` | `ec39-cd04` | PASS |
| DynamicIPv4 | 22s | `~tND` | `9cfd-b9e8` | `9cfd-b9e8` | PASS |
| DynamicIPv4 | 22s | `tyf` | `70dc-efb6` | `0fe7-47cd` | FAIL |

## [showcases/routing/manet/omnetpp.ini](../../../../../showcases/routing/manet/omnetpp.ini)

| Configuration | Limit | Ingredients | Expected | Calculated | Result |
| --- | --- | --- | --- | --- | --- |
| Dsdv | 100s | `tplx` | `e7a2-1b3d` | `e7a2-1b3d` | PASS |
| Dsdv | 100s | `~tNl` | `7c88-78ac` | `7c88-78ac` | PASS |
| Dsdv | 100s | `~tND` | `3b16-c25e` | `3b16-c25e` | PASS |
| Dsdv | 100s | `tyf` | `91a6-dbc7` | `2a2f-525e` | FAIL |

## Causes of the four visual failures

`tyf` hashes simulation times (`t`), module/channel display strings (`y`) and
canvas figures (`f`). All four failures are checksum mismatches after successful
simulation completion. The same four references pass on unchanged upstream master
`91219b0fa7` with the same runner and environment.

- **IPv4 and MultiIPv4:** [Dsdv.ned](../../../../../src/inet/routing/dsdv/Dsdv.ned)
  changes the module icon from `block/routing` to `block/network2`. This changes a
  display string included in every visual hash. UDP full/incremental advertisements
  also change event timing and packet activity compared with the reference model.
- **DynamicIPv4:** the same icon and control-traffic changes apply. Route expiry
  and deferred advertisement deadlines additionally change the timing of routing
  updates while nodes move.
- **MANET Dsdv:** the same icon change applies. The
  [showcase configuration](../../../../../showcases/routing/manet/omnetpp.ini)
  renders control packets named `DSDV`, uses 250ms full dumps with 50ms jitter,
  and displays routing tables. These traffic and routing-state changes alter the
  sampled link and routing figures; deferred advertisements also use the 10ms floor.

The visual reference values remain unchanged. The passing nonvisual comparisons
check their selected event/packet streams; they do not establish equality of the
rendered output.
