# Scenario control of a mesh peer's power mode

The command is delivered by the ordinary `ScenarioManager` module to the
production `MeshPowerSave` policy through `IScriptable`. The policy validates
the command name, mandatory `peer` and `mode`, individual literal MAC address,
enabled/operating state, and established peering before it calls the existing
`requestMode` owner. It adds no desired-mode store or retry and does not
change the existing actual ACK, peer-lifecycle, or radio owners.

`MeshPowerPolicyControl.test` uses explicit interface MAC addresses, actual
peering and the production policy. The three native cases send XML commands
through `ScenarioManager`: acknowledged LIGHT, LIGHT with its first real ACK
dropped and the retry temporarily held, and acknowledged DEEP. Each then
requests ACTIVE through the same dispatcher. Each observes a real peer ACK
on air. The lost-ACK case checks the remote has learned LIGHT while the
requester's local state is still ACTIVE and the first Null remains held;
the later ACTIVE command commits locally immediately. The holding MAC is a
test control and does not replace the power policy.

Each case also calls the policy parser directly with an unsupported command,
missing or empty attributes, invalid/multicast/zero addresses, an unknown
mode, an unestablished peer, and a disabled policy. At the end it stops the
policy and checks rejection while down. The checks compare the existing
requested mode and buffered original IDs across each rejection; these are
direct error-path controls, not additional dispatcher cases. They make no
claim about whole-interface restart or re-peering. The P3 power and peer
protocol files provide sibling controls for unchanged mode and service-period
behavior.

The focused debug run used seed 0 and passed all three native cases. The
matching-model P3 controls passed all 19 `MeshPowerModes` and 47
`MeshPeerServicePeriod` cases. Their native tuples, assertions, termination
times and event numbers, diagnostics, and stderr match the prior corrected
P3 record. Raw outputs, generated inputs, executables, loader paths and source
identities are retained in `/home/miguel/inet-mesh-s0-dev/record.json`
(SHA-256 `ab0e3b82f6f006e02ae54467bad8b2967d6afbaf18e9f95f7ab571582aeec57f`).
An earlier fixture setup attempt used an incorrectly ordered wildcard and
is preserved separately; it is not a production behavior failure. Independent
review and the final integration campaign remain outstanding.
