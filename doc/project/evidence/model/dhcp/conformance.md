# DHCP — model claims and conformance matrix

> **Kind:** what · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/dhcp/standards.md), [features.md](../../protocol/dhcp/features.md), [coverage.md](coverage.md), [results.md](results.md)

Step 8 artifact of the standards test workflow. The tests say what the model does; this
document adds what the model **intends**, and compares the two. Part 1 collects the standards
the model claims to implement, from the model itself. Part 2 cross-checks those claims against
the measured feature support, feature by feature and not test by test.

The ledger state this matrix comes from:

- Date: 2026-09-23 18:26 +0200
- INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/dhcp$'`
- Ledger: [`coverage.md`](coverage.md), achieved level 3 partial, 14 features supported, 6
  partial, 2 not supported, 2 untested, and 14 claimed statements owed a check.
- Suite: 26 tests, 21 PASS, 5 FAIL (expected), 0 FAIL (unexpected). None are undeclared now; the
  five declared ones are behavior the model does not claim. See
  [`results.md`](results.md#the-rule-that-decides-the-declaration).

## Part 1 — what the model claims

Part 1 was read on 2026-09-11 and this refresh did not read the claims again.

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
| [DHCP-F-TRANSACTION-ID](../../protocol/dhcp/features.md#dhcp-f-transaction-id) | mandatory | yes | supported | **confirmed** — XID-3 (gap 1) repaired 2026-09-15, `b72abc4696` |
| [DHCP-F-OFFER](../../protocol/dhcp/features.md#dhcp-f-offer) | mandatory | yes | supported | **confirmed** — OFF-6 (gaps 2 and 8) repaired 2026-09-15, `b72abc4696` |
| [DHCP-F-ACKNOWLEDGEMENT](../../protocol/dhcp/features.md#dhcp-f-acknowledgement) | mandatory | yes | partial | **partial** — gap check RFC2131-ACK-3 (gap 9); ACK-8 (gaps 3 and 8) repaired 2026-09-15, `b72abc4696` |
| [DHCP-F-RETRANSMISSION](../../protocol/dhcp/features.md#dhcp-f-retransmission) | mandatory | yes | supported | **confirmed** — RETX-1 and RETX-4 (gaps 13 and 14) repaired 2026-09-15, `b72abc4696` |
| [DHCP-F-REPLY-DELIVERY](../../protocol/dhcp/features.md#dhcp-f-reply-delivery) | mandatory | yes | partial | **partial** — gap check RFC2131-BCAST-5 (gap 7), and BCAST-6 with it |
| [DHCP-F-PARAMETERS](../../protocol/dhcp/features.md#dhcp-f-parameters) | mandatory | yes | supported | **confirmed** — SEL-5 (gap 4) repaired 2026-09-15, `b72abc4696` |
| [DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity) | mandatory | **no**, for the governing half | partial | **out of claim** — the two failing checks, RFC6842-CLID-1 (gap 5) and CLID-3 (gap 6), are governed by RFC 6842, which the model does not claim |
| [DHCP-F-INFORM](../../protocol/dhcp/features.md#dhcp-f-inform) | optional | yes | not supported | **declined** — the server handles no DHCPINFORM (gap 9) |
| [DHCP-F-RELEASE](../../protocol/dhcp/features.md#dhcp-f-release) | optional | yes | not supported | **declined** — the client sends no DHCPRELEASE (gap 12), with a TODO that states why |
| [DHCP-F-DUPLICATE-DETECTION](../../protocol/dhcp/features.md#dhcp-f-duplicate-detection) | optional | yes | partial | **partial** — the client's probe, DECL-1 (gap 11), passes since `bdde132792`; the server's probe, OFF-7, has no code |
| [DHCP-F-NAK](../../protocol/dhcp/features.md#dhcp-f-nak) | mandatory | yes | partial | **partial** — NAK-3, NAK-5 and NAK-6 (gap 10) repaired 2026-09-15, `b72abc4696`; NAK-8 is still owed a check |
| [DHCP-F-DECLINE](../../protocol/dhcp/features.md#dhcp-f-decline) | mandatory | yes | partial | **partial** — DECL-2, DECL-5 and DECL-6 (gap 11) pass since `bdde132792`; the same commit gave the server the code of DECL-4, which is `owed` |
| [DHCP-F-INIT-REBOOT](../../protocol/dhcp/features.md#dhcp-f-init-reboot) | optional | yes | untested | **unverified** — a check is owed; the model has the state machine and this pass wrote no check of it |
| [DHCP-F-ADDRESS-SELECTION](../../protocol/dhcp/features.md#dhcp-f-address-selection) | optional | yes | untested | **unverified** — two checks are owed; the model has the preference code |

Twenty-four features: **14 confirmed, 5 partial, 0 defect, 2 declined, 2 unverified, 1 out of claim**.

### How to read the four groups

**Zero `defect` verdicts. Both are repaired.** The word means one thing here and a different
thing in [`results.md`](results.md#the-rule-that-decides-the-declaration): in this matrix a
`defect` is a **whole mandatory feature** that the model claims and does not support, while there
a defect is one statement. The first pass found two feature-level defects, `DHCP-F-NAK` and
`DHCP-F-DECLINE`, and both had the same shape: **the mechanism was written and the trigger never
fired.** `DhcpServer::sendNak` built a correct DHCPNAK and the branch that would send it for a
foreign subnet was unreachable, because the INIT-REBOOT code tested its two conditions in the
wrong order. `DhcpClient::sendDecline` built a correct DHCPDECLINE and nothing called it, because
the address probe that would find a conflict was a commented-out block.

**Both are repaired now.** `b72abc4696` (2026-09-15) reorders the INIT-REBOOT checks, so
the DHCPNAK for a foreign subnet fires; `bdde132792` (2026-09-15) gives the client the ARP probe
RFC 2131 §4.4.1 asks for, so `sendDecline` gets a caller. `DHCP-F-DECLINE` reads `partial` now: every
check passes, and DECL-4 is `owed`, because the same commit gave the server its half.
`DHCP-F-NAK` reads `partial`, not `confirmed`, because one of its four core checks, `NAK-8`, is
still `owed`: nobody has written the check yet, the mechanism is not at fault.

An earlier version of this matrix read both as `unverified`, on the ground that a gap elsewhere had
removed the stimulus their checks needed. That was wrong twice over: the checks did run and did
fail, so the run answered the question; and `unverified` reads as "nobody looked", which is the
opposite of what happened. The rest of the matrix stands: every mandatory feature is now confirmed
or partial, except the one whose governing document the model does not claim. The model does what
it says it does, and after 2026-09-15 it does more of it.

**The five `partial` verdicts have three shapes now, not one.** `DHCP-F-ACKNOWLEDGEMENT`
fails its `must not` half alone, gap 9, unrepaired: the server still drops DHCPINFORM outright, so
the reply that half would read never exists. `DHCP-F-REPLY-DELIVERY` fails the broadcast-bit-clear
case alone, gap 7, a TODO
that names its own limit: the application cannot address a reply to a client's hardware address.
`DHCP-F-NAK` fails no check at all; it is held at `partial` solely by `NAK-8`, a check this pass
still owes, and `DHCP-F-DECLINE` has the same shape with `DECL-4`. `DHCP-F-DUPLICATE-DETECTION`
has the third: its client half passes and its server half has no code. The first pass's six `partial` verdicts were each a single field or option in an
otherwise correct mechanism — a transaction identifier redrawn, a `giaddr` holding a gateway, a
`ciaddr` holding a lease, a `flags` field written as a constant, an option returned with no value,
a reply broadcast where a unicast belonged. The list left out the sixth, `DHCP-F-RETRANSMISSION`,
whose fault was a timing law rather than a field. All four of the field faults and the timing law
are repaired now, so four of the six read `confirmed`: `b72abc4696` fixed all of them on
2026-09-15.

**The two `declined` verdicts are honest and both are documented.** DHCPINFORM and DHCPRELEASE are
absent by decision. `DhcpClient.cc:724-725` says why the release is absent and quotes RFC 2131
§4.4.6 correctly. Both statements are a `MAY`, so no rule is broken. Duplicate detection left this
group on 2026-09-15: `DHCP-F-DUPLICATE-DETECTION` is `partial` now, because the client's half,
`DECL-1`, is implemented and its check passes, while the server's own pre-offer probe, `OFF-7`,
has no code.

**The two `unverified` verdicts are the coverage debt of the pass, and neither is the model's
doing.** `DHCP-F-INIT-REBOOT` has a state machine in the model and this pass wrote no check of it;
address selection has the preference code and no check of it. Both are owed a check, both are
reachable with a richer mockup and no new tool, and
[the debt table of the ledger](coverage.md#the-coverage-debt-the-checks-this-pass-owes) says
what each needs. An `unverified` verdict is a statement about the pass and never about the model.

**The one `out of claim`** is [DHCP-F-CLIENT-IDENTITY](../../protocol/dhcp/features.md#dhcp-f-client-identity),
and it is a documentation task before it is a code task; see below.

## Headlines for the next pass

The guide says a `defect`, and an `unverified` feature with level `mandatory`, are the headlines.
There is no defect any more and no mandatory unverified feature. Items 1 and 2 below no longer
hold; they stay in the list, each marked with the commit that repaired it, because a headline that
stopped holding is kept, not deleted. Item 3 leads what remains.

1. **Close gap 10, and DHCP-F-NAK leaves `defect`.** The fix is the order of two tests in the
   INIT-REBOOT branch of `DhcpServer::processDhcpMessage`: test the subnet before the table of
   leases, as RFC 2131 §4.3.2 states it. Four ledger rows change from FAIL to a real verdict and a
   mandatory feature stops being a defect, with no change to any test. — **repaired by
   `b72abc4696`**, 2026-09-15; `DHCP-F-NAK` reads `partial` now, held there only by `NAK-8`, still
   `owed`.
2. **Close gap 11, and DHCP-F-DECLINE leaves `defect`.** The code is written; the probe that
   triggers it is a comment. Four more ledger rows change, and a second mandatory feature stops
   being a defect. — **repaired by `bdde132792`**, 2026-09-15; `DHCP-F-DECLINE` reads `partial`
   now.
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

5. **Write the thirteen checks the pass owes**, from
   [the debt table](coverage.md#the-coverage-debt-the-checks-this-pass-owes). Four mockups
   clear eleven of them, and two features leave `unverified`. This is the largest of the five items
   and the only one that is test work rather than model work.
