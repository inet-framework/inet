IEEE 802.11 implicit Block Ack extension points
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default MPDU policy leaves aggregate transmission disabled. ``HtMpduAggregationPolicy`` supplies selection for independent HT20 long-GI members with fixed DATA rates. Local aggregate transmission remains unavailable. Custom MPDU policies implement ``isEnabled()`` and ``selectFrames()``. The caller supplies receiver limits and one retained DATA mode. A policy returns borrowed member references without queue extraction or state changes.

Custom QoS data services implement batch preparation, validation, commitment, and construction through ``IOriginatorQosMacDataService``. Each new member owns one reversible preview. The batch borrows canonical members from the in-progress queue. Commitment validates every queued source before the first extraction. Failed preparation preserves sequence state and independent deadlines.

IEEE 802.11 HT aggregate construction
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The MIB parameter ``htMinimumMpduStartSpacing`` supplies the encoded receiver limit from zero through seven. Zero preserves the default with no spacing restriction. The remaining values represent 0.25, 0.5, 1, 2, 4, 8, and 16 microseconds. Full and simplified management preserve the receiver's value in each negotiated direction. The negotiated transmit direction stores the peer receiver's value.

Custom ``IMpduAggregation`` implementations must supply two HT operations. ``computeAggregateLength()`` previews the PSDU length at the supplied mode and spacing. Zero means an empty batch, unsupported mode, invalid member length, or excessive HT PSDU length. The new ``aggregateFrames()`` overload accepts source references, the mode, encoded spacing, and the peer byte limit. It returns ``nullptr`` when construction exceeds a limit, including either HT duration limit. These operations retain all source Packets and their state.

The existing pointer overload retains its consuming contract. The supplied builder copies region tags from each source's data range to the member's position. It inserts alignment padding and zero-length delimiters between members as necessary. The final member has no padding. A singleton retains its delimiter, so HT-SIG byte conversion preserves its aggregate identity. These construction operations do not enable aggregate selection or implicit Block Ack responses.

The default ``Rx`` refuses aggregate containers before standalone FCS or MAC header checks. Custom receive modules must keep these containers separate from standalone frame parsing. Member acceptance and implicit responses remain unavailable at this boundary.
