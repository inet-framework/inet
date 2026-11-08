# DSDV results

> **Kind:** report · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [checks](../../protocol/dsdv/checks.md)

At specification freeze no test had run. The plan supplies the implementation contract; the source
owner is `src/inet/routing/dsdv/`, using public packet, UDP, lifecycle, interface
and route-table APIs. No sealed source changes are required.

The independent NED declaration was written first in `/tmp/dsdv-stage/Dsdv.ned`
because the installed module name collides with the excluded implementation.
Its installation and the router's UDP attachment belong to the adapter change.
Legacy `DsdvHello` wire classes remain isolated compatibility types and are not
used as an algorithm or encoding oracle.

## State foundation, 2026-11-08

From `/home/miguel/workspace/inet-dsdv`, after `source .envrc` and activating
`/tmp/dsdv-python/bin/activate`:

```
make -j8 MODE=debug
make -j4 MODE=debug
inet_run_unit_tests -p inet -m debug --no-build \
  -f 'Dsdv(Foundation|Dissemination|Graph)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-unit-runner.log
```

Fresh debug build: exit 0. Unit cases: **3 PASS**, no expected failures,
seed 0 (the unit algorithms draw no random numbers). Raw output:
`/tmp/dsdv-unit.log`; generated artifacts: `tests/unit/work/Dsdv*/`.
Selection, signed metric and checked sequence boundaries, byte-exact encoding,
malformed input, advertised-view settling, corrective obligations, payload
overflow, separate freshness clocks, retained floors, restart state, graph
shortest paths and successor acyclicity passed these bounded checks.

An initial encoding check mutated byte 23, which changes a high sequence byte,
instead of byte 27, which changes parity. The independent test oracle was
corrected; no production change was required. Early runner attempts before the
fresh library was linked were setup errors, not behavioral evidence.

Scoped architecture and naming checks both exited 0. Production UDP integration,
kernel scheduling, lifecycle, release and feature-off checks remain pending.
