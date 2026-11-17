# IEEE 802.15.4 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [conformance.md](conformance.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Level reached: **1 (Survey)** for the model families named in the claim survey. The initial survey ran no checks. The step-0 pass below adds draft English checks,
but no protocol check has run and no higher achieved level is claimed. No protocol conformance verdict is
recorded; successful corpus retrieval validates tooling, not the simulation model.

| Area | Survey evidence | Next evidence needed |
| --- | --- | --- |
| Edition and family | Pinned 2024 base; later amendments explicitly excluded | Older 2006/2007 texts for exact comparison when a change relies on an older claim |
| MAC access and ACK/retry behavior | Claim sources mapped to 6.3 and 6.6 | Specification-derived catalog, English checks, then production-path exchanges |
| Frame representation | Serializer limitation identified; mapped to clause 7 | Independent header/serialization checks and captured frame evidence |
| Narrowband PHY | Claim sources mapped to 11–13; sensitivity approximation identified | Parameter applicability, duration checks, and appropriately scoped reception/error tests |
| UWB PHY and interface | Historical PHY claim mapped to 16; distinct MAC composition identified | Mode-specific comparison and PHY evidence; separate MAC claim selection |

## Step-0 extraction pass

Initial baseline: `fd6f800222`; latest continuation baseline: `5f3845f055`, 2026-09-19. Scope and open dependencies are in
[applicability.md](applicability.md); commands, source findings and capture inspection are in
[results.md](results.md). The [catalog](../../standard/ieee802154/catalog.md),
[feature map](../../protocol/ieee802154/features.md) and
[English checks](../../protocol/ieee802154/checks.md) define the bounded engineering audit. **Step 0 passed independent review on 2026-09-19;
implementation and executable verification remain owed.**

There are 87 extracted statements (85 base-standard statements and two imported address definitions): 84 selected for the M1 engineering subset and 3 deferred
selected-profile obligations. These are counts of the current extraction, not the denominator
of a complete profile. No executable tests exist for these new check IDs; all run verdicts are
`NOT_RUN`. Existing survey claims are not promoted to support claims by writing English checks.

### M1 engineering subset and deferred obligations

`selected` means intended for M1 implementation and verification, not implemented. `later` means
owed by a later selected profile; neither status is a justified applicability exclusion.

