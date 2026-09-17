# Repair the regressions of the model-defects branch and record its baselines

**Status:** done 2026-09-17. The branch `topic/protocol-model-defects` landed on master on top of
`c3fbe79ad0` (the three master commits after `b0c7a25e75` change only CI files, `README.md` and a
GitHub helper), and its statistical results landed on master of the statistics repository. The
branches and the worktree are deleted.
It continues [protocol-model-defects.md](protocol-model-defects.md).

## Why this plan exists

On 2026-09-17 the three recorded suites ran on the branch head and on `inet-master` at the same
base, with all features on (`opp_featuretool enable all`, as CI does). `inet-master` passes all of
them. The branch does not:

| Suite | Mode | Branch head | Base |
| --- | --- | --- | --- |
| fingerprint (`tests/fingerprint`, `-f tplx -f ~tNl -f ~tND`) | release | **108 of 1752 changed** | 1752 as expected (OK) |
| module (`inet_run_module_tests`) | debug | 334 PASS, **11 FAIL** | 345 PASS |
| statistical (`statistics` repository) | release, in the container | **117 of 929 changed** | 917 PASS, 10 SKIP and 2 ERROR, all expected |

The branch says that no fingerprint moves. That claim came from an A/B run on 2026-09-16, and the
A/B run was wrong. The plan, the probe commit message and the evidence repeat the claim.

### Where the fingerprints change

Each source commit was built and run against the 108 changed rows (scratch folder `bisect/`):

| Commit (by subject) | Rows | Scenarios | Why they move |
| --- | --- | --- | --- |
| icmpv6: fix: an error report for an unregistered protocol is dropped, then icmp: fix: no error report about a link-layer broadcast or multicast | 3 | `examples/manetrouting/gpsr` IPv6, MultiIPv6, DynamicIPv6 | to be confirmed from the diff |
| tcp: fix: the first round-trip measurement follows RFC 6298 section 2.2 | 57 | every row with TCP traffic: `examples/inet/tcpclientserver` (34), `bulktransfer` (4), `nclients` (3), BGP, the ARP examples, `voipstream`, `canvas/styling`, and others | the first measurement sets the timeout, so retransmissions come earlier |
| quic: fix: an unknown frame closes the connection, and a server pads its Initial | 42 | every `examples/quic` row | the server Initial is 1200 octets |
| dhcp: fix: seven defects in the client and the server, then dhcp: add: the client probes the granted address | 6 | `examples/dhcp` (4), `tutorials/configurator` Step9, `canvas/interfacetable` AdvancedFeatures | new retransmission times, then the probe wait |

No other commit moves a changed row.

### Why the module tests fail

| Tests | Cause | Kind |
| --- | --- | --- |
| `DHCP_2` | The client probes over PPP. The interface has no MAC address, and `Arp::sendArpProbe` stops on `ASSERT(!srcAddr.isUnspecified())`. | **defect of the probe commit** |
| `DHCP_lifecycle_1` | "Self message 'Probe Timeout' received when DhcpClient is down": the stop and crash handlers do not cancel `timerProbe`. | **defect of the probe commit** |
| `DHCP_lifecycle_2`, `DHCP_lifecycle_3` | The retransmission repair changed the log line and the retransmission times. The failing regular expression backtracks for 11 to 12 minutes. | expected output |
| `tcp_sack_1`, `_3`, `_4`, `_5`, `tcp_stresstest_2`, `_3`, `tcp_stresstest_msgq_1` | RFC 6298: the retransmission comes at 1.021 s instead of 1.787 s. The stress tests still deliver all 655360 octets. | expected output |

## Rules this plan follows

- **A regenerated baseline travels with the commit that causes it** (PR-SPLIT-BASELINE,
  TR-BASELINE-COMMIT). Fingerprints and module expected outputs go into the causing commit.
- **Every changed row is explained** (TR-BASELINE-PROVENANCE): the message names the row groups
  and says why the new values are correct. An unexplained row is a regression until shown otherwise.
- **The `Change:` trailer names what moves** (CR-OBL-BASELINE): `fingerprint`, `statistical` and
  `expected` where they apply.
- **A repair is shown by a check that can fail**: `DHCP_2` and `DHCP_lifecycle_1` fail without the
  probe repair.

## Decisions

1. **Statistical results live in the `statistics` repository**, which CI clones from its default
   branch. They cannot share a commit with INET. They go on a branch `topic/protocol-model-defects`
   of that repository, **one commit for each INET commit that changes them**. Each message names the
   INET commit by its subject, because the INET hashes change with every rewrite. The format follows
   that repository: `tcp: update statistics after the first round-trip measurement repair`.
