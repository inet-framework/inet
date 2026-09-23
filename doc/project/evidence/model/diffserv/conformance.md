# DiffServ — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/diffserv/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the DiffServ family, found with
`grep -rn -E 'RFC ?[0-9]{3,4}|draft-' src/inet/networklayer/diffserv --include=*.ned --include=*.cc --include=*.h --include=*.msg`
(the generated `Dscp_m.h`/`Dscp_m.cc` excluded), plus the user's guide:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2597, RFC 3246 | [`DiffservQueue.ned:19`](../../../../../src/inet/networklayer/diffserv/DiffservQueue.ned) | "This is an example queue that can be used in interfaces of DS core and edge nodes to support the AFxy (RFC 2597) and EF (RFC 3246) PHBs." |
| RFC 2597 | [`AFxyQueue.ned:16`](../../../../../src/inet/networklayer/diffserv/AFxyQueue.ned) | "one class of the Assured Forwarding PHB group (RFC 2597)." |
| RFC 2697 | [`SingleRateThreeColorMeter.ned:13`](../../../../../src/inet/networklayer/diffserv/SingleRateThreeColorMeter.ned), [`SingleRateThreeColorMeter.h:22`](../../../../../src/inet/networklayer/diffserv/SingleRateThreeColorMeter.h) | "Implements a Single Rate Three Color Meter (RFC 2697)." / "See RFC 2697." |
| RFC 2698 | [`TwoRateThreeColorMeter.ned:13`](../../../../../src/inet/networklayer/diffserv/TwoRateThreeColorMeter.ned), [`TwoRateThreeColorMeter.h:22`](../../../../../src/inet/networklayer/diffserv/TwoRateThreeColorMeter.h) | "Implements a Two Rate Three Color Meter (RFC 2698)." / "See RFC 2698." |
| RFC 2475, RFC 3290 | [`MultiFieldClassifier.ned:21`](../../../../../src/inet/networklayer/diffserv/MultiFieldClassifier.ned) | "See RFC 2475 2.3.1, RFC 3290 4.2.2", above the filter-list classifier. |
| RFC 2597 | [`Dscp.msg:19`](../../../../../src/inet/networklayer/diffserv/Dscp.msg) | "Assured Forwarding, RFC 2597", above the six `DSCP_AFxy` enum values. |
| RFC 2598 | [`Dscp.msg:36`](../../../../../src/inet/networklayer/diffserv/Dscp.msg) | "Expedited Forwarding, RFC 2598", above `DSCP_EF`. |
| RFC 2474 | [`Dscp.msg:39`](../../../../../src/inet/networklayer/diffserv/Dscp.msg) | "Class Selector Code Points, RFC 2474", above the seven `DSCP_CSn` enum values. |
| RFC 2474, RFC 2475, RFC 2597, RFC 2697, RFC 2698, RFC 3246, RFC 3290 | [`ch-diffserv.rst:64-79`](../../../../../doc/src/users-guide/ch-diffserv.rst) | The "Implemented Standards" section: "The implementation follows these RFCs:", then one bullet per document, each with its title, at lines 66, 69, 71, 73, 75, 77 and 79. |
| RFC 2475, RFC 3290 | [`ch-diffserv.rst:350`](../../../../../doc/src/users-guide/ch-diffserv.rst) | "More information about classifiers can be found in RFC 2475 and RFC 3290." |

