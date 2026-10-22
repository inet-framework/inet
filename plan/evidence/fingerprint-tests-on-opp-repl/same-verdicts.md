# Step I2: the old and the new fingerprint tool give the same verdicts

The check runs both tools on one build and compares their verdicts for each run. The script is [same-verdicts.sh](same-verdicts.sh). It runs in the container of the GitHub job (ubuntu 24.04, clang 16) with `MODE=release`, from the INET environment.

| Item | Value |
| --- | --- |
| INET | `e6c8520635`, the converted store |
| OMNeT++ | `omnetpp-6.x` at `c95eb4ce15`, `WITH_OSG=yes WITH_OSGEARTH=no`, all INET features |
| opp_repl | `opp_ci` at `e469285` |
| Old tool | `./fingerprinttest -f 'tplx' -f '~tNl' -f '~tND'`, the arguments of the GitHub workflow |
| New tool | `opp_run_fingerprint_tests --load @opp -p inet -m release --no-build` |

Results (2026-10-22, release mode):

| Tool | Tests | Verdicts | Wall time |
| --- | --- | --- | --- |
| Old | 1752 CSV rows with a CI ingredient | 1751 PASS, 1 expected ERROR | 2 min 9 s |
| New | 1750 runs, 4381 entries | 4378 PASS, 3 expected ERROR | 1 min 56 s |

The old tool counts two duplicate CSV rows that the store holds once (`tutorials/bgp Step1` and `showcases/tsn/cutthroughswitching`). In both tools the only ERROR is the expected one: `tests/fingerprint -f ethernet-nonstandardspeed.ini -r '$datarate==5Gbps && $duplex==false'`, which stops with "5e+09 bps Ethernet only supports full-duplex links". Its three entries are the 3 expected ERRORs of the new tool.

What the check does not show: the values of the new run-0 entries (step I3b checks them in two build modes), and the result on the GitHub runner (step I8).
