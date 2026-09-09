# IPv4 — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc6864/catalog.md](../../standard/rfc6864/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow (see
[derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)). The
procedures come from the specification only. They name no simulation model and no code.
This file holds the common mockup, the rules every check obeys, and the index. The checks
themselves live in one file per feature under [`checks/`](checks); the section names are
the anchors that the coverage ledger links to.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Datagram delivery](checks/delivery.md#datagram-delivery) | `checks/delivery.md` | RFC791-FWD-1, DLV-1, PROTO-1, HDR-1, HDR-2, HDR-3; covers RFC1122-ADDR-1 |
| [TTL decrement](checks/ttl.md#ttl-decrement) | `checks/ttl.md` | RFC791-TTL-1, CKSUM-1; covers TTL-3 |
| [TTL expiry](checks/ttl.md#ttl-expiry) | `checks/ttl.md` | RFC791-TTL-2, RFC792-TE-1 |
| [TTL 1 at the destination](checks/ttl.md#ttl-1-at-the-destination) | `checks/ttl.md` | RFC1122-TTL-2; covers RFC1122-TTL-3 |
| [Fragment and reassembly](checks/fragmentation.md#fragment-and-reassembly) | `checks/fragmentation.md` | RFC791-FRAG-1..4, REASM-1; covers RFC1122-REASM-1 |
| [Don't fragment](checks/fragmentation.md#dont-fragment) | `checks/fragmentation.md` | RFC791-FRAG-5, RFC792-DU-4; covers RFC6864-ID-6 |
| [Minimum sizes](checks/fragmentation.md#minimum-sizes) | `checks/fragmentation.md` | RFC791-FRAG-6, REASM-2; covers RFC1122-REASM-2 |
| [Interleaved reassembly](checks/fragmentation.md#interleaved-reassembly) | `checks/fragmentation.md` | RFC791-REASM-3, RFC1122-REASM-1 |
| [Same identification, different protocol](checks/fragmentation.md#same-identification-different-protocol) | `checks/fragmentation.md` | RFC791-REASM-1 (the four-field key), RFC791-REASM-3 |
| [Identification](checks/identification.md#identification) | `checks/identification.md` | RFC6864-ID-5 (governs RFC791-ID-1) |
| [Atomic identification](checks/identification.md#atomic-identification) | `checks/identification.md` | RFC6864-ID-3, RFC6864-ID-7; notes RFC6864-ID-2 |
| [Checksum discard](checks/checksum.md#checksum-discard) | `checks/checksum.md` | RFC1122-CKSUM-1 (governs RFC791-CKSUM-2) |
| [Version discard](checks/input-validation.md#version-discard) | `checks/input-validation.md` | RFC1122-VER-1 |
| [Foreign destination](checks/input-validation.md#foreign-destination) | `checks/input-validation.md` | RFC1122-ADDR-2 |
| [Invalid source address](checks/input-validation.md#invalid-source-address) | `checks/input-validation.md` | RFC1122-ADDR-3, RFC1122-ADDR-4 |
| [Unknown ICMP type](checks/input-validation.md#unknown-icmp-type) | `checks/input-validation.md` | RFC1122-ICMP-1 |
| [Host error report](checks/error-report.md#host-error-report) | `checks/error-report.md` | RFC1122-ERR-1, DU-1, ICMP-2, ICMP-4 |
| [No error about an error](checks/error-report.md#no-error-about-an-error) | `checks/error-report.md` | RFC1122-ICMP-5 |
| [No error for a broadcast](checks/error-report.md#no-error-for-a-broadcast) | `checks/error-report.md` | RFC1122-ICMP-6 |
| [No error for a link-layer broadcast](checks/error-report.md#no-error-for-a-link-layer-broadcast) | `checks/error-report.md` | RFC1122-ICMP-7 |
| [No error for a non-initial fragment](checks/error-report.md#no-error-for-a-non-initial-fragment) | `checks/error-report.md` | RFC1122-ICMP-8 |
| [No error for an invalid source](checks/error-report.md#no-error-for-an-invalid-source) | `checks/error-report.md` | RFC1122-ICMP-9 |

Twenty-two checks: seven from the level 2 pass, fifteen added at level 3.

## Common mockup

Three nodes in a row. Host A is the source. Node R is a gateway. Host B is the destination.

```
A ----------- R ----------- B
    link 1        link 2
```

- A small application on host A sends one UDP datagram to host B.
- Timing: host A sends the datagram shortly after the start. All expected events occur
  within 1 second. Observation stops after 1 second.
- Each check states its own scenario constants: data size, sender TTL, the MTU of link 2,
  and the DF flag.
- Every node computes the header checksum of the datagrams it sends and forwards. A
  simulation model may offer a mode that assumes checksums correct without computing them;
  a check that reads or corrupts the checksum runs with that mode off.

### Variants for the level 3 checks

**With a relay.** A transparent relay T is spliced into link 2 next to host B. The relay
forwards every frame unchanged unless the check tells it to hold one frame for a stated
time or to rewrite one field of one datagram. When the relay rewrites a header field, it
keeps the header checksum valid, as any deliberate rewriter would; only the checksum check
corrupts the checksum itself. "On link 2" means before the relay; "at host B's interface"
means after it.

```
A ----------- R ------ T ------ B
    link 1          link 2
```

**With two gateways.** A second gateway R2 stands between R1 and host B. Link 2 joins the
two gateways and carries the small MTU; link 3 joins R2 and host B.

```
A ----------- R1 ----------- R2 ----------- B
    link 1         link 2          link 3
```

**On one link.** Host A and host B share one link, without a gateway. The broadcast check
uses it, because a limited broadcast does not cross a gateway.

```
A ----------- B
    link 1
```

### Rules every check obeys

- **One observation confirms the stimulus.** A datagram with the crafted field is observed
  where it enters the node under test. A configuration error that voids the stimulus then
  fails the check instead of a silent pass for the wrong reason.
- **A discard is observed where the decision is made.** A silent discard leaves no datagram
  to observe. Each check that expects one therefore observes the discard record of the node
  at the moment of the decision, and only then opens its absence watches. An absence watch
  that opens after the forbidden event would have happened proves nothing; see the note
  under [TTL expiry](checks/ttl.md#ttl-expiry).
- **Silence has two halves.** "Silently discard" means that nothing goes upward and nothing
  goes back. Every silent-discard check watches both.
- **A `may` is not a violation when absent.** Where a check asserts the cooperative
  behavior of a `may` or a `should`, its notes say so, and the ledger keeps the strengths
  apart.
