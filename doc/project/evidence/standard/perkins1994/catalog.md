# Perkins and Bhagwat 1994 statement catalog

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `PB94-*` · **Stands on:** [original paper](https://doi.org/10.1145/190314.190336)

Source: *Highly Dynamic Destination-Sequenced Distance-Vector Routing (DSDV) for
Mobile Computers*, SIGCOMM 1994, pp. 234–244. Sections refer to the paper,
not PDF page numbers. [Accessible source](https://www.cse.iitb.ac.in/~mythili/teaching/cs653_spring2014/references/dsdv.pdf).

| ID | Source | Checkable statement |
| --- | --- | --- |
| PB94-SELECT | §3; §4 Damping Fluctuations | Prefer newer destination sequences, then lower hop counts; add the receiving hop once. |
| PB94-LOSS | §3 | Destinations originate even sequences; failed successors produce odd-sequence infinity, disclosed immediately. |
| PB94-CORRECT | §3 | A finite route newer than an incoming withdrawal triggers a corrective advertisement. |
| PB94-DUMPS | §3; §4 Tables 1–4 | Periodic full dumps and single-packet incrementals disseminate routes; full dumps may span packets. |
| PB94-PRIORITY | §4 Table 4 | Metric changes take priority over sequence-only changes in incrementals. |
| PB94-FAIR | §4 Table 4 discussion | When sequence-only changes exceed the remaining incremental space, select them fairly over successive incremental updates. |
| PB94-SETTLE | §3; §4 Damping Fluctuations | Forwarding and advertised views differ during settling; history estimates the wait for a better metric. |

The damping paragraph also describes replacing a route with a worse metric at the
same sequence. This conflicts with the repeated selection rule. This model follows
the freshness/lower-metric rule, including for the selected successor. A worse
same-sequence report neither replaces nor refreshes that route. Expiry handles loss.
The discriminating oracle is `(sequence=10,metric=2)` followed by candidate
`(10,3)`: keep `(10,2)`.

The paper does not specify interoperable bytes, smoothing coefficients, timeout
defaults, sequence wrap, restart persistence or withdrawal garbage collection.
Those are model choices documented by the protocol profile, never source mandates.
