# Naming Exceptions and Violations

Known places where the INET code base departs from [naming-conventions.md](naming-conventions.md).
That document states the **target** rules; this one is the ledger of reality against them, so the
conventions can stay clean and prescriptive while the deviations are tracked (and, where
appropriate, worked off) here.

Every entry has one of two **dispositions**:

- **Sanctioned** (`NS-*`) — a deliberate, permanent exception that will *not* be changed (an
  established name too costly to rename, or a form that intentionally mirrors an outside standard).
- **Violation** (`NV-*`) — the name does not follow the convention and is a **rename candidate**.
  Each carries a suggested fix and a status (`Open` / `In progress` / `Done`). Fixing one means
  renaming the offending things and moving the row to `Done` (keep the id; do not reuse it).

Ids are stable references you can cite in a commit or PR that does the rename. New audits append
here — see *Auditing* at the bottom.

---

## Sanctioned exceptions

| Id | Name / pattern | Rule it departs from | Where | Why it stays |
|---|---|---|---|---|
| NS-01 | `IPsec` | acronyms are words (`Ipsec`) | `networklayer/ipsec/`, `IPsecEspTrailer` | Canonical real-world spelling; renaming would read as wrong to domain readers. |
| NS-02 | `applications`, `networks` | packages are singular | top-level `src/inet/` packages | Two of the oldest, most-referenced top-level packages; a rename is a project-wide breaking change out of proportion to the benefit. |
| NS-03 | `bps`, `nW`, `Gy`, … unit typedefs; `NA`, `e0`, `mu` constants | aliases are `PascalCase`; constants are `ALL_CAPS` | `common/Units.h` | Deliberately mirror SI unit symbols and physical-constant notation; matching the science beats matching the code convention. |
| NS-04 | `simtime_raw_t` and other `_t` scalar aliases | aliases are `PascalCase` | `common/INETDefs.h`, etc. | Follows the OMNeT++ kernel's own scalar-typedef style, which INET intentionally tracks. |
| NS-05 | `*Exception` classes instead of `cRuntimeError` | error idiom is `throw cRuntimeError(...)` | `transportlayer/quic/exception/` | Local design choice in QUIC; the names themselves (`ConnectionClosedException`) do follow the `<Condition>Exception` rule. |

---

## Open violations

### Words and abbreviations

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-01 | `ltostr`, `dtostr`, `atod`, `atoul`, `uhex` (abbreviated helpers) | spelled-out verb-first names | `common/INETUtils.h` (already carries a `// TODO` about this) | Open |

### Packages and directories

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-02 | Plural deep packages (`…/tables`, `…/modes`, `.../messages`, `.../profiles`, `.../flavours`, `.../headers`) | singular | `routing/eigrp/tables`, `routing/pim/{modes,tables}`, `routing/eigrp/messages`, `transportlayer/rtp/profiles`, `transportlayer/tcp/flavours`, `transportlayer/tcp_common/headers` | Open |
| NV-03 | Underscored package names | run-together lowercase | `routing/ospf_common`, `transportlayer/tcp_common`, `transportlayer/tcp_lwip` | Open |
| NV-04 | `CamelCase` / underscored example-scenario folders | lowercase, run-together | `examples/bgpv4/BgpCompleteTest` (and siblings), `examples/voipstream/VoIPStreamTest`, `examples/ethernet/TenBaseT1S`, `examples/ospfv3/multiple_areas_FINAL` | Open |

### Modules and gates

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-05 | Legacy link-frame gate vector `ethg[]` | `<stem>In[]` / `<stem>Out[]` | Ethernet interface NED | Open (low priority — widely referenced) |

### Messages

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-06 | Packet/message types suffixed `Msg` / `Message` (~40) | `*Packet` / `*Frame` / `*Header` per role | across `.msg` files | Open |
| NV-07 | Abbreviated fields `src` / `dest` | `srcAddress` / `destAddress` | `linklayer/ethernet/common/EthernetMacHeader.msg` (already carries a `// TODO rename`) | Open |

### C++

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-08 | Leading-underscore member variables | plain `camelCase` members | `transportlayer/rtp/*` (`Rtcp.h`, `RtpProfile.h`, `RtpSenderInfo.h`, …), `networklayer/contract/IRoute.h`, `networklayer/ipv6/Ipv6Route.h` | Open |
| NV-09 | `m_`-prefixed members; `value_` trailing underscore | plain `camelCase` members | `common/misc/MessageChecker.h`, `common/Units.h`, `common/Traced.h` | Open |
| NV-10 | `PascalCase` static constants | `ALL_CAPS_WITH_UNDERSCORE` | e.g. `MaxPSDULength` (802.11 mode headers); headers mixing `MAX_HEADER_SIZE` with PascalCase | Open |
| NV-11 | `ClockTime::ZERO` — ALL_CAPS named-instance constant | `camelCase` like other named-instance constants (`Protocol::ipv4`) — or accept as a discrete sentinel | `clock/common/ClockTime.h` | Open (decide) |
| NV-12 | Timer / self-message name strings in mixed case (`"data_tx_over"`, `"REXMIT"`, `"BGP Start"`, `"StartTimer"`) | descriptive `camelCase`, ideally matching the member | many `.cc` files | Open (low priority) |

### Configuration

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-13 | Bare-capital iteration variables `${N1}`, `${N2}` | `camelCase` | assorted `examples/`/`showcases/` `omnetpp.ini` | Open |

### Features

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-14 | Feature ids breaking acronym-as-word: `TSN`, `OpenMP`, `BMac`, `LMac` | `Tsn`, `Openmp`?, `Bmac`, `Lmac` | `.oppfeatures` | Open (breaking: also changes `-DINET_WITH_*` flags — coordinate) |

### Tests

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-15 | `PascalCase` type-name fingerprint tags mixed with lowercase descriptive tags (`EthernetCsmaMac`) | lowercase descriptive words | `tests/fingerprint/*.csv` tag column | Open (low priority) |
| NV-16 | `.test` names using `IPv6Address`-style acronym casing / mixed case | acronym-as-word (`Ipv6Address`) | `tests/{module,unit}/*.test` | Open |

### Icons

| Id | Deviation | Should be | Where | Status |
|---|---|---|---|---|
| NV-17 | Hyphenated icon file name `europe-er.png` | run-together lowercase | `images/maps/` | Open (trivial) |

---

## Auditing

When auditing a file or area against [naming-conventions.md](naming-conventions.md), record what you
find here rather than fixing it silently in place:

1. For each name that breaks a rule, add a row: if it is a deliberate, permanent choice, add it to
   **Sanctioned** with the reason; otherwise add it to **Open violations** with a suggested rename and
   `Status = Open`.
2. Cite the rule it departs from and the exact location(s).
3. When a rename actually happens, set the row's `Status` to `Done` (keep the id) and land the rename
   as its own reviewable change.
4. Log the audited area below so coverage is visible.

### Audit coverage

| Area | Date | Findings |
|---|---|---|
| Repo-wide convention scan (packages, modules, gates, params, signals, `.msg` types/fields, C++ types/methods/members/enums/macros/constants, `.ini`, `.oppfeatures`, directories, tests, icons) | 2026-07-20 | Seeded this ledger: NS-01…05, NV-01…17. Not an exhaustive per-file audit — a sampling scan; individual files may hold further violations not yet listed. |
