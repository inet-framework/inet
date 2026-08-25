# IPv4 with ARP and ICMP — conformance review

- **Family:** IPv4 with ARP and ICMP
- **Directories:** `src/inet/networklayer/ipv4`, `src/inet/networklayer/arp/ipv4`
- **Specs:** see [spec-manifest.md](spec-manifest.md)
- **Reviewed at:** _(pending — set at P8)_
- **Method:** _(pending — set at P8)_

## 1. Scope statement

Approved at the P2 checkpoint on 2026-08-25. This section decides what counts as a gap. A
requirement outside this scope is reported as `Non-gap`, not as a defect.

### What this family models

The IPv4 network layer of a simulated host or router, in enough detail that transport protocols
above it and link layers below it see realistic datagram service.

| Component | Module | What it covers |
|---|---|---|
| IP forwarding and delivery | `Ipv4` | Sending from the transport layer, receiving, forwarding, local delivery, TTL handling, fragmentation and reassembly, multicast forwarding |
| Routing table | `Ipv4RoutingTable` | Longest-prefix unicast lookup, multicast routes, netmask routes, router id, administrative distance |
| Address resolution | `Arp`, `GlobalArp` | Request and reply exchange, a cache with a timeout, retries, proxy ARP |
| Error and diagnostic messages | `Icmp` | Error message generation for the network and transport layers, echo request and reply |
| Wire format | `Ipv4Header`, `ArpPacket`, `IcmpHeader` | Typed header definitions, plus a registered serializer, dissector, and printer for each |

### Declared fidelity

- **The header is a typed object, and it also serializes to real bytes.** Both representations
  exist. `checksumMode` selects between a declared checksum and a computed one, so a study can
  trade fidelity against speed.
- **Options are partially represented.** `Ipv4OptionType` enumerates the standard option types.
  Typed classes exist for No-Operation, End-of-Options, Record Route, Timestamp, Stream ID, and
  Router Alert. An unrecognized option is carried as `Ipv4OptionUnknown`.
- **ICMP carries typed classes for the echo pair and for Packet Too Big.** Other message types use
  the base `IcmpHeader` with a type and code.
- **Processing time is a single per-packet delay,** not a model of forwarding hardware.
- **The quoted portion of an ICMP error is a parameter,** `quoteLength`, default 8 bytes.

### Deliberately outside this family

- **IGMP.** `Igmpv2` and `Igmpv3` live in the same directory, and have 13 module tests. Excluded
  at the P0 checkpoint. They become their own family, against RFC 2236 and RFC 3376.
- **NAT.** `Ipv4NatTable` is excluded at the P0 checkpoint. RFC 3022 is a different problem.
- **IPsec.** `Ipv4NetworkLayer` can include an `IPsec` submodule. That is its own family.
- **Address configuration.** `Ipv4NodeConfigurator` and the network configurator assign addresses
  and routes. They implement no standard, and they have their own tests.
- **Routing protocols.** OSPF, RIP, PIM, and the rest populate the routing table from outside.
  This review judges the table's lookup semantics, not how routes arrive.

### Known intent that limits conformance

These are stated here so the review does not report them as surprises. Whether each one is
acceptable is a P8 decision, not a P2 one.

- INET models a **simulation** host, not a production stack. A requirement whose only purpose is
  interoperability with real hardware may be a `Non-gap`.
- The `Ipv4` module is a single simple module. The standard's split between a host and a router is
  expressed by the `forwarding` parameter, not by two implementations.

### Source of this statement

Read from `Ipv4.ned`, `Icmp.ned`, `Arp.ned`, `Ipv4RoutingTable.ned`, `Ipv4NetworkLayer.ned`,
`Ipv4Header.msg`, `IcmpHeader.msg`, and `ArpPacket.msg`.

`doc/src/developers-guide/ch-ipv4.rst` was read, and **it is out of date**. It documents a
`procDelay` parameter, a `QueueBase` base class, an `ICMPAccess` accessor, and `errOut` and
`pingIn` gates. None of these appear in the current NED or C++. The scope statement above follows
the code, not the chapter. Refreshing that chapter is separate work, and is recorded in section 6.

## 2. Summary

_(pending — written at P8)_

## 3. Coverage matrix

_(pending — P3 to P5)_

## 4. Findings

_(pending — P4 to P7)_

## 5. Defaults and configuration

_(pending — P4)_

## 6. Observations

- `doc/src/developers-guide/ch-ipv4.rst` describes an interface that no longer exists
  (`procDelay`, `QueueBase`, `ICMPAccess`, gates `errOut` and `pingIn`). It needs a refresh. Not
  chased here.

## 7. Closed

_(none yet)_

## 8. Accepted omissions

See `accepted-omissions.md`. The file is created at P8, when the first row is accepted.
