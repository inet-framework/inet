Local IEEE 802.11 interval evidence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IRx`` implementations must implement ``invalidateInterframeHistory()``.
HCF calls this operation when a rejected BAR or an agreement transition invalidates interval evidence.
The operation must discard all local interval evidence.

Rx attaches ``Ieee80211InterframeInd`` to frames that pass FCS.
Its ``related`` field reports an accepted local interval.
An absent indication supplies no interval evidence.
Custom radios must supply local reception endpoints and normal transmission completion notifications.
An aborted transmission must not emit ``transmissionEndedSignal``.
Rx supports MAC delivery at the reception completion time through the standard interface connection.
Custom connections that delay MAC delivery need their own evidence storage.
