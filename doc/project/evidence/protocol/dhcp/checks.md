# DHCP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc2131/catalog.md](../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../standard/rfc2132/catalog.md), [rfc6842/catalog.md](../../standard/rfc6842/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
group under [`checks/`](checks); the section names are the anchors that the coverage ledger
links to.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Address allocation exchange](checks/exchange.md#address-allocation-exchange) | `checks/exchange.md` | RFC2131-MSG-1, MSG-3, MSG-5, OFF-1, ACK-1, ACK-2, RFC2132-TYPE-1 |
| [Transaction identifier through the exchange](checks/exchange.md#transaction-identifier-through-the-exchange) | `checks/exchange.md` | RFC2131-XID-3; covers XID-1 |
| [Discover contents](checks/exchange.md#discover-contents) | `checks/exchange.md` | RFC2131-DISC-1, DISC-2, DISC-3, DISC-5, MSG-9 |
| [Offer contents](checks/exchange.md#offer-contents) | `checks/exchange.md` | RFC2131-OFF-2, OFF-3, OFF-4, OFF-5, OFF-6 (in part), OFF-8, RFC2132-LEASE-1, RFC2132-SRVID-1 |
| [Request contents](checks/exchange.md#request-contents) | `checks/exchange.md` | RFC2131-REQ-1, REQ-2, REQ-3, REQ-4, REQ-8, REQ-9, RFC2132-REQIP-1 |
| [Acknowledgement contents](checks/exchange.md#acknowledgement-contents) | `checks/exchange.md` | RFC2131-ACK-3, ACK-4, ACK-5, ACK-6, ACK-8 (in part) |
| [Address fields of a server reply](checks/exchange.md#address-fields-of-a-server-reply) | `checks/exchange.md` | RFC2131-OFF-6, ACK-8 (the `ciaddr` and `giaddr` half of each) |
| [Message framing on the wire](checks/message-format.md#message-framing-on-the-wire) | `checks/message-format.md` | RFC2131-MSG-2, MSG-4, MSG-7, MSG-8, MSG-11, RFC2132-FMT-1, FMT-3, FMT-4, END-1 |
| [Renewal at T1](checks/lease.md#renewal-at-t1) | `checks/lease.md` | RFC2131-LEASE-1, LEASE-4, LEASE-5, REQ-6, BCAST-3, SRVID-3, RFC2132-T1-1 |
| [Rebinding at T2](checks/lease.md#rebinding-at-t2) | `checks/lease.md` | RFC2131-LEASE-6, REQ-7, LEASE-3, RFC2132-T2-1 |
| [Lease expiry](checks/lease.md#lease-expiry) | `checks/lease.md` | RFC2131-LEASE-8, LEASE-9, LEASE-2 |
| [Negative acknowledgement for a wrong subnet](checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | `checks/nak.md` | RFC2131-NAK-2, NAK-3, NAK-5, NAK-6, REQ-5 |
| [Silence for an unknown client](checks/nak.md#silence-for-an-unknown-client) | `checks/nak.md` | RFC2131-NAK-7 |
| [Client identifier echoed](checks/client-identity.md#client-identifier-echoed) | `checks/client-identity.md` | RFC6842-CLID-1, RFC6842-CLID-2, RFC2132-CLID-1 |
| [Foreign client identifier discarded](checks/client-identity.md#foreign-client-identifier-discarded) | `checks/client-identity.md` | RFC6842-CLID-3 |
| [Foreign transaction identifier discarded](checks/client-identity.md#foreign-transaction-identifier-discarded) | `checks/client-identity.md` | RFC2131-XID-4 |
| [Duplicate address declined](checks/decline.md#duplicate-address-declined) | `checks/decline.md` | RFC2131-DECL-1, DECL-2, DECL-5, DECL-6, DECL-7 |
| [Inform answered without a lease](checks/inform.md#inform-answered-without-a-lease) | `checks/inform.md` | RFC2131-INF-1, INF-2, INF-3, INF-5, ACK-3 |
| [Release on shutdown](checks/release.md#release-on-shutdown) | `checks/release.md` | RFC2131-REL-1, REL-3, REL-5 |
| [Broadcast bit honoured](checks/reply-delivery.md#broadcast-bit-honoured) | `checks/reply-delivery.md` | RFC2131-BCAST-4, BCAST-5, BCAST-6 |
| [Discover repeated without a server](checks/retransmission.md#discover-repeated-without-a-server) | `checks/retransmission.md` | RFC2131-RETX-1, RETX-3 |
| [Request repeated when the reply is lost](checks/retransmission.md#request-repeated-when-the-reply-is-lost) | `checks/retransmission.md` | RFC2131-RETX-4 |
| [Requested parameters returned](checks/parameters.md#requested-parameters-returned) | `checks/parameters.md` | RFC2131-SEL-4, SEL-5, RFC2132-PRL-1, RFC2132-MASK-1, RFC2132-ROUTER-1 |

Twenty-three checks. Eight of them need a fault or a crafted message and are therefore
level 3: the three that inject a message ([Negative acknowledgement for a wrong
subnet](checks/nak.md#negative-acknowledgement-for-a-wrong-subnet), [Silence for an unknown
client](checks/nak.md#silence-for-an-unknown-client), [Inform answered without a
lease](checks/inform.md#inform-answered-without-a-lease)), the two that inject a crafted
reply at a client ([Foreign client identifier
discarded](checks/client-identity.md#foreign-client-identifier-discarded), [Foreign
transaction identifier discarded](checks/client-identity.md#foreign-transaction-identifier-discarded)),
and the three that remove a reply from the path ([Rebinding at
T2](checks/lease.md#rebinding-at-t2), [Lease expiry](checks/lease.md#lease-expiry),
[Request repeated when the reply is
lost](checks/retransmission.md#request-repeated-when-the-reply-is-lost)).

## Statements that no check carries

Fifty-nine statements of the in-scope set have no check in this pass. They fall into six
groups, and the group decides the reason.

| Group | Statements | Why no check |
| --- | --- | --- |
| **Needs a relay agent** | RFC2131-NAK-4, BCAST-2 | Both hold only when a relay agent put an address in `giaddr`. A relay agent is a third node type, and RFC 1542 that defines it is out of the in-scope set. `giaddr` is zero in every message of every mockup here, so the condition never holds. |
| **A distribution, not a value** | RFC2131-DISC-4, RETX-2, LEASE-7, LEASE-10, DECL-3 | Each one states a time with a random part: a start delay of one to ten seconds, a backoff of 4 then 8 then 64 seconds with a fuzz of one, a wait of half the remaining time, a fuzz on T1 and T2, a ten-second pause. A single run cannot tell a wrong distribution from an unlucky draw. Level 4 adds the statistical check and the tolerance that these need. |
| **Inside a node, with no consequence on a link** | RFC2131-MSG-6, ID-1, SRVID-1, INF-4, SEL-1, SEL-2, SEL-3, DECL-4, REL-4, MISC-1, ACK-7, OFF-7, DECL-8, XID-2, XID-5, BCAST-1, LEASE-11, ID-2 | Each one is about what a node stores or decides, or about a second scenario that a check would have to build to see the consequence: which address a server prefers, whether it took an address out of service, whether it looked for a lease. A check of two nodes on a link can see the consequence only through a second exchange, which makes the check a check of the pool and not of the rule. A later pass can reach most of them with a second client in the mockup. |
| **An encoding rule of a field the messages here do not use** | RFC2131-MSG-10, RFC2132-OVER-1, TFTP-1, BOOTF-1, PAD-1, FMT-2, FMT-5, MSGOPT-1, MAXSZ-1, VCLASS-1 | Option overload, the two options that replace an overloaded field, the pad option, the trailing-null rule, the site-specific range, the text option, the message size option and the vendor class option. A serializer unit test is the home of each one, and none of them appears in a message of the exchanges above. |
| **A permission whose absence is not a failure** | RFC2131-DISC-6, OFF-1 (the answer half), REL-1 (the existence half), INF-1 (the existence half), REQ-5 (the entry half) | A `MAY` clause is not a violation when nothing happens. Where the permission has a visible form, the check observes that form and says so; where it does not, there is nothing to observe. |
| **Covered by another check, no check of its own** | RFC2131-NAK-1, NAK-8, RETX-4 (the revert half), REQ-7 (the field half), LEASE-3, SRVID-2, ACK-6 | Each one is a `covered` entry of the ledger: a check that targets another statement establishes it on the way. |

The coverage ledger records each of these with its status and this reason. They are listed
here and not forgotten.

## Common mockups

Six mockups serve the twenty-three checks. Every one of them is one IPv4 subnet, because DHCP
without a relay agent is a protocol of one subnet.

**The plain mockup.** One server S and one client C on one link. S has a fixed address and a
pool of addresses to give out. C starts with no address at all.

```
   server S  ------------  client C
              link 1
```

**The mockup with two clients.** A second client D joins link 1, so that two clients ask the
same server, and so that a check can compare what the server sends to each of them.

```
   server S  ------  client C
        |
        +----------  client D
```

**The mockup with an occupant.** A host O that is not a DHCP client holds, from the start,
the address that the server will offer to C. O answers an address resolution request for
that address, so C can discover the conflict.

```
   server S  ------  client C
        |
        +----------  host O   (holds the address S will offer)
```

**The mockup with a relay on the path.** A relay T sits on link 1 and holds each frame for a
moment. It can drop one frame, or change one field of one frame, and it passes everything
else on without a change.

```
   server S  ------  relay T  ------  client C
            link 1a           link 1b
```

- The relay is not a node of the protocol: it has no address and it answers nothing.
- What the relay does is what a fault on the path would do. No program can produce it, and
  that is why the checks that use it are level 3.

**The mockup with no server.** Client C alone on a link, with a host at the far end that
runs no DHCP. Nothing answers C.

```
   client C  ------------  host E   (no DHCP)
              link 1
```

**The mockup with an injector.** The plain mockup, plus the ability to put one crafted
message onto link 1, addressed to the server or to the client. The message is built field by
field, so it can hold a combination that no conforming node would send.

- An injected message is not the work of a node of the mockup. It stands for a message from
  a fourth party: another client, another server, or an attacker.

## Rules every check obeys

- **One observation confirms the stimulus.** Every check names one expected observation whose
  only job is to prove that the scenario did what the procedure says. A configuration error
  that voids the stimulus then fails the check instead of passing it for the wrong reason.
  In a check that removes a reply from the path, the confirming observation is the reply
  arriving at the relay; in a check that injects a message, it is the message reaching the
  node it is addressed to.
- **The scenario constants.** The subnet is 192.168.1.0/24. The server holds 192.168.1.1. It
  gives out addresses from 192.168.1.100 upward. The lease is 300 seconds unless a check
  says otherwise, so T1 falls at 150 seconds and T2 at 262.5 seconds. Every host is on one
  Ethernet link of 100 Mbit per second.
- **Timing.** Every check states its own observation window. A check of the first exchange
  needs less than a second; a check of the lease needs a window past the expiry.
- **Names of fields.** A check names a field as RFC 2131 names it — `op`, `xid`, `secs`,
  `flags`, `ciaddr`, `yiaddr`, `giaddr`, `chaddr` — and an option by the name and the code
  that RFC 2132 gives it, for example `server identifier` (54).
- **An absent option.** Several statements of RFC 2131 demand that an option is *not* in a
  message. A check of such a statement reads the option area of the message and states that
  the code is not in it. An absence is as observable as a presence, and the checks say which
  one they mean.
- **Address resolution.** Where a check does not concern address resolution, the mockup
  resolves addresses without traffic, so that the link carries only the messages under test.
  The one check that needs an address resolution request says so.