2. **The update is `opp_repl`'s `update_statistical_test_results(mode="release", ...)`**, as asked.
   CI runs release, and the function defaults to debug, so the mode is given.
3. **The update runs in the GitHub-job container** (`/home/levy/workspace/ghci-statistical`,
   ubuntu 24.04 with clang 16), with `opp_repl` mounted read-only and installed into the job's
   environment. Error-rate statistics generated with the host's clang 23 differ from CI in the last
   bit, and CI compares bit-exactly. The container gives CI's values (proven on 2026-09-16).
   **Fallback**, if `opp_repl` does not run there: update on the host, then run the updated
   configurations in the container and replace every file that the container does not reproduce.
4. **Fingerprints are updated in the CSV files only.** Master changes the CSV files in behavior
   commits and synchronizes `tests/fingerprint/store.json` in commits of its own.
5. **The TCP checksum pair becomes one commit.** "tcp: fix: the checksum is computed by default"
   breaks `examples/inet/nclients -c lwip__inet`, and "tcp: change: withdraw…" undoes it. A
   commit whose own fingerprint run stops with an error cannot carry baselines. Together the two
   change no source and record the blocker in the evidence. The WHATSNEW entry "(withdrawn) TCP
   does NOT compute its checksum by default" describes no change and goes.
6. **Measure first, then rewrite once.** One pass builds each source commit in a detached checkout
   and records its fingerprints, module outputs and statistics. A second pass rewrites the series
   and puts the recorded values into each commit. The container needs no INET rewrite, because the
   statistics commits are keyed by subject.

## Steps

### 1. Repair the probe

