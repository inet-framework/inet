Mesh awake coordination owner roles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IMeshManagement`` implementations must implement these pure queries:

* ``getMeshWakeSchedule()`` returns a copied ``MeshWakeSchedule`` containing
  the next TBTT, local beacon interval, next beacon ordinal, pending-beacon
  flag and pending-peering flag. The ordinal names the next scheduled beacon.
* ``getOwnTbtt(ordinal)`` converts the requested future local ordinal through
  the provider's existing TSF and clock-slowing authority.
* ``getPeerTbtt(peer, notBefore)`` returns the next converted peer boundary at
  or after the absolute lower bound. Return an empty optional for unavailable
  or invalid neighbor timing. Retain the received interval with that neighbor.

Do not introduce a second clock, offset ledger or timing parameter parser.
After accepted timing/body updates and matching actual timing-frame completion
or retirement, notify the power policy through its timing methods. Call
``beaconReceived(peer, groupBuffered, individualBuffered, advertisedWindow)``
only after the timing owner has accepted and updated the received reference.
Use the AID assigned by that reporting peer when reading its individual TIM bit.

``IMeshPowerBuffer::getGroupDeliveryIdleTime()`` returns the PHY-default AC_VI
TXOP duration from the existing mode authority. It must not return the user's
configured TXOP override. ``TxopProcedure::getDefaultTxopLimit(ac)`` exposes
the existing concrete authority for providers using that implementation.

Copied buffered inventory distinguishes ``PROBE_RESPONSE = 4`` from the
existing ``TIMING = 3`` beacon category; values zero through three are unchanged.
Both are timing work excluded from TIM/group buffered-unit counts. A live Probe
Response independently requires awake until admission or retirement, whereas
the management-owned beacon-pending/rendezvous facts hold an own beacon.

Custom ``IMeshPowerSave`` providers must implement ``reviewAwakeState()``,
``requiresAwake()``, ``radioObservationChanged()``,
``managementTimingChanged()``, ``timingTransmissionComplete(window)``,
``timingTransmissionRetired()``, ``beaconReceived(...)``,
``groupFrameReceived(peer, moreData)`` and
``ppduTransmissionComplete(packet, start)``. Notifications borrow packets.
The PPDU start is the actual accepted air time, not a predicted or moved start.
Start local AW at actual beacon/probe completion; qualify transmitter PostAW
by the group PPDU start. Keep receiver continuous physical-idle evidence
separate from NAV/TXNAV, invalidating it on busy or nonreceptive observations.
The selected rolling-window interpretation qualifies starts inside either the
own AW or its timed PostAW extension, adding the nominal duration at completion.
Pending announced group delivery separately keeps awake after timed expiration;
do not strand that delivery by gating it solely on a timed endpoint.

The new ``IMeshRadioControl`` role is implemented by the concrete mesh MAC.
``requestRadioReview()`` requests a coalesced continuation rather than issuing
a radio command inline. ``getMeshRadioState()`` and
``getMeshRadioSwitchingTime(from, to)`` return initialized public observations;
``hasMeshRadioActivity()`` covers retained waits/current exchanges, and
``isMeshPhyIdle()`` reports physical CCA without virtual reservations. A custom
provider must preserve radio transition signal/timer order and wait for actual
completion publication, including a transition whose deadline equals now.

The concrete MAC completion overload ``frameTransmissionComplete(packet,
start)`` delegates to the existing one-argument hook for ordinary packets.
Extensions overriding that hook remain valid. Actual aggregate completion is
one physical-PPDU notification; retained originals and acknowledgment outcomes
continue to have their separate HCF owners. Power-managed final admission
must retain logical originals on a missed pre-air start and must not consume
retry budgets or publish a failed-airtime outcome.

Existing intermediate physically-awake/manual-opportunity test policies must
override both coordinator review and received awake-window facts consistently.
The production activation gate remains until complete mesh service periods are
implemented; this API does not supply MPSP retrieval. Rebuild dependent libraries
after implementing the new pure role methods.
