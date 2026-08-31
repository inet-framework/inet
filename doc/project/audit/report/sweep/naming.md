# Naming sweep — the mechanical rules

> **Kind:** report · **Status:** snapshot 2026-08-31 · **Seal:** none · **Owns:** — · **Stands on:** [rule/naming.md](../../../rule/naming.md)

- **Scope:** `src/inet/`, `images/`, `.github/workflows/`
- **Date:** 2026-08-31, commit `e38566236b`
- **Command:** `doc/project/enforcement/check-naming.sh`
- **Rules checked:** [NR-PKG](../../../rule/naming.md#nr-pkg), [NR-DIR](../../../rule/naming.md#nr-dir), [NR-GEN](../../../rule/naming.md#nr-gen), [NR-ASSET](../../../rule/naming.md#nr-asset), [NR-CI](../../../rule/naming.md#nr-ci)
- **Result:** FAIL — 23 hits, of which 11 are already ledgered and 12 are new.

This is the first run of the gate. It is the mechanical half of the naming rules only; the NED,
`.msg` and semantic names need `T4` review.

## What the gate found, sorted

| Hit | Disposition |
| --- | --- |
| `transportlayer/tcp_common`, `transportlayer/tcp_lwip`, `routing/ospf_common` | known — `NV-03` |
| `rtp/profiles`, `tcp/flavours`, `tcp_common/headers`, `pim/tables`, `pim/modes`, `eigrp/messages`, `eigrp/tables` | known — `NV-02` |
| `images/maps/europe-er.png` | known — `NV-17` |
| `udp/headers`, `sctp/headers`, `ipv4/headers`, `ipv6/headers` | **new** — same class as `NV-02`; the row is extended |
| `images/maps/world-er.png` | **new** — same class as `NV-17`; the row is extended |
| `images/misc/voipPhone.png`, `signal_arrival.png`, `signal_departure.png`, `signal_power_0..3.png` | **new** — `NV-18` |

`NR-GEN` and `NR-CI` are clean: every `*_m.h` has its `.msg` beside it, and every workflow file is
lowercase and hyphenated.

## One finding was the gate, not the code

The first run reported 187 hits, of which 164 were icon files with a `_vs`, `_s`, `_l` or `_vl`
suffix. That suffix is the **OMNeT++ icon size convention**, not an underscore in a name, and the
check was wrong to flag it. The check now strips the size suffix before it tests the stem, and
[NR-ASSET](../../../rule/naming.md#nr-asset) states the convention.

Worth recording: the first run of a new gate is as likely to be reporting its own defect as the
code's, and a gate that cries wolf on 164 files would have been switched off before it ever caught
anything.

## Not covered by this sweep

`NR-NED-*`, `NR-MSG-*` and `NR-CPP-TIMER` need `T4` agent review; `NR-CASE`, `NR-CPP-TYPE` and
`NR-CPP-NAME` need [check-cpp.sh](../../../enforcement/check-cpp.sh), which needs a compilation
database and has not been run. `NR-INI`, `NR-TEST` and `NR-TOOL` have no check yet.
