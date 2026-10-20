IEEE 802.11 Block Ack Recipient Delivery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Block Ack recipients return complete units in cyclic sequence order. External C++ callers of ``BlockAckReordering::processReceivedQoSFrame()`` and ``processReceivedBlockAckReq()`` must use ``BlockAckReordering::ReleasedFrames`` for their results. This type is a vector of sequence-number and fragment-group pairs. The indexed receive buffer retains its map storage.

Replace explicit ``BlockAckReordering::ReorderBuffer`` result declarations with ``BlockAckReordering::ReleasedFrames``. Iterate over the vector in its supplied order. Keep that order through reassembly, A-MSDU deaggregation, and delivery. Do not insert released units into a numeric sequence map. For example, a release across sequence wrap supplies 4095 before 0.

``ReceiveBuffer::getLength()`` counts occupied sequence positions. All Basic fragments of one MSDU count once. ``isFull()`` uses that same count. Custom diagnostics that require a fragment count must count the fragments in each stored position separately.

The protected ``getEarliestCompleteMsduOrAMsduIfExists()`` helper remains available for one release but is deprecated. It retains its lookup behavior for external subclasses and returns fragments owned by the receive buffer. The helper does not remove frames or transfer ownership. Its returned pointers remain valid only while the buffer retains those fragments. For delivery, consume the ordered ``ReleasedFrames`` results from ``processReceivedQoSFrame()`` or ``processReceivedBlockAckReq()``. These methods remove complete units from storage and transfer their ownership to the caller.

No configuration change is necessary. Gap release, window movement, and BAR before DATA can change delivery times and fingerprints in affected Basic Block Ack scenarios. These repairs do not enable Compressed Block Ack.
