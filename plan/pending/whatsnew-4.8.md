# The release notes of INET 4.8

## Summary

The release notes of the version in development are 159 small entry files in `doc/whatsnew/`. At the release, `whatsnew.py release` writes them into `WHATSNEW` as three numbered lists. That gives about 1070 lines and 87 backward incompatible entries. The sections of 4.7 and 4.6 are different: an introduction with the highlights, and about 20 large entries, each about one theme. A reader who decides about an upgrade reads that form much faster.

This plan merges the entries into themes, adds the user-visible changes since 4.7 that have no entry, names the external contributors, and writes the introduction. The release tool stays the way to make the release: the themes are entry files, and the introduction is text in the section of the version in development.

## Facts

- Revision: `origin/master` d69fc0eaff (2026-11-24). Branch `topic/whatsnew-4.8`, worktree `/home/levy/workspace/inet-whatsnew-4.8`.
- 1816 commits since `v4.7.0` (2026-07-07), 735 of them change `src/`. 549 of the `src/` commits change no release note.
- `doc/whatsnew/`: 87 incompatible, 32 compatible and 40 fix entries. `whatsnew.py preview --rev origin/master` gives 1069 lines.
- `whatsnew.py release` orders the entries of a list by file name. Text in the section of the version in development, other than the sentence that points to `doc/whatsnew/`, becomes the introduction of the release (`doc/whatsnew/README.md`, "Make a release").
- The 4.7 section points to `src/ChangeLog`, but `src/ChangeLog` has no part for 4.7 or 4.8. Its last part is 4.6.0.
- `src/inet/common/INETDefs.h` needs OMNeT++ 6.4 or later. The CI workflows build with the `omnetpp-6.x` branch, now version 6.5.0.
- Authors of the entry files (the author of the first text of the entry, also for the entries that moved from `WHATSNEW` on 2026-10-20): Miguel González-López 35, Afshin Zanganeh 2, Gyorgy Szaszko 20, tabgab 2, the core team 100.

## Decisions

The owner chose D-1 to D-3 on 2026-11-24.

- **D-1 Themes, as in 4.7.** Merge the small entries and the missing changes into themed entries. Keep the user-visible facts of each small entry.
- **D-2 Prepare, release later.** Do not run `whatsnew.py release`. The themes are entry files in `doc/whatsnew/`, and the introduction is in the section of the version in development. A pull request that lands before the release still adds its own entry to 4.8.
- **D-3 Credits.** Name Miguel González-López and Afshin Zanganeh, in the entry of their work ("contributed by", as 4.6 and 4.7 do) and in the introduction. Do not name tabgab, Gyorgy Szaszko or MakerYuichi.
- **D-4 File names.** A themed file keeps the name `<area>-<topic>.txt` of RR-WHATSNEW-FILE. The tool then orders the entries by area, not by importance. The introduction gives the highlights in the order of importance.
- **D-5 Changed results in one place.** One entry lists the models whose simulation results changed on purpose, and why (RR-FINGERPRINT-NOTE).
- **D-6 No pointer to `src/ChangeLog`.** That file has no part for 4.7 or 4.8.
- **D-7 Kernel versions.** The introduction gives the minimum OMNeT++ version and the version that the CI used (RR-KERNEL-VERSION).

## Steps

1. [x] Audit. Five agents read the commits of `src/` (transport, link and physical layers, network layer and routing, common and queueing) and the commits outside `src/` (examples, showcases, tests, build, user documentation). Each one reports the user-visible changes without an entry, the entries that a later commit made wrong, and the commits that moved a fingerprint or statistical baseline. No commit.
2. [x] Themes. Choose the themes from the entries and the audit, and write the list into this plan. Commit: the plan.
3. [ ] Themed entries, one commit for each list: incompatible, compatible, fixes. Each commit removes the small files of its list and adds the themed files.
4. [ ] The entry of the changed results (D-5). Commit.
5. [ ] The introduction in `WHATSNEW` (D-3, D-6, D-7). Commit.
6. [ ] Check. `whatsnew.py check`; a trial `whatsnew.py release` in a scratch clone, compared with the 4.7 section; a reviewer agent that compares the themed text with the 159 old entries and with the audit; `check-pull-request.sh` on the branch.
7. [ ] Move this plan to `plan/done/`. Commit.

## Audit results (step 1)

Six read-only agents wrote their reports to `/var/tmp/claude-whatsnew48/audit-{A..F}.md` (outside the repository). A to E found 63 gaps, F listed the baseline moves of about 190 commits in 10 model areas. I checked a sample of each report against the tree. The facts that change the plan:

