# DSDV host routing on a mobile line

Run from this directory with the INET environment loaded:

```
inet --debug -u Cmdenv -c Static -r 0
inet --debug -u Cmdenv -c Mobile -r 0
```

Four nodes use unique static IPv4 addresses, local ARP, forwarding and a 110m
radio range. No connected, default, subnet or configurator multihop route can
bypass the protocol's /32 next-hop routes. Startup transmissions deliberately
coincide; later periodic phases are randomized using the assigned module RNG.
The small payload budget also exercises split full dumps.

In Mobile, the second node moves away from t=8 to t=12, waits until t=20,
and returns by t=24. Pings initially succeed, fail during the partition, and
resume after newer destination sequences repair the withdrawn paths. These are
deterministic correctness fixtures, not performance measurements.

The protocol models IPv4 host routes on one bidirectional interface, using
Perkins/Bhagwat (SIGCOMM 1994) with explicit model choices. It has its own UDP
wire format; it does not claim external interoperability. Retained withdrawals
prevent delayed stale resurrection but grow memory and full dumps over a run.
The contributor evidence is in `doc/project/evidence/model/dsdv/`.
