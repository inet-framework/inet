# S3a plan for explicit Basic Block Ack compatibility

S3a preserves historical Basic demonstrations before a later Compressed default change. Basic remains the temporary default. This series adds profile validation before explicit Basic settings in the existing demonstrations and fixtures.

The source parent is `1cdb362a2a4b97f564566487079d9d156a5d213d`. S1 recipient repairs exist in that parent. S2 and S3b remain outside this series.

## Step 1: Validate the local profile

HCF owns the profile selector. At local initialization, HCF reads `blockAckVariant` once and stores a typed value. The default and explicit `basic` values retain the existing Basic implementation. Unknown values stop initialization. The known `compressed` value also stops initialization because its runtime exchange path does not exist. Both rejection paths apply when Block Ack support is disabled.

The module fixture checks six initialization cases through production HCF. Its separate INI file leaves the selector unset for the default case. No frame builder, serializer, timer, or agreement parameter changes.

## Step 2: Preserve historical Basic studies

The next commit will select `basic` explicitly in six INI files and twelve existing fixtures. It will preserve their topology, traffic, PHY modes, parameter loops, and assertions. The documentation will explain the historical Basic format and the temporary selector behavior.

The observation fixture will check accepted ADDBA setup, Block Ack-policy DATA, Basic BAR, and matched positive Basic responses. Parent and candidate comparisons will cover thirteen demonstration samples, eight capability probes, and nineteen full fingerprint rows. Existing expected protocol failures will retain their assertions and failure steps. The final execution record will state the commands, results, and limits.

## Scope limits

S3a does not add Compressed exchanges or capability-gap fallback. S3b must replace the temporary Compressed rejection when its admission path exists. Independent BAR work, production rate publication, and later default activation remain separate stages.
