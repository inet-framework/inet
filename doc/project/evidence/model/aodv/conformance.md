# AODV — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/aodv/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the AODV family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 3561 | [`Aodv.ned:26-27`](../../../../../src/inet/routing/aodv/Aodv.ned) | "This implementation is based on RFC 3561. For more information, you may refer to the following link: ...". This is the documentation comment of the module, which the module reference publishes. |
| RFC 3561 §6.6.3 | [`Aodv.ned:37`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `askGratuitousRREP` parameter: "See RFC 3561: 6.6.3". |
| RFC 3561 §6.9 | [`Aodv.ned:38`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `useHelloMessages` parameter: "See RFC 3561: 6.9". |
| RFC 3561 §6.12 | [`Aodv.ned:39`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `useLocalRepair` parameter: "See RFC 3561: 6.12 *not implemented yet*". |
| RFC 3561 §5.1 | [`Aodv.ned:40`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `destinationOnlyFlag` parameter: "See RFC 3561: 5.1". |
| RFC 5148 | [`Aodv.ned:45`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `maxPeriodicJitter` parameter carries the bound of RFC 5148 §5.4 verbatim, unlabeled: "It MUST NOT be negative; it MUST NOT be greater than MESSAGE_INTERVAL/2; it SHOULD NOT be greater than MESSAGE_INTERVAL/4." |
| RFC 5148 | [`Aodv.ned:48-54`](../../../../../src/inet/routing/aodv/Aodv.ned) | The `maxJitter` and `jitter` parameters, above a comment labeled "RFC 5148:" that quotes the MAXJITTER advice of §5.4. |
| RFC 3561 §6.1 | [`Aodv.h:112`](../../../../../src/inet/routing/aodv/Aodv.h) | The `sequenceNum` field: "it helps to prevent loops in the routes (RFC 3561 6.1 p11.)". |
| RFC 3561, AODV Terminology | [`Aodv.cc:1247`](../../../../../src/inet/routing/aodv/Aodv.cc) | "it means invalid, see 3. AODV Terminology p.3. in RFC 3561". |
| RFC 5148 | [`Aodv.cc:1292`, `Aodv.cc:1369`](../../../../../src/inet/routing/aodv/Aodv.cc) | Two comments labeled "RFC 5148:" above the jitter computation for a forwarded and a generated message. |
| RFC 3561 | [`ch-adhoc-routing.rst:58`](../../../../../doc/src/users-guide/ch-adhoc-routing.rst) | "The :ned:`Aodv` module type implements AODV, based on RFC 3561." The user's guide repeats the claim of the NED documentation. |

Every citation names RFC 3561 or RFC 5148. No comment, parameter, `.msg` field or line of the
user's guide names any other document.

Mapped onto the standards map, [`standards.md`](../../protocol/aodv/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 3561 | yes, `base` | **yes**, in the published documentation and at the level of clauses | The module documentation, the user's guide, and per-parameter comments that name §5.1, §6.1, §6.6.3, §6.9 and §6.12. |
| RFC 5148 | yes, `companion` | **yes**, in four source comments | No documentation comment of the `Aodv` module names RFC 5148; the citations sit next to the `maxJitter`/`jitter`/`periodicJitter` parameters and the forwarding code, where a reader of the module reference does not see them. |

**The finding of the survey.** Both documents of the in-scope set are current: neither RFC 3561
nor RFC 5148 has an `obsoleted_by` or `updated_by` entry in the register
([`standards.md`](../../protocol/aodv/standards.md#document-list)), so AODV carries none of
the obsolete-citation headline that RIP, ARP and RTP each show in this wave. The two documents
the model names are the two documents that govern today.

The headline of this survey is a different kind of gap: a citation that reads as a refusal but
that the workflow's own rule for a claim does not let a later pass treat as one.

`Aodv.ned:39` documents `useLocalRepair` as "*not implemented yet*", and the code at
`Aodv.cc:1100` carries only `// TODO Implement: local repair` at the point the mechanism would
run — no branch, no partial state machine, nothing beyond the switch and the comment. Read in
isolation, this looks like the third row of
[the claim principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test):
a stated limitation, so a check that fails would declare the failure expected. The guide's own
rule says otherwise. The comment gives no reason the mechanism cannot be built and no argument
that it is not needed — it is the "TODO implement X" row of the guide's table, a plain promise,
and the guide names exactly this shape as the giveaway that settles it: **"if the parameter...
the behavior needs is already there and carries a placeholder value, the mechanism is claimed
whatever the comment says."** `useLocalRepair` is that parameter. Local repair is therefore a
**claim**: a level 2 or 3 check of it that fails is a defect, not a statement the model declined
in words. RFC 3561 itself marks the mechanism `MAY` (`rfc3561.txt:1410`), so the standard would
not have penalized a true refusal; the model chose to expose the switch anyway, and that choice
is what makes it a claim.

For the level 2 pass, four facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. **Local repair is a claim, not a declared refusal.** See the finding above. A check of
   RFC 3561 §6.12 that fails is a **defect**.
2. **Actions after reboot (§6.13) is the same shape.** `Aodv.cc:1581` carries a bare
   `// TODO Implement: Actions After Reboot`, followed by a paraphrase of the MUST/SHOULD text
   of §6.13. No reason is given. A related check that fails is a defect, not a declared
   expected failure.
3. **The model assumes one interface throughout.** `Aodv.cc:808` and `Aodv.cc:1060` each carry
   a bare `// TODO Implement: support for multiple interfaces` /
   `// TODO IMPLEMENT: multiple interfaces`, and `Aodv.cc:1321` adds `// FIXME Drop the queued
   datagrams.` at a related point. RFC 3561 §6.14 states the multi-interface behavior as
   description, not as a `MAY`; the model's own TODOs are bare, so this is a claim by the same
   rule, and a check that needs a second interface meets an unfinished mechanism, not an
   unclaimed one.
4. **The claimed status of both documents is below Internet Standard.** RFC 3561 is
   Experimental and RFC 5148 is Informational
   ([`standards.md`](../../protocol/aodv/standards.md#document-list)). A later pass states a
   `MUST` of RFC 3561 as an AODV-interoperability requirement, not as an IETF-wide one.
