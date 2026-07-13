# OMNeT++ ↔ INET topic/infrastructure coupling verdict (reconstructed)

Reconstructed from the Phase 0.1/0.2 agent's final report after the scratchpad
copy was lost. Source branches: omnetpp `topic/inet-infrastructure` @ dd8d2f278e
vs `origin/omnetpp-6.x` (13 ahead / 16 behind).

**Verdict: INET topic/infrastructure cannot safely link against plain omnetpp-6.x.**

## Hard requirements (build/ABI)

- `4f52bcee` — "sort before taking into account fingerprints of events having the
  same simulation time": adds two private data members (`simtime_t lastEventTime`,
  `std::vector<uint32_t> sameSimulationTimeHashes`) to `cSingleFingerprintCalculator`
  in `cfingerprint.h`. INET's `FingerprintCalculator` and
  `SelfDocumenterFingerprintCalculator` subclass it → header/lib layout mismatch =
  runtime crash. Also changes same-simtime fingerprint sorting semantics
  (global fingerprint-baseline impact — D6 question).
- `dc427ca5` — "changes required for topic/infrastructure branch in INET": adds
  `cSimulation::EventExecutor` typedef + member + `setEventExecutor()`, disables the
  re-entrancy guard in `executeEvent`; ABI change to `cSimulation`. Currently only
  needed to ACTIVATE the dormant verification machinery (D9): INET's
  `installCoroutineEventExecution()` has no callers and its body is commented out.

## Runtime/diagnostic only (not build requirements today)

- `3b8a3ea1` + `d68f116c` — ±0.1% randomization of `s`-unit time parameters
  (fingerprint-robustness testing aid; behavior change)
- `948f0e89` — disable parameter-impl sharing
- `b6f6bc3e` — mmap/guard-page allocation for cCoroutine stacks (SIGSEGV on overflow)
- `30d284b7` — `cEndSimulationEvent::dup()` fix
- `dd8d2f27` — "Temporary:" zero-delay intra-node-send detector in `arrived()`
- `63c0efd7` — eventlog fix for non-cMessage events
- `1314ecc2` (RNG logging), `53b31dee` (qtenv packet log a-->b), `7b48b304`
  (qtenv executed-events GOI), `6c2fb176` (unitconversion test comment-out)

## Build status (Phase 0.2, 2026-07-02)

- omnetpp companion branch: fresh (release re-linked qtenv/samples only)
- INET release: exit 0, 15 warnings (all pre-existing `-Wdeprecated-this-capture`)
- INET debug: exit 0, 16 warnings (same class)
