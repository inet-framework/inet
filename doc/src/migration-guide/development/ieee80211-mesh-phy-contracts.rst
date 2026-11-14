IEEE 802.11 mesh PHY contracts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The OFDM mesh profile remains the default. Select ``phyProfile = "ht"`` or ``"vht"`` on ``Ieee80211MeshInterface`` to use the new profile defaults, including the complete PHY catalog, dimensional radio and airtime metric ID 2. An explicit metric ID 1 remains supported. The profile's radio uses ``contiguousChannelAllocation = true``. Its existing band, channel index and bandwidth determine the allocation; the channel index is not an IEEE channel number.

Device capability and operating bandwidth are separate. A VHT device supports the mandatory 20/40/80 MHz baseline even when operating at 20 MHz. Set ``mib.meshVht160MhzCapable = true`` before selecting a 160 MHz operating allocation. ``meshShortGuardInterval`` advertises receive support; ``meshShortGuardIntervalTransmissionEnabled`` independently activates local short-GI transmission. Peer receive permission and a legal catalog entry are also required.

For a fixed tuple, configure ``mac.hcf.rateSelection.dataFramePhyFamily``, ``dataFrameMcs``, ``dataFrameGuardInterval`` and the existing ``dataFrameBandwidth``. VHT also requires ``dataFrameNumSpatialStreams``. HT MCS determines its stream count, so an explicitly supplied count must agree. A conflicting bitrate, absent tuple or incomplete explicit tuple produces a diagnostic rather than falling back to a different tuple.

External implementations must add these methods:

* ``IRx::prepareReceivedFrame(Packet *)`` admits radio-normalized transport before MAC observations. It borrows the packet and returns false for the opaque erroneous transport path; it must not delete or deliver it.
* ``IQosRateSelection::computeResponseTxVector(...) const`` returns the actual response format and occupied bandwidth. A duplicate OFDM response keeps its legal per-copy OFDM mode and duration while occupying the eliciting width. ``computeMeshPeerModes(...)`` previews candidate directional peer support without committing it.
* ``IMeshLinkTiming::previewMeshLinkTiming(...)`` evaluates candidate peer rates and PHY capabilities without publishing them. The result is used before peer mutation and to compare the accepted effective tuple before resetting an airtime estimate.

``Ieee80211ModeReq`` and ``Ieee80211ModeInd`` now store one ``Ieee80211TxVector``. Existing ``getMode`` and ``setMode`` accessors project its mode pointer; consumers needing actual occupied width or duplicate format must use the vector. ``singleMpdu`` describes the PPDU that was sent or received, even after radio decapsulation has removed the single-member envelope.

The radio owns that envelope on the outgoing duplicate and removes it before valid receive observations. Tx completion callbacks still receive the bare member. An invalid layout remains PHY-wrapped and tagged, with a bit error, until the ordinary corrupt-frame path discards it. A custom radio or PHY dissector must preserve this matching representation and tag; retagging an opaque PSDU as a bare MAC frame is unsafe.

In the fixed-allocation profile, equivalent per-packet channel requests are normalized to the radio's immutable canonical channel; different allocations are refused. Request and indication pointers remain borrowed. Runtime channel-change commands retain their existing ownership transfer: an idempotent request does not replace the canonical channel. Retained indications are valid within the radio's lifetime, including supported stop/start and sleep/wake, without a promise to outlive the radio module.

Wide CCA requires the medium's initialized ``supportsInterferenceObservation()`` promise, a dimensional analog model and an unprotected compatible neighborhood. The allocation is fixed at runtime. Noncontiguous 80+80 MHz, protection, adaptive control and aggregation beyond the single VHT member are not enabled by this change.
