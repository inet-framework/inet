# IEEE 802.11 mesh — standards map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [Derive tests from a standard](../../../guide/derive-tests-from-a-standard.md)

This map selects the requirements for an unsecured, single-channel mesh with HWMP, airtime path selection, HT/VHT data transport and mesh power save. It is a mesh-only slice of IEEE 802.11; it is not a survey of the whole standard. The [catalog](../../standard/ieee80211/catalog.md) states the requirements, the [feature map](features.md) groups them, and the [checks](checks.md) describe observations without choosing a simulation implementation.

## In-scope set

| Document | Identity | Use |
| --- | --- | --- |
| IEEE Std 802.11-2024 | `ieee80211-2024`, base standard | Sole normative edition, including the mesh, HT, VHT, aggregation and Block Ack dependencies |

The local source filename is `80211ax-2024.pdf`; its filename does not identify an amendment. It is the 5,956-page base standard with SHA-256 `c08beb0e16bf8c36465a331d349dd8b8ea79b28d4a320288312ddeb5ce8d9bb1`. Page references in this evidence set are physical PDF pages. A canonical locator such as `ieee80211-2024:clause:14.3.3` identifies the corresponding structural node in the local standards corpus. Clauses and tables cited here are normative unless explicitly identified as informative guidance.

## Boundaries

The selected profile uses mesh stations as endpoints, one primary channel, open peering, LLC/SNAP, and no EPD support. It includes both airtime metric identifiers, on-demand discovery, proactive PREQ and RANN, peer-specific power modes, and directional mesh peer service periods. HT/VHT requirements apply only where the corresponding local and peer capabilities permit them. The above-54-Mb/s recommendation selects the high PHY rate metric as the default for such a profile; it does not permit neighboring members to use different metric identifiers.

Security, SAE/AMPE, proxy stations, mesh portals/gates, multi-interface selection, channel switching, MCCA, optional beacon collision avoidance, HE/EHT, 80+80 MHz, beamforming/MU-MIMO and LDPC are outside this selection. HT SMPS and infrastructure APSD are distinct from mesh power save. Selecting mesh power save does not select either of them. The mesh dependencies in 14.2 also constrain advertised capabilities; unsupported optional mechanisms must not be advertised as available.

## Source navigation

| Subject | Governing source | Physical PDF pages |
| --- | --- | --- |
| Mesh dependencies, profile, basic sets | 14.2; 14.3.2–14.3.4; 14.3.7; 10.6.4; 11.51 | 3192–3195; 1938–1939; 2930 |
| Open peering and state machine | 14.4; 14.5, including their algorithm children | 3196–3216 |
| Mesh Control and mesh A-MSDU | 9.2.4.8.3; 9.3.2.2.3; Figure 9-124 | 707–708; 775 |
| HWMP elements and action | 9.4.2.110–9.4.2.113; 9.6.16.3 | 1222–1227; 1773 |
| Forwarding | 10.37; 10.37.2 | 2144–2145 |
| Link reports and airtime | 14.9.3; 14.10.1–14.10.2; Tables 14-4–14-5 | 3224–3226 |
| HWMP algorithms | 14.11.7–14.11.12, including algorithm children and tables | 3231–3257 |
| Timestamp and neighbor synchronization | 11.1.3.1; 11.1.3.9; 14.14.2.2; 14.14.3.1 | 2428–2429; 2442–2443; 3268–3270 |
| Mesh power modes and signaling | 14.15.2–14.15.5 | 3278–3281 |
| Awake windows, doze and MPSPs | 14.15.6; 14.15.8; 14.15.9; Table 14-33 | 3281–3285 |
| Aggregation and Block Ack | 10.12; 10.12.8; 10.25, including algorithm children | 1976–1982; 2049 onward |

## Interpretation and extraction limits

Table 14-25 on PDF pages 3253–3254 refers to a sequence-number increment in 14.11.10.4.3 step b), which is a PREP procedure. The increment itself is specified in the PERR procedure, 14.11.11.4.3 step b). Use the explicit PERR procedure for the propagated sequence number. The source PDF confirms this cross-reference mismatch; resolving a corpus link does not establish that its target is the intended procedure.

The airtime formula and mesh A-MSDU figure require visual confirmation because text extraction loses mathematical grouping and layout. The source formula on PDF page 3224 is `(O/n + Bt/r) / (1 - ef)`; Figure 9-124 on PDF page 775 includes Mesh Control in each mesh A-MSDU subframe. Corpus lint diagnostics and the remaining extraction work are recorded in the [model ledger](../../model/wifi/hwmp-coverage.md). They must not be interpreted as conformance results.

PREQ receipt and propagation use differently worded sequence predicates: 14.11.9.4.3 compares target sequence in its second receipt alternative, while the E1/E2/E3 propagation cases compare originator sequence and discovery identity. The source PDF confirms both texts. Keep the guards attached to their respective actions. Clause 10.11's mesh-source wording includes a textual defect, using “mesh STA” where the surrounding address rules identify mesh SA; interpret it in the context of its explicit matching source-address requirement. Mesh sequence allocation and aggregate lifetime follow their explicit clauses, rather than inferring independent identities/lifetimes from the presence of per-subframe Mesh Control fields.

## Override table

There are no overrides between editions in this selection. Earlier base editions and amendments do not govern these checks.
