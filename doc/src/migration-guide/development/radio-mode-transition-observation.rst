Radio mode transition observation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``IRadio`` adds two pure virtual const queries. External implementations must
provide both overrides and rebuild their dependent libraries:

.. code-block:: cpp

    std::optional<simtime_t> getRadioModeSwitchingTime(RadioMode from, RadioMode to) const override;
    std::optional<RadioModeState> getRadioModeState() const override;

The duration query accepts the stable modes ``OFF`` through ``TRANSCEIVER``.
Return the effective initialized switching duration, with zero for an idempotent
request. ``SWITCHING`` is not a valid endpoint. Return ``std::nullopt`` when the
observation is unsupported, uninitialized or the endpoints are invalid. Do not
use zero to represent unavailable timing, and do not parse another component's
configuration to implement this query.

``RadioModeState`` is a copied value containing ``mode`` and an optional
``RadioModeTransition``. An active transition contains ``targetMode`` and the
absolute ``endTime``. A present snapshot without a transition describes a
supported stable state; an absent snapshot describes unavailable observation.
A transition whose deadline equals the current time remains active until its
actual completion event executes. Do not infer that event has already run.

Packet-level ``Radio`` supplies both queries from its existing switching matrix,
mode fields and scheduled timer after module initialization completes. Its
existing synchronous SWITCHING notification precedes timer insertion, so the
snapshot is temporarily unavailable inside that callback. Completion signals
also precede existing transceiver-state updates. Consumers schedule or coalesce
a continuation before acting on mode signals, then query again; they must not
issue reentrant mode commands, infer readiness from signal order or poll with
repeated zero-time events. The snapshot itself does not assert Tx/Rx readiness.

``ShortcutRadio`` and ``NoiseSource`` explicitly return unavailable. An external
provider without the required observations may do the same in its concrete
overrides. Do not add default unavailable bodies to the pure interface, change
existing notification/timer order, introduce a parallel deadline or enable a
power policy merely to implement these queries.
