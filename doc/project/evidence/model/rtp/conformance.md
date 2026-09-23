# RTP — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/rtp/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the RTP family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 1889 | [`README:4`](../../../../../src/inet/transportlayer/rtp/README) | "implements subset of rtp v2 (rfc 1889)". |
| RFC 1890 | [`README:9`](../../../../../src/inet/transportlayer/rtp/README) | "provided profile: audio/video profile with minimal control (rfc 1890)". |
| RFC 2250 | [`README:10`](../../../../../src/inet/transportlayer/rtp/README) | "provided packetization scheme: simulated mpeg video (rfc 2250)". |
| RFC 1890 | [`README:11-12`](../../../../../src/inet/transportlayer/rtp/README) | "provided packetization scheme: uncompressed audio (payload type 10 of rfc 1890: 16 bit, 2 channels, 44100 Hz)". |
| RFC 3550 | [`Rtcp.cc:233`](../../../../../src/inet/transportlayer/rtp/Rtcp.cc) | "[RFC 3550], by Ahmed ayadi", inside the RTCP reporting-interval formula. The one citation of the current base document anywhere in the model. |

No NED documentation comment names a document: `Rtp.ned` describes the module's role
("communicates with the application, and sends and receives Rtp data packets") and `Rtcp.ned`
says only "RTCP end system." Neither names RTP, RTCP or a standard by number. The user's guide
([`ch-transport.rst:296-309`](../../../../../doc/src/users-guide/ch-transport.rst)) names the
protocols by name — "The Real-time Transport Protocol (RTP)", "The RTP Control Protocol
(RTCP)" — and cites no document at all. No `.msg` file, and no file under
`profiles/avprofile/` or `src/inet/applications/rtpapp/`, names a document.

Mapped onto the standards map, [`standards.md`](../../protocol/rtp/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 3550 | yes, `base` | **yes, once, in a comment** | `Rtcp.cc:233`, inside one formula; no documentation comment of the `Rtp` or `Rtcp` module names it. |
| RFC 3551 | yes, `base` | **no** | The README claims payload type 10 by citing RFC 1890, the document RFC 3551 obsoleted; nothing names RFC 3551 itself. |
| RFC 1889 | no, obsoleted by RFC 3550 in 2003 | **yes** | `README:4`. The whole of the model's own stated scope ("implements subset of rtp v2") is framed against the obsolete document. |
| RFC 1890 | no, obsoleted by RFC 3551 in 2003 | **yes, twice** | `README:9` for the profile itself, `README:11-12` for the one payload format the README claims by number. |
| RFC 2250 | no, level 5 | **yes** | `README:10`. Out of scope for this pass by level; see the finding below for what the claim is worth even at a later pass. |

**The finding of the survey.** The model's only claim of any current document is one code
comment inside a formula, not the documentation a reader of the module reference sees. Three of
the four claims in the README — the file a person opens to learn what this directory
implements — name a document RFC-editor metadata shows obsoleted since 2003:
RFC 1889 for the base protocol, RFC 1890 for the profile and for the one payload format the
README claims by number. The obsoleting document is over twenty years old itself
([`standards.md`](../../protocol/rtp/standards.md#document-list)); this is not a recent
change the README missed, it is a citation that was already wrong when INET's own history
places RTP's integration into the framework (`ch-history.rst`, `__TODO`, dated 2007-2008 in the
model's own notes, four years after RFC 1889 and RFC 1890 were obsoleted).

The one payload format the README claims twice — payload type 10, L16 audio — is also the one
row of RFC 3551's payload-type table that the base documents themselves define
(`rfc3551.txt:1812`: "10 L16 A 44,100 2"), so the content of the claim is right even though its
citation names the wrong generation. RFC 2250, the MPEG payload format the README also claims,
is a second, separate document outside the in-scope set of this pass
([`standards.md`](../../protocol/rtp/standards.md#in-scope-set)); the model's
own serializer evidence, in `tests/serializer/REMAINING_GAPS.md:222-227`, already records that
the wire format INET writes for it "is not the one RFC 2250 describes: the serializer writes a
payload length and a picture type in four octets, with the standard layout sitting commented
out beside it." That finding stands on its own, independent of which generation the README
cites, and a level 2 or later pass inherits it as a known, already-documented gap rather than
one this survey needs to rediscover.

For the level 2 pass, four facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. **Every reachable claim of the model names an obsolete document.** A level 2 catalog that
   quotes the model's own stated scope quotes RFC 1889 and RFC 1890 by name; the governing
   text is RFC 3550 and RFC 3551. The content the README describes (a header, a profile, one
   payload format) did not change between the generations for the parts this pass put in
   scope; the citation is what is stale.
2. **RFC 2250 is claimed, and the claim is already known to be wrong on the wire.** A check of
   the MPEG payload format, when it enters scope at level 5, meets a **defect** by the guide's
   classification (the model claims the behavior and gets it wrong), not an unimplemented
   feature — the serializer writes a payload header, it is simply not the one the claimed
   document defines. Whether the defect's repair is blocked, and so whether the failure is
   ever declared expected, is a level 5 decision, not one this survey makes.
3. **Half the AV profile is excluded from the build.** Four pairs of source files under
   `profiles/avprofile/` carry a `.cc.off`/`.h.off` extension, which the build system excludes:
   `RTPAVProfilePayload10Sender`/`Receiver` — the very payload type 10 the README claims twice
   — and `RTPAVProfileSampleBasedAudioSender`/`Receiver`. `RtpProfile.cc:153-154` builds the
   name of the class to instantiate for a payload type at run time from the payload type
   number (`"Rtp" + profileName + "Payload" + payloadType + "Sender"`), so a session that
   negotiates payload type 10 looks up a class that the build never compiled. A level 2 check
   that relies on the claimed uncompressed-audio format meets an absent module, not a running
   one; `RtpAvProfilePayload32Sender`/`Receiver` (payload type 32, MPEG video) are compiled, so
   the video path the README also claims stays reachable even though the video format itself
   is the RFC 2250 defect of fact 2.
4. **The model's own maintainer note calls the result unstable.**
   `src/inet/transportlayer/rtp/__TODO` records, from 2008: "it compiles, runs, and does
   *something*, but crashes frequently." No later note in the tree revises this. A level 2
   pass should expect a check to double as a crash reproduction, not only a field assertion.
