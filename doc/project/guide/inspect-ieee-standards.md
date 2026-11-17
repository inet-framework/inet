# Inspect a local IEEE standard

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](derive-tests-from-a-standard.md)

Use this procedure to retrieve source evidence before designing, changing, or reviewing a
standards-dependent behavior. It supports IEEE 802.11 and IEEE 802.15.4 through the shared
`inet_process_standards` tool in the separate `inet-skills` repository. No agent skill is
required to use the tool. The optional `ieee-standards` skill supplies the same tool entry
point; `ieee80211-standards` is a compatibility route.

## Locate and validate the corpus

Identify the active INET checkout, the `inet-skills` tool checkout, and the directory actually
containing the source PDFs. A shared standards repository and the INET checkout's local
`standards/` are distinct locations. Source files may be read-only. Choose a writable output
directory ignored by Git, normally the active checkout's `standards/processed/`.

Run from the tool checkout, substituting the identified absolute paths:

```sh
./bin/inet_process_standards status --standards-dir <source-directory> --output <corpus-output> --json
./bin/inet_process_standards build --standards-dir <source-directory> --output <corpus-output>
./bin/inet_process_standards lint --output <corpus-output> --document ieee802154-2024 --json
```

Build when the requested document is missing, stale, partial, or incompatible. Discovery uses
reviewed filename profiles, not every PDF in the directory: confirm the requested identity
appears in `status`. The profiles include `802154-2024.pdf` as `ieee802154-2024`,
`80211ax-2024.pdf` as the **base** `ieee80211-2024`, and `80211be-2024.pdf` as its amendment
`ieee80211be-2024`. Inspect lint findings before trusting affected objects.

## Retrieve and cite

```sh
./bin/inet_process_standards get clause 6.3.2.1 --document ieee802154-2024 --output <corpus-output> --json
./bin/inet_process_standards refs clause 6.3.2.1 --document ieee802154-2024 --output <corpus-output> --json
./bin/inet_process_standards get table 7-1 --document ieee802154-2024 --output <corpus-output> --json
./bin/inet_process_standards define association --document ieee802154-2024 --output <corpus-output> --json
./bin/inet_process_standards search 'CSMA-CA' --document ieee802154-2024 --output <corpus-output> --json
```

Use the exact document identity even when clause labels overlap. Cite the edition, clause or
object, canonical node ID, and physical PDF pages or returned source-span locator. Determine
normative versus informative status from the source context; retrieval confidence is not a
normative-status flag. Record material cross-references, including unresolved ones.

Open the original PDF when a table layout, diagram, equation, ambiguous extraction, or page
verification matters. A figure lookup can locate a caption without representing the diagram's
arrows or semantics. Do not derive an algorithm from its caption alone. Generated text and
indexes are local build outputs, not tracked project evidence.

## Connect evidence to a change

Follow [derive-tests-from-a-standard.md](derive-tests-from-a-standard.md) for the ownership and
flow of standards maps, catalogs, English checks, tests, and model evidence. Pin the edition
and operating mode before selecting obligations. The
[IEEE 802.15.4 survey](../evidence/model/ieee802154/conformance.md) records existing model
claims separately from the 2024 reference. Corpus validation establishes retrieval behavior;
it does not establish protocol conformance.
