IEEE 802.11 MIB Rate State and Listeners
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``Ieee80211Mib`` adds rate state to the Management Information Base (MIB).
``Ieee80211RateSetState`` contains ``supported``, ``basic``, and ``operational``
rate sets. Each ``Ieee80211RateSet`` has a ``known`` flag, legacy rates in bits
per second, and HT modulation and coding scheme (MCS) indexes.
The default ``known=false`` means that the information is unknown.
A set with ``known=true`` and no rates is a known empty set.
Check ``known`` before you use an empty rate set.

Read the state with ``getLocalRateSet()``, ``getBssRateSet()``, and
``findPeerRateSet(address)``. BSS means Basic Service Set.
``findPeerRateSet()`` returns ``nullptr`` when no peer record exists.
The getters return views that remain valid until the corresponding update
or clear operation. Copy any state that you need after such an operation.
This includes an update that a synchronous signal listener makes.

Use ``setLocalRateSet()`` for local state.
Use ``setBssRateSet()`` for BSS state.
Use ``installBssAndPeerRateSets()`` to install BSS and peer state together.

These methods validate all input before they change the rate records.
Unknown sets must contain no rates. Known basic and operational sets must
respect the subset checks in ``validateIeee80211RateSetState()``.
An equal rate-set update emits no signal.

``clearBssRateSet()`` restores unknown BSS rate state.
``removePeerRateSet()`` and ``clearPeerRateSets()`` remove peer rate records.

Subscribe to ``Ieee80211Mib::rateStateChangedSignal`` for state notifications.
The signal name is ``rateStateChanged``. A notification carries boolean
``true`` and no details object. The MIB emits it after a committed rate,
HT capability, or channel-operation change. Query the MIB in the listener
to obtain current state.
``installBssAndPeerRateSets()`` publishes both records before one notification.

``releaseAssociationId()`` and ``clearAssociationIds()`` remove peer HT
capabilities and peer rate sets before notification. A teardown that removes
either kind of peer state emits one signal. Empty teardown emits no signal.
A listener can install replacement peer state from the callback.
The teardown leaves that replacement intact.
Standalone HT and rate removal methods retain their own notifications when
they remove state.

Existing configuration parameters and MIB method signatures need no change.
