# IEEE 802.15.4 — bounded reference dispositions

> **Kind:** report · **Status:** snapshot 2026-09-19 · **Seal:** none · **Owns:** — · **Stands on:** [applicability.md](applicability.md), [catalog.md](../../standard/ieee802154/catalog.md)

Baseline `5f3845f055`. This is the explicit disposition of the earlier catalog's 122 extracted
reference occurrences: 106 distinct source-to-target edges and 86 distinct targets, from 50 source
nodes. It is a bounded frontier inventory, not proof that the corpus extracts every reference or
that all IEEE requirements have tests. Later manual service, clause-6 and companion-source
lookups are recorded in the applicability audit and results, not silently included in these counts.

Canonical node IDs in the table have prefix `ieee802154-2024:`; pages are physical PDF pages.
The incoming source column identifies the predicate to inspect, rather than implying that a
resolved link by itself establishes applicability. “Selected” applies only to the selected
branches/rows of a multi-mode clause or table. It never imports every PHY mentioned in that node.

## Disposition predicates

| Disposition | Reason / stopping boundary | Check or later owner |
| --- | --- | --- |
| selected | Required legacy layout, data service, unsecured early returns, unslotted access, monitor format or selected O-QPSK parameters. Common heading nodes resolve through the cataloged descendants. IEEE 802 address reference is pinned separately; four-octet CRC dependencies do not apply to the selected two-octet FCS. | Existing C-WIRE/ACCESS/ACK/RECEIVE/SECURITY/PHY/SERVICE and the applicability tables |
| timestamp | M1 declares optional Data timestamp capability false. Typed MCPS Timestamp is invalid; internal PHY event times are still required. Representation under disabled capability is an explicit model decision. | Optional timestamp extension; no current open source lookup |
| managed | Scan/beacon notification and indirect Data Request dependencies are owed later; they are not selected away from a full profile. The Data Request CSMA exception does not apply to a direct Data frame. | M2 steps 7–9; C-SCAN and indirect C-ACK variant |
| beacon | Table 8-38 includes legacy beacon auto-response as well as enhanced request fields; its whole title cannot justify an enhanced-only exclusion. | M2 managed beacon-response audit; E1 enhanced fields |
| security | Outgoing level zero returns in 9.2.2(a); nonzero with disabled security returns in (b). Incoming secured legacy returns in 9.2.4(a)/(b), and unsecured disabled-profile validation returns in 9.2.5(a). No key/device/IE/AEAD processing is reached. | Operational security work; no successful transform or deep auxiliary-header validation claimed in M1 |
| scheduled | No ATI/superframe, slotted access, TSCH, CSL/LE or PCA is selected. Fields selecting those modes must not activate them accidentally. Ordinary immediate ACK and unslotted access remain selected. | M3 or corresponding optional-mode extension |
| enhanced | Legacy versions 0/1 have no sequence suppression or IEs; Table 7-2's version-2 layout is not a legacy oracle. Enhanced-beacon fields are unselected. | E1 and its prerequisite audit |
| ranging | No ranging is selected and its result descriptor is invalid; mode-specific counters/timing are not ordinary ACK-timeout inputs. | Ranging extension |
| other-phy | Selected O-QPSK uses DataRate selector zero, ordinary 127-octet PSDU and two-octet FCS. SUN/TVWS/MSK/LECIM/UWB/TASK/RS-GFSK branches and exceptions are false for that selection. | Relevant PHY extension and independent PHY evidence |

Table 8-37 is retained as selected capability-reporting metadata: optional feature capability
flags report false in M1, corresponding enable flags remain false, and writes follow read-only
markers or unsupported-value INVALID_PARAMETER as appropriate. Its false feature branches do
not require implementations of the referenced optional mechanisms. Table 9-8's security tables
are not consulted by the selected early returns. Their support/absence must remain explicit in
the 1b attribute matrix, not silently masquerade as working security.

