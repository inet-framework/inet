# Repair the regressions of the model-defects branch and record its baselines

**Status:** in progress since 2026-09-17. Steps 1 and 2 are done. Branch `topic/protocol-model-defects`, worktree
`/home/levy/workspace/inet-protocol-model-defects`, 23 commits on master `b0c7a25e75`.
It continues [protocol-model-defects.md](protocol-model-defects.md).

## Why this plan exists

On 2026-09-17 the three recorded suites ran on the branch head and on `inet-master` at the same
base, with all features on (`opp_featuretool enable all`, as CI does). `inet-master` passes all of
them. The branch does not:

| Suite | Mode | Branch head | Base |
| --- | --- | --- | --- |
| fingerprint (`tests/fingerprint`, `-f tplx -f ~tNl -f ~tND`) | release | **108 of 1752 changed** | 1752 as expected (OK) |
| module (`inet_run_module_tests`) | debug | 334 PASS, **11 FAIL** | 345 PASS |
| statistical (`statistics` repository) | release | not run yet | — |

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

### 3. Measure every source commit

- [ ] Run the three suites at the new head to get the changed sets: fingerprints (release, all
      rows), module tests (debug, all), statistics (release, container, all). Record each set.
- [ ] Run the statistical suite in the container at the base as a control. Every configuration
      must pass, or its result is noted as outside this branch.
- [ ] For each source commit, in order, in a detached checkout: build release and debug, run the
      changed fingerprint rows, the changed module tests, and `update_statistical_test_results`
      for the changed configurations in the container. Save the outputs by commit subject.
- [ ] Explain each row group from the diff of its commit. A group without an explanation stops
      the plan: it is a regression, and it is repaired before any baseline is written.

### 4. Rewrite the series with its baselines

- [ ] For each commit that moves a recorded value, add to that commit:
  - the changed rows of `tests/fingerprint/*.csv`,
  - the new `%contains` blocks of the module tests that it changes,
  - the obligations in its `Change:` trailer,
  - a paragraph per row group: which behavior moved and why the new values are correct.
- [ ] Remove every "no fingerprint moves" claim: the probe commit message, the plan commit
      message, `protocol-model-defects.md`, and the evidence.
- [ ] Correct the base in `protocol-model-defects.md`: `b0c7a25e75`, not `d402df789c`.

### 5. Commit the statistics

- [ ] Create `topic/protocol-model-defects` in the `statistics` repository from its `origin/master`,
      in the worktree `ghci-statistical/inet/statistics`.
- [ ] One commit for each INET commit that changes statistics, with the updated `.sca` files of that
      commit only. The message names the INET commit and explains each group, as in step 4.

### 6. Verify the head

- [ ] Fingerprints, release and debug: all 1752 pass.
- [ ] Module tests, debug: all 345 pass.
- [ ] Protocol tests, debug: unchanged from 2026-09-16 (two expected checksum-default failures).
- [ ] Statistical tests, release, in the container with the topic branch of `statistics`: all pass.
- [ ] Gates: classification, commits, includes, NED parameters, links, seals, source seals, and
      `check-series-builds.sh`.
- [ ] Each source commit passes its own changed fingerprint rows (from step 3, run again on the
      rewritten series).

### 7. Hand over

- [ ] Report the result and the row groups.
- [ ] Ask before pushing: the INET branch needs `--force-with-lease`, and the `statistics` branch is
      new.
- [ ] Move the `ghci-statistical` worktrees back to their `origin` tips.
- [ ] Move this plan and `protocol-model-defects.md` to `plan/done/` when the branch lands.

## Not in this plan

- The two open protocol findings, `tcp/Rfc9293ChecksumDefault` and `udp/Rfc1122ChecksumDefault`.
- A synchronization of `tests/fingerprint/store.json` (decision 4).
- Fingerprints that `-f tyf` covers; CI does not check them.
