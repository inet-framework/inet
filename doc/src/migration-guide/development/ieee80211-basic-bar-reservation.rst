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

Custom ``InProgressFrames`` classes that override ``peekFrameToTransmit()``
should move their candidate ordering into ``peekFrameToTransmitExcluding()``
and inherit the ordinary peek. The ordinary peek passes a null exclusion.
The forecast passes the DATA it is forecasting after. Return a borrowed
candidate using the same ordering in both cases.