- No entry describes OSPFv3, the HT associations of IEEE 802.11, the 802.11 frames on the wire (management, Block Ack Request, A-MPDU), the new protocol elements, the log formatting option, the serializers and dissectors of many protocols, gPTP, the new test suites, the new examples and showcases, or the documentation website.
- `incompatible/ipsec-ah-and-esp-corrections.txt` says that both new gates `ipIn` and `ipOut` connect to the dispatcher. `IPsec.ned` says that `ipIn` is not used.
- `fixes/tcp-parameter-repairs.txt` and `incompatible/tcp-dynamic-right-sizing.txt` point to other entries by number. The numbers are wrong after a merge, so the themes use titles.
- `bin/inet_run_serializer_tests` is new since 4.7. The old entry says that it "stays".
- Credits: the work of Miguel González-López is the source of all IEEE 802.11 entries except the rate control per receiver, the per-station rate statistics, the VHT mode queries, the part durations and the responses outside an exchange. It is also the source of the PCAPng link type and capture limit entries, of PR #1130 (Radiotap capture, which our commits redesigned), of the QUIC STREAM payload fix and of the fragment FCS fix. The QUIC entry about unknown frames came to him only through a format commit, and it is not his work. Afshin Zanganeh is the source of the vector recovery.

## Themes (step 2)

The tool orders each list by file name, so the order below is the order of the release. "Gap" names a gap of an audit report, for example B-G3.

Backward incompatible changes:

| File | Old entries and gaps |
| --- | --- |
| `applications-tcp-apps-and-tcplwip.txt` | A-G5 GenericAppMsgReq, A-G2, A-G6 |
| `build-opp-repl-test-tools.txt` | build-opp-repl-tools, tests-fingerprint-store, E-G8 |
| `common-log-formatting-and-packet-descriptions.txt` | D-G6, D-G5 |
| `dhcp-address-conflict-detection.txt` | dhcp-client-checks-granted-address, dhcp-seven-defects |
| `ethernet-csma-cd-and-ethertypes.txt` | ethernet-half-duplex-attempt-limit, ethernet-csma-cd-medium-access-delay, ethernet-csma-cd-retry-limit-drops, ethernet-ethertype-rendering, ethernet-frames-without-fcs, ethernet-switches-forward-unknown-ethertypes |
| `ieee80211-block-ack.txt` | ieee80211-immediate-compressed-block-ack, -basic-block-ack-selection, -block-ack-agreement-lifecycle, -basic-block-ack-transfer-completion, -block-ack-delivery-order, -block-ack-wire-types, -bounded-mpdu-deaggregation, -block-ack-data-retry, -qos-data-preview, -reversible-data-preparation |
| `ieee80211-channel-access.txt` | ieee80211-single-protection-and-shared-txnav, -multiple-protection, -recipient-responses-keep-local-nav, -local-interframe, -radio-commands-wait-for-accepted-frame, -management-recovery-per-edcaf, -qos-data-lifetime, -retry-counters-per-receiver, -response-outside-frame-exchange, B-G5, B-G6 |
| `ieee80211-frames-on-the-wire.txt` | ieee80211-wire-and-reception-corrections, B-G1, B-G2, B-G8 |
| `ieee80211-ht-support.txt` | ieee80211-ac-interfaces-expose-ht-support, -mapped-5ghz-channels, -signal-extension, -rifs-permission, -phy-mode-response-rate-properties, -vht-mode-queries, -response-queries-caller-role, B-G3, B-G4 |
| `ieee80211-rates-per-receiver.txt` | ieee80211-rate-control-per-receiver, -per-receiver-data-frame-rates, -per-station-rate-statistics, -mib-rate-state, -mib-commit-operations, -management-publishes-rate-state, B-G7 |
| `ieee8021q-s-tags-and-c-tags.txt` | linklayer-c-tag-tag-names, linklayer-s-tag-tags, ieee8021q-pcp-sources, linklayer-mac-forwarding-table-vlan-id, linklayer-ieee8021ad-policy, ieee8021q-tag-protocol-printer, B-G15 |
| `ipsec-ah-and-esp-corrections.txt` | ipsec-ah-and-esp-corrections (the gate sentence corrected) |
| `ipv4-ipv6-icmp-and-udp-corrections.txt` | icmp-no-error-about-link-layer-broadcast, icmp-unrecognized-type-discarded, icmpv6-error-report-for-unregistered-protocol, ipv4-header-checksum-and-reassembly-key, ipv6-packet-too-big-mtu-and-fragment-length, ipv6-reassembly-payload-length, ipv6-configuration-routes-return-with-carrier, common-packet-too-big-drop-reason, arp-unprocessable-packet-discarded, udp-received-datagram-reported-once |
| `ipv6-neighbor-discovery.txt` | icmpv6-ipv6-duplicate-address-detection-ends-when-its, icmpv6-ipv6-router-discovery-keeps-one-process-per, icmpv6-router-interface-holds-its-configured-global-ipv6, icmpv6-solicited-neighbor-advertisement-restarts, ipv6-ra-without-prefix-information, ipv6tunneling-ipv6-network-configurator-keeps-each-ipv6-tunnel, C-G2, C-G3, C-G11 |
| `mipv6-mobile-ipv6-and-pmipv6.txt` | mipv6-binding-acknowledgement-proxy-flag, pmipv6-proxy-mobile-ipv6-gateway-reports-the-handoff, ipv6-tunnel-exit-point-survives-a-dropped-decapsulated, C-G4 |
| `mpls-rfc-3032.txt` | mpls-ethernet-links, mpls-explicit-null-and-implicit-null, mpls-fragments-labeled-datagram, mpls-ttl-rules-of-rfc-3032, C-G6 |
| `networklayer-deleted-interfaces.txt` | networklayer-interface-on-scalar-ifin-ifout, C-G5, A-G12, B-G16 |
| `ospfv3-adjacencies-and-dr-election.txt` | C-G1 |
| `physicallayer-radio-and-radio-medium.txt` | radio-concurrent-transmissions-and-receptions, radio-every-transmission-and-reception-in-progress, radio-interference-for-a-caller-listening, radio-reception-path-loss-knows-receiver, radio-compute-distance-path-loss, physicallayer-snir-interval, radio-correlated-log-normal-shadowing, radio-log-normal-shadowing-per-reception, radio-medium-purge-without-timer |
| `protocolelement-new-elements-and-forwarding.txt` | protocolelement-resending-accepts-offered-packets, protocolelement-receiver-keeps-packet-bit-error, D-G1 |
| `queueing-queries-without-side-effects.txt` | the 15 queueing- entries, queueing-red-early-drop-reason, queueing-packet-filters-can-retain-local-state, D-G9, D-G10 |
| `quic-corrections.txt` | quic-unknown-frame-type-and-initial-padding, A-G8 |
| `recorder-pcap-and-pcapng.txt` | recorder-helper-direction, recorder-pcapng-capture-limits, ieee80211-radiotap-capture, recorder-pcapng-block-api, recorder-pcapng-crc-errors, recorder-pcapng-link-types, recorder-pcapng-without-interface, recorder-wireless-receiver-copy, D-G2 |
| `rip-and-ripng-corrections.txt` | rip-and-ripng-corrections, the RIP header of C-G7 |
| `rtp-payload-type-10-removed.txt` | A-G11 |
| `tcp-algorithm-submodule.txt` | tcp-algorithm-submodule, tcp-dctcpfamily-removed |
| `tcp-connection-setup-and-stop.txt` | tcp-connection-setup-timeout, tcp-stop-closes-connections |
| `tcp-model-rework.txt` | tcp-rfc-defaults-linuxtcp-and-legacytcp, tcp-first-round-trip-measurement, tcp-dynamic-right-sizing, tcp-linux-receive-buffer, tcp-linux-drops-syn-data, tcp-urgent-out-of-band-byte, A-G1, A-G7 |
| `tests-changed-simulation-results.txt` | audit F (D-5) |

