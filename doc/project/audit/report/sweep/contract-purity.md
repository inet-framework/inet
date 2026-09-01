# Contract purity sweep

> **Kind:** report · **Status:** snapshot 2026-09-01 · **Seal:** none · **Owns:** — · **Stands on:** [rule/architecture.md](../../../rule/architecture.md)

- **Scope:** every C++ contract header under `src/inet/`
- **Date:** 2026-09-01, commit `b83e354523`
- **Rule checked:** [AR-ORG-CONTRACT-PURITY](../../../rule/architecture.md#ar-org-contract-purity)
- **Result:** **PASS.** 176 contract headers, 8 static non-signal members in 2 files, all of one
  sanctioned kind. No violation in the tree.

This sweep ran when the rule was written, to find out whether the rule describes what INET does or
asks for a change. It describes what INET does.

## How the sweep was made

A contract header is a file `I<Capital>*.h` that declares `class INET_API <the file's own name>` with
at least one pure virtual. In each, every `static` member that is not a `simsignal_t` was collected.

```bash
# the shape of it; the sweep itself was a short Python script
grep -l 'class INET_API I' src/inet/**/I[A-Z]*.h
```

**A signal identity is excluded by the rule**, not by the sweep: `static simsignal_t
datarateSelectedSignal` is part of the vocabulary a contract defines, and seven contract headers
under `linklayer/ieee80211/mac/contract/` each carry one with a small `.cc` that defines nothing
else.

## Findings

| File | Members | Disposition |
|---|---|---|
| `networklayer/contract/IRoute.h` | `sourceTypeName(SourceType)` | `AS-02` |
| `physicallayer/wireless/common/contract/packetlevel/IRadio.h` | `getRadioModeName`, `getRadioReceptionStateName`, `getRadioTransmissionStateName`, and the `radioModeEnum`, `receptionStateEnum`, `transmissionStateEnum` registrations | `AS-02` |

All eight are the printable form of an enum that the contract itself declares. A caller and an
implementor need the same word for `IRadio::RadioMode`, which makes the name part of the vocabulary
the contract defines — the same argument that admits a signal identity. Sanctioned as `AS-02` in
[architecture-exceptions.md](../../architecture-exceptions.md).

## One false positive, worth recording

`networklayer/ipsec/IPsec.h` was caught by the name heuristic and is **not a contract**: `IPsec` is a
`SimpleModule`, and its `parseProtocol` and `parseEnumElem` are module-internal helpers. A sweep that
selects a contract by the shape of its *name* will keep finding it. A sweep that selects by "declares
a NED `moduleinterface`" would not, and that is the better filter if this becomes a `T3` check.

## What this sweep does not cover

- **NED `moduleinterface` files.** The rule applies to them equally; nothing here reads a `.ned`.
- **Non-static behavior.** A non-trivial inline body in a contract is the same fault and no static
  keyword marks it. That case needs `T4` agent review, which is where the rule sits.
