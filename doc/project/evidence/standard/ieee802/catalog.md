# IEEE 802 — imported address definitions

> **Kind:** what · **Status:** draft · **Seal:** none · **Owns:** IEEE802-ADDRESS-* · **Stands on:** [source.md](source.md)

Edition: **IEEE Std 802-2024**. These definitions are from normative clause 8.2.2, not an
informative tutorial. Physical PDF page locators refer to the pinned source; the document is not
registered in the processed corpus. This is a bounded address extraction, not a full catalog.

## Index

| ID | Statement |
| --- | --- |
| [IEEE802-ADDRESS-1](#ieee802-address-1) | The first octet's least-significant bit distinguishes individual and group addresses. |
| [IEEE802-ADDRESS-2](#ieee802-address-2) | The all-ones address is the all-stations broadcast group address. |

## IEEE802-ADDRESS-1

**The first octet's least-significant bit distinguishes individual and group addresses.**

- Source: 8.2.2, physical PDF p. 41 (printed p. 40); Figure 10 on physical p. 43 illustrates conventional EUI-64 octet notation.
- Source excerpt: “The least significant bit (LSB) of the first octet is the individual/group (I/G) address bit” (line-wrap normalized).
- Strength: `description`; observation class: `encoding`.
- Condition: 48-bit or 64-bit MAC address structure. I/G=0 identifies an individual address; I/G=1 identifies a group address. Conventional notation and the MAC's transmission order must be distinguished.
- Check idea: Toggle I/G while keeping the remaining bits fixed; changing the last octet must not change the classification. Group MAC address values are not EUI identities.

## IEEE802-ADDRESS-2

**The all-ones address is the all-stations broadcast group address.**

- Source: 8.2.2, physical PDF p. 41 (printed p. 40).
- Source excerpt: “The all-stations broadcast MAC address is a special group MAC address of all ones.”
- Strength: `description`; observation class: `encoding`.
- Condition: MAC address width determines the number of one bits; for a 64-bit address all eight octets are FF.
- Check idea: Classify the all-ones value as both group and broadcast, while a different I/G-set value is group but not broadcast. Reception and ACK predicates remain defined by the applicable MAC standard.