## Target inventory

| Target | Disposition | Physical PDF pages | Incoming sources |
| --- | --- | --- | --- |
| `clause:2` | selected | 42–43 | `clause:7.2.11` |
| `clause:4.5.1` | selected | 50–51 | `clause:7.1` |
| `clause:6.3.2` | selected | 63–63 | `clause:6.6.1` |
| `clause:6.3.2.1` | selected | 63–64 | `table:8-29`, `table:8-31`, `table:8-36` |
| `clause:6.5.3` | timestamp | 68–69 | `table:8-31`, `table:8-32` |
| `clause:6.6.2` | selected | 70–72 | `clause:10.23.1`, `clause:6.6.3.3`, `figure:7-16`, `table:8-32` |
| `clause:6.6.3` | selected | 72–72 | `table:8-30`, `table:8-31` |
| `clause:6.6.4` | scheduled | 73–74 | `clause:6.6.3.3` |
| `clause:7.2.2.5` | selected | 80–80 | `clause:6.6.3.2` |
| `clause:7.2.11` | selected | 84–85 | `clause:6.6.2` |
| `clause:8.2.2` | selected | 112–114 | `table:8-31` |
| `clause:8.2.4.3` | managed | 116–116 | `table:8-36` |
| `clause:8.2.8.2` | managed | 127–128 | `clause:6.4.1.1` |
| `clause:8.3.3` | selected | 142–142 | `table:8-32` |
| `clause:9.2.2` | selected | 160–162 | `clause:6.6.1`, `clause:6.6.3.4`, `figure:7-16` |
| `clause:9.2.3` | security | 162–163 | `clause:9.2.2` |
| `clause:9.2.4` | selected | 163–163 | `clause:6.6.2`, `clause:9.2.5` |
| `clause:9.2.5` | selected | 165–166 | `clause:9.2.4` |
| `clause:9.2.6` | security | 166–166 | `clause:9.2.5` |
| `clause:9.2.7` | security | 166–166 | `clause:9.2.5` |
| `clause:9.2.8` | security | 166–167 | `clause:9.2.5` |
| `clause:9.2.10` | security | 167–168 | `clause:9.2.5` |
| `clause:9.3.4` | security | 170–170 | `clause:9.2.2` |
| `clause:10.2` | scheduled | 182–182 | `table:8-36` |
| `clause:10.2.6` | scheduled | 188–189 | `table:8-36` |
| `clause:10.3.2.3` | scheduled | 200–200 | `clause:7.2.2.4` |
| `clause:10.5.3` | scheduled | 262–262 | `clause:7.2.2.4` |
| `clause:10.10.2.1` | scheduled | 314–315 | `table:8-29` |
| `clause:10.11` | scheduled | 319–320 | `clause:6.3.2.1` |
| `clause:10.15.2.1` | other-phy | 337–337 | `table:8-29` |
| `clause:10.22` | managed | 374–374 | `clause:7.2.2.4`, `table:8-30` |
| `clause:10.22.4.1` | managed | 377–378 | `clause:6.3.2.1` |
| `clause:10.23` | selected | 379–379 | `table:8-32` |
| `clause:10.29.1.4` | ranging | 438–438 | `table:12-2` |
| `clause:11.1.3.1` | selected | 565–566 | `table:12-2` |
| `clause:11.1.3.7` | other-phy | 569–569 | `clause:11.1.3.3` |
| `clause:11.1.3.9` | other-phy | 572–572 | `clause:11.1.3.3` |
| `clause:11.1.3.11` | other-phy | 584–584 | `clause:11.1.3.3` |
| `clause:11.2.7` | selected | 596–596 | `table:8-32` |
| `clause:11.2.8` | selected | 596–597 | `table:12-2` |
| `clause:12.3` | selected | 601–601 | `clause:11.1.3.1` |
| `clause:12.3.2` | selected | 601–601 | `clause:11.2.8` |
| `clause:16.2.6` | other-phy | 650–650 | `clause:11.2.8` |
| `clause:16.2.7` | other-phy | 655–655 | `table:8-29` |
| `clause:16.4.1` | other-phy | 671–671 | `clause:11.1.3.1` |
| `clause:16.6` | other-phy | 680–680 | `clause:11.2.8` |
| `clause:18.1.3` | other-phy | 688–688 | `clause:7.2.11` |
| `clause:23.3` | other-phy | 802–802 | `clause:6.6.3.3` |
| `clause:31.3` | other-phy | 895–895 | `table:8-29` |
| `clause:32.2` | other-phy | 913–913 | `table:8-29` |
| `figure:4-4` | selected | 51–51 | `clause:4.5.1` |
| `figure:6-2` | selected | 64–64 | `clause:6.3.2.1` |
| `figure:6-5` | selected | 72–72 | `clause:6.6.3.2` |
| `figure:6-6` | selected | 73–73 | `clause:6.6.3.3` |
| `figure:7-1` | selected | 78–79 | `clause:10.23.1` |
| `figure:7-4` | selected | 85–86 | `clause:7.2.11` |
| `figure:7-15` | selected | 91–91 | `clause:7.3.3` |
| `table:7-2` | enhanced | 80–81, 81–81 | `clause:7.2.2.6` |
| `table:7-3` | selected | 81–82 | `clause:7.2.2.11`, `clause:7.2.2.9`, `figure:7-16` |
| `table:7-7` | enhanced | 98–99, 99–99 | `table:8-30` |
| `table:7-8` | enhanced | 100–100 | `table:8-30` |
| `table:7-9` | enhanced | 102–103, 103–104, 104–105, 105–105 | `table:8-30` |
| `table:7-10` | enhanced | 105–106 | `table:8-30` |
| `table:8-2` | selected | 114–114 | `table:8-30`, `table:8-32` |
| `table:8-27` | ranging | 137–138, 138–139 | `table:8-30` |
| `table:8-28` | ranging | 139–140, 140–141, 141–142 | `table:8-31`, `table:8-32` |
| `table:8-29` | selected | 142–143, 143–143 | `table:8-30` |
| `table:8-36` | selected | 152–153, 153–154, 154–155, 155–156, 156–156 | `clause:8.4.3.1`, `table:8-11`, `table:8-9` |
| `table:8-37` | selected | 156–157, 157–158 | `clause:8.4.3.1` |
| `table:8-38` | beacon | 158–158 | `clause:8.4.3.1` |
| `table:8-39` | enhanced | 158–159, 159–160 | `clause:8.4.3.1` |
| `table:9-1` | security | 162–162 | `clause:9.2.2` |
| `table:9-2` | security | 163–165 | `clause:9.2.4` |
| `table:9-8` | security | 176–177, 177–177 | `table:8-11`, `table:8-9` |
| `table:10-63` | scheduled | 317–317 | `table:8-31` |
| `table:12-1` | selected | 601–601 | `clause:11.2.2`, `clause:11.2.3` |
| `table:12-2` | selected | 601–602, 602–603 | `clause:11.2.8` |
| `table:12-3` | selected | 603–603 | `table:12-2` |
| `table:18-1` | other-phy | 689–689 | `table:8-29` |
| `table:19-1` | other-phy | 693–694, 694–694 | `table:8-29` |
| `table:21-10` | other-phy | 744–745 | `clause:11.2.8`, `table:8-29` |
| `table:22-1` | other-phy | 761–762 | `table:8-29` |
| `table:31-5` | other-phy | 895–896 | `table:8-29` |
| `table:31-6` | other-phy | 896–896 | `table:8-29` |
| `table:31-7` | other-phy | 896–897, 897–897 | `table:8-29` |
| `table:32-2` | other-phy | 913–913 | `table:8-29` |

