IEEE 802.11 EDCA contention recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS recovery procedures receive ``edcaTransmissionFailed()`` for an access-category failure and ``txopCompleted()`` when a TXOP ends on a successful holder transmission. Move EDCA contention handling out of the former ``controlFrameTransmissionFailed()``, ``ctsFrameReceived()``, ``blockAckFrameReceived()``, and ``multicastFrameTransmitted()`` hooks. ``ackFrameReceived()`` still resolves retry state for the acknowledged DATA frame.

A custom contention-window calculator for QoS recovery implements ``IRecoveryProcedure::IEdcaCwCalculator``. Store the access category's retry count with its contention window. ``incrementCw()`` advances the count and derives the window from it. ``resetCw()`` resets both, and ``getEdcaRetryCount()`` exposes the count. The built-in EDCAF uses ``recoveryProcedure.shortRetryLimit`` for the shared EDCA count. Management and QoS recovery retain their own per-frame retry limits.

The EDCAF emits and records ``contentionWindowChanged`` for both management and QoS traffic. Update signal subscriptions and result selectors to use the EDCAF module path. Custom EDCA calculators emit the signal from the module that owns the window.

Subscribe before ``INITSTAGE_LAST`` to receive the initial contention window. Keep ``retryLimitReached`` subscriptions on the recovery procedure for the relevant frame type.

The recovery-procedure contention-window paths are removed without a deprecation period. Each old path recorded the changes made by one recovery procedure. A compatibility alias to the shared vector would silently add the other procedure's samples to existing analyses and could count each window change twice in selectors that match both paths. For example, replace ``edca.edcaf[i].recoveryProcedure`` with ``edca.edcaf[i]`` to select the full window history for access category ``i``.
