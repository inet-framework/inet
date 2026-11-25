Custom originator QoS ACK policies and Basic feedback reservations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Implementations of ``IOriginatorQoSAckPolicy`` must implement
``computeBlockAckReqParametersAfterData()``. The query receives a read-only
pointer to the active ``TxopProcedure``. HCF calls it before DATA transmission,
so its time-dependent accessors describe the current time.

Apply the same request selection rules as ``computeBlockAckReqParameters()``,
treating the supplied DATA as outstanding and excluding it from the next
eligible DATA candidates. Leave TXOP, ACK, agreement and queue state intact.
Return the usual no-request tuple with a TID of ``-1`` if no request is due.
Policies derived from ``OriginatorQosAckPolicy`` can inherit the implementation
when they use its request selection rules. Override the forecast alongside
any custom request selection.

Custom ``InProgressFrames`` classes that override ``peekFrameToTransmit()``
should move their candidate ordering into ``peekFrameToTransmitExcluding()``
and inherit the ordinary peek. The ordinary peek passes a null exclusion.
The forecast passes the DATA it is forecasting after. Return a borrowed
candidate using the same ordering in both cases.
