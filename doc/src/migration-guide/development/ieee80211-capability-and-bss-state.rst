IEEE 802.11 Capability and BSS State Ownership
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom modules must replace ``ModeSetListener`` and ``modesetChanged`` with the configured catalog from ``IIeee80211MacConfiguration``. Catalog consumers must derive from ``ModeSetModuleBase``. Declare a ``modeSetModule`` NED parameter. Call the base initialization before you use ``modeSet`` at ``INITSTAGE_LINK_LAYER``. Keep ``NUM_INIT_STAGES``. The MAC supplies the default path for its descendants. Standalone consumers must specify a module that implements the C++ and NED configuration contracts.

Move algorithm initialization from the old signal callback to ``INITSTAGE_LINK_LAYER``. Catalog queries must preserve algorithm state.

Custom transmitters must implement ``IIeee80211TransmitterCapabilities`` in C++ and NED. Custom receivers must implement ``IIeee80211ReceiverCapabilities`` in C++ and NED. Report channel-width support independently of active BSS operation. Receivers must also report short-GI support. The MAC calls ``prepareLocalCapabilities()`` after the PHY is ready. Repeated preparation preserves active protocol state.

Replace direct reads of MIB fields with ``getBssData()``, ``getBssStationData()``, ``getBssAccessPointData()``, and ``getLocalHtCapabilities()``. Management uses ``commitBss()``, ``clearBss()``, and the association and peer methods for writes. Use ``installLocalHtCapabilities()`` to install a profile. A changed profile requires an inactive BSS with no peer relationships.

Custom simplified AP management must implement ``IIeee80211BssProvider``. This contract supplies AP preparation and peer installation and removal. ``installSimplifiedPeer()`` receives the peer address, optional HT capabilities, and rate state. Another module must not change the AP MIB directly.

The MIB retains the Boolean signals ``bssStateChanged`` and ``rateStateChanged``. Management calls ``publishStateChange()`` after it completes the state change, timers, and transactions. Both signals describe complete state. Query the MIB during the synchronous callback. Do not change the MIB during signal delivery. Do not retain raw BSS or peer references across a mutation.

Use ``hasActiveBss()`` to test BSS presence. Use ``hasHtOperation()`` to test accepted HT operation. Supply operation and HT eligibility separately from cached capabilities to peer mode selection. A retained capability result does not preserve permission to transmit HT frames.

The old initialization signal and writable BSS and profile fields have no compatibility adapters. Custom implementations must change with their consumers.
