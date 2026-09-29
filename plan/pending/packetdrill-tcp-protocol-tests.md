# Run the packetdrill TCP corpus as INET protocol tests — the INET half

Status: **in progress.** Branch `topic/tcp-packetdrill-tests`, worktree
`/home/levy/workspace/inet-tcp-packetdrill-tests`, based on `topic/tcp-new-audit-fixes` (#1155).

**The design and every decision are in the `inet-gpl` plan**, branch `rh/packetdrill`,
`plan/pending/packetdrill-tcp-protocol-tests.md`. This file tracks the steps that land in INET, so
that the INET commits name a plan inside this repository.

## What INET receives

- `tests/protocol/tcp/packetdrill/run` — runs one corpus script through `inet-gpl`, or skips when
  `inet-gpl` is absent.
- One wrapper per script, `tests/protocol/tcp/packetdrill/<corpus>/<script_id>.test`: 306 files,
  generated in `inet-gpl`, each naming its script and matching one verdict line. No wrapper holds
  script text; the scripts are GPL-2.0 and stay in `inet-gpl`.
- `tests/protocol/tcp/packetdrill/README.md` — the scripts that have no wrapper, and why.
- CI, evidence and guide changes (steps 6 to 8).

## The INET steps

**Step 2 — the runner. — done 2026-09-29.** `run <corpus:script_id>` prints `#SKIPPED` when
`INETGPL_ROOT` is unset or holds no oracle, and otherwise hands the id to `oracle.py inet-one`. It
ignores the simulation arguments that `opp_repl` appends. Checked on three outcomes: a pass
(`gtests:fast_retransmit/fr-4pkt-sack`), a fail (`gtests:fast_retransmit/fr-4pkt-fack-last-byte`,
one of the seven baseline divergences) and a skip.

A wrapper reaches `run` by a **relative path**, not through `INET_ROOT`: `opp_repl` passes the
caller's environment on and sets a `<NAME>_ROOT` only for the projects that the tested project
uses, so `INET_ROOT` is not guaranteed. `opp_repl` always runs a test in `work/<test name>/`
beside its `.test` file, so the generator can compute the path.

**Step 3 — the wrappers** (generated in `inet-gpl`, committed here).

**Step 4 — the runner builds nothing for a wrapper** (`opp_repl`); verified here with
`inet_run_protocol_tests`.

**Step 6 — CI.** The *Test: protocol* job builds `inet-gpl` and fails if every wrapper skips.

**Steps 7 and 8 — the evidence and the guide.** `doc/project/evidence/` and the guide
`derive-tests-from-a-standard.md` do not exist at this branch's base, which predates them. These
two steps land after the rebase of #1155 onto master, or on a branch of their own from master.
