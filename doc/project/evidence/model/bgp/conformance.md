# BGP-4 — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/bgp/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the BGP family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 4271 | [`Bgp.ned:13`](../../../../../src/inet/routing/bgpv4/Bgp.ned) | "Implements the BGP Version 4 routing protocol (RFC 4271)." The documentation comment of the module, which the module reference publishes. |
| RFC 4271 | [`ch-routing.rst:160-161`](../../../../../doc/src/users-guide/ch-routing.rst) | "The model implements RFC 4271, with some limitations." The user's guide repeats the claim. |
| RFC 4271, with named limitations | [`__TODO:2-7`](../../../../../src/inet/routing/bgpv4/__TODO) | A dedicated limitations file: "The model implements RFC 4271, with the following limitations: NOTIFICATION message is not implemented; MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer are not implemented; Optional UPDATE message Path Attributes are not implemented; Optional Final State Machine events are not implemented." |
| RFC 4271, by chapter | [`__TODO:9-25`](../../../../../src/inet/routing/bgpv4/__TODO) | A chapter-by-chapter table against RFC 4271 §3 to §10: "implemented", "not implemented", or "implemented except ...", down to which of ORIGIN/AS_PATH/NEXT_HOP/MULTI_EXIT_DISC/LOCAL_PREF/ATOMIC_AGGREGATE/AGGREGATOR is implemented. |
| RFC 4271, §8.1.2 to §8.1.5 | [`BgpFsm.h:39,50,66,95`](../../../../../src/inet/routing/bgpv4/BgpFsm.h) | Section headers above the groups of FSM event declarations: Administrative, Timer, TCP Connection-Based, and BGP Message-Based Events, each with the event's number, definition and status copied from the RFC. |
| RFC 4271, §8.2.2 | [`BgpFsm.cc:23,51,146,239,345,442`](../../../../../src/inet/routing/bgpv4/BgpFsm.cc) | One comment per FSM state handler: "RFC 4271 - 8.2.2. Finite State Machine - IdleState" and the same for Connect, Active, OpenSent, OpenConfirm and Established. |
| RFC 4271, §6.8 | [`BgpRouter.cc:374,393,401`](../../../../../src/inet/routing/bgpv4/BgpRouter.cc); [`WHATSNEW:559`](../../../../../WHATSNEW) | "RFC 4271 §6.8 connection collision detection" above the code, two more comments that name it at the point it fires, and the release notes: "connection collision detection was implemented according to RFC 4271". |
| RFC 4271, §9.1, §9.1.2.2, §9.2 | [`BgpRouter.h:161,194,201`](../../../../../src/inet/routing/bgpv4/BgpRouter.h) | Doxygen comments above the update-send process, the decision process, and the tie-breaking method, each citing the clause it follows. |
| RFC 4271, the BGP Identifier | [`BgpCommon.h:29`](../../../../../src/inet/routing/bgpv4/BgpCommon.h) | "the BGP Identifier is a 4-octet router-id (RFC 4271), even for IPv6 BGP". |
| RFC 4271 and RFC 6286, the `routerId` parameter | [`Bgp.ned:44,56`](../../../../../src/inet/routing/bgpv4/Bgp.ned); [`ch-routing.rst:257`](../../../../../doc/src/users-guide/ch-routing.rst) | "the 4-octet BGP Identifier (RFC 4271/6286)", once in the module documentation and once in the parameter comment; the user's guide repeats it. This is the one place the model names RFC 6286. |
| RFC 4271, the session timers | [`Bgp.ned:62-64`](../../../../../src/inet/routing/bgpv4/Bgp.ned) | The `connectRetryTime`, `holdTime` and `keepAliveTime` parameter comments each cite "(RFC 4271)". |
| RFC 4760 | [`Bgp.ned:43`](../../../../../src/inet/routing/bgpv4/Bgp.ned); [`ch-routing.rst:253-254`](../../../../../doc/src/users-guide/ch-routing.rst); [`WHATSNEW:544-545`](../../../../../WHATSNEW) | "IPv6 / MP-BGP (RFC 4760)" in the module documentation, "using the Multiprotocol Extensions of RFC 4760" in the user's guide, and the same in the release notes. |
| RFC 4760, by clause | [`BgpSession.cc:167-169,245`](../../../../../src/inet/routing/bgpv4/BgpSession.cc); [`BgpHeader.msg:97,144-145,252,261,276`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeader.msg); [`BgpHeaderSerializer.cc:196,210,518,546`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeaderSerializer.cc) | Source comments that cite RFC 4760 by section for the Multiprotocol Extensions capability, the MP_REACH_NLRI and MP_UNREACH_NLRI attributes, and their serialization. |
| RFC 5492 | [`BgpHeader.msg:86,110,114`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeader.msg); [`BgpSession.cc:168`](../../../../../src/inet/routing/bgpv4/BgpSession.cc); [`BgpHeaderSerializer.cc:87,292,302`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeaderSerializer.cc) | Source comments on the Capabilities Optional Parameter and its serialization. No NED documentation comment and no line of the user's guide names RFC 5492; the comments are the whole claim. |
| RFC 1997, named and declined | [`BgpHeader.msg:230`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeader.msg), [`BgpHeaderSerializer.cc:572`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeaderSerializer.cc) | "specifically (e.g. COMMUNITIES, RFC 1997)" and "preserved verbatim so the message still round-trips (e.g. COMMUNITIES, RFC 1997)" — the attribute is passed through unparsed, by name. |
| RFC 7911, named and not built | [`BgpHeader.msg:247`](../../../../../src/inet/routing/bgpv4/bgpmessage/BgpHeader.msg) | A commented-out field declaration: "specified in RFC 7911, optional 4 octets, wireshark detect existing of this field with heuristical algorithm". |
| none (a stub) | [`BgpSession.h:88`, `BgpSession.cc:323`](../../../../../src/inet/routing/bgpv4/BgpSession.cc) | `sendNotificationMessage()` is declared and defined; its body is a bare `// TODO` followed by seven lines of commented-out code that would build a `BgpNotificationMessage`. No such class exists in `BgpHeader.msg`, and nothing in the tree calls this method. |

