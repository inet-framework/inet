IEEE 802.11 implicit Block Ack extension points
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default MPDU policy leaves aggregate transmission disabled. ``HtMpduAggregationPolicy`` supplies selection for independent HT20 long-GI members with fixed DATA rates. Local aggregate transmission remains unavailable. Custom MPDU policies implement ``isEnabled()`` and ``selectFrames()``. The caller supplies receiver limits and one retained DATA mode. A policy returns borrowed member references without queue extraction or state changes.

Custom QoS data services implement batch preparation, validation, commitment, and construction through ``IOriginatorQosMacDataService``. Each new member owns one reversible preview. The batch borrows canonical members from the in-progress queue. Commitment validates every queued source before the first extraction. Failed preparation preserves sequence state and independent deadlines.

``IRx::lowerAggregateReceived()`` returns separately owned correct members and immutable delimiter history. The caller retains the envelope. Custom recipient services pass ``BlockAckReceiveContext`` to the reorder owner. This context supplies physical interval and delimiter facts; it does not replace receipt state. Custom agreement handlers implement ``implicitBlockAckRequested()`` to refresh inactivity for valid implicit DATA.

Every custom rate selector implements ``computeCompressedBlockAckResponseMode()`` for implicit callers. A selector without this capability must throw a diagnostic from its concrete override. Explicit Compressed BAR and implicit DATA share this rate calculation without a fabricated BAR.

Custom recipient procedures construct a Compressed response directly from peer, TID, and current agreement state. Local aggregate transmission remains unavailable. Custom Tx implementations need no aggregate submission operation at this boundary.

IEEE 802.11 HT aggregate construction
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The MIB parameter ``htMinimumMpduStartSpacing`` supplies the encoded receiver limit from zero through seven. Zero preserves the default with no spacing restriction. The remaining values represent 0.25, 0.5, 1, 2, 4, 8, and 16 microseconds. Full and simplified management preserve the receiver's value in each negotiated direction. The negotiated transmit direction stores the peer receiver's value.

Custom ``IMpduAggregation`` implementations must supply two HT operations. ``computeAggregateLength()`` previews the PSDU length at the supplied mode and spacing. Zero means an empty batch, unsupported mode, invalid member length, or excessive HT PSDU length. The new ``aggregateFrames()`` overload accepts source references, the mode, encoded spacing, and the peer byte limit. It returns ``nullptr`` when construction exceeds a limit, including either HT duration limit. These operations retain all source Packets and their state.

The existing pointer overload retains its consuming contract. The supplied builder copies region tags from each source's data range to the member's position. It inserts alignment padding and zero-length delimiters between members as necessary. The final member has no padding. A singleton retains its delimiter, so HT-SIG byte conversion preserves its aggregate identity. These construction operations do not enable aggregate selection or implicit Block Ack responses.

The default ``Rx`` sends aggregate containers through a separate member-validation path before standalone FCS or MAC header checks. A failed common header or incomplete content prevents member acceptance. Custom receive modules must keep aggregate containers separate from standalone frame parsing until they implement member acceptance. MPDU deaggregation can extract bounded recovered members from incomplete content. Extraction alone grants no MAC acceptance.
