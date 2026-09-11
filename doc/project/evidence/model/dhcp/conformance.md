# DHCP — model claims and conformance matrix

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/dhcp/standards.md), [features.md](../../protocol/dhcp/features.md), [coverage.md](coverage.md), [results.md](results.md)

Step 8 artifact of the standards test workflow. The tests say what the model does; this
document adds what the model **intends**, and compares the two. Part 1 collects the standards
the model claims to implement, from the model itself. Part 2 cross-checks those claims against
the measured feature support, feature by feature and not test by test.

The ledger state this matrix comes from:

- Date: 2026-09-11 11:25 +0200
- INET: branch `topic/rfc-tests-dhcp-level3`, commit `4e20c74e82`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w dhcp`
- Ledger: [`coverage.md`](coverage.md), achieved level 3 partial, 10 features supported, 7
  partial, 3 not supported, 4 untested.
- Suite: 26 tests, 13 PASS, 8 FAIL (expected), 5 FAIL (unexpected). The five undeclared failures
  are the six defects of [`results.md`](results.md#the-class-of-every-failure); the eight declared
  ones are features the model does not implement.

## Part 1 — what the model claims

### The claims, with their source

| Claim | Where | Exact words |
| --- | --- | --- |
| RFC 2131 | `src/inet/applications/dhcp/DhcpClient.ned:15-17` | "Implements the DHCP client protocol. DHCP (Dynamic Host Configuration Protocol), described in RFC 2131, provides configuration parameters to Internet hosts." |
| RFC 2131 | `src/inet/applications/dhcp/DhcpServer.ned:15-17` | "Implements the DHCP server protocol. DHCP (Dynamic Host Configuration Protocol), described in RFC 2131, provides configuration parameters to Internet hosts." |
| RFC 2131 | `src/inet/applications/dhcp/DhcpMessage.msg:90-91` | "Represents a DHCP message. DHCP (Dynamic Host Configuration Protocol, RFC 2131) provides a framework for passing configuration information to hosts on a TCP/IP network." |
| RFC 2131 **and RFC 2132** | `WHATSNEW:4776-4777`, under the heading `INET-2.4.0 (June 12, 2014)` at `WHATSNEW:4741` | "The whole implementation has been reviewed to bring it closer to the standards defined in RFC 2131 and 2132." |
| RFC 2131 §4.4.5 | `DhcpMessage.msg:51`, `DhcpClient.cc:684`, `:690`, `DhcpServer.cc:351`, `:433` | "DHCP timer types (RFC 2131 4.4.5)" and four `// RFC 4.4.5` comments on the T1 and T2 computations |
| RFC 2131 figure 5 | `DhcpClient.h:29` | "DHCP client states (RFC 2131, Figure 5: state transition diagram)" |
| RFC 2131 §4.1, §4.3.1, §4.3.2, §4.3.6, §4.4.3 | `DhcpServer.cc:175`, `:200`, `:314`, `:378`, `:459`; `DhcpClient.cc:45`, `:547` | Section citations on the message dispatch, on the three copies of the reply-delivery rule of §4.1, and on the response timeout |
| RFC 1497 | `DhcpMessageSerializer.cc:76`, `:257` | "is the same magic cookie as is defined in RFC 1497 [17])", quoted from RFC 2131 §3 |

### What the claims come to

The claim is **RFC 2131 and RFC 2132**, and it is stated at four levels: in the NED
documentation of both applications, in the message definition, and in the release notes. The
section citations are dense and accurate — every one of the eight sections named above is the
right section for the code it sits on — which makes this a model that was written against the
text and not against a summary of it.

