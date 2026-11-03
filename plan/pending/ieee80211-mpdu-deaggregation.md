# Bounded A-MPDU deaggregation

> **Kind:** reference · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [Packet design](../../doc/project/design/packet-anatomy.md)

The deaggregator must extract complete MPDUs from typed chunks and serialized bytes. Before this change, it trusts delimiter lengths without bounds checks. A malformed length can cause an out-of-bounds request.

The shared traversal is the static method `MpduDeaggregation::visitSubframes`. It visits the A-MPDU subframes of IEEE Std 802.11-2024, 9.7.1; each subframe is a delimiter, an optional MPDU, and padding. `MpduDeaggregation` owns the method because it already reads the A-MPDU format that `MpduAggregation` writes. The method reads delimiters and pops complete MPDUs from a bounded Packet view. Callbacks receive the chunks in packet order. The traversal stores no intermediate layout. The deaggregator creates output packets directly from those chunks. It copies region tags from their absolute source positions.

This PR stops at the first invalid delimiter. A separate recovery PR adds a search for later delimiters and partial PHY reception. Neither PR enables MAC aggregate acceptance or implicit Block Ack.

## Commit responsibilities

1. The plan commit defines the boundary and expected checks. It changes no runtime behavior.
2. The traversal commit adds `MpduDeaggregation::visitSubframes` and direct unit checks. It prepares deaggregation without a production caller.
3. The deaggregation commit uses the visitor and adds its protocol fixture and reviewer document. It changes production extraction.

## Expected verification

The unit fixture checks lengths, alignment, truncation, invalid delimiter flags, and final padding. Its filename is `tests/unit/Ieee80211MpduDeaggregation_1.test`. Its assertions inspect callbacks rather than a production layout type.

The protocol fixture calls production deaggregation with typed chunks and reconstructed bytes. Complete inputs must return three exact MPDUs. An overlong middle delimiter must stop extraction after the first MPDU. The parent requests an out-of-bounds slice instead. The reviewer document explains these conditions and ownership.

Build and test execution are deferred at the user's request. The revised commits have no new runtime evidence.
