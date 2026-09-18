IEEE 802.11 Capability and BSS State Ownership
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom modules that used ``ModeSetListener`` or subscribed to ``modesetChanged``
must obtain the configured catalog through ``IIeee80211MacConfiguration``.
For catalog consumers, derive from ``ModeSetModuleBase``, declare a
``modeSetModule`` NED parameter, and call the base initialization before using
``modeSet`` at ``INITSTAGE_LINK_LAYER``. Keep ``NUM_INIT_STAGES``. The built-in
MAC supplies the descendant parameter default; standalone consumers must point
it at a module implementing the C++ and NED configuration contracts. Move
algorithm initialization formerly performed by the signal callback to that
initialization stage. Ordinary catalog queries must not reset algorithm state.

Custom transmitters and receivers that contribute HT capabilities implement
``IIeee80211TransmitterCapabilities`` and ``IIeee80211ReceiverCapabilities``,
respectively, in C++ and NED. Report implemented width support and, for receivers,
short-GI support independently of the current BSS operation. The MAC's
``prepareLocalCapabilities()`` assembles the local profile after PHY readiness;
repeated preparation preserves active protocol state.

Replace direct access to MIB BSS/profile fields with ``getBssData()``,
``getBssStationData()``, ``getBssAccessPointData()`` and
``getLocalHtCapabilities()`` for reads. Management owners use ``commitBss()``,
``clearBss()`` and the association/peer methods for writes. Profile installation
uses ``installLocalHtCapabilities()``; replacing a changed profile requires an
inactive BSS with no peer relationships. Custom simplified AP management
implements ``IIeee80211BssProvider`` for explicit preparation and peer
installation/removal instead of allowing another module to modify its MIB.

Subscribe to the MIB's Boolean ``bssStateChanged`` signal to observe accepted
state transitions. Query the MIB synchronously; the signal carries no borrowed
state object. Management calls ``publishStateChange()`` after completing the
transition's timer and transaction bookkeeping. Observers must not mutate the
MIB during publication or retain raw BSS/peer references across a mutation.
Use ``hasActiveBss()`` and ``hasHtOperation()`` explicitly, and supply operation
and HT eligibility separately from cached capabilities to peer mode selection.

The old initialization signal and writable BSS/profile fields are removed
without compatibility adapters because they permit missed initialization or
state changes that bypass owner bookkeeping and notification. Existing custom
implementations must migrate together with their consumers.
