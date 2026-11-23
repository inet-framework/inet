Mesh discovery contracts
~~~~~~~~~~~~~~~~~~~~~~~~

External implementations of the native mesh contracts must implement the new pure virtual methods:

* ``IMeshPathSelection::startDiscovery(destination)`` and ``cancelDiscovery(destination)`` start or cancel one queue-driven destination attempt. Cancellation releases attempt/retry state; a provider that rate limits PREQs must retain relevant per-target minimum-interval history.
* ``IMeshPathSelection::processHwmp(body, transmitter, receiver)`` receives an immutable, fully decoded HWMP body and the immediate MAC identities. Validate the whole supported body and committed transmitter eligibility before mutating protocol state. The production ``Hwmp`` currently handles PREQ/PREP; path errors and proactive roots are separate procedures.
* ``IMeshForwarding::pathAvailable(destination)`` drains only eligible pending source packets, rechecking the current route and packet deadline before each handoff. ``discoveryFailed(destination)`` drops the destination's pending packets. ``hasPendingSource(destination)`` expires due packets and answers whether an eligible source packet remains. Path selection calls this before delayed initial/retry attempts; it must not copy packet deadlines into protocol state.
* ``IMeshManagement::sendHwmp(body, receiver)`` hands the typed body to management's existing mesh action/MAC path. Check ``isMeshManagementActive()`` before counting a queue-driven attempt. This synchronous accepted handoff is the production HWMP pacing boundary; it does not report actual PHY transmission time.

Forwarding retains pending ``Packet *`` ownership until it removes shared accounting and hands a packet to the MAC. Discovery failure and readiness callbacks transfer no packet ownership. A path provider must commit route/precursor/validity state and cancel the corresponding active discovery before calling readiness. Its stop/crash path must clear active protocol state before calling destination failure; its destructor must not call potentially destroyed neighboring modules.
