Mesh Service Period Providers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``Ieee80211MeshInterface.powerSave=true`` enables the complete built-in
directional service-period policy. It remains false by default. Existing
always-awake configurations need no change. The policy's
``missingAckRetryLimit`` accepts 0 through 100; its default of 7 is a model
choice, and does not replace the existing ordinary MAC retry limit.

Replacing IMeshPowerSave
^^^^^^^^^^^^^^^^^^^^^^^

A replacement implementation must now supply ``reviewServicePeriods``,
``canTransmitSet``, ``authorizeTransmissionStart``, ``transmissionResolved``,
``recipientMemberReceived``, ``prepareRecipientResponse``,
``recipientResponseComplete`` and ``recipientAgreementRetired``. These are
pure role methods. Deriving from ``MeshPowerSave`` retains its implementation.

Keep admission queries reversible. Authorize the complete selected set only
at actual start, after all readiness, membership, lifetime and exchange
checks. A refusal must not debit an attempt. ``MeshPowerResolution`` contains
copied original identities and explicit retirement causes. An unset
``barPending`` means that the batch supplies no recovery disposition; it must
not erase a surviving member's outstanding recovery. A retirement callback
alone is not an expiry or an ordinary retry-limit fact.

``recipientMemberReceived`` precedes payload duplicate deletion and reorder
release. Its duplicate verdict concerns payload; it does not automatically
veto the current-period control fields. Do not deliver the payload again or
perform a second duplicate lookup. Fresh directions also require an eligible
awake opportunity: the built-in policy uses the receiver-derived PPDU start
for the own-awake-window case, so a frame starting inside that window may
complete after it. An existing direction may still end outside the window.
Committing local ACTIVE withdraws the recipient direction before publishing
the MIB change; any independent owner direction is retained.
Recipient-response preparation may stamp
copied local proof on the existing ACK/BA packet. Confirm that proof only on
actual response completion, never on cancellation or accepted receipt alone.
Retiring an agreement clears unstamped sequence evidence; an already-stamped
proof still belongs to the same live peer and period. Peer or period cleanup
invalidates it. These tags are never interpreted as received wire identity.

Controlled intermediate-policy fixtures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Earlier P1/P2 fixtures explicitly control awake opportunities. Their policy
adapters now also isolate automatic service-period planning, selected-set
admission and actual-start charging, and route received fields through the
earlier mode-learning role. They retain their earlier QoS Null admission at
the intermediate eligibility seam and delegate other frame eligibility to
the production base. This preserves those fixtures' original
mechanism assertions. A production replacement must implement its intended
service-period policy rather than copying these test isolation overrides.
