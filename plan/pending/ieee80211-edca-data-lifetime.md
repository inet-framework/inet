# EDCA DATA lifetime contract

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [lifetime handler](../../src/inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.cc), [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc)

HCF enforces a finite deadline for every QoS DATA frame that it accepts. This contract applies with Block Ack enabled or disabled. DCF lifetime, complete management lifetime, and MAC stop/start recovery remain outside the contract.

## Previous PRs and S4 scope

Merged [PR #1331](https://github.com/inet-framework/inet/pull/1331) supplies the lifetime mechanism described here. It includes identity-based deadlines, the 0.512-second default, expiry, fragment and aggregate rules, and safe completion across expiry. Merged [PR #1343](https://github.com/inet-framework/inet/pull/1343) uses that deadline during agreement setup and teardown recovery. These mechanisms are already present in S4's master base.

This S4 branch, [PR #1344](https://github.com/inet-framework/inet/pull/1344), connects lifetime disposal to transmit-window advancement and final-transfer completion. HCF removes resolved positions without sequence reuse. If the final unresolved DATA expires and no DATA remains, HCF can start completion DELBA at a safe exchange boundary.

The recipient's complete-unit release on ordinary teardown comes from PR #1343. S4 supplies the final-expiry trigger that reaches that existing path. S4 does not change the lifetime default, restart retry deadlines, or introduce another lifetime store. The [main scope table](ieee80211-compressed-blockack-replacement.md#previous-prs-and-s4-scope) gives the wider implementation boundary.

## 1. Core behavior

The per-EDCAF parameter `msduLifetime` defaults to 0.512 seconds. Queue residence, setup deferral, retries, and ACK-policy changes consume that deadline. The parameter requires a positive, finite value that simulation time can represent. Zero, negative, and nonfinite values fail initialization. There is no unlimited mode.

Hypothetical example: HCF accepts DATA at zero seconds and channel access remains blocked until 0.7 seconds. The default deadline expires at 0.512 seconds. A one-second lifetime permits an attempt after channel access resumes, subject to other eligibility and disposal rules.

## 2. Current source and standard support

[Edca](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edca.cc) owns the lifetime handler. [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) registers QoS DATA before queue insertion and uses one timer for the earliest deadline. The queue owner removes expired frames. HCF clears their acknowledgment and recovery state.

IEEE Std 802.11-2024 Clause 10.23.2.12.1 covers QoS transmit lifetime. Clause 10.25.6.7.1 supplies related Block Ack retransmission conditions. Annex C.3 supplies 500 TU defaults through `dot11EDCATableMSDULifetime` and `dot11QAPEDCATableMSDULifetime`. The conversion is 500 TU = 0.512 seconds. These references define the requirement basis; this document adds no standards retrieval result.

## 3. Implemented ownership and behavior

The handler keys deadlines by local frame identity. Two peers can use the same sequence number without a shared deadline. The QoS data service clears borrowed source references before source deletion. Fragments retain their source deadline. Retransmission and ACK-policy changes do not restart it.

A new A-MSDU starts a deadline at aggregate construction. The current builder adds all sources at one simulation time. Only tracked sources participate in lifetime replacement, including when `qOsCheck=false` permits non-QoS or mixed sources. A new QoS aggregate receives its own deadline even when no source was tracked. Expired inputs cannot enter an aggregate.

Hypothetical example: an MSDU accepted at zero seconds joins an A-MSDU at 0.4 seconds without an active or pending agreement. The new aggregate expires at 0.912 seconds. Its source deadline was 0.512 seconds. An expired input cannot receive this extension. Uncertain setup prevents aggregation that could restart the deferred batch's deadlines.

### Compatibility setting

For a ten-second study, `**.mac.hcf.edca.edcaf[*].msduLifetime = 100s` prevents lifetime expiry of newly accepted DATA during that interval. Acknowledgment, teardown, queue loss, and terminal failure can still remove DATA. This setting guarantees neither identical fingerprints nor complete compatibility. Recovery that depends on expiry inherits the longer delay.

### Deadline and safe completion

HCF checks the deadline before submission to Tx, with the proposed IFS. No new DATA attempt can start at or after the deadline. An accepted attempt can finish across it and receive successful acknowledgment.

HCF retains referenced frames until exchange cleanup releases the context. It prevents another attempt and excludes expired retained entries from timer selection. This permits safe completion without repeated zero-time expiration events.

## 4. Agreement integration

Setup and DELBA recovery consume the existing DATA deadline. HCF advances the transmit window when lifetime disposal resolves an outstanding position. It uses the same identity owner rather than a separate agreement lifetime store.

Final expiry can trigger completion DELBA even when the recipient still holds acknowledged complete units behind a gap. Ordinary recipient teardown delivers those complete units before resource release. Association reset discards old buffered units instead. The [lifecycle contract](ieee80211-blockack-agreement-lifecycle.md#recipient-buffer-disposition-at-teardown) defines these separate outcomes.

Hypothetical example: repeated Block Ack feedback leaves DATA 100 unresolved. Its original deadline expires despite agreement activity. HCF clears its transmit state and permits window progress without sequence reuse. With no further DATA, completion teardown can release complete recipient units behind that gap.

## 5. Verification and limits

The source includes lifetime, identity, aggregation, arbitration, and congestion fixtures under `tests/module`. The [current S4 verification record](ieee80211-compressed-blockack-s4-execution.md) defines the executed selection. It includes lifetime interactions through Block Ack fixtures but does not report a new standalone lifetime campaign.

The source contract applies independently of Block Ack support. Current S4 results do not establish complete Compressed recovery, general performance, or MAC stop/start behavior. Lifetime disposal, inactivity teardown, queue overflow, and retry-limit disposal remain distinct causes.
