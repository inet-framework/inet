IEEE 802.11 Response Query Roles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Response selectors now distinguish sender estimates from actual recipient responses. Previously, sender queries used the frame's transmitter address and could replace a valid HT response with a legacy mode.

External implementations of ``IRateSelection`` must provide these signatures:

.. code-block:: c++

   const IIeee80211Mode *computeResponseAckFrameMode(Packet *packet,
           const Ptr<const Ieee80211DataOrMgmtHeader>& header, ResponseQueryRole role) override;
   const IIeee80211Mode *computeResponseCtsFrameMode(Packet *packet,
           const Ptr<const Ieee80211RtsFrame>& rts, ResponseQueryRole role) override;

``IQosRateSelection`` requires the same ACK and CTS changes. It also requires this signature:

.. code-block:: c++

   const IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *packet,
           const Ptr<const Ieee80211BlockAckReq>& bar, ResponseQueryRole role) override;

The enum is in ``inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h``. Update external callers and overrides as follows:

- Use ``ResponseQueryRole::PREDICT`` for sender reservations, response timeouts, and exchange admission estimates.
- Use ``ResponseQueryRole::RESPOND`` for actual ACK, CTS, or Basic Block Ack selection and recipient response duration calculations.
- Forward the role when an override calls another selector.

The frame argument identifies the frame that requires a response. Prediction uses its receiver address and ``localRxPeerTx``, the capability for peer transmission to the local receiver. A recipient response uses its transmitter address and ``localTxPeerRx``, the capability for local transmission to the peer receiver. Frame mode tags do not determine the role.

For example, compatible associated peers can now retain the same 13 Mbps HT response in both queries. The old sender query could select 12 Mbps legacy ERP-OFDM in ``n(mixed-2.4Ghz)``. Existing HT simulations can therefore change reservations, response timeouts, and TXOP admission. See :ref:`ug:sec:80211:response-prediction` for agreement conditions and unknown Tx limits.
