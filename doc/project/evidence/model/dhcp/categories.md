# DHCP — test category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test-anatomy.md](../../../design/test-anatomy.md), [testing.md](../../../rule/testing.md), [coverage.md](coverage.md)

Step 9 artifact of the standards test workflow. Which test category each check belongs to, and
why. The categories and what each can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the rule that the category
must match the claim is [TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

This document records a decision and not a run, so it carries no run record.

The usual mapping from the observation class that step 3 recorded to a category:

| Observation class | Usual category |
| --- | --- |
| wire, end-to-end, error-signal | protocol test |
| encoding, algorithm on one message | unit test |
| internal state with a scalar signal | protocol test with a state-signal step |
| internal state without a signal | module test |
| timer distributions, throughput bounds | statistical test |
| whole-trajectory regression lock | fingerprint test |

## The decision, per check

All 26 checks of this pass are **protocol tests**, and all 26 are in `tests/protocol/dhcp/`.
The prediction that step 3 made from the observation class held for every one of them, which is
worth stating because it did not have to: DHCP is an application-layer protocol whose whole
behavior is a sequence of messages between two nodes, so `wire` and `end-to-end` dominate its
catalogs and both map to a protocol test.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) | wire | protocol test | Four messages between two nodes, read as a sequence. |
| [transaction-identifier-through-the-exchange](../../protocol/dhcp/checks/exchange.md#transaction-identifier-through-the-exchange) | wire | protocol test | One field followed across four messages of one exchange. |
| [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) | wire | protocol test | Fields and absent options of one message on a link. |
| [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) | wire | protocol test | The same, plus a comparison against the message it answers. |
| [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) | wire | protocol test | The same. |
| [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents) | wire | protocol test | The same. |
| [address-fields-of-a-server-reply](../../protocol/dhcp/checks/exchange.md#address-fields-of-a-server-reply) | wire | protocol test | Two fields of two replies, against the two requests. |
| [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) | **encoding** | protocol test, by exception | See the note below. |
| [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) | wire | protocol test | A message at a stated instant, with stated fields and a stated destination. |
| [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) | wire | protocol test | The same, with a fault on the path to reach the state. |
| [lease-expiry](../../protocol/dhcp/checks/lease.md#lease-expiry) | wire plus end-to-end | protocol test | A message at the expiry, and an absence of traffic from the expired address. |
| [nak-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) | wire | protocol test | A crafted request and the reply it draws. |
| [silence-for-an-unknown-client](../../protocol/dhcp/checks/nak.md#silence-for-an-unknown-client) | wire | protocol test | A crafted request and an absence of any reply. |
| [client-identifier-echoed](../../protocol/dhcp/checks/client-identity.md#client-identifier-echoed) | wire | protocol test | One option compared between a request and two replies, for two clients. |
| [foreign-client-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-client-identifier-discarded) | end-to-end | protocol test | A crafted reply and an absence of the two things a client would send if it had accepted it. |
| [foreign-transaction-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-transaction-identifier-discarded) | end-to-end | protocol test | The same, with two crafted replies. |
| [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) | wire | protocol test | An address resolution request, a reply to it, and the DHCP message that follows. |
| [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) | wire | protocol test | A crafted request and the fields and absent options of the reply. |
| [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) | wire | protocol test | A message at a scenario event, with stated fields. |
| [broadcast-bit-clear](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-clears-the-broadcast-bit) | wire | protocol test | The two destination addresses of two replies. |
| [broadcast-bit-set](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-sets-the-broadcast-bit) | wire | protocol test | The same, for the other value of the bit. |
| [the-flags-field-of-a-server-reply](../../protocol/dhcp/checks/reply-delivery.md#the-flags-field-of-a-server-reply) | wire | protocol test | One field of a reply, against the request. |
| [discover-repeated-without-a-server](../../protocol/dhcp/checks/retransmission.md#discover-repeated-without-a-server) | wire, as a time | protocol test, with a bound | See the note below. |
| [request-repeated-when-the-reply-is-lost](../../protocol/dhcp/checks/retransmission.md#request-repeated-when-the-reply-is-lost) | wire | protocol test | A repeated message, and an absence that separates a repetition from a restart. |
| [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) | wire | protocol test | Options of two replies, and the order of their octets. |
| [a-parameter-with-no-value](../../protocol/dhcp/checks/parameters.md#a-parameter-the-server-has-no-value-for) | wire | protocol test | Two absent options of two replies. |

### The one exception: an encoding check in the protocol suite

[message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire)
carries nine statements whose observation class is `encoding`, and the table above sends an
`encoding` statement to a unit test. This check is a protocol test anyway, and the decision is
deliberate.

The reason is what the check actually does. A unit test of a serializer builds a message, writes
it, reads it back, and compares — it establishes that the writer and the reader agree with each
other. That is not the question RFC 2132 §2 asks. The question is whether the octets that
**travel** parse by the rule "tag, length, value" from the first octet of the option area to an
`end` option with nothing left over, and that is a property of a real message produced by a real
exchange. So the check observes the four messages of an exchange on a link, writes their octets,
and walks the option area the way a receiver's parser would.

The cost is that the check cannot reach the encoding statements that no message of an ordinary
exchange carries: the option overload rules, the pad option, the trailing-null rule of a text
option, the site-specific code range, and the two options that replace an overloaded field. Those
seven are exactly the `later (unit test)` rows of the ledger, and a serializer unit test suite is
their home. This tree has no such suite for DHCP yet, and creating one is the seventh sharpening
candidate of [`results.md`](results.md#sharpening-candidates-for-the-next-pass).

### The one check with a timing bound, and why it is not a statistical test

[discover-repeated-without-a-server](../../protocol/dhcp/checks/retransmission.md#discover-repeated-without-a-server)
reads two intervals and compares them. A timing statement is usually a statistical test, and
this one is not, because of what it asserts and what it refuses to assert.

It asserts that the second interval is **longer than** the first. That is an ordering of two
observed values, and one run establishes or refutes it. It deliberately asserts no bound on
either instant: RFC 2131 §4.1 names 4 seconds, then 8, doubling to at most 64, each with a fuzz
of one second, and those are the parameters of a distribution. A check that named them would
fail on an unlucky draw, and a check that widened its window until it could not fail would
establish nothing.

So the statement the check carries,
[RFC2131-RETX-1](../../standard/rfc2131/catalog.md#rfc2131-retx-1), is in the protocol suite for
its ordering half, and
[RFC2131-RETX-2](../../standard/rfc2131/catalog.md#rfc2131-retx-2), the figures, is a `later
(statistical)` row of the ledger. The same split applies to the four other timing statements the
ledger files as `later (statistical)`: the start delay
([DISC-4](../../standard/rfc2131/catalog.md#rfc2131-disc-4)), the renewal wait
([LEASE-7](../../standard/rfc2131/catalog.md#rfc2131-lease-7)), the fuzz on T1 and T2
([LEASE-10](../../standard/rfc2131/catalog.md#rfc2131-lease-10)), and the pause after a decline
([DECL-3](../../standard/rfc2131/catalog.md#rfc2131-decl-3)).

The three lease checks also name an instant with a tolerance of one second, and none of them is
a statistical test either. T1 and T2 in those scenarios are fixed fractions of a fixed lease, so
the instant is a value; the tolerance covers the microseconds the first exchange itself needed,
and nothing else.

## The statements that belong in another suite

Twelve statements of the ledger carry `later`, which is the ledger's way of naming a statement
whose category is not the protocol suite. They split into two groups, and this document is where
the target category is recorded.

| Statements | Target category | Why |
| --- | --- | --- |
| [RFC2131-MSG-10](../../standard/rfc2131/catalog.md#rfc2131-msg-10), [RFC2132-OVER-1](../../standard/rfc2132/catalog.md#rfc2132-over-1), [RFC2132-TFTP-1](../../standard/rfc2132/catalog.md#rfc2132-tftp-1), [RFC2132-BOOTF-1](../../standard/rfc2132/catalog.md#rfc2132-bootf-1), [RFC2132-PAD-1](../../standard/rfc2132/catalog.md#rfc2132-pad-1), [RFC2132-FMT-2](../../standard/rfc2132/catalog.md#rfc2132-fmt-2), [RFC2132-FMT-5](../../standard/rfc2132/catalog.md#rfc2132-fmt-5) | **unit test** of the serializer | Seven `encoding` statements about octets that no message of an ordinary exchange carries. A unit test can build such a message; an exchange cannot produce one. |
| [RFC2131-RETX-2](../../standard/rfc2131/catalog.md#rfc2131-retx-2), [DISC-4](../../standard/rfc2131/catalog.md#rfc2131-disc-4), [LEASE-7](../../standard/rfc2131/catalog.md#rfc2131-lease-7), [LEASE-10](../../standard/rfc2131/catalog.md#rfc2131-lease-10), [DECL-3](../../standard/rfc2131/catalog.md#rfc2131-decl-3) | **statistical test** | Five distributions with a random part. Each one needs many runs and a stated tolerance, which the guide files at level 4. |

The catalog entries of all twelve stay where they are and keep their IDs. A later pass writes
the test in the other suite; nothing in the catalogs or the feature map changes when it does.

## A category this pass did not need

Nine statements carry `no check` with the reason that nothing on a link can see them, and the
usual mapping would send the `internal` ones among them to a **module test** — a test that
watches a gate inside a node rather than a link between two nodes. This pass wrote none, and the
reason is that a module test is not the cheapest way to reach them.

Take [RFC2131-ID-2](../../standard/rfc2131/catalog.md#rfc2131-id-2), the rule that a server keys
a lease on the `client identifier` when there is one and on `chaddr` otherwise. A module test
could read the server's table of leases directly. A protocol test with a **second exchange** can
see the same thing from the link: change the hardware address, keep the identifier, ask again,
and read which address comes back. The second is a check of the behavior and the first is a check
of a data structure, and the guide's rule that a category must match the claim prefers the
former.

Twelve of the twenty-six `no check` statements are of that shape, and the ledger records them as
reachable at level 3 with a richer mockup and no new tool. The ones a module test would genuinely
be needed for are the three that name an option the model's message class has no field for, and
those are a model change before they are a test.
