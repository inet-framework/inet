IEEE 802.11 EDCA contention recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS recovery procedures receive ``edcaTransmissionFailed()`` for an access-category failure and ``txopCompleted()`` when a TXOP ends on a successful holder transmission. Move EDCA contention handling out of the former ``controlFrameTransmissionFailed()``, ``ctsFrameReceived()``, ``blockAckFrameReceived()``, and ``multicastFrameTransmitted()`` hooks. ``ackFrameReceived()`` still resolves retry state for the acknowledged DATA frame.

A custom contention-window calculator for QoS recovery implements ``IRecoveryProcedure::IEdcaCwCalculator``. Store the access category's retry count with its contention window. ``incrementCw()`` advances the count and derives the window from it. ``resetCw()`` resets both, and ``getEdcaRetryCount()`` exposes the count. The built-in EDCAF uses ``recoveryProcedure.shortRetryLimit`` for the shared EDCA count. Management and QoS recovery retain their own per-frame retry limits.