Backward compatible changes:

| File | Old entries and gaps |
| --- | --- |
| `configurator-declared-lans.txt` | configurator-declared-lans |
| `doc-documentation-and-website.txt` | E-G10, E-G11, E-G12 |
| `examples-new-and-repaired-examples.txt` | E-G1, E-G2, E-G3, E-G4 |
| `packet-api-additions.txt` | packet-region-tag-removal-by-type, packet-type-aware-field-serializers, networklayer-host-name-of-mac-address, D-G3, C-G9 |
| `redundancy-vector-recovery.txt` | redundancy-vector-recovery, redundancy-vector-sequence-recovery |
| `serializer-captured-packets.txt` | C-G7 except RIP, A-G9, A-G10, B-G12, B-G13 |
| `tests-new-test-suites.txt` | E-G5, E-G6, E-G7, E-G9 |
| `tsn-gptp-and-streaming-phy.txt` | B-G11, C-G8 |

Bug fixes:

| File | Old entries and gaps |
| --- | --- |
| `common-other-fixes.txt` | packet-chunk-flags, visualizer-packet-drop-animation-uses-the-dy-parameter, examples-europe-map-equirectangular, D-G4, D-G7, D-G8, C-G10, B-G19 (the msg properties) |
| `ieee80211-fixes.txt` | ieee80211-5ghz-band-centers, -ht-mode-band-and-preamble, -ht-vht-part-durations, -max-transmission-duration, B-G9 |
| `linklayer-fixes.txt` | ethernet-frame-bit-error-discarded, ethernet-static-mac-table-entries-do-not-age, ieee8021q-user-priority-passed-down, linklayer-relay-keeps-vlan-tag, linklayer-relay-upper-layer-vlan-tag, linklayer-relay-vlan-lookup, mrp-relay-multicast-forwarding, lmac-slot-too-short, ackingmac-incorrect-header, B-G14, B-G17 |
| `physicallayer-fixes.txt` | dielectricobstacleloss-loss-exponent, physicallayer-error-model-initialization, physicallayer-separate-parts-late-interferer, B-G18, the part decision count of B-G19 |
| `tcp-fixes.txt` | the 10 tcp- fix entries, tcp-negative-timer-checks, tcp-tcpspoof-dest-address, A-G3, A-G4 |

Left out: D-G11 (`@bit`, `FunctionBase::printStructure()`), the print and log details of B-G19, the `bin/inet_start_jekyll_server` question of E (a file, not a change), and the GitHub workflows of E-G9 that only a maintainer sees.

## Verification

- No fact of an old entry is lost, unless the audit shows that a later commit made it wrong (step 6, reviewer).
- Each user-visible change of the audit is in a theme or is listed below as left out, with the reason.
- `whatsnew.py check` passes, and the trial release gives one 4.8 section with the introduction and three lists.
