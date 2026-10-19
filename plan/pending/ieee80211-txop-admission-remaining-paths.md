# Implementation plan: TXOP admission for Basic and Normal Ack paths

Plan revision: 10, 2026-10-19. Revision 10 records the accepted R1 decision and the C5 execution results.

Implementation status: local production work and focused checks are complete. The user requested execution on 2026-10-19. The [execution record](ieee80211-txop-admission-remaining-execution.md) identifies the checks, fingerprint differences, and external integration requirement.

Basic activation requires [R1, the RTS exhaustion rule](#r1-basic-rts-exhaustion-under-an-accepted-agreement), and [B1, the legal Basic baseline](#b1-required-baseline-for-legal-basic-operation). The user accepted lifetime retention for R1. The legal probe supplies B1. C5 now uses commitment before RTS after admission. Combined verification with the separate peer-permission activation remains pending.

The [summary with diagrams](ieee80211-txop-admission-remaining-paths-summary.md) explains the proposal for human review.

## 1. Problem and intended behavior

The retained reproduction establishes TXOP overruns with Block Ack disabled. It also records overruns under the historical HT Basic configuration. Those Basic runs do not establish a failure for a legal non-HT Basic peer pair. The Compressed-only preparation path identifies a coverage gap for Basic. A failing non-HT Basic baseline remains required before C5.

For transmission admission, HCF must calculate the complete exchange before the data service removes queued MSDUs. If the exchange cannot fit the current tail, HCF must try a permitted smaller form. If no form qualifies, HCF must end the TXOP without that extraction. Admission refusal itself must preserve source identities, queue order, ownership, absolute deadlines, sequence counters, and recovery state. A real internal collision has a separate commitment rule for recovery. An impossible fresh-TXOP exchange has the explicit terminal action below.

The correction must also permit progress on a fresh TXOP. The ordinary form carries one unfragmented MSDU in one MPDU. Without an accepted Block Ack agreement, an initial ordinary MSDU has no general exception to a positive limit. If that exchange exceeds a fresh limit, the service must prepare permitted nondynamic fragments. Repeated refusal until lifetime expiry does not correct this case.

The concrete continuation failure uses a one-millisecond TXOP, 24 Mbit/s DATA, A-MSDU aggregation, RTS protection, and multiple protection. With Block Ack disabled, the second A-MSDU ends at 1.001102 seconds against a 1.001-second bound. Its ACK ends at 1.001146 seconds. The correction must defer that exchange before source extraction, or select a smaller form that fits.

Basic admission evidence will use the 2016 peer conditions. Basic remains permitted between non-HT peers and between an HT peer and a non-HT peer. Two HT peers must use Compressed for Block Ack. The separate Basic peer-permission work owns enforcement of that restriction. Its plan remains local context outside this publication. Successful Basic exchanges between two HT peers are outside this plan's required scope.

## 2. Checkout, current path, and observed evidence

### Source revisions and local work

The reproduction baseline is `/home/user/omnetpp_ws/inet-compressed-blockack-s5` at `1dd4ca23785b14b058693e9e20434a4257c3831f`. Its local edits affect `AGENTS.md` and plan documents. They do not change the compiled MAC source.

This plan resides in `/home/user/omnetpp_ws/inet-ieee80211-ampdu-implicit-blockack-a1-a7` at `d841dc77db7d0b141bf31e5e215a0de9bb281957`. This checkout has a local `AGENTS.md` edit. This task preserves that edit. Its A-MPDU work adds integration requirements beyond S5.

The S5 exclusion appears at `Hcf.cc:1982`. The [current preparation entry](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) also requires an active Compressed agreement. Before implementation, pin the selected target again. Use S5 for the baseline comparison; apply this plan to the selected target with its current contracts.

### Confirmed mechanism

[HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) selects the frame sequence. Without a Block Ack handler, its initial selector calls the destructive frame accessor before the preparation callback. That call can already extract queued MSDUs. Thus, removal of the profile guard alone cannot preserve the queue.

[InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc) owns committed frames. Its destructive accessor calls the data service when no eligible committed frame exists. The [data service](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) can aggregate sources, delete their original representations, assign a sequence number, and create fragments. A later admission refusal cannot recover the original queue form through the current API.

HCF's special single-protection guard applies when `isBlockAckSupported` is false. It evaluates continuation cost at final submission. It does not constrain the initial exchange through the same test. Multiple protection has continuation checks for start time and reservation legality, but those checks do not prove complete-exchange containment.

The current reversible path retains duplicate previews through the [QoS frame context](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h). It validates an accepted Compressed agreement before admission. It therefore cannot directly serve a null agreement, an accepted Basic agreement, or Basic fragmentation. Its single-frame commit operation also cannot transfer a group of prepared fragments.

### Executed reproduction

The retained [probe](../reproductions/ieee80211-txop-admission-remaining/TxopAdmissionProbe.test) derives its topology and traffic from [the Compressed exchange fixture](../../tests/module/Ieee80211CompressedBlockAckExchange_1.test). It uses real association, production queues, HCF, frame sequences, the radio, and the peer MAC. The probe adds observations through HCF overrides. Every override calls the production method; the probe changes no admission decision.

All eight S5 runs use seed zero and debug mode. The medium has zero propagation time. Both hops use HT mixed operation in 2.4 GHz. The source sends thirty 700-byte UDP payloads through the AP. The RTS threshold is 500 bytes. The configured fragmentation threshold is 2346 bytes, which leaves these MSDUs unfragmented.

The Basic runs use two HT peers and therefore violate the 2016 variant rule independently of their TXOP overruns. Their retained probe and observations are historical evidence. They impose no requirement to preserve successful HT Basic exchanges. New legal Basic fixtures must establish the corrected admission behavior.

The counts below identify observation boundaries, not distinct failed MSDUs. One exchange can exceed the bound at both DATA completion and ACK completion. Basic DATA under Block Ack can have no immediate ACK; DATA completion is therefore necessary evidence.

| Run | DATA rate | Block Ack profile | Protection | DATA completions beyond the applicable bound | ACK completions beyond the applicable bound |
| --- | --- | --- | --- | ---: | ---: |
| 0 | 6 Mbit/s | Basic | Single | 30 | 7 |
| 1 | 6 Mbit/s | Basic | Multiple | 29 | 6 |
| 2 | 6 Mbit/s | Disabled | Single | 31 | 31 |
| 3 | 6 Mbit/s | Disabled | Multiple | 32 | 32 |
| 4 | 24 Mbit/s | Basic | Single | 2 | 0 |
| 5 | 24 Mbit/s | Basic | Multiple | 14 | 0 |
| 6 | 24 Mbit/s | Disabled | Single | 0 | 0 |
| 7 | 24 Mbit/s | Disabled | Multiple | 16 | 16 |

An accepted Basic agreement also reproduces a distinct initial failure. In run 0, the first A-MSDU ends at 1.003914 seconds against a 1.002728-second bound. The excess is 1186 microseconds. This frame has no Retry bit. It cannot use the initial ordinary-MSDU exception because it contains an A-MSDU.

Run 5 confirms the continuation failure under an accepted Basic agreement. Its second A-MSDU ends at 1.004902 seconds against a 1.004592-second bound. The excess is 310 microseconds. A retry cannot use the single-frame exception after another Data or Management frame in the same TXOP.

Run 2 confirms the initial failure without an agreement. Its ordinary DATA ends 166 microseconds beyond the limit. Its ACK ends 226 microseconds beyond the limit. This case requires fragmentation or another permitted transmission choice on a fresh TXOP.

All eight diagnostic runs exit zero and deliver thirty payloads. Zero exit status means that the observation run completes; it does not mean that TXOP admission is correct. Run 6 is a clean control for this configuration only. It does not prove all single-protection paths correct.

Runs 0, 5, 6, and 7 also execute against this checkout's debug library. Their DATA, violation, and summary observations match S5 exactly. The [observation record](../reproductions/ieee80211-txop-admission-remaining/observations.json) records library hashes, commands, counts, and selected events. Raw logs remain under `/tmp/inet-txop-admission-remaining-20261018/`.

These runs establish airtime violations. They do not measure queue preservation on refusal, RTS failure recovery, or fragmentation behavior. The regression cases below must establish those properties after implementation.

### B1: Required baseline for legal Basic operation

B1 now has a failing non-HT reproduction. The [execution record](ieee80211-txop-admission-remaining-execution.md) records eighteen continuation violations at 24 Mbit/s under multiple protection. The [legal Basic probe](../reproductions/ieee80211-txop-admission-remaining/BasicNonHtTxopAdmissionProbe.test) uses accepted agreements without aggregation. The retained HT Basic runs remain historical evidence.

The diagnostic uses production association, queues, HCF, frame sequences, radio, and peer MAC. Observation overrides call their production methods without a changed admission decision. The bounded conditions below define that campaign.

Use the following bounded conditions:

1. Use seed zero and non-HT `g(erp)` peers with a confirmed accepted Basic agreement.
2. Disable A-MSDU and A-MPDU aggregation.
3. Use legal MSDU sizes and a legal fragmentation threshold, initially 2346 bytes.
4. Test 6 and 24 Mbit/s DATA. Use a 1024-microsecond positive TXOP limit. Use an RTS threshold of 500 bytes.
5. Test both protection classes with the same traffic and association procedure.

At least one continuation must exceed its positive TXOP bound before the correction. Record actual DATA completion, agreement state, earlier DATA in the TXOP, and the complete applicable exchange cost. Basic DATA with Block Ack policy has no immediate ACK. An oversized initial ordinary MSDU under an agreement can use an exception; that case alone does not prove the defect. The baseline must expose the continuation failure without that exception.

Retain raw logs and exact checkout, library, command, configuration, run, and seed identities. Record the legal baseline separately from the historical HT observations. If these cases do not expose a violation, reassess the Basic defect scope before C5. Do not label an unexecuted or nonfailing case as a failing baseline.

## 3. Standards contract and scope

### Applicable requirements

Use [IEEE Std 802.11-2024](../../standards/80211ax-2024.pdf) for the Compressed profile and Normal Ack outside the Basic profile. The source filename contains `ax`, but the document is the 2024 base standard. The clauses below contain normative requirements unless the table identifies a note. Basic uses the separate 2016 contract below, including its Normal Ack fallback.

| Source | Requirement for this correction | Corpus identity and physical PDF pages |
| --- | --- | --- |
| Clause 10.23.2.9 | A positive TXOP limit contains the complete exchange and immediate responses, except under a listed exception. | `ieee80211-2024:clause:10.23.2.9`, pages 2012–2014 |
| Clause 10.23.2.9 | A zero limit permits one allowed data unit, its required protection, and acknowledgments. Nondynamic fragments of one MSDU can use multiple SU PPDUs. | Same node, pages 2012–2014 |
| Clause 10.23.2.9 | An initial ordinary MSDU can exceed a positive limit under a Block Ack agreement. It must not belong to an A-MSDU or a multi-MPDU A-MPDU. | Same node, pages 2012–2014 |
| Clause 10.23.2.9 | A same-size retransmitted MPDU can use the stated exception. The TXOP must contain at most one Data or Management frame. | Same node, pages 2012–2014 |
| Clause 10.23.2.9 | Outside an applicable exception, the first fragment of an initial individually addressed MSDU must fit the positive limit. | Same node, pages 2012–2014 |
| Clause 10.23.2.8 | An additional exchange must meet the TXNAV condition and the separate TXOP restriction. TXNAV is the holder's reservation timer. | `ieee80211-2024:clause:10.23.2.8`, pages 2010–2012 |
| Clause 9.2.5.2 | The first frame selects the protection class for the TXOP. Duration fields follow that class and its reservation rules. | `ieee80211-2024:clause:9.2.5.2`, pages 710–712 |
| Clause 10.2.7 | An MSDU under an HT-immediate Block Ack agreement must not be fragmented, even above the configured fragmentation threshold. | `ieee80211-2024:clause:10.2.7`, page 1882 |
| Clause 10.4 | Nondynamic fragments use one sequence number and ordered fragment numbers. Equal-size fragments precede the final fragment, which can be smaller. Nonfinal fragment lengths are even. | `ieee80211-2024:clause:10.4`, pages 1935–1936 |
| Clause 10.4 | A transmitted fragment retains its body and length across retries. Ordinary nondynamic fragmentation does not fragment an A-MSDU. | Same node, pages 1935–1936 |
| Clause 10.11 | A-MSDU transmission requires local HT implementation and a received peer HT Capabilities element for these non-DMG, non-S1G stations. | Canonical node unavailable; original PDF pages 1971–1973 |
| Clause 10.23.2.12.1 | Failed associated RTS attempts increment the frame retry count outside a Block Ack agreement. Admission refusal is not such an attempt. | `ieee80211-2024:clause:10.23.2.12.1`, pages 2016–2017 |
| Clause 10.23.2.9, Note 3 | The note explains initial-transmission limits. It is informative; the earlier normative paragraphs establish the requirements. | `ieee80211-2024:clause:10.23.2.9`, page 2013 |

### Basic contract under IEEE 802.11-2016

Use [IEEE Std 802.11-2016](../../standards/80211-2016.pdf) for the Basic profile's EDCA admission and retry contract. This edition defines Basic BAR and Block Ack, although it marks both formats as obsolete. It permits the eligible initial ordinary-MSDU exception under a Block Ack agreement. The source PDF supplies this evidence because the corpus does not index this edition. These are normative clauses and tables; no canonical corpus node is available.

| Source and physical PDF pages | Required Basic treatment |
| --- | --- |
| Clauses 9.3.1.8.2 and 9.3.1.9.2, pages 675–676 and 679 | Preserve Basic BAR and the 128-octet Block Ack bitmap, including fragment bits. |
| Clause 10.22.2.8, pages 1388–1389 | Contain the complete positive-limit TXOP, including immediate responses, unless a listed exception applies. Zero permits the allowed data unit, protection, and responses. |
| Clause 10.22.2.8, page 1388 | An initial ordinary MSDU under an accepted agreement can use the stated exception. It must not belong to an A-MSDU or multi-MPDU A-MPDU. The TXOP must contain at most one Data or Management frame. |
| Clause 10.22.2.8, pages 1388–1389 | Apply the retransmission, previous-fragment retry, and sixteen-fragment exceptions under their stated conditions. The 2005 lower-rate test does not apply. Outside an exception, prepare permitted fragments when necessary for initial containment. |
| Clause 10.22.2.11.1, pages 1391–1392 | Maintain short and long retry counts for each MSDU or A-MSDU. A failed associated RTS increments the MSDU short count and the access-category short count. Resolve R1 before Basic activation under an accepted agreement. |
| Clause 10.24.3, pages 1419–1421 | Preserve the Basic DATA/BAR/Block Ack exchange. MSDUs sent with Block Ack use the lifetime rule rather than DATA retry limits. Normal Ack can operate within the agreement. |
| Clauses 10.24.6 and 10.12, pages 1422 and 1366 | Two HT stations must use Compressed Block Ack. Basic can serve peers when at least one is non-HT. Basic fixtures must exclude A-MSDU aggregation. |
| Clauses 10.2.7 and 10.5, pages 1302 and 1343 | HT-immediate and HT-delayed agreements prohibit MSDU fragmentation. Permitted nondynamic fragments retain equal even nonfinal lengths and fixed transmitted bodies and lengths. |

Basic retains the current reservation checks as model constraints in addition to this 2016 admission contract. Its initial ordinary-MSDU exception comes from 2016, without a borrowed 2024 rule. Its short and long retry counts remain distinct from the 2024 single frame retry count. An enabled Basic handler selects this contract without a new configuration parameter. Normal Ack fallback in that enabled profile uses the 2016 contract too. Disabled Block Ack uses the 2024 contract even if the unused variant parameter says `basic`.

Tables 9-27 and 9-28 of [IEEE Std 802.11-2020](../../standards/80211-2020.pdf) reserve Basic's former encoding on physical PDF pages 801 and 804. The 2024 Tables 9-36 and 9-37 also reserve that encoding. The 2016 edition therefore supplies the selected Basic contract. The existing Basic reproduction uses two HT peers and A-MSDU aggregation. Retain it only as historical evidence, without a requirement for continued successful operation. It does not establish conformance to either complete edition.

The 2016 restriction applies to the peer pair, not to an HT station alone. Positive Basic tests will cover non-HT peers and an HT peer with a non-HT peer. Positive tests between two HT peers will use Compressed for Block Ack. The separate peer-permission plan owns Basic rejection tests and their compatibility effects. No Basic label overrides the fragmentation prohibition under an actual HT-immediate agreement.

TXOP admission will retain the current profile-permission decisions. This scoped claim does not establish complete 2016 conformance for the Basic profile. In particular, it does not correct Basic setup between two HT peers. The peer-permission change needs its own PR and impact inventory. It is not a prerequisite for C1–C5.

The 2016 and 2020 PDFs now reside under this checkout's `standards/` directory. These relative links require the local standards inputs, as does the existing 2024 PDF link. The PDFs remain ignored workspace inputs. A local link check establishes file resolution; it does not establish public availability.

Basic conformance fixtures must also use legal MSDU lengths and fragmentation thresholds. Existing showcase configurations do not themselves establish conformance. This revision does not change showcase modes, aggregation, traffic sizes, or recorded results. Such configuration corrections remain separate work.

The corpus reports fresh document sources. Its linter reports global ambiguities and unresolved references. Clause 10.23.2.9 itself resolves uniquely, and its ten extracted references resolve. The relevant references include Clauses 10.4, 10.2.7, and 10.6.

The corpus cannot retrieve Clause 10.11 as a canonical node. Text from original PDF pages 1971–1973 supplies that clause; the applicable HT conditions appear on page 1973. The 2016 evidence uses original PDF text and its clause cross-references. The listed 2016 clauses resolve directly in that PDF. The other 2024 citations need no PDF fallback. Basic does not use the 2005 or 2012 lower-rate contract.

### R1: Basic RTS exhaustion under an accepted agreement

Resolution, 2026-10-19: the user selected "Retain the MSDU until lifetime expiry." The [decision note](ieee80211-txop-admission-r1-decision.md) records the interpretation, owners, and terminal outcomes. The paragraphs below preserve the question that controlled activation. They describe the preimplementation state.

R1 identifies an unresolved standards interpretation. Clause 10.22.2.11.1 explicitly increments the MSDU short counter after CTS failure. Its general discard rule applies when the short counter reaches `dot11ShortRetryLimit`. Clause 10.24.3 instead makes MSDUs sent with Block Ack subject to lifetime rather than retry limits. The current citations do not establish how that exemption applies to failed associated RTS attempts. An accepted agreement alone does not resolve this overlap.

Current HCF increments the short counter and discards the protected MSDU at RTS exhaustion, including on its legacy Basic path. That is observed source behavior, not evidence that this interpretation is correct. The plan must not silently preserve or remove that disposal rule as a consequence of the admission change.

The author responsible for Basic recovery in C5 owns R1. That author must provide the clause interpretation, terminal outcomes, ownership decision, and direct test expectations for maintainer review. This revision assigns the responsibility to that role; it does not identify an appointed contributor. No default interpretation authorizes activation. C5 remains unscheduled until the maintainer accepts R1 and B1 supplies the legal failing baseline.

Before C5 activates Basic admission, record a justified interpretation with exact clause references and observable outcomes. The resolution must specify these cases:

1. State whether short-limit exhaustion discards the MSDU before its first local DATA transmission.
2. State whether short-limit exhaustion discards the MSDU after an earlier local DATA transmission under the agreement.
3. State how internal-collision exhaustion interacts with the same MSDU and agreement.
4. State the corresponding group disposal, deadline, ACK, and recovery cleanup actions.

Both candidate interpretations retain the required MSDU short-counter update and access-category recovery. One applies the RTS short limit independently of Block Ack DATA lifetime. The other applies the lifetime exemption to the associated RTS failures too. Neither interpretation is an approved implementation rule in this revision. The named exhaustion tests below require R1's expected outcomes before Basic activation.

### Acceptance criteria

The correction is complete only when all these conditions hold:

1. For transmission admission, Basic, disabled Block Ack, and Normal Ack fallback reach admission before extraction of queued QoS DATA. Internal-collision recovery can commit one ordinary source before that source's channel grant.
2. Every admitted exchange contains the selected DATA form, required protection, applicable IFSs, and actual immediate response type.
3. Admission refusal itself preserves queued source identities, order, ownership, deadlines, sequence state, recovery state, and queue service observations.
4. Aggregate refusal can select one ordinary MSDU without a second adaptive DATA mode query.
5. An ordinary MSDU outside an agreement can use legal fragments when a fresh positive TXOP cannot contain it. An accepted Basic agreement can instead permit the conditional exception in criterion 6.
6. Zero limits and the applicable positive-limit exceptions retain their distinct conditions. Accepted Basic agreements can use the 2016 initial ordinary-MSDU exception. Initial A-MSDUs and multiple Data frames in one TXOP cannot use that exception.
7. A real RTS failure retains its required recovery under the selected edition. Basic activation requires the explicit exhaustion outcomes from R1. Fragment replacement and fragment ACKs preserve the required MSDU retry state. A refusal produces no RTS or DATA failure outcome.
8. Accepted Compressed and A-MPDU operation retain their current preparation, Block Ack, and ownership contracts.
9. Positive-limit equality is permitted where the standard permits containment. The separate TXNAV comparison retains its strict condition.
10. Both protection classes use valid Duration fields and preserve reservation behavior after a permitted exception.
11. HT-immediate Block Ack prohibits MSDU fragmentation. A Basic profile label or Normal Ack policy must not bypass that agreement condition.
12. When no permitted form fits a fresh TXOP at the selected mode, HCF stops with a diagnostic before further source consumption. It must not return ordinary deferral or repeat contention for that condition.

The required scope covers individually addressed QoS DATA through HCF. It includes committed retries, permitted Basic fragments under 2016, and Normal Ack before or outside an agreement. It also includes fallback when no usable HT agreement exists.

DCF, HCCA, new acknowledgment policies, dynamic fragmentation, multiuser operation, and MAC stop/start support remain separate work. Management and group-addressed paths require regression controls because they share selectors. This plan does not claim to audit all their TXOP behavior.

## 4. Proposed design and reasons

### Comparison with current Compressed admission

The plan extends the admission mechanism that accepted Compressed agreements already use. The Basic extension and the Normal Ack extension outside accepted agreements remain proposed. HCF will prepare copies, retain one selected DATA mode, and check the complete exchange before source extraction. Admission refusal itself will preserve queued MSDUs without a retry increment.

Commitment is the point at which the data service consumes sources and transfers prepared MPDUs to the committed-frame owner. For unnumbered sources, commitment also allocates the sequence identity. The table compares individual QoS DATA paths. The current A-MPDU path retains its separate preparation and response contracts.

| Condition | Current Compressed, accepted agreement | Proposed Basic, accepted agreement | Proposed Normal Ack, no accepted agreement |
| --- | --- | --- | --- |
| Commitment with RTS | Keep original MSDUs queued through RTS/CTS. Commit immediately before DATA. | Commit before RTS after admission. The accepted R1 rule retains the MSDU beyond retry limits under the agreement. | After admission succeeds, commit immediately before RTS. |
| Actual RTS failure | Update access-category recovery. Do not create a frame counter under a provisional sequence number. | Update the 2016 MSDU short retry count and access-category recovery. | Update the modern MSDU frame retry count and access-category recovery. |
| Initial ordinary MSDU exceeds a fresh positive TXOP | The initial-MSDU exception can apply. | The 2016 initial-MSDU exception can apply under the accepted agreement. | That exception does not apply. Prepare legal fragments when necessary. |
| Fragmentation | Preserve the prohibition under HT-immediate Block Ack. | Add reversible preparation for permitted Basic fragments under 2016. | Account for TXOP airtime and the configured threshold when fragmentation is permitted. |
| Response cost | Individual DATA with Block Ack policy has no immediate ACK. BAR/Block Ack has separate admission. | Use the same exchange separation, with Basic response lengths. | Include SIFS and ACK airtime in the DATA exchange. |

The initial ordinary-MSDU exception requires the stated aggregate restrictions and at most one Data or Management frame in the TXOP. An A-MSDU cannot use that initial-MSDU exception. A retransmission must satisfy its separate exception conditions.

The selected retry contract controls commitment and RTS recovery. Normal Ack DATA under an accepted Compressed agreement follows the modern agreement-scoped rule. Basic's 2016 associated RTS rule updates its MSDU short counter even when an agreement exists. Basic commits before RTS after admission. ACK policy controls response airtime. Without RTS, these paths commit immediately before DATA, unless an earlier internal collision already owns the source.

The earlier commitment outside an agreement gives the recovery owner a stable identity for failed associated RTS attempts. After actual RTS submission, the admitted form belongs to retry and lifetime procedures. A later cancellation must not restore it to the queue as new MSDUs.

Hypothetical example: a later exchange needs 300 microseconds, but only 200 microseconds remain. All three paths defer before RTS and preserve queued sources. If an ordinary MSDU also exceeds a fresh positive TXOP, modern no-agreement operation needs permitted fragments. Accepted Basic and Compressed agreements can instead permit the stated initial ordinary-MSDU exception. The exception requires at most one Data or Management frame in that TXOP.

### Fragmentation eligibility

IEEE Std 802.11-2024, Clause 10.2.7 prohibits MSDU fragmentation under an HT-immediate Block Ack agreement. The service must preserve this prohibition even when the configured fragmentation threshold would otherwise require fragments. The current Compressed path retains this gate.

Basic fragment support follows the 2016 agreement and PPDU conditions. Clause 10.2.7 of that edition prohibits MSDU fragmentation under HT-immediate and HT-delayed agreements. A Basic profile label does not override that prohibition. HCF must establish fragmentation eligibility before the service constructs fragment previews. Non-HT Basic cases can use permitted fragments under Clause 10.5.

### Read-only selection before admission

HCF will use the frame preview without extraction for initial classification and continuation selection. [HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) will preserve BAR-first arbitration. It will invoke preparation before an accessor can call the destructive data-service path. A null Block Ack handler must select the Normal Ack preparation path.

The [data-service contract](../../src/inet/linklayer/ieee80211/mac/contract/IOriginatorQosMacDataService.h) will retain its source-ID mechanism. Preparation will own duplicates and borrow source identities from the queue. HCF will select one actual DATA mode through rate selection. HCF will reuse that decision for aggregate evaluation, ordinary fallback, protection, and submission.

Agreement validation will accept an explicit absence of agreement for Normal Ack operation. If the agreement state changes across an event boundary, HCF will cancel the preparation. An accepted Basic agreement will use its own permission and ACK policy. Compressed and A-MPDU checks will retain their current feature conditions.

The current [originator policy](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementPolicy.cc) and [recipient policy](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementPolicy.cc) allow Basic regardless of their HT inputs. HCF calls these policies before setup initiation and request acceptance. The separate peer-permission plan owns changes to those decisions, warnings, and unknown-peer behavior. This TXOP plan changes admission after path selection; it does not change agreement negotiation.

A-MSDU preparation must also require local HT implementation and the peer's received HT capabilities under Clause 10.11. An absent agreement or handler does not establish that permission. Without those capabilities, HCF must use an ordinary MSDU or permitted fragments. The Normal Ack fallback cases must verify this condition.

### Complete cost and permitted forms

HCF will calculate the exchange cost through the selected PHY mode objects. The cost starts with the first holder frame. It includes DATA, required RTS/CTS, internal IFSs, and the immediate response when required. The separate TXOP comparison also includes the IFS before a continuation.

Normal Ack DATA includes SIFS and ACK duration. Basic DATA with Block Ack policy does not acquire a fictitious immediate ACK or Block Ack response. A later BAR/Block Ack exchange retains its own admission and arbitration. Its response cost uses the actual Basic or Compressed frame length.

HCF will evaluate these forms in order:

1. Evaluate the permitted A-MSDU form with the retained DATA mode.
2. If that form fails, evaluate the selected ordinary MSDU with the same mode.
3. If ordinary DATA needs permitted fragmentation, evaluate a legal fragment preview.
4. If no permitted form fits this TXOP, end the sequence without source extraction.

A fragment group will use a size bound for a fresh TXOP at the retained mode. The current TXOP still admits each fragment through its own complete cost. The service must not divide an MSDU into smaller fragments solely to consume a short TXOP tail. This choice avoids tail-dependent fragment sizes and preserves the required equal-length relation.

This choice can leave unused time at the end of a TXOP. It is an implementation policy, not a requirement to use every available interval.

HCF will distinguish an empty candidate set, temporary admission refusal, an impossible fresh-TXOP form, and an actual transmission failure. Temporary refusal uses `DEFERRED`; an impossible fresh-TXOP form raises the diagnostic specified below. HCF will retain `LEGACY` only for paths outside this QoS DATA scope. An ordinary scoped candidate must not escape admission through that result.

### Nondynamic fragment preparation

This preparation applies only after the [fragmentation eligibility check](#fragmentation-eligibility). It must not create MSDU fragments under an HT-immediate Block Ack agreement. Normal Ack outside an agreement and permitted Basic cases under 2016 can use the fragment mechanism.

The data service will extend its reversible preparation to own a complete group of fragment previews. The first preview must retain its pointer through frame-sequence steps. Additional fragment previews belong to the same preparation. The source MSDU remains queued until the selected commitment boundary.

HCF will derive a maximum MPDU length from complete-exchange airtime through PHY mode queries. The data service will pass that bound to the fragmentation policy. The policy will combine it with the configured fragmentation threshold. The bound must account for actual header length, FCS, response mode, and RTS threshold transitions.

The [fragmentation contract](../../src/inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h) will gain a bounded operation. Its unbounded operation will remain available to current callers. [BasicFragmentationPolicy](../../src/inet/linklayer/ieee80211/mac/fragmentation/BasicFragmentationPolicy.cc) will own fragment-size calculation. HCF must not duplicate that calculation or change a module parameter for one frame.

The new operation remains pure virtual under [contract purity](../../doc/project/rule/architecture.md#ar-org-contract-purity). This deliberate source change requires migration instructions in C1. An unbounded result followed by rejection cannot supply required smaller fragments. A fallback must not silently ignore the bound or report physical impossibility when only the selected policy lacks bounded support. This plan adds no default body or general adapter to the interface.

The service will use the current [fragment builder](../../src/inet/linklayer/ieee80211/mac/fragmentation/Fragmentation.cc) on a duplicate. It will validate even nonfinal lengths, the final length, and the sixteen-fragment bound before source extraction. It will never apply nondynamic fragmentation to an A-MSDU. If needed, it will first abandon the aggregate preview.

Fragment commitment will consume the source once. For an unnumbered queued source, it will assign one sequence number and one allocation identity to the whole group. A numbered source will retain those identities. Every fragment will inherit the source's absolute lifetime deadline. InProgressFrames will adopt the already constructed fragments through its current ownership mechanism. Commitment must not reconstruct or delete the first preview that a frame-sequence step references.

The positive-limit exception check will identify actual transmission history. A Retry bit inherited from another hop is insufficient. Same-size retry validation must establish a prior local MPDU transmission and unchanged length. A fragment group will retain the history needed for the standard's previous-fragment retry exception after an earlier fragment leaves InProgressFrames.

InProgressFrames will own that narrow fragment-group history, keyed by allocation identity. Group completion, expiry, applicable retry exhaustion, or agreement disposal will remove this history. Both selected editions permit the previous-fragment retry and sixteen-fragment exceptions under their stated conditions. Basic needs no initial PHY mode record or lower-rate comparison. Unrelated Data or Management transmissions in the TXOP still invalidate an applicable single-frame exception.

A collision or failed RTS does not establish DATA transmission history. Basic's 2016 retransmission exception still requires an actual prior local MPDU transmission. It does not require a lower selected PHY rate. Fragment retransmissions retain the body and length of the transmitted fragment.

A configuration can leave no legal first fragment at the selected mode. The bounded policy must distinguish no fragmentation requirement, valid fragment sizes, and no legal partition. HCF must test all permitted candidate forms and applicable exceptions before it treats that last result as an unsupported exchange. An empty queue remains a separate condition.

When no permitted form fits a fresh TXOP, HCF will raise `cRuntimeError` before further source extraction, replacement, sequence allocation, or recovery mutation. The diagnostic will identify the peer, TID, selected mode, TXOP limit, and failed size or airtime constraint. This is a deliberate limit of the model at the selected mode. It does not establish that every PHY mode would fail. HCF must not return `DEFERRED`, discard the source, or request another contention attempt for this condition.

This fatal diagnostic remains the proposed model policy. IEEE does not require a fatal simulation error. The policy preserves the source and avoids repeated contention without progress at the selected mode. A drop with a new reason would change the source-retention requirement. Automatic selection of another DATA mode would add a separate admission policy.

When a form fits a fresh TXOP but not the current tail, HCF will return `DEFERRED` and end this TXOP normally. Queued originals remain queued. A source that an earlier internal collision already committed remains under recovery. This terminal error requires no new timer, retry controller, or preparation outcome.

### MSDU retry state and fragment disposal

The current recovery maps use sequence and fragment numbers and separate counters at the RTS threshold. Those maps cannot alone preserve the required MSDU retry state after fragment replacement or a partial fragment ACK. Sequence preservation is therefore insufficient. C2 prepares MSDU recovery before C4 or C5 activates a new admission path.

QosRecoveryProcedure will own one retry record for each committed MSDU or A-MSDU, keyed by its current allocation identity. Every fragment in an ordinary MSDU group refers to that record. The modern no-agreement path will use one cumulative frame retry count under Clause 10.23.2.12.1. Basic will use the short and long MSDU counts under the 2016 Clause 10.22.2.11.1. These records replace the scoped per-fragment counter representation; they must not mirror its values in a second store. Current Compressed and A-MPDU recovery remain outside this replacement.

The modern frame count uses `dot11ShortRetryLimit` regardless of the RTS threshold class. Basic uses the 2016 short or long retry limit where its retry procedure applies. MSDUs sent with Basic Block Ack instead follow Clause 10.24.3's lifetime rule. Normal Ack outside an agreement retains one retry record for a permitted A-MSDU. Accepted Basic fixtures use ordinary MSDUs or permitted fragments, without A-MSDU aggregation.

Recovery calls that need the MSDU record must receive the protected packet or its existing allocation identity. A header-only call cannot recover that identity from a provisional sequence or fragment number. HCF already has the affected packet at these dispatch points. Agreement transitions must preserve or retire the existing record through its owner, without a parallel counter initialization.

An actual collision, associated RTS failure, or DATA failure updates the record through the recovery owner under the selected contract. Modern frame counts do not split or reset when the fragment number or RTS threshold class changes. A successful nonfinal fragment ACK does not reset that modern MSDU count. Basic applies the 2016 increment and reset conditions, including their agreement conditions. Access-category recovery remains separate and follows the selected edition's EDCA procedure.

Under an accepted Basic agreement, a DATA acknowledgment failure must not acquire a new MSDU retry limit from this correction. Clause 10.24.3 governs those DATA retries through lifetime. A failed associated RTS still updates the MSDU short count under Clause 10.22.2.11.1. A Normal Ack policy does not itself remove the agreement or change its scope.

The following comparison applies to DATA outside an accepted agreement, unless the row states otherwise. Each row identifies a separate requirement and its direct test. Basic uses its 2016 contract even before agreement acceptance. The 2024 contract applies to the modern no-agreement path.

| Trigger | Basic under 2016 | Modern operation under 2024 | Required direct test |
| --- | --- | --- | --- |
| DATA fails at or below the RTS threshold. | Increment the MSDU short count. | Increment the single frame count. | Test PSDU lengths below, equal to, and above the threshold. |
| DATA fails above the RTS threshold. | Increment the MSDU long count. Apply the long limit. | Increment the single frame count. Apply the short limit. | Use different short and long limits with repeated failures. |
| A nonfinal fragment receives its ACK. | Reset the applicable MSDU count under the stated success conditions. | Retain the cumulative MSDU count. | Fail another fragment after that ACK. |
| An ordinary source becomes fragments across the RTS threshold. | Retain both MSDU counts and the allocation identity. | Retain the single count and the allocation identity. | Replace a numbered source after a real collision. |
| The selected retry limit expires. | Check both applicable counts rather than only the current fragment's size class. | Check the cumulative count against the short limit. | Carry a prior short count into a larger form, and a prior long count into smaller fragments. |
| EDCA recovery processes failures and success. | Retain the 2016 QSRC and QLRC conditions. | Apply the 2024 QSRC and contention-window conditions. | Observe AC counters and window changes separately from MSDU counters. |
| An associated RTS fails under an accepted agreement. | Increment the MSDU short count. Retain the allocation beyond the retry limit under the accepted R1 rule. | Current Compressed preparation updates AC recovery without a provisional MSDU count. | Execute both named Basic RTS exhaustion cases. |

Hypothetical example: a collision affects an ordinary MSDU above the RTS threshold before any DATA transmission. The service later replaces it with fragments below that threshold. The allocation identity and retry record remain the same. The first fragment inherits the collision state rather than a new counter value. A later fragment failure updates that same MSDU record under the applicable contract.

For numbered-source replacement, the service will transfer the allocation identity without another recovery initialization. InProgressFrames will replace the original frame and its ACK entry with the prepared fragment entries at the selected commitment boundary. QosRecoveryProcedure will retain the existing MSDU record through that replacement. For a new queued source, the committed group creates one record with the selected contract's initial values.

At an applicable retry-limit disposal, HCF will cancel retained preparation that refers to the affected allocation identity. InProgressFrames will dispose of every remaining fragment in that group through its existing deferred-deletion mechanism. HCF will remove their ACK and lifetime entries through the current owners. QosRecoveryProcedure will retire the MSDU retry record once. Expiry and agreement disposal use the same group cleanup. A final successful ACK removes the record only after no member remains; unrelated allocations remain intact.

These cleanup requirements do not decide whether Basic RTS exhaustion triggers disposal under an accepted agreement. R1 must supply that trigger before C5 activates Basic.

### Commitment and RTS recovery

Commitment before RTS gives MSDU recovery a stable allocation identity. This is an implementation choice, not a standard queue boundary. The selected standard requires correct retry counts and disposal at the applicable limits. Neither selected contract prescribes when INET must extract queued sources or assign sequence numbers.

Under the modern contract outside an accepted Block Ack agreement, a failed associated RTS must increment the MSDU or A-MSDU frame retry count. This requirement applies even when DATA never transmits. Clause 10.23.2.12.1 supplies this requirement in the [standards contract](#applicable-requirements). Basic's 2016 Clause 10.22.2.11.1 instead increments the MSDU short count and access-category short count after a failed associated RTS.

The [data service](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) gives a preview the next sequence number without allocation. Commitment assigns the sequence and allocation identities and transfers the prepared form to InProgressFrames. The [recovery owner](../../src/inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.cc) then retains the MSDU record described above. Basic's commitment before RTS lets that owner retain the required 2016 associated RTS count. The accepted R1 rule controls exhaustion.

Hypothetical example: no Block Ack agreement applies, admission succeeds, and HCF sends RTS. No CTS arrives, so DATA never transmits. The frame retry count must still increase. The proposed service commits before RTS so the same committed form retains that count across later attempts. If a disposable preview instead owned that count, preview disposal could lose its frame association. A later candidate could reuse the provisional sequence and acquire retry state for different sources.

Transmission admission precedes commitment of a new source for that transmission. An earlier internal collision can already commit one ordinary source for recovery before its channel grant. Modern no-agreement operation and Basic commit a new admitted form immediately before actual RTS submission. An accepted Compressed agreement retains reversible preparation across RTS/CTS. The current [HCF failure handler](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) updates AC recovery for that Compressed path without a provisional DATA retry entry.

The selected contract and agreement state control this distinction. The ACK policy bit alone does not control it. Normal Ack DATA under an accepted Compressed agreement follows its agreement-scoped commitment rule. Normal Ack in the Basic profile follows its 2016 MSDU recovery rule and commits before RTS. Without RTS, new transmission sources commit immediately before DATA. Collision-owned sources retain their earlier commitment.

This earlier boundary applies only after complete-exchange admission succeeds. A refusal before RTS preserves all original queued sources. After actual RTS submission, the accepted committed form belongs to ordinary retry and lifetime procedures. A later capability change can cancel DATA, but it must not restore that attempted form as new queued MSDUs. This distinction separates admission refusal from recovery after an actual protection attempt.

The implementation must distinguish an RTS retry from a DATA retransmission. An RTS failure before any DATA transmission does not prove the same-size MPDU exception. Outside an applicable initial-MSDU exception, it must not permit an oversized first DATA transmission merely because RTS failed.

Commitment before DATA could also satisfy an MSDU recovery contract that requires RTS retry counts. The accepted design instead reuses the committed owner before RTS. The comparison below describes implementation costs rather than standard queue boundaries.

| Basic ownership choice | State and behavior | Cost and limit |
| --- | --- | --- |
| Commit before RTS after admission. | The existing committed-frame owner retains the allocation, deadline, and MSDU counter through CTS failure. | Reuse the committed recovery record. After an actual RTS attempt, cancellation retains committed ownership. |
| Commit before DATA. | Sources remain queued through RTS/CTS. Persistent state outside the disposable preview must retain each source's RTS count. | Define identity transfer and cleanup for expiry, cancellation, and source replacement. |
| Reuse Compressed's AC-only failure path directly. | The preview disappears after CTS failure without an MSDU counter update. | This omits Basic's explicit 2016 counter requirement and is insufficient. |

Basic commits before RTS because that boundary reuses the committed owner. A different retry contract alone does not require that choice. Absence of retry-limit disposal does not remove the mandatory short-counter update. Both viable choices use the same two temporal boundaries: before RTS and before DATA.

### Internal collision before DATA transmission

The internal-collision handler is another destructive entry. Its ordinary-only preparation currently covers active Compressed agreements. Other paths can create an aggregate before any admission decision. Extend the ordinary-only collision path to Basic and absent-agreement QoS DATA.

An internal collision is a real recovery event. The handler can commit one ordinary source to obtain the current recovery identity. It must preserve unrelated queued MSDUs. It must not form an unadmitted aggregate or query adaptive DATA selection solely for collision recovery.

This is the explicit exception to commitment after transmission admission. A later admission refusal preserves this committed source and its retry record in InProgressFrames. It does not restore that source to the pending queue or undo the collision count. The refusal still preserves every source that remains queued.

That committed ordinary source can still require fragmentation before its first DATA transmission. The fragment preparation operation must accept this numbered, never-transmitted source from InProgressFrames as well as an unnumbered queued source. Its duplicates remain reversible until admission succeeds. Replacement must preserve sequence identity, allocation identity, absolute deadline, and the MSDU's retry count.

[QosAckHandler](../../src/inet/linklayer/ieee80211/mac/originator/QosAckHandler.h) distinguishes a frame with no DATA transmission from a frame that awaits acknowledgment. Reuse that distinction before the first fragment form. A collision or RTS retry alone must not activate the same-size DATA retransmission exception. After a real DATA transmission, the service must preserve the transmitted body and length.

```text
Queued MSDUs
    |
    v
Owned duplicate previews -- no permitted form --> End TXOP; retain queued MSDUs
    |
    | Complete exchange qualifies
    v
No RTS: commit before DATA
RTS with accepted Compressed agreement: retain sources through CTS; commit before DATA
RTS with no modern agreement: commit before RTS; use MSDU recovery
RTS with 2016 Basic: commit after admission; retain the MSDU under the accepted R1 rule
    |
    v
InProgressFrames owns the admitted MPDU or fragment group
```

### Validation, timing, and state lifetime

The QoS context will retain the selected DATA, RTS, CTS, and ACK modes across the real RTS/CTS event boundary. Validation will use pure permission and response queries. It must not make another adaptive DATA selection merely to validate an unchanged preparation. Context disposal will release unused previews after dependent frame-sequence steps.

HCF will validate source membership, eligibility, deadlines, sequence state, agreement state, mode permissions, and the complete cost before the first submission. It will validate retained inputs again before DATA after CTS. Expiry or a capability change must cause cancellation without a stale pointer or synthetic transmission failure.

Positive-limit admission permits complete cost plus the IFS before the exchange to equal the available TXOP time. A continuation must also satisfy the applicable strict TXNAV comparison at the previous exchange completion. Preserve the current reference point before that IFS. Do not replace these comparisons with one rounded deadline.

For multiple protection, preserve the raw reservation interval and the current whole-microsecond Duration encoding rules. A permitted initial or retry exception must protect its actual exchange. Admission must not reduce the reservation to zero because that exchange exceeds the nominal positive limit.

A zero limit counts the permitted data unit, not simply one DATA frame. A nondynamic fragment burst can contain several PPDUs for one MSDU. It must not admit a second unrelated MSDU. Required responses, protection, and separately permitted BAR exchanges retain their standard conditions.

This design adds no timer, zero-delay event, random draw, configuration switch, general transaction controller, or generic queue API. The real RTS/CTS boundary justifies retained preparation state. The current owners retain sequence, lifetime, recovery, and TXOP state.

## 5. Affected components and compatibility

| Component | Proposed responsibility and affected files | Compatibility effect |
| --- | --- | --- |
| HCF | Admission, agreement conditions, selected modes, commitment edge, and recovery dispatch in `Hcf.cc` and `Hcf.h`. | Basic and Normal Ack trajectories can change where an old exchange exceeds a bound. |
| Frame sequences | Classification and continuation without extraction in `HcfFs.cc`, `TxOpFs.cc`, and `FrameSequenceContext.*`. | Preserve BAR-first arbitration, management selection, and A-MPDU selection. |
| QoS data service | Fragment previews, source validation, and group commitment in `IOriginatorQosMacDataService.h` and `OriginatorQosMacDataService.*`. | Add explicit fragment operations; retain single-frame and A-MPDU batch semantics. |
| Fragmentation policy | Bounded fragment-size calculation in `IFragmentationPolicy.h` and `BasicFragmentationPolicy.*`. | External policy implementations must implement the bounded contract; document the source migration. |
| Committed-frame owner | Group adoption and necessary local fragment history in `InProgressFrames.*`. | Preserve queue service effects, ACK state, and group disposal. |
| Recovery and lifetime owners | MSDU retry records and identity-aware dispatch in `QosRecoveryProcedure.*`; group cleanup through the existing lifetime owner. | The 2024 and 2016 counters follow their selected contracts. Fragment replacement does not initialize another count. |
| Tests and user documents | Direct cases under `tests/module/` and `tests/unit/`; user effects in `WHATSNEW` and the migration guide. | Explain changed timing, bounded-policy migration, and the scoped 2016 Basic evidence. The separate peer-permission PR owns negotiation migration. |

The interface additions serve HCF and the QoS data service, both current consumers. A single-frame preview cannot represent group ownership or transfer. An unbounded fragmentation query cannot honor the actual TXOP airtime constraint. Extend those current contracts instead of a new module or parallel service. Apply [minimal-design guidance](../../doc/project/rule/quality.md#qr-design-minimal) to the final patch.

No NED parameter, wire field, serializer, generic Packet API, or PHY duration formula needs a change. The listed MAC paths are outside the [sealed Packet subtree](../../doc/project/audit/seal-list.md). Preserve the current A-MPDU batch contract in this checkout. Fragment preparation must not reuse that contract as if fragments were an A-MPDU batch.

## 6. Implementation steps and dependencies

Each implementation commit must contain its direct tests. These steps define the design. The execution record identifies the implemented steps and their verification.

| Step and purpose | Responsible component and files | Proposed change and expected result | Verification |
| --- | --- | --- | --- |
| C1. Prepare legal fragment forms. | QoS data service, fragmentation policy, InProgressFrames, and their contracts. | Add reversible fragment preparation and explicit group commitment. Preserve current dispatch and single-frame operations. Add the bounded-policy migration entry with the interface change. | Extend the preparation fixture. Check cancellation, pointer transfer, sequence allocation, deadlines, lengths, and disposal. Add bounded-size unit cases. |
| C2. Prepare MSDU recovery and group cleanup. | QosRecoveryProcedure, identity-aware dispatch contracts, InProgressFrames, and lifetime integration. | Add the tested retry records and group disposal operations. Keep current production dispatch until activation. Retain distinct 2016 and 2024 rules. | Use the real recovery and committed-frame owners for threshold, fragment ACK, allocation-transfer, and sibling-disposal cases. Require unchanged Compressed and A-MPDU counter histories and cleanup. R1-dependent Basic outcomes remain unavailable. |
| C3. Prepare selectors and common admission. | HCF, HcfFs, TxOpFs, and the QoS context. | Add read-only selection and complete-cost operations for the planned callers. Preserve current path selection until C4 or C5 activates them. | Exercise the production selectors, HCF admission operations, QoS context, and InProgressFrames. Check BAR arbitration, one DATA mode decision, cost boundaries, and Compressed and A-MPDU controls. |
| C4. Activate modern Normal Ack admission. | HCF and frame-sequence dispatch for the 2024 no-agreement path. | Select the prepared admission, fragment, and recovery operations from C1–C3. Apply commitment before RTS or DATA and the terminal diagnostic. | Add production admission and fragment cases with this commit. Include `NormalAckZeroLimitFragmentBurst` with protection and ACKs. Check queue preservation, RTS exhaustion, collision replacement, and disposal. |
| C5. Activate Basic admission under 2016. | HCF and frame-sequence dispatch for the enabled Basic profile, with or without an accepted agreement. | R1 selects lifetime retention under an accepted agreement. B1 supplies a legal failing continuation. Commit after admission before RTS. Activate Basic admission and its no-agreement Normal Ack fallback with the 2016 recovery rules. Preserve agreement negotiation. | Correct the legal baseline with the named non-HT cases. Assert R1's exhaustion outcomes and the DATA lifetime rule. Retain HT Compressed controls. Complete joint verification when peer permission is active. |
| C6. Complete integration evidence and user documentation. | Focused module suites, protection units, `WHATSNEW`, and the migration guide. | Cover shared selectors and user effects. Complete the direct evidence and documentation from C1–C5. | Run focused campaigns and required project gates. Record every changed fingerprint without a baseline update. |

C1–C3 prepare tested mechanisms without activation of new admission paths. C4 and C5 activate only paths whose fragment, recovery, and disposal prerequisites already exist. Each activation commit includes its direct production regressions. C6 cannot defer those direct cases. The order follows [preparation before activation](../../doc/project/rule/pull-request.md#pr-split-prepare).

R1 prevents C5 from activating Basic with an unspecified exhaustion outcome. B1 prevents activation without a failing legal Basic baseline. The C5 owner must satisfy both conditions before that step receives an implementation schedule. Neither condition prevents the independent C1–C4 work described here. The full plan remains incomplete until Basic also satisfies its acceptance criteria. The separate peer-permission PR has its own activation commit and the combined verification obligations below.

## 7. Verification methods and expected results

### Direct test matrix

Create `tests/module/Ieee80211TxopAdmission_1.test` for production admission and queue invariants. Create `tests/module/Ieee80211TxopFragmentAdmission_1.test` for fragmentation through the same caller. Create `tests/module/Ieee80211TxopMsduRecovery_1.test` for direct retry-owner and group-disposal cases. Create `tests/unit/Ieee80211BoundedFragmentation_1.test` for fragment-size boundaries. These proposed files do not yet exist.

C2 tests must call the production recovery and committed-frame owners directly before new dispatch activates. They must transfer, replace, acknowledge, and dispose of real committed frames through InProgressFrames. C3 tests must execute the production selectors and HCF admission operations with the real QoS context and committed-frame owner. Direct mechanism tests do not require full wireless exchanges before C4. Test substitutes may provide stimuli or observe results; they must not replace recovery, ownership, selector, or admission logic under test.

C2 must also preserve the current Compressed and A-MPDU recovery behavior. The current Compressed admission fixture checks AC counter histories and absence of provisional MSDU counters after RTS failure. The A-MPDU admission fixture checks absence of member counters after RTS failure. Those assertions remain required controls.

Extend the production exchange fixtures to assert exact counter increments after DATA or member failures. Check successful acknowledgment cleanup and unchanged AC recovery. Compare both short and long size classes with the parent revision.

The Compressed checks belong in `Ieee80211CompressedBlockAckAdmission_1.test` and its exchange fixture. The A-MPDU checks belong in `Ieee80211AmpduAdmission_1.test` and `Ieee80211AmpduExchange_1.test`. Cover partial Block Ack and absent Block Ack responses. Unacknowledged members must retain their individual counts; acknowledged members must leave no retry entry. A delivery assertion or Retry bit alone cannot establish this requirement.

C4 owns `NormalAckZeroLimitFragmentBurst` in the proposed fragment-admission fixture. Use two queued MSDUs and a legal fragment partition for the first MSDU. Require all fragments of that MSDU within one zero-limit TXOP, with the required ACKs. Require the second MSDU to wait for another TXOP. Repeat with RTS disabled and forced, under both protection classes. Check one sequence identity, one allocation, absolute deadlines, and final ACK completion.

The following proposed configurations identify the direct non-HT Basic evidence. They use `g(erp)`, explicit Basic selection, no A-MSDU aggregation, legal MSDU sizes, and legal fragmentation thresholds. Their names identify cases within the proposed files; they do not require one new file per case. The existing Basic compatibility fixture remains a control for production exchanges and fragment receipt bits.

| Proposed file and configuration | Required evidence |
| --- | --- |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtInitialException` | Admit the eligible initial ordinary MSDU under an accepted agreement. Reject the exception without that agreement or after earlier DATA. |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtContinuation` | Refuse a later exchange that cannot fit. Preserve queued sources, counters, and deadlines. |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtRtsExhaustionBeforeData` | Under an accepted agreement, exercise short-limit exhaustion before any local DATA transmission. Assert R1's selected retention or disposal outcome. |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtRtsExhaustionAfterData` | Repeat exhaustion after an earlier local DATA transmission. Apply R1's selected outcome and cleanup. |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtNormalAckUnderAgreement` | Include ACK airtime while the agreement remains accepted. Verify the final commitment and recovery rules from R1. |
| `Ieee80211TxopAdmission_1.test`: `BasicNonHtDataLifetime` | Lose Basic Block Ack responses. Preserve MSDUs until the applicable lifetime disposal rather than a DATA retry limit. |
| `Ieee80211TxopFragmentAdmission_1.test`: `BasicNonHtFragments` | Prove legal fragment sizes, one allocation, inherited deadlines, fragment history, and the Basic receipt bitmap. |
| `Ieee80211TxopMsduRecovery_1.test`: `BasicNonHtRecoveryBoundaries` | Check 2016 counters below, at, and above the RTS threshold. Compare distinct retry limits and successful fragment resets. |

The two RTS exhaustion configurations must also check AC recovery, group siblings, ACK entries, and absolute deadlines. R1 must supply their expected terminal outcomes before C5. A test with an unspecified expected retention or disposal result does not count as evidence.

The following configurations provide legal controls through production setup and frame exchanges. They belong in the proposed `Ieee80211TxopAdmission_1.test` file. Each configuration uses one fixed seed. The separate peer-permission plan owns invalid-pair tests and changes to current negotiation expectations.

| Proposed configuration | Required evidence |
| --- | --- |
| `BasicMixedHtPeerAllowed` | Use production infrastructure association with one HT peer and one non-HT peer. Require known classifications and Basic exchanges without A-MSDU aggregation. Repeat with the endpoint roles reversed and DATA in both directions. |
| `CompressedHtControl` | Use two HT peers with explicit Compressed selection. Require an accepted Compressed agreement and valid DATA/BAR/Block Ack exchanges. Preserve the current delivery and recovery assertions. |

| Condition | Required direct observation | Failure exposed before the correction |
| --- | --- | --- |
| Legal Basic agreement, later ordinary MSDU | Check actual DATA completion, TXOP start, frame count, agreement, and peer capabilities. Refuse a continuation that cannot fit. | Current continuation checks do not prove complete-exchange containment. The historical HT Basic overrun does not replace this legal case. |
| Block Ack disabled, 24 Mbit/s, multiple protection | Check actual DATA and ACK completion on both hops. | A later aggregate exceeds the complete-exchange bound. |
| Block Ack disabled, 6 Mbit/s, fresh positive TXOP | Observe legal fragments and complete payload delivery. | The first unfragmented ordinary DATA exchange exceeds the bound. |
| Aggregate fails, ordinary MSDU fits | Check exact source IDs after commitment. | Destructive aggregation consumes more sources than the admitted form needs. |
| Neither form fits the current tail | Snapshot queue order, owners, deadlines, sequence counters, retry counters, and queue service observations before preparation. Compare after refusal. | Current early extraction changes source state before admission. |
| No permitted form fits a fresh TXOP | Require the terminal diagnostic before further source consumption, sequence allocation, or recovery mutation. Check retained sources and absence of a new contention request. | Ordinary `DEFERRED` cannot distinguish an unsupported exchange from a short tail. |
| RTS disabled or threshold crossed by fallback | Observe the actual first holder frame and cost. | A retained protection decision can describe the rejected larger form. |
| No agreement, repeated real CTS timeouts | Check the same committed identity, cumulative frame retry count, AC recovery, retry-limit disposal, and preserved absolute deadlines. | Generic Compressed RTS recovery omits the frame retry increment. Preview disposal can also lose the count or attach it to different sources. |
| Internal collision before a channel grant | Check ordinary-only commitment, one recovery outcome, and intact unrelated sources. Admit a legal first fragment form from a numbered source. | A legacy collision can form an oversized aggregate before admission. |
| Accepted Basic agreement, real CTS timeout | Check the 2016 MSDU short count and AC recovery. Assert R1's exhaustion outcomes before and after local DATA. Verify the finalized ownership boundary. | Reuse of Compressed control-only recovery omits the MSDU update. Unspecified exhaustion can silently change disposal behavior. |
| Normal Ack DATA under an accepted agreement | Require commitment before DATA for accepted Compressed. Use Basic's final boundary after R1. Include SIFS and ACK airtime. | ACK policy alone does not select the applicable recovery contract. |
| Collision before fragmentation across the RTS threshold | Preserve one allocation and its retry record through replacement. Check 2024 count retention and the 2016 count-specific increment and reset conditions. | A per-fragment key or size-based counter switch can lose the collision state. |
| Failure after a nonfinal fragment ACK | Retain the 2024 MSDU count across the ACK. Apply Basic's 2016 reset rules only under their specified conditions. | Per-fragment counters cannot establish the required MSDU state. |
| Distinct short and long retry limits | Require the modern frame count to stop at the short limit across both size classes. Check Basic's 2016 limits outside an agreement. | Reuse of the old size-selected limit can violate the modern frame-count contract. |
| Basic contract under 2016 | Use non-HT peers without A-MSDU and legal data sizes. Test the initial ordinary-MSDU exception under an accepted agreement. Without an agreement, require initial containment and permitted fragments. | A 2005 admission rule would wrongly reject the eligible initial ordinary-MSDU exception. |
| Basic DATA failure under an accepted agreement | Apply Clause 10.24.3's lifetime rule. Verify that ordinary DATA retry limits do not discard the Block Ack MSDU. | Extension of MSDU retry records could impose a new retry limit on Block Ack DATA. |
| Basic between an HT peer and a non-HT peer | Observe accepted Basic setup and admitted exchanges in both directions. Exclude A-MSDU aggregation. | This control preserves legal Basic permission through the admission changes. |
| HT-immediate agreement with a low fragmentation threshold | Require no fragmented MSDUs. Repeat with Normal Ack DATA under the accepted agreement. | A profile-based extension could bypass the agreement's fragmentation prohibition. |
| Source expiry or capability change during CTS wait | Check cancellation, preview disposal, retained committed ownership where applicable, and absence of stale DATA. | A wider preparation path can retain invalid source or mode inputs. |
| AARF and Onoe | Count adaptive DATA queries. Compare admitted and submitted modes. | Independent preview and submission queries can change the chosen mode. |
| Positive-limit boundaries | Test complete cost one simulation tick below, equal to, and above the available interval. | Start-time checks do not establish complete containment. |
| TXNAV boundaries | Test cost below, equal to, and above TXNAV at the correct reference point. | A nonstrict comparison or shifted reference admits or rejects the wrong continuation. |
| Zero limit; direct C4 requirement | Test one ordinary MSDU and one permitted A-MSDU. Run `NormalAckZeroLimitFragmentBurst` for every fragment of one MSDU, with protection and ACKs. Reject a second unrelated MSDU in that TXOP. | A one-DATA-frame counter incorrectly ends a legal fragment burst. |
| Initial and retry exceptions | Test the eligible initial ordinary-MSDU exception under both accepted agreement profiles. Reject it for an initial A-MSDU or earlier DATA in the TXOP. | A profile-only exception admits an exchange without the required form and frame-count conditions. |
| Fragment history | Test prior fragment retry, ACK removal, group cleanup, and sixteen-fragment exceptions under both selected editions. Require no Basic lower-rate test. | Lost history can reject a permitted exception or permit an unsupported overrun. |
| Lifetime and retry exhaustion | Check all fragment siblings and group state after disposal. Check an independent peer and TID. | Partial cleanup can retain stale history or dispose unrelated frames. |
| Compressed and A-MPDU recovery; direct C2 requirement | Require unchanged RTS and DATA failure counts, per-member increments, acknowledgment cleanup, AC counters, and contention-window changes. | Shared recovery code can change counters even when delivery and Retry bits still look correct. |
| Compressed, A-MPDU, management, and group controls | Observe the intended production sequence and current delivery or recovery assertions. | A shared-selector change can alter paths outside the correction. |

Use one fixed seed for each deterministic invariant. Vary only the named rate, profile, protection, RTS, limit, and event-order conditions. No throughput, fairness, probability, or broad robustness claim belongs to this verification. Additional random seeds cannot replace the exact boundary cases.

### Cross-plan verification and activation order

Whichever PR lands second must run the other plan's applicable cancellation tests against the combined source. Include capability changes during CTS wait, retained preview disposal, committed ownership, and agreement teardown. Run `BasicCapabilityChange` from the peer-permission fixture when it exists. Run the source-expiry and capability-change cases from this plan's admission fixture. Record absent cases as pending, not as executed evidence.

The joint configuration is `BasicRefusalNormalAckTxopAdmission` in the proposed `Ieee80211TxopAdmission_1.test` fixture. It requires peer-permission activation P2 and this plan's Basic-profile activation C5. C4 covers the 2024 no-agreement path. C5 covers the enabled Basic profile's 2016 fallback. C5 now satisfies R1 and B1. The joint refusal case remains pending until P2 exists.

The joint case must establish a known two-HT pair through production infrastructure association. Both endpoints explicitly select Basic. Require refused setup and Normal Ack DATA, without an automatic variant change or an accepted Basic agreement. Check complete TXOP containment, queue preservation on a short-tail refusal, and legal fragments when a fresh TXOP requires them. Use Basic's 2016 recovery rules for that fallback.

The later required activation owns this joint configuration and its passing assertions. If P2 lands between C4 and C5, its independent tests establish refusal and Normal Ack operation only. Joint TXOP admission remains pending until C5. If C5 lands first, P2 must include the joint case with its activation. This integration obligation does not combine the production commits.

After both sets of fixtures exist, use this additional focused filter from the implementation checkout:

```sh
inet_run_module_tests -m debug -f 'Ieee80211(BasicPeerPermission|TxopAdmission|TxopFragmentAdmission|PreparedCancellation|PreparedQosData)_.*\.test'
```

### Commands for implementation verification

Use the [focused build procedure](../../doc/project/guide/run-the-gates.md) after compiled inputs change. Run these commands from the implementation checkout. Confirm that each selector includes its proposed files before execution.

```sh
make MODE=debug -j"$(nproc)"
inet_run_unit_tests -m debug -f 'Ieee80211(BoundedFragmentation|SingleProtection|MultipleProtection|TxopProcedure)_.*\.test'
inet_run_module_tests -m debug -f 'Ieee80211(TxopAdmission|TxopFragmentAdmission|TxopMsduRecovery|PreparedQosData|CompressedBlockAckAdmission|CompressedBlockAckExchange|BasicCompatibility|BlockAckVariant|BlockAckFragmentRetirement|QosRecoveryPeerIdentity)_.*\.test'
inet_run_module_tests -m debug -f 'Ieee80211(EdcaDataLifetime.*|HcfInternalCollision|HcfManagementRecovery|HcfSingleProtection|HcfMultipleProtection|SingleProtectionGroup|PreparedCancellation|AmpduAdmission|AmpduPreparation|AmpduRecovery|AmpduExchange)_.*\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
```

Repeat the Block Ack-disabled reproduction runs 2, 3, 6, and 7 with corrected-timing assertions or the specified unsupported result. Keep Basic runs 0, 1, 4, and 5 as historical evidence. Their successful HT Basic exchanges are not an acceptance target. The named legal Basic cases must establish admission and recovery behavior. The separate peer-permission PR owns refusal of the invalid peer combination.

Repeat the directly affected protocol fragmentation cases through their current suite selector. Select the affected IEEE 802.11 fingerprint rows from the current CSV through an explicit filter. Record the exact rows and causes before any proposed baseline change.

Before publication, run both debug and release compilation and the current project-wide gates. This plan grants no publication or fingerprint-baseline authority. No implementation verification above ran during plan preparation.

### Reproduce the observed baseline

This procedure repeats the historical observation campaign against the pinned baseline libraries. It is separate from corrected-behavior verification above. Use the configured OMNeT++ environment and a debug INET library that matches the selected checkout. The procedure builds only the diagnostic executable. The observation record identifies the libraries actually used.

The commands below rebuild the retained probe outside either checkout. The probe reports violations; it deliberately has no corrected-behavior assertion. Preserve the raw output before any extraction of observations.

```sh
probe_checkout=/home/user/omnetpp_ws/inet-compressed-blockack-s5
probe_plan=/home/user/omnetpp_ws/inet-ieee80211-ampdu-implicit-blockack-a1-a7/plan
probe_parent="$(mktemp -d /tmp/inet-txop-admission.XXXXXX)"
cp "$probe_plan/reproductions/ieee80211-txop-admission-remaining/TxopAdmissionProbe.test" "$probe_parent/"
cd "$probe_parent"
mkdir work
opp_test gen -v -w work TxopAdmissionProbe.test
cd work/TxopAdmissionProbe
opp_makemake -f --deep -lINET_dbg "-L$probe_checkout/src" "-I$probe_checkout/src"
make MODE=debug -j2
for probe_run in 0 1 2 3 4 5 6 7; do
    ./TxopAdmissionProbe_dbg -u Cmdenv -n ".;$probe_checkout/src" --check-signals=false -c General -r "$probe_run" > "$probe_parent/run-$probe_run.log" 2>&1
done
```

The historical comparison used runs 0, 5, 6, and 7 on this checkout at `d841dc77db7d0b141bf31e5e215a0de9bb281957`. To repeat that comparison, select its matching library before Makefile generation. Those run numbers describe historical controls, not corrected-behavior acceptance. After implementation, use runs 2, 3, 6, and 7 with the assertions specified above. Never load S5 headers with this checkout's library or the reverse.

## 8. Risks, decisions, and implementation boundary

Revision 9 requires the legal Basic baseline B1 before C5. The execution campaign supplies B1. The user accepted R1's lifetime interpretation on 2026-10-19. The [decision note](ieee80211-txop-admission-r1-decision.md) records that decision and the commitment boundary before RTS. C5 now uses those rules. The original HT Basic reproduction remains historical evidence. C4 retains its modern no-agreement scope. Joint fallback admission still requires peer permission P2.

The current preparation callback occurs too late for Block Ack-disabled initial classification. This source fact requires the selector change. A final submission guard alone cannot meet the queue-preservation criterion.

Fragment ownership and Normal Ack RTS recovery are necessary parts of the correction. Removal of the Compressed guard without those changes would create incomplete admission and recovery behavior. The plan selects concrete owners and commitment boundaries for both.

The correction can change event order, contention, delivery time, and fingerprints where earlier exchanges exceeded a bound. Those changes require causal evidence. The clean control does not authorize a general claim of identical trajectories.

The bounded fragmentation operation changes the C++ extension contract. The current inventory finds one built-in implementation, BasicFragmentationPolicy, and callers in the QoS and non-QoS data services. The existing non-QoS caller retains the unbounded operation. Recheck that inventory before C1 lands. Provide the external migration entry with C1. An implementation must not silently ignore the supplied airtime bound.

An unsatisfiable fresh-TXOP condition raises the specified diagnostic before further source consumption. The implementation must distinguish that condition from temporary refusal in a short TXOP tail. This choice stops that simulation instead of a contention loop that cannot transmit the selected form. The initial ordinary-MSDU exception remains unavailable without the required accepted agreement, form, and frame-count conditions.

The execution record distinguishes current implementation evidence from the earlier reproduction and proposed verification. The working tree now contains production changes. No recorded fingerprint baseline, commit, or remote branch changes during execution.

The active target is the separate `inet-ieee80211-txop-admission-remaining` checkout at the pinned revision in the execution record. The plan and reproduction remain outside the committed PR #1364 changes. The execution request grants no publication or fingerprint-baseline authority.

This plan uses the current [project map](../../doc/project/README.md), [plan procedure](../../doc/project/guide/write-an-implementation-plan.md), and [plan rules](../../doc/project/rule/planning.md). The plan procedure and rules remain drafts. The user's execution request supersedes the earlier document-only task boundary. The user supplied R1's explicit decision on 2026-10-19.
