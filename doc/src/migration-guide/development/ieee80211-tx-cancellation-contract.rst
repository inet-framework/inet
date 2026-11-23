IEEE 802.11 Transmission Cancellation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``inet::ieee80211::ITx`` implementations must implement the new pure virtual ``bool cancelPendingTransmission(ICallback *callback)`` method.

Return true only when the supplied callback owns an accepted transmission that is still waiting for its inter-frame space and no PPDU has started. Cancel the IFS timer, dispose the retained packet copy, and clear the callback and saved NAV-update choice. Do not report transmission completion, update NAV, start the PHY, or report an error sample for the canceled operation.

Return false without changing state when idle, when the callback does not match, or when the PPDU is already on air. An on-air frame completes through the existing callback. The supplied ``Tx`` implements these semantics; existing ``ITx::ICallback`` implementations need no new method.

Custom ``IFrameSequenceHandler`` implementations used with mesh must also support ``interruptFrameSequence()`` for a retired TRANSMIT step after Tx cancellation succeeds. Mark that step rejected and finish the sequence without invoking transmitted-frame or failed-frame callbacks. Preserve the existing RECEIVE interruption behavior. The supplied ``FrameSequenceHandler`` implements both paths.
