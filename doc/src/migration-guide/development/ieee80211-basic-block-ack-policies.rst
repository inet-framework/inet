IEEE 802.11 Basic Block Ack policy interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Protected Basic Block Ack exchanges can reserve time for a queued DATA frame
before the receiver returns its bitmap. Update custom implementations of the
following C++ interfaces and rebuild the library that contains them.

* In ``IMsduAggregationPolicy``, change the override to
  ``computeAggregateFrames(queueing::IPacketQueue *queue, const std::function<bool(b)>& fitsMpdu = {})``.
  If the caller supplies a predicate, test each proposed aggregate prefix using
  the complete MPDU length in bits. Include the outer MAC header and FCS, each
  subframe header, and padding between subframes. Stop extending the prefix
  when it exceeds the budget. Return a null pointer if the fitting prefix
  cannot form an aggregate. An empty predicate leaves the policy's ordinary
  aggregation limits in effect.
* In ``IOriginatorQoSAckPolicy``, add the length-and-header overload of
  ``isBlockAckPolicyEligibleFrame``. The packet overload can forward the packet's
  data length and header to it. Implement ``computeBlockAckReqParametersForPreview``
  to predict the recipient, starting sequence number and TID at the supplied
  completion time. Account for the candidate and any preceding DATA exactly
  once, leaving the agreement and the transmit queue untouched. Return an
  unspecified recipient when no request is due. Implement
  ``getNextBlockAckReqDeadline`` to return the earliest absolute simulation
  time at which a deferred request needs attention, or ``SIMTIME_MAX`` when
  no wake-up is needed.
* In ``IOriginatorQosMacDataService``, add the ``prepareFrame`` overload with
  the MPDU fit predicate. Pass it to aggregate selection and keep the preview
  reversible. The existing two-argument overload can forward an empty
  predicate. Implement ``previewBasicFirstMpdu`` to return an owned copy of
  the first fragment when fragmentation applies, or a null pointer when the
  caller can preview the original MPDU. Implement ``commitBasicFrames`` to
  revalidate the prepared sources, consume them and their sequence number
  only on success, and return the committed MPDU or its fragments in order.
  Return a null pointer when the prepared sources can no longer be committed.
