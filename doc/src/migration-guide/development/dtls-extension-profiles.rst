DTLS Extension Profiles and Application Policy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Existing configurations continue to select DTLS 1.3 with optional capabilities
disabled. Enable legacy operation explicitly with ``versions = "1.2"`` or
``"1.3 1.2"``. A dual offer prefers 1.3; timeouts never enable fallback. Legacy
resumption uses the same ``resumptionEnabled`` preference; setting
``dtls12SessionTicketsEnabled = false`` selects session IDs instead of tickets.
Legacy records account for an eight-byte explicit nonce, and CID records include
an inner content type. Query the selected association's application budget rather
than subtracting a fixed 1.3 expansion yourself.

Custom ``IDtls`` engines and callbacks must implement the extension operations
and callback methods described in the individual development migration sections
for sessions, early data, CID updates, migration, PHA and path MTU discovery.
Early delivery needs an explicit application admission policy and must never be
automatically replayed after rejection. A queued application packet may receive
``sendError`` after migration shrinks its budget without failing the association.
Keep callback objects alive through terminal notifications and support synchronous
callbacks when processing costs are zero.

``DtlsSocketIo`` now accepts optional ScenarioManager application commands. They
act on its current client or latest accepted server association; applications
that control multiple server associations should keep their own socket handles.
No scenario action is scheduled by default. The sixteen configurations and
asserting verifier in ``examples/dtls`` demonstrate the finite supported profiles.
All cryptographic operations remain modeled placeholders.