- [x] In `DhcpClient::handleDhcpAck`, probe only where `Ipv4` uses ARP. `Ipv4` does not use ARP
      when `!ie->isBroadcast() || ie->getMacAddress().isUnspecified()` (`Ipv4.cc`, "we can't do
      ARP"). Elsewhere the client binds at once, as with `probeWait = 0s`.
- [x] Cancel `timerProbe` wherever the client cancels its other timers: `finish()`,
      `handleStopOperation()`, `handleCrashOperation()`, and the restart paths.
- [x] Fold both into "dhcp: add: the client probes the granted address before it takes it".
      Record them in `doc/project/evidence/model/dhcp/notes.md`.
- [x] A third defect showed up in step 3: the client probed on renewal as well, and bound the
      address a second time. The probe is now only for an address the client begins to use,
      after REQUESTING or REBOOTING (RFC 2131 sections 3.1 and 3.2, RFC 5227 section 2.1). The
      repaired probe commit was measured again in step 3.
- [x] Check: without the repair both tests stop with an error. With it, `DHCP_2` passes, and
      `DHCP_lifecycle_1` runs to the end but needs new expected output (step 4): the probe wait
      moves the binding by 1 s, onto the 1-s marks of its scenario, so the client now stops
      during the probe. That is the case that found the crash, so the test keeps it.

### 2. Fold the TCP checksum pair

- [x] Squash the withdrawal into the default commit. The result changes no source, keeps the
      blocker in the TCP evidence, and drops its WHATSNEW entry. The entries after it are
      renumbered. The subject and trailer describe what is left: evidence at `doc` depth.
      Done: the folded commit changes only `tcp/results.md`, `tcp/coverage.md` and
      `udp/results.md`. It also removes the comment "the checksum is required, so it is computed
      by default" that the withdrawal had left above `default("declared")`, and it corrects the
      UDP sentence that said the TCP change had landed. Its message is still the old one.

### 3. Measure every source commit — DONE 2026-09-17

- [x] The changed sets at the head: 108 fingerprints (the same rows and values as before steps 1
      and 2), 10 module tests (the 7 TCP tests and the 3 DHCP lifecycle tests; `DHCP_2` passes
      now), and 117 statistical results.
- [x] The container control at the base: 929 tests, 917 PASS, 10 SKIP and 2 ERROR, all expected.
- [x] Every source commit measured. What each one moves:

      | Commit | Fingerprints | Statistics | Module tests |
      | --- | --- | --- | --- |
      | icmpv6: an error report for an unregistered protocol is dropped | 3 (GPSR IPv6) | — | — |
      | icmp: no error report about a link-layer broadcast or multicast | the same 3 again | 3 (GPSR IPv6) | — |
      | tcp: the first round-trip measurement (RFC 6298) | 57 | 85 | 7 TCP |
      | quic: unknown frame, padded server Initial | 42 | 23 | — |
      | dhcp: seven defects | 6 | 6 | `DHCP_lifecycle_2`, `_3` |
      | dhcp: the address probe | the same 6 again | 5 | `DHCP_lifecycle_1`, `_2`, `_3` |

      No other source commit moves a value.
- [x] Every group is explained. The GPSR rows needed a run with the log: before the ICMPv6
      commit, `Ipv6` dropped 27 reports about protocol 0 (the Hop-by-Hop header of GPSR) with a
      warning, and `Icmpv6` now drops them itself, one event earlier. Before the ICMP commit the
      nodes sent 36 Destination Unreachable reports about datagrams in link-layer broadcast or
      multicast frames, and none is sent now. Every row of the RFC 6298 group runs TCP (BGP runs
      over TCP), every row of the QUIC group is a QUIC scenario, and every row of the DHCP groups
      runs a DHCP client.

      Facts that cost time:
      - `opp_repl` runs in the container with three additions in the driver
        (`ghci/update_stats.py` in the scratch folder): an explicit
        `import IPython.terminal.interactiveshell`, an explicit load of the bundled `omnetpp.opp`
        and `inet.opp`, and `run_unbounded=True`. Without the last one it skips the 21 QUIC
        configurations, which have no time limit and which INET's runner does run. The driver
        reproduced the stored QUIC values ("KEEP") at a commit before the QUIC change, and CI's
        runner accepted the files it wrote.
      - `inet_run_module_tests` ignores `--exclude-filter`; a negative lookahead in `-f` works.
      - A failing `%contains-regex` with many `.*` lines backtracks for 10 to 20 minutes.
        `opp_test` also removes trailing blanks from the text and reads `_defaults.ini`, which sets
        `cmdenv-log-prefix = ""`. A fast line-by-line matcher with the same meaning checked the new
        patterns.
      - `opp_test` stops at the first failing `%contains` block, so the files of later blocks are
        left over from the previous run.

### 4. Rewrite the series with its baselines — DONE 2026-09-17

- [x] For each commit that moves a recorded value, add to that commit:
  - the changed rows of `tests/fingerprint/*.csv`,
  - the new `%contains` blocks of the module tests that it changes,
  - the obligations in its `Change:` trailer,
  - a paragraph per row group: which behavior moved and why the new values are correct.
- [x] Remove every "no fingerprint moves" claim: the probe commit message, the plan commit
      message, `protocol-model-defects.md`, and the evidence.
- [x] Correct the base in `protocol-model-defects.md`: `b0c7a25e75`, not `d402df789c`.
- [x] Done in one scripted rebase onto `c3fbe79ad0` and one message rewrite. Every "Baselines:" line
      now states the measured result, and the suite counts after the fold are corrected. The
      new expected outputs match their own commit and fail on the commit before. The plan
      commits between the fold and the head keep the status text they had; they were written
      when the TCP checksum default still counted as passing.

### 5. Commit the statistics — DONE 2026-09-17

- [x] Create `topic/protocol-model-defects` in the `statistics` repository from its `origin/master`,
      in the worktree `ghci-statistical/inet/statistics`.
- [x] One commit for each INET commit that changes statistics, with the updated `.sca` files of that
      commit only. The message names the INET commit and explains each group, as in step 4.
      Five commits: icmp (3 files), tcp (85), quic (23), dhcp seven defects (6), dhcp probe (5).
- [x] CI's statistical suite passes at the repaired probe commit with this branch: 929 tests,
      917 PASS, 10 SKIP and 2 ERROR, all expected.

### 6. Verify the head — DONE 2026-09-17

- [x] Fingerprints, release and debug: all 1752 pass.
- [x] Module tests, debug: all 345 pass, in 49 s.
- [x] Protocol tests, debug: unchanged from 2026-09-16 (the two checksum-default failures).
- [x] Statistical tests, release, in the container with the topic branch of `statistics`: 917 PASS,
      10 SKIP and 2 ERROR, all expected, and no changed file.
- [x] Gates: classification, commits, includes, NED parameters, links, seals, source seals, and
      `check-series-builds.sh` (all 24 commits build in debug).
- [x] Each source commit passes its own changed fingerprint rows (from step 3, run again on the
      rewritten series), the module suite, and its own DHCP lifecycle patterns.

### 7. Hand over

- [x] Report the result and the row groups.
- [x] Land both branches, on request. The statistics commits went to master of the statistics
      repository first, so that CI finds them when the INET commits arrive.
- [x] Move the `ghci-statistical` worktrees back to their `origin` tips. The topic branch of
      `statistics` stays as a local branch of that repository.
- [x] Move this plan and `protocol-model-defects.md` to `plan/done/` when the branch lands.

## Not in this plan

- The two open protocol findings, `tcp/Rfc9293ChecksumDefault` and `udp/Rfc1122ChecksumDefault`.
- A synchronization of `tests/fingerprint/store.json` (decision 4).
- Fingerprints that `-f tyf` covers; CI does not check them.