## Selected-target expansion

The 34 selected target nodes produced 80 further reference occurrences: 79 resolved and one
unresolved parser record. Eighteen resolved targets were outside the first 86-target set;
Figure 7-16 was already one of the original 50 source nodes, so only 17 were new to the union.
The following dispositions close this additional frontier without recursively entering disabled
feature branches.

| Additional target(s) | Predicate / disposition |
| --- | --- |
| 9.4.4.2, 9.4.4.3, Table 9-7 | Table 8-2 ignores key-identification parameters when SecurityLevel=0; other outgoing levels return UNSUPPORTED_SECURITY while security is disabled. No key-source/index lookup is reached. |
| Table 9-6 | Selected level 0 has no security attributes or MIC; nonzero transforms are unsupported. Level 4 is reserved/deprecated, not an encryption-only option to implement. Incoming disabled-security early return still precedes deep header validation; no protected plaintext delivery is permitted. The table's sole reference returns to already reviewed 9.2.4. |
| 10.3, 10.4, 10.5, 10.13, 10.16, 10.27, 10.28, 10.37, 10.37.3 | Table 8-37 capability/enabled flags are false. The service exposes the false capability, not the optional procedure. Local simulation statistics do not imply the optional IEEE MAC-metrics capability. |
| 10.7.3.10 | Figure 6-6 continuation expressly conditions Timestamp Difference IE on TVWS ranging; selected O-QPSK and immediate ACK do not satisfy it. |
| 19.6 | Table 12-1's LRP UWB postamble timing is not an O-QPSK timing parameter. |
| 23.3.4 | Figure 7-1 conditions this format on Fragment packet type, which is outside the selected Data/Imm-Ack formats. Its unextracted Frak/Extended format references 23.3.7.3 and 7.3.6 have the same explicit nonselected-type boundary. |
| Figure 4-5 | Selected address-order illustration; physical p. 51 was visually checked against 4.5.1 and C-WIRE vectors. No outgoing references. |
| Figure 7-16 | Enhanced layout is outside M1, but its following legacy ACK prose is selected and was already extracted as WIRE-15 plus the receive/ACK rules. Its selected references return to already classified nodes. |

