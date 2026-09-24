# Standards tests: the missing notes, and the expected outputs in the guide

The passes before RIP each wrote `model/<proto>/notes.md`: the model quirks, the scenario and
tooling traps, and the ordered follow-ups — what a person learned and would otherwise have to
learn again. The RIP, ND, IGMP and MLD passes wrote none, because the guide names the file
nowhere. Their lessons live in plan decision logs, in the "Other findings" of `results.md`, and
in private memory. This plan writes the four missing files and makes the guide name every
output that a pass must deliver.

## Steps

1. [x] **IGMP and MLD notes** on `topic/standards-tests-igmp-mld-level2`:
   `model/igmp/notes.md`, `model/mld/notes.md`, in the form of `model/dhcp/notes.md`; the
   tooling quirks that any protocol can meet go into `model/ipv4/notes.md#tooling-quirks` too.
2. [x] **The guide** on the same branch: a section on the outputs of a pass — each file, the
   step that writes it, and what it holds — with `notes.md`, the helper header of a suite, the
   plan and its decision log, and the scripts outside git; the table of the steps and the tree
   of "Where everything lives" name `notes.md`.
3. [ ] **RIP notes** on `topic/standards-tests-rip-level2`: `model/rip/notes.md`, from the plan
   decision log, `results.md` and the lessons of the pass.
4. [ ] **ND notes** on `topic/standards-tests-nd-level2`: `model/nd/notes.md`, the same way.
5. [ ] Gates on the three branches, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- The shared quirks go into `model/ipv4/notes.md` on this branch only, also the ones that the
  RIP and ND passes found (a timer check that passes by chance, the split of a check, the
  `require` sentence, the `::` of a test module class, the IPv6 configurator). The RIP and ND
  branches then add no line to `ipv4/notes.md`, and the three branches merge without a conflict.
  Their own notes link to `ipv4/notes.md#tooling-quirks` only, because the link gate checks
  anchors and the new headings do not exist on their branches.
- The guide got a section "What a pass delivers" after step 9: one table row per output, with
  the place, the step, the lowest level that needs it and the contents; three rules (what stays
  out of git, a commit per step with the gates, the plan is not the notes); and a subsection
  "What notes.md holds" with the sections of the file and two checkable rules: every gap of
  `results.md` appears in a follow-up by its number, and a later pass adds to the file.
- The ledger and part 1 of the conformance matrix are level 1 outputs: every level 1 protocol
  of wave 0 has `coverage.md` and `conformance.md`.
