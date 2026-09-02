# RFC-driven protocol tests — workflow bring-up (IPv4 examples)

Goal: define a repeatable workflow that turns an RFC into protocol tests, and show it on
RFC 791 (IPv4) with three complete examples. The workflow document is the final deliverable.

## Principle: specification first

The catalog (step 2) and the English check descriptions (step 3) come from the RFC text only.
They contain no INET module names, no parameters, and no code references. A look at the code
is permitted only to select practical candidates. INET names first appear in the `.test` file
(step 4). Code analysis first appears in the run-analysis document (step 5).

## Steps and artifacts

| Step | Action | Artifact |
| --- | --- | --- |
| 1 | Download the RFC and its error-signal companion | `doc/project/evidence/rfc/ipv4/rfc791.txt`, `rfc792.txt` |
| 2 | Extract checkable statements | `doc/project/evidence/rfc/ipv4/rfc791-checklist.md` |
| 3 | Write the English check procedure with a mockup | `doc/project/evidence/rfc/ipv4/rfc791-checks.md` |
| 4 | Write the protocol test with `tests/protocol/lib` | `tests/protocol/ipv4/Rfc791*.test` |
| 5 | Run the tests and analyze the simulation model | `doc/project/evidence/rfc/ipv4/rfc791-results.md` |
| 6 | Decide the category of each check | `doc/project/evidence/rfc/ipv4/rfc791-categories.md` |
| — | Describe the whole process | `doc/project/guide/derive-tests-from-an-rfc.md` |

## Task list

- [x] Step 1: cache rfc791.txt and rfc792.txt
- [x] Step 2: rfc791-checklist.md — catalog with IDs, quotes, class, strength, status
- [x] Step 3: rfc791-checks.md — three sections: TTL decrement, Fragment and reassembly, Don't fragment
- [x] Step 4: Rfc791TtlDecrement.test, Rfc791FragmentReassembly.test, Rfc791DontFragment.test
- [x] Step 5: run the ipv4 suite; write rfc791-results.md — 4 of 4 tests PASS
- [x] Step 6: rfc791-categories.md; the ledger in rfc791-checklist.md names the tests
- [x] RFC-WORKFLOW.md
- [x] Move this plan to plan/done/

## Decision log

- 2026-09-02: The user requires the specification-first order (see Principle above). The
  first draft tied catalog entries to code locations; that draft was discarded.
- 2026-09-02: The three examples share one mockup: source host A — router R — destination
  host B, with a small MTU on the R–B link. One scenario serves all three checks.
- 2026-09-02: Both authoring failures were test errors, not model gaps: the MTU parameter
  lives on the MAC module (`eth[1].mac.mtu`), unit-bearing fields need unit literals, and
  the ICMP dissector name is `icmpv4`. Details in doc/project/evidence/rfc/ipv4/rfc791-results.md.
- 2026-09-02: Final state: 4 of 4 ipv4 protocol tests PASS; the INET model conforms to all
  selected RFC 791/792 statements in this pass.
- 2026-09-02: Naming revision on user request: identifiers use the RFC791- prefix (was
  R791-), the per-RFC artifacts carry the RFC number in the file name (rfc791-checklist.md,
  rfc791-checks.md, rfc791-results.md, rfc791-categories.md), and all English procedures
  live in one file with one section per check.
- 2026-09-02: Relocation on user request: the document artifacts moved to
  doc/project/evidence/rfc/ipv4/, and the workflow description became
  doc/project/guide/derive-tests-from-an-rfc.md. Each document received the doc/project
  header line, the checklist follows DR-INDEX and DR-ID-HEADING, and the README map lists
  both entries. The tests stay in tests/protocol/ipv4/.
- 2026-09-02: Branch move on user request: the whole work now lives on the branch
  `topic/rfc-tests` in the worktree `/home/levy/workspace/inet-rfc-tests`; the master
  branch in inet-master carries none of these commits.