The unresolved record in clause 2 is the footnote at physical p. 42,
`ieee802154-2024@240974:240982`, stating that standards/products referred to in “Clause 2” are
IEEE trademarks. It is a publication/legal footnote, not a protocol dependency; the extractor
misclassified its qualification. It is retained as an unresolved tool record and dispositioned as
non-normative for this audit, not falsely converted to a resolved external standard.

## Manual-reference boundaries

The source inventory is not restricted to automatically extracted links. Clause-6 continuations,
Table 6-1 IE references, Tables 8-1/8-26 service references, complete Tables 8-36/8-37/12-2,
Table 12-3, the selected channel structure, source CRC example and IEEE Std 802 address reference
were inspected separately. Their applicable branches and deferred procedures are recorded in
[applicability.md](applicability.md). Common fields needed for bounds/filtering remain selected;
deep auxiliary-security syntax is not a prerequisite to disabled-security early returns.

This stopping rule is predicate-based: a selected edge is followed until its meaning is supplied
by an inspected definition/procedure, a previously classified node, or an explicitly false
branch condition. A deferred mandatory profile capability is recorded as debt, not an exclusion.
Newly enabled features must reopen their stopped branches. This record is ready for a bounded
step-0 closure review; it does not itself declare the review passed or establish runtime support.

The manual timing root 6.3.1 also selects Figure 6-1 and Table 8-35's 18-octet
aMaxSifsFrameSize. ACCESS-3/C-ACK now explicitly checks the 18/19-octet transition with and
without ACK. Other Table 8-35 constants govern nonselected superframe/CAP/beacon procedures;
its references to 10.2, 7.3.1.5 and 10.10.2.1 stop at those false M1 predicates. This manual
root is additional to the historical 50-source extraction, not silently included in its count.
