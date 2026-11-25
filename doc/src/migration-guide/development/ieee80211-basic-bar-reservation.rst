Custom originator QoS ACK policies and Basic feedback reservations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Implementations of ``IOriginatorQoSAckPolicy`` must implement
``computeBlockAckReqParametersAfterData()``. Add ``simtime_t dataEnd`` after
the DATA packet argument in existing overrides and pass it through when
delegating to the base implementation. It gives the scheduled end of the
DATA PPDU. The read-only ``TxopProcedure`` pointer still describes the
active TXOP at the time of the query.

Apply the same request selection rules as ``computeBlockAckReqParameters()``.
Read outstanding frames with ``getOutstandingFrames(dataEnd)``. Count the
supplied DATA as outstanding only if its lifetime extends past ``dataEnd``,
and exclude it from the next eligible DATA candidates. Leave TXOP, ACK,
agreement and queue state intact.
Return the usual no-request tuple with a TID of ``-1`` if no request is due.
Policies derived from ``OriginatorQosAckPolicy`` can inherit the implementation
when they use its request selection rules. Override the forecast alongside
any custom request selection.

Update custom ``InProgressFrames`` overrides to
``getOutstandingFrames(simtime_t time)``. Forward ``time`` to any base call
and apply lifetime filtering at that time, excluding frames whose deadline
is at or before it. The base declaration defaults ``time`` to ``simTime()``
for live selection.

Custom ``InProgressFrames`` classes that order in-progress DATA in
``peekFrameToTransmit()`` or ``peekFrameToTransmitExcluding()`` should move
that ordering into ``peekInProgressFrameToTransmitExcluding(const Packet *,
simtime_t time)`` and inherit the ordinary lookup and extraction methods.
Skip the excluded packet and use ``isEligibleToTransmit(packet, time)``
for each candidate. Return a borrowed in-progress frame, or null when none
qualifies. The caller handles pending DATA when appropriate. Live selection
passes the current time. A replacement retry forecast passes the outgoing
DATA and its scheduled completion time.

The one-argument ``peekFrameToTransmitExcluding()`` override was introduced
on the development branch after INET 4.7.0. It is replaced before its first
release because it cannot receive the time at which to check eligibility
and may return a frame from the pending queue. Keeping it as a separate
live-ordering hook would allow the sender to reserve airtime for one retry
and transmit another. Calls to the one-argument method still work. Declaring
it final makes the compiler identify overrides that need to move.