Mapped onto the in-scope set of [`standards.md`](../../protocol/dhcp/standards.md#in-scope-set):

| In-scope document | Claimed | Note |
| --- | --- | --- |
| RFC 2131 | **yes**, four times | The base document. The claim is unqualified. |
| RFC 2132 | **yes**, once, in the release notes | Not claimed in either NED file. The options the model implements are exactly thirteen of RFC 2132, enumerated in `DhcpMessage.msg:35-49` as the `DhcpOptionCode` enum. |
| RFC 6842 | **no** | Named nowhere in the model. |

### The gap between the claim and the register

This is the finding of the level 1 survey, and it is the reason the guide says to choose the
in-scope set from the standards register and never from what the model claims.

**The model claims RFC 2131 and never names RFC 6842.** The RFC-editor metadata for RFC 2131
lists four documents that update it — RFC 3396, RFC 4361, RFC 5494 and RFC 6842 — and one of
them, RFC 6842, reverses a `MUST NOT` of the base text into a `MUST`. A model that implements
RFC 2131 exactly, as this one intends to, is therefore **guaranteed** to fail RFC 6842, and
the run confirms it: gaps 5 and 6 of [`results.md`](results.md#the-model-gaps) are both halves
of RFC 6842 and neither is implemented.

So the two RFC 6842 failures are not defects in the ordinary sense. They are the cost of a
claim that was accurate in 2014 and is thirteen years out of date now. The verdict column below
records them as `out of claim` and not as `defect`, and the recommendation that follows is a
documentation change first: either name RFC 6842 and implement it, or state in the NED
documentation that the model implements RFC 2131 without its updates.

**The three other updating documents are out of the in-scope set** of this pass, with reasons in
the standards map. They are a scope gap for a later pass and not a verdict: RFC 3396 (long
options), RFC 4361 (the DUID form of the client identifier) and RFC 5494 (an IANA registry
change).

### One claim the model makes about its own limits

`WHATSNEW:4789-4790`, in the same release note:

> "Limitation: The client module currently does not support multiple DHCP servers and BOOTP
> relay agents."

This is worth recording beside the claims, because it is a **correct and useful** statement of
scope, and because it agrees exactly with what this pass found. The relay agent half explains
why [RFC2131-NAK-4](../../standard/rfc2131/catalog.md#rfc2131-nak-4) and
[RFC2131-BCAST-2](../../standard/rfc2131/catalog.md#rfc2131-bcast-2) carry `no check`: the
statements need a node type the model does not have. The multiple-server half is the reason
several statements of the ledger need a richer mockup, and it also explains gap 1 in part: a
client that never sees two servers has no visible reason to keep one transaction identifier.

A limitation stated in the release notes is not a claim to conform, so the matrix treats the
features these limits cover as `out of claim` rather than as defects.

## Part 2 — the conformance matrix

The rule of step 8, from the guide:

| Level | Claimed | Support | Verdict |
| --- | --- | --- | --- |
| any | yes | supported | `confirmed` |
| any | yes | partial | `partial` — list the gap checks |
| mandatory, unstated | yes | not supported | `defect` |
| optional | yes | not supported | `declined` |
| any | yes | untested | `unverified` |
| any | no | supported | `undocumented` |
| any | no | partial, not supported, or untested | `out of claim` |

A feature is `claimed` when the claims of part 1 cover its **governing** source document. Every
feature of this protocol is governed by RFC 2131 or RFC 2132 except
[DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity), whose
governing document is RFC 6842 for the two statements RFC 6842 overrides.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [DHCP-F-MESSAGE-FORMAT](../../protocol/dhcp/features.md#dhcp-f-message-format) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-TRANSPORT](../../protocol/dhcp/features.md#dhcp-f-transport) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-OPTION-ENCODING](../../protocol/dhcp/features.md#dhcp-f-option-encoding) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-DISCOVERY](../../protocol/dhcp/features.md#dhcp-f-discovery) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-SELECTION](../../protocol/dhcp/features.md#dhcp-f-selection) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-LEASE](../../protocol/dhcp/features.md#dhcp-f-lease) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-RENEW](../../protocol/dhcp/features.md#dhcp-f-renew) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-REBIND](../../protocol/dhcp/features.md#dhcp-f-rebind) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-EXPIRY](../../protocol/dhcp/features.md#dhcp-f-expiry) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-SERVER-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-server-identity) | mandatory | yes | supported | **confirmed** |
| [DHCP-F-TRANSACTION-ID](../../protocol/dhcp/features.md#dhcp-f-transaction-id) | mandatory | yes | partial | **partial** — gap check RFC2131-XID-3 (gap 1) |
| [DHCP-F-OFFER](../../protocol/dhcp/features.md#dhcp-f-offer) | mandatory | yes | partial | **partial** — gap check RFC2131-OFF-6 (gaps 2 and 8) |
| [DHCP-F-ACKNOWLEDGEMENT](../../protocol/dhcp/features.md#dhcp-f-acknowledgement) | mandatory | yes | partial | **partial** — gap check RFC2131-ACK-8 (gap 3) |
| [DHCP-F-RETRANSMISSION](../../protocol/dhcp/features.md#dhcp-f-retransmission) | mandatory | yes | partial | **partial** — gap checks RFC2131-RETX-1 (gap 13) and RETX-4 (gap 14) |
| [DHCP-F-REPLY-DELIVERY](../../protocol/dhcp/features.md#dhcp-f-reply-delivery) | mandatory | yes | partial | **partial** — gap check RFC2131-BCAST-5 (gap 7), and BCAST-6 with it |
| [DHCP-F-PARAMETERS](../../protocol/dhcp/features.md#dhcp-f-parameters) | mandatory | yes | partial | **partial** — gap check RFC2131-SEL-5 (gap 4) |
| [DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity) | mandatory | **no**, for the governing half | partial | **out of claim** — the two failing checks, RFC6842-CLID-1 (gap 5) and CLID-3 (gap 6), are governed by RFC 6842, which the model does not claim |
| [DHCP-F-INFORM](../../protocol/dhcp/features.md#dhcp-f-inform) | optional | yes | not supported | **declined** — the server handles no DHCPINFORM (gap 9) |
| [DHCP-F-RELEASE](../../protocol/dhcp/features.md#dhcp-f-release) | optional | yes | not supported | **declined** — the client sends no DHCPRELEASE (gap 12), with a TODO that states why |
| [DHCP-F-DUPLICATE-DETECTION](../../protocol/dhcp/features.md#dhcp-f-duplicate-detection) | optional | yes | not supported | **declined** — neither side probes (gap 11), and the client's half is written out as a comment |
| [DHCP-F-NAK](../../protocol/dhcp/features.md#dhcp-f-nak) | mandatory | yes | untested | **unverified** — gap 10 removed the stimulus; the model does have a `sendNak` path this pass could not reach |
| [DHCP-F-DECLINE](../../protocol/dhcp/features.md#dhcp-f-decline) | mandatory | yes | untested | **unverified** — gap 11 removed the stimulus |
| [DHCP-F-INIT-REBOOT](../../protocol/dhcp/features.md#dhcp-f-init-reboot) | optional | yes | untested | **unverified** — reached only by a crafted message in this pass |
| [DHCP-F-ADDRESS-SELECTION](../../protocol/dhcp/features.md#dhcp-f-address-selection) | optional | yes | untested | **unverified** — both core statements need a second exchange |

Twenty-four features: **10 confirmed, 6 partial, 3 declined, 4 unverified, 1 out of claim**.

### How to read the four groups

**Not one `defect` verdict in this matrix.** The word means one thing here and a different thing
in [`results.md`](results.md#the-class-of-every-failure), and the two must not be confused. In this
matrix a `defect` is a **whole mandatory feature** that the model claims and does not support, and
there is none: six of the fourteen findings are defects at the level of a *statement*, and every
one of them sits inside a feature that otherwise works, which is why those features read `partial`
and not `defect`. That distinction is the point of a matrix that works on features and not on
tests:
every mandatory feature is either confirmed or partial, except the two that a model gap left
unverified and the one whose governing document the model does not claim. The model does what it
says it does, in the large.

**The six `partial` verdicts are all one shape.** Each one is a single field or a single option
in an otherwise correct mechanism: a transaction identifier redrawn, a `giaddr` holding a gateway,
a `ciaddr` holding a lease, a `flags` field written as a constant, an option returned with no
value, a reply broadcast when it should be a unicast. Five of the six are one line of code. None
of them stops an exchange, which is exactly why they survived: an integration test that watches
whether a client gets an address cannot see any of them, and a conformance check that reads the
field against the table sees all six.

**The three `declined` verdicts are honest and two of them are documented.** DHCPINFORM,
DHCPRELEASE and duplicate detection are absent by decision. `DhcpClient.cc:724-725` says why the
release is absent and quotes RFC 2131 §4.4.6 correctly; `DhcpClient.cc:315-320` writes the
duplicate detection out as a comment with the `SHOULD` above it. All three statements are a `MAY`
or a `SHOULD`, so no rule is broken. What the matrix adds is that the third one, duplicate
detection, is the mechanism DHCP's central promise rests on — that no address serves two clients
at once — so a scenario that models an address conflict cannot use this model as it stands.

**The four `unverified` verdicts are the coverage debt of the pass, and three of them are the
model's own doing.** DHCP-F-NAK and DHCP-F-DECLINE have checks that ran and could not be read,
because gaps 10 and 11 removed the stimulus. That is the most interesting structural finding
here: **a model gap in one place can make a second feature unmeasurable.** The model has a
complete `sendNak` and a complete `sendDecline`; this pass reached neither, and the reason is
not the toolset.

**The one `out of claim`** is [DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity),
and it is a documentation task before it is a code task; see below.

## Headlines for the next pass

The guide says a `defect`, and an `unverified` feature with level `mandatory`, are the headlines.
There is no `defect`, so the headlines are the two mandatory unverified features, and then the
claim.

1. **Close gap 10, and DHCP-F-NAK becomes measurable.** The fix is the order of two tests in the
   INIT-REBOOT branch of `DhcpServer::processDhcpMessage`: test the subnet before the table of
   leases, as RFC 2131 §4.3.2 states it. Four ledger rows change from `untested` to a verdict and
   one mandatory feature leaves `unverified`, with no change to any test.
2. **Close gap 11, and DHCP-F-DECLINE becomes measurable.** The code is written; the probe that
   triggers it is a comment. Four more ledger rows change, and a second mandatory feature leaves
   `unverified`.
3. **Decide what the model claims about RFC 6842.** Two options, and the matrix does not choose
   between them. Either implement it — the server returns the option it received and the client
   compares it, which is a small change on both sides — or state in the NED documentation of both
   applications that the model implements RFC 2131 without the documents that update it. The
   second option costs nothing and makes the `out of claim` verdict correct rather than merely
   true; the first closes gaps 5 and 6.
4. **A documentation task with no code behind it.** RFC 2132 is claimed in a release note from
   2014 and in neither NED file, although the model implements thirteen of its options and cannot
   work without §9.6. Naming it beside RFC 2131 in both NED files would make the claim match the
   code.

Item 4 is the `undocumented` direction of the matrix in a mild form: the model does more than it
claims, and the table above has no row for it because the missing claim is a document and not a
feature.