No comment, NED documentation, or line of the user's guide names RFC 6793, RFC 8212, RFC 6608,
RFC 7606, RFC 7607, RFC 7705, RFC 8654, RFC 9072, RFC 9687, RFC 9774, RFC 4724, or RFC 4456: a
tree-wide search for each number returns nothing outside this table.

Mapped onto the standards map, [`standards.md`](../../protocol/bgp/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 4271 | yes, `base` | **yes**, in the published documentation and at the level of clauses | The module documentation, the user's guide, a dedicated limitations file, and comments that name §6.8, §8.1.2 to §8.2.2, and §9.1 to §9.2. |
| RFC 4760 | yes, `companion` | **yes**, in the published documentation and at the level of clauses | The module documentation, the user's guide, the release notes, and comments that name §3 and §4. |
| RFC 5492 | yes, `companion` | named, only in source comments | No documentation comment of any module and no line of the user's guide names it; three source files cite it. |
| RFC 6793 | yes, level 2 | **no** | Nothing names it. The AS number field is `uint16_t` everywhere in `BgpHeader.msg`: the model cannot represent a four-octet AS number at all, negotiated or not. |
| RFC 8212 | yes, level 2 | **no** | Nothing names it. Route filtering is opt-in only, through the `DenyRoute`/`DenyAS` family of the XML configuration; absent any such rule, every route is eligible and disseminated. |
| RFC 6286 | no, level 5 | **yes**, twice, next to RFC 4271 | `Bgp.ned:44,56` and `ch-routing.rst:257` cite "RFC 4271/6286" for the `routerId` parameter. A claimed document outside the in-scope set; see the facts below. |
| RFC 6608, RFC 7606, RFC 7607, RFC 7705, RFC 9072, RFC 9774 | no, level 3 | no | — |
| RFC 8654, RFC 9687 | no, level 4/5 | no | — |
| RFC 4724 | no, level 5 | no | Not named anywhere in the BGP sources, the NED documentation, or the user's guide. |
| RFC 1997 | no, level 5 | named, and declined | Two comments name it; the attribute is round-tripped unparsed, a deliberate refusal. |
| RFC 7911 | no, level 5 | named | One comment, on a field that does not exist in the compiled message: it is commented out. |
| RFC 4456 | no, level 5 | no | No comment, NED documentation, or user's guide line names it. |

**The finding of the survey.** The model claims RFC 4271 by number in every place a reader
looks: the module documentation, the user's guide, and a dedicated limitations file that lists
what is missing. No claim is obsolete — RFC 4271 is the current base document, RFC 4760 is the
current multiprotocol document, and the one incidental claim of RFC 6286 next to RFC 4271 also
names a current document.

The larger finding is what the claims leave out. RFC 4271 has been updated twelve times since
2006; two of those updates change the normal OPEN and UPDATE exchange, not an edge case: RFC 6793
lets a BGP speaker carry a four-octet AS number, and RFC 8212 sets the default route-propagation
policy of an EBGP session with no configured policy. The model names neither. For RFC 6793 this
is not a documentation gap alone: the AS number field is a fixed `uint16_t` through the whole
message tree, so the model could not represent a four-octet AS number even if it tried. For
RFC 8212 the gap is a policy default: absent a configured rule, the model disseminates every
route, which is the opposite of RFC 8212's "SHALL NOT ... if no explicit Export Policy has been
applied".

## Facts for the level 2 pass

1. **The `__TODO` file's four limitations, checked one by one:**
   - *NOTIFICATION message is not implemented.* The message type code exists
     (`BgpHeader.msg:41`, `BGP_NOTIFICATION = 3`), and a send method is declared and defined
     (`BgpSession.h:88`, `BgpSession.cc:323`). The method's body is a bare `// TODO` with the
     rest commented out; it builds nothing and sends nothing, and no `BgpNotificationMessage`
     class exists in `BgpHeader.msg` for it to build. Nothing in the tree calls the method. A
     bare `// TODO` with no reason is a claim, by the guide's table; this is a half-written
     mechanism, so a check of it is a defect, not a declarable absence — except that no check
     can be built at all, because there is no message class to observe on the wire. That is an
     **untestable claim**, the second row of the guide's four-case table.
   - *MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer are not implemented.*
     Confirmed: no field, no variable and no comment names either timer anywhere in the tree
     outside the `__TODO` file itself. No code path exists. This is a plain unimplemented
     feature.
   - *Optional UPDATE message Path Attributes are not implemented.* Checked against
     `BgpHeaderSerializer.cc`: message classes and serializer cases exist for
     `MULTI_EXIT_DISC`, `LOCAL_PREF`, `ATOMIC_AGGREGATE` and `AGGREGATOR` (read, write, and a
     print path in `BgpRouter.cc`). `LOCAL_PREF` is also read from the XML configuration
     (`BgpRouter.cc:259`), carried in every route entry, and it **is** used to break ties in
     the decision process (`BgpRouter.cc:1117-1118`,
     `candidate->getLocalPreference() > current->getLocalPreference()`). This directly
     contradicts the `__TODO` table's own row, "LOCAL_PREF ... implemented". `MULTI_EXIT_DISC`
     and `AGGREGATOR` are parsed, serialized and printed but never read by
     `BgpRouter::isBetterRoute`; `ATOMIC_AGGREGATE` carries no value beyond its presence. The
     claim is not uniform: LOCAL_PREF is a working, claimed feature that the `__TODO` file
     miscategorizes as missing; MULTI_EXIT_DISC and AGGREGATOR have a message class and
     wire support but no decision-process effect, which is a half-written mechanism (a claim,
     so a failure of a check that expects tie-breaking by MED would be a defect, not a
     declared absence).
   - *Optional Final State Machine events are not implemented.* No "Optional" marker exists
     anywhere in `BgpFsm.h`; the RFC's own Mandatory/Optional split is not tracked in code at
     all. Two events the RFC marks **Mandatory** have no virtual method: Event 2, ManualStop
     (`BgpFsm.h:44-48`, commented out as `// virtual void event2() {}`), and Event 16,
     Tcp_CR_Acked (`BgpFsm.h:67-74`, commented out the same way). A level 2 pass needs to check
     event by event; the `__TODO` file's "except optional ones" does not match the code for at
     least these two.
2. **RFC 6793 and RFC 8212 are unclaimed and, for RFC 6793, architecturally absent.** See the
   finding above. A level 2 check of either normally observes an absence (no capability
   negotiated, no filtering), which is itself the finding; RFC 6793 additionally cannot be
   tested by configuration alone, because no field in the wire format could carry the result.
3. **RFC 6286 is claimed outside the in-scope set, next to RFC 4271, for one parameter
   comment.** The claim is accurate in substance — a `routerId` distinct across peers is what
   RFC 6286 relaxed the requirement to — but nothing in the model depends on the distinction
   RFC 6286 makes (AS-wide vs. globally unique), so the claim adds no obligation a level 2 or 3
   check would need to reach.
4. **RFC 5492 is claimed only in source comments**, the same shape as RIP's and ARP's headline
   findings for their own base documents: a reader of the module reference sees RFC 4271 and
   RFC 4760, and not RFC 5492, even though the Capabilities Optional Parameter it defines
   carries both the MP-BGP and the (unimplemented) four-octet AS number negotiation.
