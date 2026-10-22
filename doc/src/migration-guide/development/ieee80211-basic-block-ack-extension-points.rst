IEEE 802.11 Basic Block Ack extension points
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HCF previews the next eligible DATA before BAR selection. Preview must preserve queue ownership and the next sequence number. Custom QoS data services must implement ``IOriginatorQosMacDataService``. This contract extends ``IOriginatorMacDataService`` with ``peekEligibleFrame()``, ``getNextSequenceNumber()``, and ``hasAMsduAggregation()``.

Basic single protection reserves only an already prepared successor. HCF ends the TXOP before further DATA extraction. A queued unit can change size or require agreement setup before transmission. Custom frame sequences must preserve this boundary when they use the same forecast.

Custom QoS ACK policies must add ``IOriginatorBlockAckAgreementHandler *agreements`` to ``isBlockAckReqNeeded()`` and ``computeBlockAckReqParameters()``. The argument supplies current agreement windows. Policies must keep peer and TID identities separate. The supplied policy can request BAR without a DATA candidate.

Callers must supply the agreement handler explicitly through both the interface and concrete policy types. An omitted argument fails compilation. Pass ``nullptr`` explicitly when no agreement handler is available; the supplied policy then requests no BAR. Custom overrides must also require the argument.

Calls to ``QosAckHandler::processTransmittedBlockAckReq()`` must supply the originator agreement explicitly. The agreement restricts processing to the negotiated window. An explicit ``nullptr`` retains bitmap-only coverage without that restriction. Production HCF always supplies the active agreement.

Custom ``InProgressFrames`` classes must add ``bool extract = true`` to their ``getPendingFrameFor()`` override. When ``extract`` is false, the method must return a borrowed candidate without extraction or sequence assignment. The default value preserves ordinary extraction. Custom candidate sources must also implement ``peekFrameToTransmit()`` without extraction.
