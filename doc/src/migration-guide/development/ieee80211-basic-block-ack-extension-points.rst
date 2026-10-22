IEEE 802.11 Basic Block Ack extension points
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

HCF previews the next eligible DATA before BAR selection. Preview must preserve queue ownership and the next sequence number. Custom QoS data services must implement ``IOriginatorQosMacDataService``. This contract extends ``IOriginatorMacDataService`` with ``peekEligibleFrame()``, ``getNextSequenceNumber()``, and ``hasAMsduAggregation()``.

Custom ``InProgressFrames`` classes must add ``bool extract = true`` to their ``getPendingFrameFor()`` override. When ``extract`` is false, the method must return a borrowed candidate without extraction or sequence assignment. The default value preserves ordinary extraction. Custom candidate sources must also implement ``peekFrameToTransmit()`` without extraction.
