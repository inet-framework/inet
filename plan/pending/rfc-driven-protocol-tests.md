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
| 1 | Download the RFC and its error-signal companion | `tests/protocol/ipv4/rfc/rfc791.txt`, `rfc792.txt` |
| 2 | Extract checkable statements | `tests/protocol/ipv4/rfc/checklist.md` |
| 3 | Write the English check procedure with a mockup | `tests/protocol/ipv4/rfc/checks/*.md` |
| 4 | Write the protocol test with `tests/protocol/lib` | `tests/protocol/ipv4/Rfc791*.test` |
| 5 | Run the tests and analyze the simulation model | `tests/protocol/ipv4/rfc/results.md` |
| 6 | Decide the category of each check | `tests/protocol/ipv4/rfc/categories.md` |
| — | Describe the whole process | `tests/protocol/RFC-WORKFLOW.md` |

## Task list

- [ ] Step 1: cache rfc791.txt and rfc792.txt
- [ ] Step 2: checklist.md — catalog with IDs, quotes, class, strength, status
- [ ] Step 3: checks/ttl-decrement.md, checks/fragment-reassembly.md, checks/dont-fragment.md
- [ ] Step 4: Rfc791TtlDecrement.test, Rfc791FragmentReassembly.test, Rfc791DontFragment.test
- [ ] Step 5: run the ipv4 suite; write results.md
- [ ] Step 6: categories.md; fill the ledger in checklist.md
- [ ] RFC-WORKFLOW.md
- [ ] Move this plan to plan/done/

## Decision log

- 2026-09-02: The user requires the specification-first order (see Principle above). The
  first draft tied catalog entries to code locations; that draft was discarded.
- 2026-09-02: The three examples share one mockup: source host A — router R — destination
  host B, with a small MTU on the R–B link. One scenario serves all three checks.
