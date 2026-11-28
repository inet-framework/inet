IEEE 802.11 Tx request cancellation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Both overloads of ``ITx::transmitFrame()`` accept a final optional
``int64_t *requestId`` output parameter. Existing callers may omit it. Custom Tx
implementations must supply the ID before calling
``Ieee80211Mac::shouldDeferTransmissionStart()`` and use
a distinct ID even when the frame and completion callback are reused.
Rebuild custom modules against the new interface.

Implement ``cancelPendingTransmission(id)`` to withdraw the matching request
before radio handoff and release the retained frame without a caller callback.
Implement ``startPendingTransmission(id)`` to hand off the matching frame once,
at its admitted start time. Both methods return whether they acted. Before
handing the frame to the radio, notify the caller through
``ITx::ICallback::transmissionStarted()``. That callback borrows the retained
frame for the duration of the call. Custom callers implement it and may leave
it empty when they need only the completion or deadline outcome.
If a deferred request misses that deadline, release it and notify its caller
through ``ITx::ICallback::transmissionStartFailed()``. Custom callbacks must
implement this method and release any exchange waiting for transmission.
A custom ``IFrameSequenceHandler`` must implement ``transmissionWithdrawn()``
to finish an unstarted exchange without reporting a successful transmission.

A MAC that needs to wait for radio readiness may override
``deferTransmissionStart(id, admittedStart)``. The hook runs in MAC context
at submission, so it can wake the radio during IFS and schedule a readiness
check. Return true to defer the start and pass the saved ID to Tx when ready
at the admitted time. Schedule the readiness event ahead of Tx's deadline
event, which runs at the lowest scheduling priority at that time. On explicit
withdrawal or a missed deadline, Tx releases the request and calls
``Ieee80211Mac::notifyTransmissionWithdrawn(id)``. Override
``pendingTransmissionWithdrawn(id)`` to discard any readiness work for that ID
and restore the previous usable radio mode. If a radio switch is in progress,
wait for the radio mode change before requesting another switch. The default
MAC starts at IFS expiry, or immediately for zero IFS.