Mapped onto the standards map, [`standards.md`](../../protocol/diffserv/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 2474 | yes, `base` | **yes**, in the user's guide list and in `Dscp.msg` | `ch-diffserv.rst:66`, `Dscp.msg:39`. Current, not superseded. |
| RFC 2475 | yes, `base` | **yes**, in the user's guide list, twice, and in `MultiFieldClassifier.ned` | `ch-diffserv.rst:69,350`, `MultiFieldClassifier.ned:21`. |
| RFC 2597 | yes, `base` | **yes**, in the user's guide list and in two modules | `ch-diffserv.rst:71`, `DiffservQueue.ned:19`, `AFxyQueue.ned:16`, `Dscp.msg:19`. |
| RFC 3246 | yes, `base` | **yes**, in the user's guide list and in `DiffservQueue.ned` | `ch-diffserv.rst:77`, `DiffservQueue.ned:19`. Current, not superseded — and see the inconsistency below. |
| RFC 2697 | yes, `base` | **yes**, in the user's guide list and in the meter module | `ch-diffserv.rst:73`, `SingleRateThreeColorMeter.ned:13`, `.h:22`. |
| RFC 2698 | yes, `base` | **yes**, in the user's guide list and in the meter module | `ch-diffserv.rst:75`, `TwoRateThreeColorMeter.ned:13`, `.h:22`. |
| RFC 3260 | yes, `updates` | **no** | Nothing names it, although the model's own EF citation (RFC 3246) already reflects the correction RFC 3260 records. |
| RFC 2598 | no, obsoleted | **yes**, in one place | [`Dscp.msg:36`](../../../../../src/inet/networklayer/diffserv/Dscp.msg): "Expedited Forwarding, RFC 2598". **An obsolete claim.** |
| RFC 3290 | no, level 5 | **yes**, in the user's guide list, twice, and in `MultiFieldClassifier.ned` | `ch-diffserv.rst:79,350`, `MultiFieldClassifier.ned:21`. A claimed document outside the in-scope set. |
| RFC 4115 | no, level 5 | no | Nothing names it; the model implements only RFC 2698's algorithm. |
| RFC 3168 | no, level 5 | no | Nothing names it; the DiffServ modules read and write the six-bit DSCP field only (see `standards.md`'s override table). |

**The finding of the survey.** The model's claim is broad and mostly accurate: the user's
guide states outright, under a heading of its own, "The implementation follows these RFCs"
and lists seven documents, and six of the seven name the current, governing text. The seventh
citation, RFC 3290, points at a document this pass places at level 5 (a management and
configuration model, not a per-hop-behavior rule), so it is a scope gap for a later pass, not
a verdict. The headline finding is internal, not against the register: `Dscp.msg:36` cites
RFC 2598 for the Expedited Forwarding code point, and RFC 2598 is obsolete — RFC 3246
obsoletes it, and the model's own `DiffservQueue.ned:19` and the user's guide's own list
(`ch-diffserv.rst:77`) already cite RFC 3246 for the same PHB. The model contains two claims
about the same per-hop behavior, one current and one obsolete, two files apart. RFC 2597 for
the Assured Forwarding code points (`Dscp.msg:19`) and RFC 2474 for the Class Selector
code points (`Dscp.msg:39`) are both current, so the inconsistency is confined to the one EF
row.

For the level 2 pass, facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. **The EF citation conflict decides which text a check of `DSCP_EF` targets.** RFC 3246
   §2 (`rfc3246.txt:151`) is the governing text by the register; a check of the EF
   code point or PHB targets RFC 3246, and the RFC 2598 citation at `Dscp.msg:36` is a
   documentation correction for a later pass, not a second, competing requirement.
2. **No `TODO` or `FIXME` exists inside the DiffServ `.cc` files themselves**
   (confirmed by a direct search of `src/inet/networklayer/diffserv/*.cc`); the
   `cRuntimeError` throws that do exist are input-validation guards on malformed
   configuration (an out-of-range DSCP, a missing filter field), not missing behavior. Every
   claimed mechanism — the two classifiers, the three meters, the marker, and the two PHB
   queues — has a complete implementation to check against.
3. **DiffServ has no dissector and no serializer**, and needs none: the DSCP value is a
   field of the IPv4 or IPv6 header, already reachable once a packet is dissected as `ipv4`
   or `ipv6`. A level 2 check observes the DSCP value before and after a classifier, a
   marker, or a meter, on the same node; it does not observe a link between two DiffServ
   peers, because the family defines none.