| Statement | Selection | Delivery | English check | Executable / verdict |
| --- | --- | --- | --- | --- |
| [IEEE802154-ADDRESS-1](../../standard/ieee802154/catalog.md#ieee802154-address-1) | selected; owed | 1a, 1d, 2 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-ADDRESS-2](../../standard/ieee802154/catalog.md#ieee802154-address-2) | selected; owed | 1a, 1d, 2 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-1](../../standard/ieee802154/catalog.md#ieee802154-wire-1) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-2](../../standard/ieee802154/catalog.md#ieee802154-wire-2) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-3](../../standard/ieee802154/catalog.md#ieee802154-wire-3) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-4](../../standard/ieee802154/catalog.md#ieee802154-wire-4) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-5](../../standard/ieee802154/catalog.md#ieee802154-wire-5) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-6](../../standard/ieee802154/catalog.md#ieee802154-wire-6) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-ACCESS-1](../../standard/ieee802154/catalog.md#ieee802154-access-1) | selected; owed | 5 | [IEEE802154-C-ACCESS](../../protocol/ieee802154/checks.md#ieee802154-c-access) | None / `NOT_RUN` |
| [IEEE802154-ACCESS-2](../../standard/ieee802154/catalog.md#ieee802154-access-2) | selected; owed | 5 | [IEEE802154-C-ACCESS](../../protocol/ieee802154/checks.md#ieee802154-c-access) | None / `NOT_RUN` |
| [IEEE802154-SEQUENCE-1](../../standard/ieee802154/catalog.md#ieee802154-sequence-1) | selected; owed | 6 | [IEEE802154-C-SEQUENCE](../../protocol/ieee802154/checks.md#ieee802154-c-sequence) | None / `NOT_RUN` |
| [IEEE802154-SEQUENCE-2](../../standard/ieee802154/catalog.md#ieee802154-sequence-2) | selected; owed | 6 | [IEEE802154-C-SEQUENCE](../../protocol/ieee802154/checks.md#ieee802154-c-sequence) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-1](../../standard/ieee802154/catalog.md#ieee802154-receive-1) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-2](../../standard/ieee802154/catalog.md#ieee802154-receive-2) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-3](../../standard/ieee802154/catalog.md#ieee802154-receive-3) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-4](../../standard/ieee802154/catalog.md#ieee802154-receive-4) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-ACK-1](../../standard/ieee802154/catalog.md#ieee802154-ack-1) | selected; owed | 6 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |
| [IEEE802154-ACK-2](../../standard/ieee802154/catalog.md#ieee802154-ack-2) | selected; owed | 6 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |
| [IEEE802154-ACK-3](../../standard/ieee802154/catalog.md#ieee802154-ack-3) | selected; owed | 6 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |
| [IEEE802154-ACK-4](../../standard/ieee802154/catalog.md#ieee802154-ack-4) | selected; owed | 6 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |
| [IEEE802154-ACK-5](../../standard/ieee802154/catalog.md#ieee802154-ack-5) | later; owed | M2 step 7 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |
| [IEEE802154-SECURITY-1](../../standard/ieee802154/catalog.md#ieee802154-security-1) | selected; owed | 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802154-SECURITY-2](../../standard/ieee802154/catalog.md#ieee802154-security-2) | selected; owed | 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802154-SECURITY-3](../../standard/ieee802154/catalog.md#ieee802154-security-3) | selected; owed | 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802154-SECURITY-4](../../standard/ieee802154/catalog.md#ieee802154-security-4) | selected; owed | 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802154-SECURITY-5](../../standard/ieee802154/catalog.md#ieee802154-security-5) | selected; owed | 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802154-PHY-1](../../standard/ieee802154/catalog.md#ieee802154-phy-1) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-2](../../standard/ieee802154/catalog.md#ieee802154-phy-2) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-3](../../standard/ieee802154/catalog.md#ieee802154-phy-3) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-4](../../standard/ieee802154/catalog.md#ieee802154-phy-4) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-5](../../standard/ieee802154/catalog.md#ieee802154-phy-5) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-6](../../standard/ieee802154/catalog.md#ieee802154-phy-6) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-7](../../standard/ieee802154/catalog.md#ieee802154-phy-7) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-8](../../standard/ieee802154/catalog.md#ieee802154-phy-8) | selected; owed | 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-1](../../standard/ieee802154/catalog.md#ieee802154-service-1) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-1](../../standard/ieee802154/catalog.md#ieee802154-pib-1) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-2](../../standard/ieee802154/catalog.md#ieee802154-pib-2) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-3](../../standard/ieee802154/catalog.md#ieee802154-pib-3) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SCAN-1](../../standard/ieee802154/catalog.md#ieee802154-scan-1) | later; owed | M2 step 8 | [IEEE802154-C-SCAN](../../protocol/ieee802154/checks.md#ieee802154-c-scan) | None / `NOT_RUN` |
| [IEEE802154-SCAN-2](../../standard/ieee802154/catalog.md#ieee802154-scan-2) | later; owed | M2 step 8 | [IEEE802154-C-SCAN](../../protocol/ieee802154/checks.md#ieee802154-c-scan) | None / `NOT_RUN` |
| [IEEE802154-WIRE-7](../../standard/ieee802154/catalog.md#ieee802154-wire-7) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-8](../../standard/ieee802154/catalog.md#ieee802154-wire-8) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-9](../../standard/ieee802154/catalog.md#ieee802154-wire-9) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-10](../../standard/ieee802154/catalog.md#ieee802154-wire-10) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-11](../../standard/ieee802154/catalog.md#ieee802154-wire-11) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-12](../../standard/ieee802154/catalog.md#ieee802154-wire-12) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-13](../../standard/ieee802154/catalog.md#ieee802154-wire-13) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-14](../../standard/ieee802154/catalog.md#ieee802154-wire-14) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-15](../../standard/ieee802154/catalog.md#ieee802154-wire-15) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-WIRE-16](../../standard/ieee802154/catalog.md#ieee802154-wire-16) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-5](../../standard/ieee802154/catalog.md#ieee802154-receive-5) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-6](../../standard/ieee802154/catalog.md#ieee802154-receive-6) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-7](../../standard/ieee802154/catalog.md#ieee802154-receive-7) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-8](../../standard/ieee802154/catalog.md#ieee802154-receive-8) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-9](../../standard/ieee802154/catalog.md#ieee802154-receive-9) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-10](../../standard/ieee802154/catalog.md#ieee802154-receive-10) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-11](../../standard/ieee802154/catalog.md#ieee802154-receive-11) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-12](../../standard/ieee802154/catalog.md#ieee802154-receive-12) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-13](../../standard/ieee802154/catalog.md#ieee802154-receive-13) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-14](../../standard/ieee802154/catalog.md#ieee802154-receive-14) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-15](../../standard/ieee802154/catalog.md#ieee802154-receive-15) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-PHY-9](../../standard/ieee802154/catalog.md#ieee802154-phy-9) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-10](../../standard/ieee802154/catalog.md#ieee802154-phy-10) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-11](../../standard/ieee802154/catalog.md#ieee802154-phy-11) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-12](../../standard/ieee802154/catalog.md#ieee802154-phy-12) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-13](../../standard/ieee802154/catalog.md#ieee802154-phy-13) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-14](../../standard/ieee802154/catalog.md#ieee802154-phy-14) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PHY-15](../../standard/ieee802154/catalog.md#ieee802154-phy-15) | selected; owed | 1c, 4 | [IEEE802154-C-PHY](../../protocol/ieee802154/checks.md#ieee802154-c-phy) | None / `NOT_RUN` |
| [IEEE802154-PIB-4](../../standard/ieee802154/catalog.md#ieee802154-pib-4) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-5](../../standard/ieee802154/catalog.md#ieee802154-pib-5) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-6](../../standard/ieee802154/catalog.md#ieee802154-pib-6) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-7](../../standard/ieee802154/catalog.md#ieee802154-pib-7) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-8](../../standard/ieee802154/catalog.md#ieee802154-pib-8) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-PIB-9](../../standard/ieee802154/catalog.md#ieee802154-pib-9) | selected; owed | 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-2](../../standard/ieee802154/catalog.md#ieee802154-service-2) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-3](../../standard/ieee802154/catalog.md#ieee802154-service-3) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-4](../../standard/ieee802154/catalog.md#ieee802154-service-4) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-5](../../standard/ieee802154/catalog.md#ieee802154-service-5) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-6](../../standard/ieee802154/catalog.md#ieee802154-service-6) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-7](../../standard/ieee802154/catalog.md#ieee802154-service-7) | selected; owed | 1a, 3, 6 | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-WIRE-17](../../standard/ieee802154/catalog.md#ieee802154-wire-17) | selected; owed | 2–3 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-RECEIVE-16](../../standard/ieee802154/catalog.md#ieee802154-receive-16) | selected; owed | 6 | [IEEE802154-C-RECEIVE](../../protocol/ieee802154/checks.md#ieee802154-c-receive) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-8](../../standard/ieee802154/catalog.md#ieee802154-service-8) | selected; owed | 1a, 6 | [IEEE802154-C-SECURITY](../../protocol/ieee802154/checks.md#ieee802154-c-security) | None / `NOT_RUN` |
| [IEEE802-ADDRESS-1](../../standard/ieee802/catalog.md#ieee802-address-1) | selected; owed | 1a, 2, 6 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802-ADDRESS-2](../../standard/ieee802/catalog.md#ieee802-address-2) | selected; owed | 1a, 2, 6 | [IEEE802154-C-WIRE](../../protocol/ieee802154/checks.md#ieee802154-c-wire) | None / `NOT_RUN` |
| [IEEE802154-SERVICE-9](../../standard/ieee802154/catalog.md#ieee802154-service-9) | selected; owed | 1a, 1b | [IEEE802154-C-SERVICE](../../protocol/ieee802154/checks.md#ieee802154-c-service) | None / `NOT_RUN` |
| [IEEE802154-ACCESS-3](../../standard/ieee802154/catalog.md#ieee802154-access-3) | selected; owed | 1c, 6 | [IEEE802154-C-ACK](../../protocol/ieee802154/checks.md#ieee802154-c-ack) | None / `NOT_RUN` |

### Planned evidence categories

| Check | Intended evidence | Current support evidence |
| --- | --- | --- |
| IEEE802154-C-WIRE | Unit/serializer plus native-interface production fixture and independent octets | None; no executable check |
| IEEE802154-C-ACCESS | Module plus protocol trace reaching actual transmit/no-transmit outcome | None; no executable check |
| IEEE802154-C-SEQUENCE | Protocol exchange through serialization and receiver indication | None; no executable check |
| IEEE802154-C-RECEIVE | Protocol injection plus upper indication and ACK observation | None; no executable check |
| IEEE802154-C-ACK | Protocol loss injection and PHY timing; indirect variant deferred to M2 | None; no executable check |
| IEEE802154-C-SECURITY | Protocol/service fixture separating status, ACK and delivery | None; no executable check |
| IEEE802154-C-PHY | Module timing/energy observations; separate external validation | None; no executable check |
| IEEE802154-C-SERVICE | Module PIB/reset plus production request capacity tests | None; no executable check |
| IEEE802154-C-SCAN | Module/protocol scan evidence, deferred to M2 | None; no executable check |

All nine feature groups remain without executable support evidence. Category assignments are
provisional until actual fixtures demonstrate that they reach the claimed observation boundary.

## Pass log

| Date | Pass | Outcome |
| --- | --- | --- |
| 2026-09-18 | Standards/claim survey | Level 1; no simulation checks |
| 2026-09-19 | Initial step-0 extraction | 40 source-checked entries, nine English procedures, role/version/receive matrices and first consumer/capture inventory; applicability closure and executable tests still owed |
| 2026-09-19 | Extended step-0 audit | 82 entries and nine expanded English procedures; complete base MAC/PHY PIB field inventory, service/PICS comparison and PHY feasibility inspection; no runtime support verdict |
| 2026-09-19 | Service/integration dependency closure | 83 statements; direct security-error service routing, channel descriptor, clause-6 procedure dispositions and native consumer compatibility recorded; runtime tests remain NOT_RUN |
| 2026-09-19 | Companion address reference | Pinned IEEE Std 802-2024; two imported definitions and three address vectors resolve group-bit layout; sender/receiver AR distinction resolved without an invented receive-filter rule |
| 2026-09-19 | Bounded M1 step-0 closure | PASS after independent review; 87 mapped statements and nine procedures, explicit policy/timing/admission decisions and bounded reference dispositions; no executable support evidence |
