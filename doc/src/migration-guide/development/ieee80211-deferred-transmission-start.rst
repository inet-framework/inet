Cancellable Wi-Fi transmission starts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``ITx`` providers must implement ``resumePendingTransmission()``. It resumes
the existing retained copy, rather than accepting another packet or moving
packet ownership to the caller. The ordinary ``Ieee80211Mac`` keeps its legacy
immediate/IFS path; a concrete MAC may override the readiness hooks to use the
deferred path.

The concrete MAC hooks have distinct roles. ``defersTransmissionStart()``
selects the deferred path. ``isTransmissionReady()`` reports actual readiness;
``isRadioReadyForTransmission(start)`` reports whether readiness is still
possible at the admitted start. ``requestTransmissionReadiness()`` asks the MAC
to reconsider its current pending transmission. A MAC resumes Tx after the
relevant state publication by calling ``resumePendingTransmission()``. It must
not reinterpret a missed start as permission to transmit later. The existing
MAC hooks retain legacy-ready defaults, and native HCF/DCF final admission
remains unconditional in this preparation. A later policy consumer supplies
its own coherent eligibility and attempted-frame bookkeeping.

``ITx::ICallback`` implementations must implement these pure methods:

* ``prepareTransmissionStart(const Packet *, simtime_t)`` accepts or refuses
  the final pre-air start. The packet is borrowed for this call.
* ``transmissionCancelled(const Packet *)`` reports that the accepted copy
  never reached the air. Tx has already detached its timer, copy and callback
  registration. The packet remains borrowed until this call returns. A callback
  may submit another transmission without the old cleanup clearing it.

Ordinary callbacks can return true from final-start admission. Cancellation is
distinct from completion or a failed on-air attempt: it must not synthesize
ACK timeout, NAV, retry or link-failure processing. The explicit external
``cancelPendingTransmission(callback)`` remains silent and now covers both IFS
and readiness waits. It still cannot cancel an on-air PPDU.

``IFrameSequenceHandler`` providers must implement
``cancelPendingTransmission()`` to reject a detached TRANSMIT step and finish
normally while retaining live originals. The existing interrupt contract for
retired originals and response waits remains separate.
