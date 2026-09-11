# DHCP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc2131/catalog.md](../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../standard/rfc2132/catalog.md), [rfc6842/catalog.md](../../standard/rfc6842/catalog.md), [features.md](../../protocol/dhcp/features.md), [results.md](results.md)

The single place that holds the changing state of the DHCP workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-11 11:25 +0200
- INET: branch `topic/rfc-tests-dhcp-level3`, commit `4e20c74e82`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w dhcp`
- Target level: 3

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level, or another test category),
`no check` (no check of two nodes on a link can observe it; the reason is in the row and in
[`checks.md`](../../protocol/dhcp/checks.md#statements-that-no-check-carries)).

A verdict of **untested** on a `selected` row is not the same as a FAIL. It means the check
ran, its verdict is recorded against the statement that failed, and this statement had nothing
to be read on because the model's behavior removed the stimulus. Twelve rows are in that state,
and every one of them names the gap that caused it.

A FAIL on a row says nothing about whether the test that found it is **declared** to fail. That is
a separate question, and [`results.md`](results.md#the-class-of-every-failure) answers it: six of
the fourteen failing statements are defects in code that means to be right, and their five tests
carry no declaration and fail the run; the other eight are features the model does not implement,
and their tests declare the failure expected. The suite reports 13 PASS, 8 FAIL (expected) and
5 FAIL (unexpected), and therefore FAIL overall.

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC2131-MSG-1](../../standard/rfc2131/catalog.md#rfc2131-msg-1) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS |
| [RFC2131-MSG-2](../../standard/rfc2131/catalog.md#rfc2131-msg-2) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2131-MSG-3](../../standard/rfc2131/catalog.md#rfc2131-msg-3) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange), [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131AddressAllocation.test`, `Rfc2131MessageFraming.test` | PASS, from two sides: the option names every message, and the octets carry it |
| [RFC2131-MSG-4](../../standard/rfc2131/catalog.md#rfc2131-msg-4) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2131-MSG-5](../../standard/rfc2131/catalog.md#rfc2131-msg-5) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS |
| [RFC2131-MSG-6](../../standard/rfc2131/catalog.md#rfc2131-msg-6) | no check | — | — | needs a reply padded to 576 octets, which no node of a mockup sends |
| [RFC2131-MSG-7](../../standard/rfc2131/catalog.md#rfc2131-msg-7) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2131-MSG-8](../../standard/rfc2131/catalog.md#rfc2131-msg-8) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2131-MSG-9](../../standard/rfc2131/catalog.md#rfc2131-msg-9) | selected | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | `Rfc2131DiscoverContents.test` | PASS |
| [RFC2131-MSG-10](../../standard/rfc2131/catalog.md#rfc2131-msg-10) | later (unit test) | — | — | option overload; an encoding rule of fields the messages here do not use |
| [RFC2131-MSG-11](../../standard/rfc2131/catalog.md#rfc2131-msg-11) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS for the first half, no code twice; the concatenation half is RFC 3396 and out of scope |
| [RFC2131-XID-1](../../standard/rfc2131/catalog.md#rfc2131-xid-1) | covered | [transaction-identifier](../../protocol/dhcp/checks/exchange.md#transaction-identifier-through-the-exchange) | `Rfc2131TransactionIdentifier.test` | PASS as far as the check reaches: one value is chosen and the DHCPOFFER carries it back |
| [RFC2131-XID-2](../../standard/rfc2131/catalog.md#rfc2131-xid-2) | no check | — | — | two clients must pick two values; a statement about randomness, and it needs a second client |
| [RFC2131-XID-3](../../standard/rfc2131/catalog.md#rfc2131-xid-3) | selected | [transaction-identifier](../../protocol/dhcp/checks/exchange.md#transaction-identifier-through-the-exchange) | `Rfc2131TransactionIdentifier.test` | **FAIL**, gap 1 |
| [RFC2131-XID-4](../../standard/rfc2131/catalog.md#rfc2131-xid-4) | selected | [foreign-transaction-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-transaction-identifier-discarded) | `Rfc2131ForeignTransactionId.test` | PASS, both halves: a foreign identifier and an unrequested DHCPACK |
| [RFC2131-XID-5](../../standard/rfc2131/catalog.md#rfc2131-xid-5) | no check | — | — | the same discard in RENEWING; needs a crafted DHCPACK during a renewal |
| [RFC2131-DISC-1](../../standard/rfc2131/catalog.md#rfc2131-disc-1) | covered | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | `Rfc2131DiscoverContents.test` | PASS |
| [RFC2131-DISC-2](../../standard/rfc2131/catalog.md#rfc2131-disc-2) | selected | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | `Rfc2131DiscoverContents.test` | PASS |
| [RFC2131-DISC-3](../../standard/rfc2131/catalog.md#rfc2131-disc-3) | selected | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | `Rfc2131DiscoverContents.test` | PASS |
| [RFC2131-DISC-4](../../standard/rfc2131/catalog.md#rfc2131-disc-4) | later (statistical) | — | — | a random delay of one to ten seconds; level 4 |
| [RFC2131-DISC-5](../../standard/rfc2131/catalog.md#rfc2131-disc-5) | selected | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | `Rfc2131DiscoverContents.test` | PASS |
| [RFC2131-DISC-6](../../standard/rfc2131/catalog.md#rfc2131-disc-6) | no check | — | — | a may; no scenario asks the client to suggest an address |
| [RFC2131-OFF-1](../../standard/rfc2131/catalog.md#rfc2131-off-1) | covered | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS |
| [RFC2131-OFF-2](../../standard/rfc2131/catalog.md#rfc2131-off-2) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2131-OFF-3](../../standard/rfc2131/catalog.md#rfc2131-off-3) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2131-OFF-4](../../standard/rfc2131/catalog.md#rfc2131-off-4) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2131-OFF-5](../../standard/rfc2131/catalog.md#rfc2131-off-5) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS for the three rows that stand; the `client identifier` row is overridden by RFC6842-CLID-1 |
| [RFC2131-OFF-6](../../standard/rfc2131/catalog.md#rfc2131-off-6) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents), [address-fields-of-a-server-reply](../../protocol/dhcp/checks/exchange.md#address-fields-of-a-server-reply), [the-flags-field-of-a-server-reply](../../protocol/dhcp/checks/reply-delivery.md#the-flags-field-of-a-server-reply) | `Rfc2131OfferContents.test`, `Rfc2131ReplyAddressFields.test`, `Rfc2131ReplyFlagsField.test` | PASS for `hops`, `secs`, `ciaddr` and `chaddr`; **FAIL** for `giaddr`, gap 2, and for `flags`, gap 8 |
| [RFC2131-OFF-7](../../standard/rfc2131/catalog.md#rfc2131-off-7) | no check | — | — | the server's probe before it offers; inside the node, and gap 9 territory |
| [RFC2131-OFF-8](../../standard/rfc2131/catalog.md#rfc2131-off-8) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2131-REQ-1](../../standard/rfc2131/catalog.md#rfc2131-req-1) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-REQ-2](../../standard/rfc2131/catalog.md#rfc2131-req-2) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-REQ-3](../../standard/rfc2131/catalog.md#rfc2131-req-3) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-REQ-4](../../standard/rfc2131/catalog.md#rfc2131-req-4) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-REQ-5](../../standard/rfc2131/catalog.md#rfc2131-req-5) | covered | [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet), [silence-for-an-unknown-client](../../protocol/dhcp/checks/nak.md#silence-for-an-unknown-client) | `Rfc2131NakWrongSubnet.test`, `Rfc2131SilentUnknownClient.test` | PASS as the shape of the crafted stimulus; never observed on a message a node of the mockup built |
| [RFC2131-REQ-6](../../standard/rfc2131/catalog.md#rfc2131-req-6) | selected | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS |
| [RFC2131-REQ-7](../../standard/rfc2131/catalog.md#rfc2131-req-7) | selected | [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) | `Rfc2131RebindAtT2.test` | PASS |
| [RFC2131-REQ-8](../../standard/rfc2131/catalog.md#rfc2131-req-8) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-REQ-9](../../standard/rfc2131/catalog.md#rfc2131-req-9) | selected | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2131-ACK-1](../../standard/rfc2131/catalog.md#rfc2131-ack-1) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS |
| [RFC2131-ACK-2](../../standard/rfc2131/catalog.md#rfc2131-ack-2) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS |
| [RFC2131-ACK-3](../../standard/rfc2131/catalog.md#rfc2131-ack-3) | selected | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents), [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131AckContents.test`, `Rfc2131InformWithoutLease.test` | PASS for the `must` half; **untested** for the `must not` half, because gap 9 removes the answer it would read |
| [RFC2131-ACK-4](../../standard/rfc2131/catalog.md#rfc2131-ack-4) | selected | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents) | `Rfc2131AckContents.test` | PASS |
| [RFC2131-ACK-5](../../standard/rfc2131/catalog.md#rfc2131-ack-5) | selected | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents) | `Rfc2131AckContents.test` | PASS for the three rows that stand; the `client identifier` row is overridden |
| [RFC2131-ACK-6](../../standard/rfc2131/catalog.md#rfc2131-ack-6) | covered | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents) | `Rfc2131AckContents.test` | PASS for the three parameters every exchange carries |
| [RFC2131-ACK-7](../../standard/rfc2131/catalog.md#rfc2131-ack-7) | no check | — | — | no second probe; it needs the first probe of OFF-7 to be visible |
| [RFC2131-ACK-8](../../standard/rfc2131/catalog.md#rfc2131-ack-8) | selected | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents), [address-fields-of-a-server-reply](../../protocol/dhcp/checks/exchange.md#address-fields-of-a-server-reply), [the-flags-field-of-a-server-reply](../../protocol/dhcp/checks/reply-delivery.md#the-flags-field-of-a-server-reply) | `Rfc2131AckContents.test`, `Rfc2131ReplyAddressFields.test`, `Rfc2131ReplyFlagsField.test` | PASS for `xid`, `hops`, `secs` and `chaddr`; **FAIL** for `ciaddr`, gap 3; the `flags` half PASSES for a clear bit and is untested for a set one |
| [RFC2131-NAK-1](../../standard/rfc2131/catalog.md#rfc2131-nak-1) | no check | — | — | a DHCPNAK for an address already given away; needs a second client |
| [RFC2131-NAK-2](../../standard/rfc2131/catalog.md#rfc2131-nak-2) | selected | [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | `Rfc2131NakWrongSubnet.test` | **FAIL**, gap 10 |
| [RFC2131-NAK-3](../../standard/rfc2131/catalog.md#rfc2131-nak-3) | selected | [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | `Rfc2131NakWrongSubnet.test` | **untested**: gap 10 means no DHCPNAK was sent, so the delivery rule had nothing to be read on |
| [RFC2131-NAK-4](../../standard/rfc2131/catalog.md#rfc2131-nak-4) | no check | — | — | needs a relay agent, and RFC 1542 is out of scope |
| [RFC2131-NAK-5](../../standard/rfc2131/catalog.md#rfc2131-nak-5) | selected | [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | `Rfc2131NakWrongSubnet.test` | **untested**, for the same reason as NAK-3 |
| [RFC2131-NAK-6](../../standard/rfc2131/catalog.md#rfc2131-nak-6) | selected | [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | `Rfc2131NakWrongSubnet.test` | **untested**, for the same reason as NAK-3 |
| [RFC2131-NAK-7](../../standard/rfc2131/catalog.md#rfc2131-nak-7) | selected | [silence-for-an-unknown-client](../../protocol/dhcp/checks/nak.md#silence-for-an-unknown-client) | `Rfc2131SilentUnknownClient.test` | PASS |
| [RFC2131-NAK-8](../../standard/rfc2131/catalog.md#rfc2131-nak-8) | no check | — | — | the client's restart after a DHCPNAK; no check of this pass makes a server send one to a client |
| [RFC2131-LEASE-1](../../standard/rfc2131/catalog.md#rfc2131-lease-1) | covered | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS for the unit of seconds; the value for infinity is not exercised |
| [RFC2131-LEASE-2](../../standard/rfc2131/catalog.md#rfc2131-lease-2) | covered | [lease-expiry](../../protocol/dhcp/checks/lease.md#lease-expiry) | `Rfc2131LeaseExpiry.test` | PASS weakly: the send instant and the arrival instant differ by tens of microseconds, which the tolerance cannot separate |
| [RFC2131-LEASE-3](../../standard/rfc2131/catalog.md#rfc2131-lease-3) | covered | [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) | `Rfc2131RebindAtT2.test` | PASS as three events, T1 before T2 before the expiry |
| [RFC2131-LEASE-4](../../standard/rfc2131/catalog.md#rfc2131-lease-4) | selected | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS |
| [RFC2131-LEASE-5](../../standard/rfc2131/catalog.md#rfc2131-lease-5) | selected | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS |
| [RFC2131-LEASE-6](../../standard/rfc2131/catalog.md#rfc2131-lease-6) | selected | [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) | `Rfc2131RebindAtT2.test` | PASS |
| [RFC2131-LEASE-7](../../standard/rfc2131/catalog.md#rfc2131-lease-7) | later (statistical) | — | — | half the remaining time, at least 60 seconds; level 4 |
| [RFC2131-LEASE-8](../../standard/rfc2131/catalog.md#rfc2131-lease-8) | selected | [lease-expiry](../../protocol/dhcp/checks/lease.md#lease-expiry) | `Rfc2131LeaseExpiry.test` | PASS |
| [RFC2131-LEASE-9](../../standard/rfc2131/catalog.md#rfc2131-lease-9) | selected | [lease-expiry](../../protocol/dhcp/checks/lease.md#lease-expiry) | `Rfc2131LeaseExpiry.test` | PASS for the address the client held; the other half needs a server whose pool has moved on |
| [RFC2131-LEASE-10](../../standard/rfc2131/catalog.md#rfc2131-lease-10) | later (statistical) | — | — | the random fuzz on T1 and T2; level 4 |
| [RFC2131-LEASE-11](../../standard/rfc2131/catalog.md#rfc2131-lease-11) | no check | — | — | the server does send options 58 and 59; whether the values are adjusted needs two renewals to compare |
| [RFC2131-RETX-1](../../standard/rfc2131/catalog.md#rfc2131-retx-1) | selected | [discover-repeated-without-a-server](../../protocol/dhcp/checks/retransmission.md#discover-repeated-without-a-server) | `Rfc2131DiscoverRetransmission.test` | **FAIL**, gap 13 |
| [RFC2131-RETX-2](../../standard/rfc2131/catalog.md#rfc2131-retx-2) | later (statistical) | — | — | the 4, 8 and 64-second figures; level 4. Gap 13 already says what the answer will be |
| [RFC2131-RETX-3](../../standard/rfc2131/catalog.md#rfc2131-retx-3) | selected | [discover-repeated-without-a-server](../../protocol/dhcp/checks/retransmission.md#discover-repeated-without-a-server) | `Rfc2131DiscoverRetransmission.test` | PASS |
| [RFC2131-RETX-4](../../standard/rfc2131/catalog.md#rfc2131-retx-4) | selected | [request-repeated-when-the-reply-is-lost](../../protocol/dhcp/checks/retransmission.md#request-repeated-when-the-reply-is-lost) | `Rfc2131RequestRetransmission.test` | **FAIL**, gap 14 |
| [RFC2131-DECL-1](../../standard/rfc2131/catalog.md#rfc2131-decl-1) | selected | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | `Rfc2131DuplicateAddressDeclined.test` | **FAIL**, gap 11 |
| [RFC2131-DECL-2](../../standard/rfc2131/catalog.md#rfc2131-decl-2) | selected | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | `Rfc2131DuplicateAddressDeclined.test` | **untested**: gap 11 means the client never finds a conflict, so the rule has no stimulus |
| [RFC2131-DECL-3](../../standard/rfc2131/catalog.md#rfc2131-decl-3) | later (statistical) | — | — | the pause of at least ten seconds; level 4 |
| [RFC2131-DECL-4](../../standard/rfc2131/catalog.md#rfc2131-decl-4) | no check | — | — | the server marks the address unavailable; needs a second client. Gap 9 shows it is not implemented |
| [RFC2131-DECL-5](../../standard/rfc2131/catalog.md#rfc2131-decl-5) | selected | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | `Rfc2131DuplicateAddressDeclined.test` | **untested**, for the same reason as DECL-2 |
| [RFC2131-DECL-6](../../standard/rfc2131/catalog.md#rfc2131-decl-6) | selected | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | `Rfc2131DuplicateAddressDeclined.test` | **untested**, for the same reason as DECL-2 |
| [RFC2131-DECL-7](../../standard/rfc2131/catalog.md#rfc2131-decl-7) | selected | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | `Rfc2131DuplicateAddressDeclined.test` | **untested**, for the same reason as DECL-2 |
| [RFC2131-DECL-8](../../standard/rfc2131/catalog.md#rfc2131-decl-8) | no check | — | — | the announcement after the address goes into service |
| [RFC2131-REL-1](../../standard/rfc2131/catalog.md#rfc2131-rel-1) | selected | [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) | `Rfc2131ReleaseOnShutdown.test` | **FAIL**, gap 12 |
| [RFC2131-REL-2](../../standard/rfc2131/catalog.md#rfc2131-rel-2) | covered | [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) | `Rfc2131ReleaseOnShutdown.test` | **untested**: no DHCPRELEASE is sent, so the field rule has nothing to be read on |
| [RFC2131-REL-3](../../standard/rfc2131/catalog.md#rfc2131-rel-3) | selected | [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) | `Rfc2131ReleaseOnShutdown.test` | **untested**, for the same reason as REL-2 |
| [RFC2131-REL-4](../../standard/rfc2131/catalog.md#rfc2131-rel-4) | no check | — | — | the server frees the address; needs a second client. Gap 9 shows it is not implemented |
| [RFC2131-REL-5](../../standard/rfc2131/catalog.md#rfc2131-rel-5) | selected | [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) | `Rfc2131ReleaseOnShutdown.test` | **untested**, for the same reason as REL-2 |
| [RFC2131-INF-1](../../standard/rfc2131/catalog.md#rfc2131-inf-1) | covered | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131InformWithoutLease.test` | PASS as the shape of the crafted stimulus; never observed on a message a node of the mockup built |
| [RFC2131-INF-2](../../standard/rfc2131/catalog.md#rfc2131-inf-2) | selected | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131InformWithoutLease.test` | **FAIL**, gap 9 |
| [RFC2131-INF-3](../../standard/rfc2131/catalog.md#rfc2131-inf-3) | selected | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131InformWithoutLease.test` | **untested**: gap 9 means there is no answer to read |
| [RFC2131-INF-4](../../standard/rfc2131/catalog.md#rfc2131-inf-4) | no check | — | — | a server that looked for a lease and found none looks like one that did not look |
| [RFC2131-INF-5](../../standard/rfc2131/catalog.md#rfc2131-inf-5) | covered | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131InformWithoutLease.test` | PASS as the shape of the crafted stimulus |
| [RFC2131-INF-6](../../standard/rfc2131/catalog.md#rfc2131-inf-6) | covered | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | `Rfc2131InformWithoutLease.test` | PASS as the shape of the crafted stimulus |
| [RFC2131-BCAST-1](../../standard/rfc2131/catalog.md#rfc2131-bcast-1) | no check | — | — | what a client's own software can receive is not visible on a link |
| [RFC2131-BCAST-2](../../standard/rfc2131/catalog.md#rfc2131-bcast-2) | no check | — | — | needs a relay agent, and RFC 1542 is out of scope |
| [RFC2131-BCAST-3](../../standard/rfc2131/catalog.md#rfc2131-bcast-3) | selected | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS |
| [RFC2131-BCAST-4](../../standard/rfc2131/catalog.md#rfc2131-bcast-4) | selected | [broadcast-bit-set](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-sets-the-broadcast-bit) | `Rfc2131BroadcastBitSet.test` | PASS |
| [RFC2131-BCAST-5](../../standard/rfc2131/catalog.md#rfc2131-bcast-5) | selected | [broadcast-bit-clear](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-clears-the-broadcast-bit) | `Rfc2131BroadcastBitClear.test` | **FAIL**, gap 7 |
| [RFC2131-BCAST-6](../../standard/rfc2131/catalog.md#rfc2131-bcast-6) | selected | [broadcast-bit-clear](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-clears-the-broadcast-bit), [broadcast-bit-set](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-sets-the-broadcast-bit) | `Rfc2131BroadcastBitClear.test`, `Rfc2131BroadcastBitSet.test` | **FAIL**: the set half passes and the clear half fails, so the server is not reading the bit |
| [RFC2131-SRVID-1](../../standard/rfc2131/catalog.md#rfc2131-srvid-1) | no check | — | — | needs a server with two addresses on the client's subnet |
| [RFC2131-SRVID-2](../../standard/rfc2131/catalog.md#rfc2131-srvid-2) | covered | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS: the identifier is the address the reply was sent from |
| [RFC2131-SRVID-3](../../standard/rfc2131/catalog.md#rfc2131-srvid-3) | selected | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS |
| [RFC2131-ID-1](../../standard/rfc2131/catalog.md#rfc2131-id-1) | no check | — | — | uniqueness within a subnet; a statement about configuration |
| [RFC2131-ID-2](../../standard/rfc2131/catalog.md#rfc2131-id-2) | no check | — | — | the server's key; visible only through a second exchange after a hardware address changes |
| [RFC2131-SEL-1](../../standard/rfc2131/catalog.md#rfc2131-sel-1) | no check | — | — | the order of preference for an address; needs a second exchange after a release |
| [RFC2131-SEL-2](../../standard/rfc2131/catalog.md#rfc2131-sel-2) | no check | — | — | an offered address held back; needs two clients that do not answer |
| [RFC2131-SEL-3](../../standard/rfc2131/catalog.md#rfc2131-sel-3) | covered | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS for the branch the scenario reaches: the configured default appears in the offer |
| [RFC2131-SEL-4](../../standard/rfc2131/catalog.md#rfc2131-sel-4) | selected | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) | `Rfc2131RequestedParameters.test` | PASS for the two branches the scenario reaches; the Host Requirements default branch needs RFC 1122 in scope |
| [RFC2131-SEL-5](../../standard/rfc2131/catalog.md#rfc2131-sel-5) | selected | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned), [a-parameter-with-no-value](../../protocol/dhcp/checks/parameters.md#a-parameter-the-server-has-no-value-for) | `Rfc2131RequestedParameters.test`, `Rfc2131ParameterWithoutValue.test` | PASS for the no-duplicate half; **FAIL** for the prohibition, gap 4 |
| [RFC2131-MISC-1](../../standard/rfc2131/catalog.md#rfc2131-misc-1) | no check | — | — | one exchange per interface; needs a client with two interfaces |
| [RFC2132-FMT-1](../../standard/rfc2132/catalog.md#rfc2132-fmt-1) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2132-FMT-2](../../standard/rfc2132/catalog.md#rfc2132-fmt-2) | later (unit test) | — | — | the trailing-null rule of a text option |
| [RFC2132-FMT-3](../../standard/rfc2132/catalog.md#rfc2132-fmt-3) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2132-FMT-4](../../standard/rfc2132/catalog.md#rfc2132-fmt-4) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2132-FMT-5](../../standard/rfc2132/catalog.md#rfc2132-fmt-5) | later (unit test) | — | — | the site-specific code range; no message here uses it |
| [RFC2132-PAD-1](../../standard/rfc2132/catalog.md#rfc2132-pad-1) | later (unit test) | — | — | the pad option; no message here uses it |
| [RFC2132-END-1](../../standard/rfc2132/catalog.md#rfc2132-end-1) | selected | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | `Rfc2131MessageFraming.test` | PASS |
| [RFC2132-MASK-1](../../standard/rfc2132/catalog.md#rfc2132-mask-1) | selected | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) | `Rfc2131RequestedParameters.test` | PASS, including the order rule |
| [RFC2132-ROUTER-1](../../standard/rfc2132/catalog.md#rfc2132-router-1) | selected | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) | `Rfc2131RequestedParameters.test` | PASS |
| [RFC2132-REQIP-1](../../standard/rfc2132/catalog.md#rfc2132-reqip-1) | covered | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | `Rfc2131RequestContents.test` | PASS |
| [RFC2132-LEASE-1](../../standard/rfc2132/catalog.md#rfc2132-lease-1) | selected | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2132-OVER-1](../../standard/rfc2132/catalog.md#rfc2132-over-1) | later (unit test) | — | — | option overload; the model has no field for it |
| [RFC2132-TYPE-1](../../standard/rfc2132/catalog.md#rfc2132-type-1) | selected | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | `Rfc2131AddressAllocation.test` | PASS: every message of the exchange names its own type |
| [RFC2132-SRVID-1](../../standard/rfc2132/catalog.md#rfc2132-srvid-1) | covered | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | `Rfc2131OfferContents.test` | PASS |
| [RFC2132-PRL-1](../../standard/rfc2132/catalog.md#rfc2132-prl-1) | selected | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) | `Rfc2131RequestedParameters.test` | PASS for the list; the `must try` half of the order is not observable |
| [RFC2132-MSGOPT-1](../../standard/rfc2132/catalog.md#rfc2132-msgopt-1) | no check | — | — | the text option; the model has no field for it |
| [RFC2132-MAXSZ-1](../../standard/rfc2132/catalog.md#rfc2132-maxsz-1) | no check | — | — | the message size option; the model has no field for it |
| [RFC2132-T1-1](../../standard/rfc2132/catalog.md#rfc2132-t1-1) | covered | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | `Rfc2131RenewAtT1.test` | PASS: the option is present and the client renews at the time it names |
| [RFC2132-T2-1](../../standard/rfc2132/catalog.md#rfc2132-t2-1) | covered | [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) | `Rfc2131RebindAtT2.test` | PASS: the option is present and the client rebinds at the time it names |
| [RFC2132-VCLASS-1](../../standard/rfc2132/catalog.md#rfc2132-vclass-1) | no check | — | — | the vendor class option; the model has no field for it |
| [RFC2132-CLID-1](../../standard/rfc2132/catalog.md#rfc2132-clid-1) | covered | [client-identifier-echoed](../../protocol/dhcp/checks/client-identity.md#client-identifier-echoed) | `Rfc6842ClientIdentifierEchoed.test` | PASS: each identifier is at least two octets and the two clients differ |
| [RFC2132-TFTP-1](../../standard/rfc2132/catalog.md#rfc2132-tftp-1) | later (unit test) | — | — | an option that only an overloaded message carries |
| [RFC2132-BOOTF-1](../../standard/rfc2132/catalog.md#rfc2132-bootf-1) | later (unit test) | — | — | an option that only an overloaded message carries |
| [RFC6842-CLID-1](../../standard/rfc6842/catalog.md#rfc6842-clid-1) | selected | [client-identifier-echoed](../../protocol/dhcp/checks/client-identity.md#client-identifier-echoed) | `Rfc6842ClientIdentifierEchoed.test` | **FAIL**, gap 5 |
| [RFC6842-CLID-2](../../standard/rfc6842/catalog.md#rfc6842-clid-2) | no check | — | — | needs a client that sends no `client identifier` option; the model's client always sends one |
| [RFC6842-CLID-3](../../standard/rfc6842/catalog.md#rfc6842-clid-3) | selected | [foreign-client-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-client-identifier-discarded) | `Rfc6842ForeignClientIdentifier.test` | **FAIL**, gap 6 |

132 entries, one per catalog entry of the three in-scope documents: 106 for RFC 2131, 23 for
RFC 2132 and 3 for RFC 6842.

| Status | Count |
| --- | --- |
| `selected` | 75 |
| `covered` | 19 |
| `later` | 12 |
| `no check` | 26 |

Of the 94 rows that a check targets or covers, 68 carry a PASS and nothing else, 14 carry a
FAIL that names a model gap, and 12 carry `untested` for at least one half because a gap removed
the stimulus their check needed. One row is in two of those groups at once:
[RFC2131-ACK-3](../../standard/rfc2131/catalog.md#rfc2131-ack-3) passes for its `must` half and
is untested for its `must not` half. Four of the fourteen failing rows likewise pass for one
half and fail for another, which is what the table's verdict column says row by row.

The 14 failing rows are the 14 model gaps of [`results.md`](results.md#the-model-gaps), spread
over 13 tests: 6 defects in 5 tests that fail the run, and 8 unimplemented features in 8 tests
that declare their failure. The 12 `later` rows split into 5 that need a statistical check with a stated
tolerance, which is level 4, and 7 that belong to a serializer unit test. The 26 `no check` rows
are grouped by reason in
[`checks.md`](../../protocol/dhcp/checks.md#statements-that-no-check-carries).

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported` when
every core check that ran failed as a model gap, `untested` when no core check ran.

One reading needs stating, because four features depend on it. A statement that a check
established only on a **crafted** message — the shape of an injected DHCPREQUEST or
DHCPINFORM — is not evidence about the model: the test wrote those octets itself. Such a row
carries a PASS in the table above with the words "as the shape of the crafted stimulus", and
it does **not** count as a core check that passed when the support value is computed. To count
it would let a test establish a feature by asserting its own input.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [DHCP-F-MESSAGE-FORMAT](../../protocol/dhcp/features.md#dhcp-f-message-format) | MSG-1, MSG-2, MSG-3, MSG-4, RFC2132-TYPE-1 all PASS | **supported** |
| [DHCP-F-TRANSPORT](../../protocol/dhcp/features.md#dhcp-f-transport) | MSG-5, MSG-9 PASS; INF-6 PASS on the crafted stimulus, and the two ports are established by MSG-5 on real messages | **supported** |
| [DHCP-F-OPTION-ENCODING](../../protocol/dhcp/features.md#dhcp-f-option-encoding) | RFC2132-FMT-1, FMT-3, RFC2131-MSG-11 all PASS | **supported** |
| [DHCP-F-TRANSACTION-ID](../../protocol/dhcp/features.md#dhcp-f-transaction-id) | XID-1 PASS, XID-4 PASS, XID-3 **FAIL** gap 1 | **partial** |
| [DHCP-F-DISCOVERY](../../protocol/dhcp/features.md#dhcp-f-discovery) | DISC-1, DISC-2, DISC-3, DISC-5 all PASS | **supported** |
| [DHCP-F-OFFER](../../protocol/dhcp/features.md#dhcp-f-offer) | OFF-1 to OFF-5 PASS, OFF-6 **FAIL** gaps 2 and 8 | **partial** |
| [DHCP-F-SELECTION](../../protocol/dhcp/features.md#dhcp-f-selection) | REQ-1, REQ-2, REQ-3, REQ-4 all PASS | **supported** |
| [DHCP-F-ACKNOWLEDGEMENT](../../protocol/dhcp/features.md#dhcp-f-acknowledgement) | ACK-1, ACK-2, ACK-4, ACK-5 PASS; ACK-3 PASS for its `must` half; ACK-8 **FAIL** gap 3 | **partial** |
| [DHCP-F-LEASE](../../protocol/dhcp/features.md#dhcp-f-lease) | LEASE-1, LEASE-2, LEASE-3, RFC2132-LEASE-1 all PASS | **supported** |
| [DHCP-F-RENEW](../../protocol/dhcp/features.md#dhcp-f-renew) | LEASE-5, REQ-6 PASS | **supported** |
| [DHCP-F-REBIND](../../protocol/dhcp/features.md#dhcp-f-rebind) | LEASE-6, REQ-7 PASS | **supported** |
| [DHCP-F-EXPIRY](../../protocol/dhcp/features.md#dhcp-f-expiry) | LEASE-8, LEASE-9 PASS | **supported** |
| [DHCP-F-INIT-REBOOT](../../protocol/dhcp/features.md#dhcp-f-init-reboot) | REQ-5 established only on the crafted stimulus | **untested** |
| [DHCP-F-NAK](../../protocol/dhcp/features.md#dhcp-f-nak) | NAK-3, NAK-5, NAK-6 all untested because gap 10 sent no DHCPNAK; NAK-8 has no check | **untested** |
| [DHCP-F-DECLINE](../../protocol/dhcp/features.md#dhcp-f-decline) | DECL-2, DECL-5, DECL-6 all untested because gap 11 finds no conflict; DECL-4 has no check | **untested** |
| [DHCP-F-RELEASE](../../protocol/dhcp/features.md#dhcp-f-release) | REL-1 **FAIL** gap 12; REL-3 and REL-5 untested; REL-4 has no check | **not supported** |
| [DHCP-F-INFORM](../../protocol/dhcp/features.md#dhcp-f-inform) | INF-2 **FAIL** gap 9; INF-3 untested; INF-1 and INF-5 established only on the crafted stimulus | **not supported** |
| [DHCP-F-RETRANSMISSION](../../protocol/dhcp/features.md#dhcp-f-retransmission) | RETX-3 PASS; RETX-1 **FAIL** gap 13; RETX-4 **FAIL** gap 14 | **partial** |
| [DHCP-F-REPLY-DELIVERY](../../protocol/dhcp/features.md#dhcp-f-reply-delivery) | BCAST-3, BCAST-4 PASS; BCAST-5 **FAIL** gap 7 | **partial** |
| [DHCP-F-SERVER-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-server-identity) | SRVID-2, SRVID-3, OFF-8, RFC2132-SRVID-1 all PASS | **supported** |
| [DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity) | REQ-9 PASS; RFC6842-CLID-1 **FAIL** gap 5; RFC6842-CLID-3 **FAIL** gap 6; CLID-2 and ID-2 have no check | **partial** |
| [DHCP-F-PARAMETERS](../../protocol/dhcp/features.md#dhcp-f-parameters) | SEL-4, REQ-8, RFC2132-PRL-1 PASS; SEL-5 **FAIL** gap 4 | **partial** |
| [DHCP-F-ADDRESS-SELECTION](../../protocol/dhcp/features.md#dhcp-f-address-selection) | SEL-1 and SEL-2 both have no check | **untested** |
| [DHCP-F-DUPLICATE-DETECTION](../../protocol/dhcp/features.md#dhcp-f-duplicate-detection) | DECL-1 **FAIL** gap 11; OFF-7 has no check | **not supported** |

Twenty-four features: **10 supported, 7 partial, 3 not supported, 4 untested**.

What the four groups have in common is worth seeing, because it says something about the model
that no single row does.

1. **The normal path works.** Every feature of the ordinary exchange is supported: the message
   format, the transport, the option encoding, the discovery, the selection, the lease and both
   of its timers, the expiry, and the server's identity. A client gets an address, keeps it,
   renews it, rebinds it and gives it up at the right instants, and every field and option that
   RFC 2131 marks mandatory in those messages is there. The two reacquisition features and the
   expiry needed a relay on the path to reach at all, and all three passed.
2. **The seven partial features fail on a detail of one field, not on a mechanism.** A wrong
   `giaddr`, a wrong `ciaddr`, a constant `flags`, a new transaction identifier, an option with
   no value, a reply broadcast when it should be a unicast. Each one is a line of code and none
   of them stops an exchange.
3. **The three unsupported features are the three messages the server does not handle**, plus
   the client probe that would produce the fourth. DHCPRELEASE, DHCPINFORM and the duplicate
   detection are absent by decision, and two of the three carry a TODO that says so.
4. **The four untested features are untested because of gaps 9, 10 and 11.** Not one of them is
   untested for want of a check: a check exists for each, it ran, and the model's behavior
   removed the stimulus the check needed. `DHCP-F-NAK` is the clearest case — the model has a
   working `sendNak` that this pass could not reach, because the one request that should draw a
   DHCPNAK draws silence instead.

Point 4 is the coverage lesson of the pass: **a model gap can hide a feature from every check
that targets it.** A ledger that recorded those twelve statements as FAIL would claim knowledge it
does not have, and one that left them out would lose them.

## Achieved level

**Level 3 partial.** Target: level 3, from
[`standards.md`](../../protocol/dhcp/standards.md#target-level).

The exit criterion of level 3 is that the catalogs hold every mandatory statement of the
in-scope documents, including each `MUST NOT`, and that each one has a check.

**Hold** — satisfied. The catalogs hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and
`MAY` of RFC 2131 that states a behavior of a client or a server, every field value of its four
tables, the whole of RFC 2132 §2 and §9 with the two RFC 1497 extensions an IPv4 host cannot
work without, and all three statements of RFC 6842. What is left out is named with a reason at
the end of each catalog, and the largest exclusion — the sixty per-parameter option definitions
of RFC 2132 — is level 5 material that a serializer unit test owns.

**Does not hold** — the second half. 94 of the 132 statements have a check, and of those 12
could not be read. 26 carry `no check` and 12 carry `later`. Four groups block the level, and
they are different in kind:

1. **Twelve statements whose check a model gap disarmed.** These are the ones to fix first, and
   the fix is in the model and not in the tests. Close gap 9, gap 10 or gap 11 and twelve rows
   change from `untested` to a verdict without a line of test code changing. Gap 10 is a defect and
   is already failing the run, so it is the cheapest of the three.
2. **Twelve statements that need a second client, a second exchange or a second interface in the
   mockup.** The address selection rules, the server's half of the decline and the release, the
   server's use of the client identifier as a key, the uniqueness rules, and the multi-homing
   rule. All twelve are reachable at level 3 with a richer mockup; none needs a new tool.
3. **Twelve statements that need a tool this level does not have**: five a statistical check with
   a stated tolerance, which is level 4 by the guide's own table, and seven a serializer unit
   test.
4. **Two statements that need a relay agent**, which brings RFC 1542 into the in-scope set and is
   level 5 work, plus three that name an option the model has no field for and nine that a check
   of two nodes on a link cannot observe at all.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins an in-scope set of three documents chosen from the RFC-editor register and not from the model's claim; [`conformance.md`](conformance.md) maps the claim onto it and names what the claim lacks. |
| 2, Core | **reached** | All 24 features are written from the standards, and every one of the 19 mandatory features has a core check that ran and has a verdict. The normal path of the four-message exchange, the lease and both timers passes end to end. |
| 3, Edge | **partial** | Every mandatory statement of the three documents is in the catalogs, including each `MUST NOT`. Ten checks need a fault or a crafted message and all ten ran: three inject a message at a server, two inject a crafted reply at a client, three remove a reply from the path, and two ask a server the same question with a field at its two values. Fourteen model gaps found. Twelve statements have a check that a gap disarmed. |
| 4, Dynamics | not started | Five statements state a distribution: the retransmission backoff, the start delay, the renewal wait, the fuzz on T1 and T2, and the pause after a decline. Gap 13 already shows what the first of them will say. |
| 5, Complete | not started | RFC 951, RFC 1542 and the relay agent; RFC 3396 and long options; RFC 4361 and the DUID form of the client identifier; the sixty per-parameter options of RFC 2132. |

The honest summary of the level: the **catalog** side of level 3 is finished and the **run**
side is not, and what blocks the run side is mostly the model rather than the toolset. That is
the opposite of the UDP pass, where the toolset was the bound.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-11 | **3, partial** | The first pass for this protocol, and it covers levels 1 to 3 in one go, because DHCP had no earlier pass. In-scope set RFC 2131, RFC 2132 and RFC 6842; 132 catalog entries; 24 features; 26 checks; 26 tests | 26 tests: 13 PASS, 8 FAIL (expected), 5 FAIL (unexpected), so the suite reports FAIL. 14 model gaps: 6 defects and 8 unimplemented features. 10 features supported, 7 partial, 3 not supported, 4 untested; 12 statements untested because a gap disarmed their check; 26 carry `no check` and 12 `later`, each with a reason; see [`results.md`](results.md) |

Six checks of this pass were **split** out of a check the plan named, every one of them for the
same reason: a wrong field would otherwise have taken the verdict of its neighbours with it.
Recording them here keeps the count of 26 checks honest against the 20 the plan started with.

| Split-out check | Split from | What the split saved |
| --- | --- | --- |
| [transaction-identifier-through-the-exchange](../../protocol/dhcp/checks/exchange.md#transaction-identifier-through-the-exchange) | address-allocation-exchange | Gap 1 stops the program at the DHCPREQUEST. Without the split the verdict on the DHCPACK and on the negative observation would have been lost. |
| [address-fields-of-a-server-reply](../../protocol/dhcp/checks/exchange.md#address-fields-of-a-server-reply) | offer-contents, acknowledgement-contents | Gaps 2 and 3 are in two address fields. Without the split, the four mandatory options of both replies would have had no verdict. |
| [a-parameter-the-server-has-no-value-for](../../protocol/dhcp/checks/parameters.md#a-parameter-the-server-has-no-value-for) | requested-parameters-returned | Gap 4 is in the prohibition. Without the split, the four requirements of the same rule would have had no verdict. |
| [reply-to-a-client-that-clears-the-broadcast-bit](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-clears-the-broadcast-bit) and [-that-sets-the-broadcast-bit](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-sets-the-broadcast-bit) | broadcast bit honoured | One verdict for the pair would have said "fails" and hidden the finding that matters: the server broadcasts whichever value the bit carries, so it is not reading the bit at all. |
| [the-flags-field-of-a-server-reply](../../protocol/dhcp/checks/reply-delivery.md#the-flags-field-of-a-server-reply) | reply-to-a-client-that-sets-the-broadcast-bit | Gap 8 is in the field and gap 7 is in the destination. Without the split, the one thing the server gets right about a set bit would have had no verdict. |

## Out of scope

What this pass left out, and where each part is recorded:

- **RFC 951 and RFC 1542**, the BOOTP base and the relay agent, with the reason in
  [`standards.md`](../../protocol/dhcp/standards.md#in-scope-set). Their absence costs two
  statements of RFC 2131 that no mockup here can reach, NAK-4 and BCAST-2.
- **RFC 3396, RFC 4361, RFC 5494, RFC 3442, RFC 3942 and RFC 4833**, all level 5, each with a
  reason in the standards map.
- **The sixty per-parameter option definitions of RFC 2132**, level 5, with the reason at the
  end of [`rfc2132/catalog.md`](../../standard/rfc2132/catalog.md#out-of-scope-in-this-catalog).
  Two of them are in scope, the subnet mask and the router, and only because an IPv4 host cannot
  use an address without them.
- **The twelve `later` statements** above: five for level 4 and seven for a serializer unit test
  suite that this tree does not yet have.
