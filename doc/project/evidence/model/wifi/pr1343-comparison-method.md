# Parent-to-commit protocol comparisons

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test rules](../../../rule/testing.md), [protocol authoring](../../../../../tests/protocol/lib/AUTHORING.md)

Each comparison record checks one implementation commit against its first parent. The after revision is the commit that adds the selected protocol fixture. The before revision is that commit's parent. The same fixture inputs and successful assertions apply to both revisions.

The parent lacks the new fixture. The comparison therefore copies the fixture and its listed test components into the parent worktree. It copies no production file. A parent failure must reach the intended protocol assertion. A compilation failure, initialization error, or unavailable observation point does not establish the stated repair.

## Resolve the two revisions

Each record names its fixture and implementation subject. Use the fixture path to locate the fix in the current history:

```sh
fixture_path=tests/protocol/wifi/common/WifiBlockAckDuplicateAddba.test
fix_commit="$(git log --diff-filter=A --format=%H -- "$fixture_path")"
before_commit="$(git rev-parse "$fix_commit^1")"
```

Replace `fixture_path` with the path from the selected record. The current series adds each fixture once. The lifecycle commit adds both the BAR and inactivity fixtures; either path selects that same commit.

## Prepare identical test inputs

1. Create separate worktrees at `before_commit` and `fix_commit`.
2. Copy the record's listed fixture inputs from `fix_commit` into the parent worktree.
3. Preserve the parent production source and its existing test framework.
4. Source `.envrc` from each worktree root.
5. Run `make makefiles` in each worktree.
6. Run `make MODE=debug -j12` in each worktree.
7. Run the record's filtered protocol command in each worktree.

The after worktree supplies the listed paths without another copy. Original topology, PHY configuration, and protocol framework files remain part of each revision. The before and after builds must match their respective production inputs. The protocol runner builds test executables; it does not replace the INET library build.

## Read the result

Each fixture retains its ordinary PASS expectation. A failed protocol assertion on the parent remains FAIL. A control can pass while another run in the same file fails. The report therefore separates test files from simulation runs and records each control.

All recorded protocol runs use debug mode, configuration `General`, and `seed-set = 0`. Each fixture supplies its run selection and observation windows. The fixtures use the INET Basic compatibility profile. Their guides explain the stimulus and standards limits.

## Identify the executed inputs

Each record pins the before and after source revisions used by the recorded executions. Those hashes identify the actual runs. A later rewrite of documentation can change commit IDs. The rewrite must preserve both compared production inputs and all fixture inputs before it reuses the recorded result.

The production tree hashes identify the tracked `src/` contents independently of commit metadata. Check the current pair with this command:

```sh
git rev-parse "$before_commit:src" "$fix_commit:src"
```

Both values must equal the record's production tree hashes. Fixture identity requires a separate check of every listed input. A source tree match alone does not establish fixture identity or execute a test.

The four pairs were executed during the protocol fold. Local evidence resides under `/tmp/pr1343-fold-20261020/`. The parent output directories contain the failed assertions and controls that pass. The after logs contain the successful fixture verdicts at the selected fix. `verification.json` maps those logs to the executed revisions. These local artifacts are outside Git.

This record revision reuses those executions. It does not claim another simulation campaign. Per-commit source and fixture comparisons establish that the documentation rewrite preserves the recorded inputs. The published fixtures and commands permit another execution.

## Standards corpus

The guides preserve the standard references for their own scenarios. The original retrieval used a rebuilt corpus. Its lint output reports 14 heading ambiguities elsewhere in the document, plus other extraction diagnostics. The cited entries resolved to distinct canonical nodes. Table 9-36 also received direct PDF inspection to confirm its reserved value and table alignment.

The checked outgoing references for 11.5.4, 10.3.2.14.3, 11.3.5.5, Table 9-466, and Table 9-467 all resolve. The receiver-cache table and reassociation cross-reference resolve. These checks do not establish a clean verdict for the complete corpus. Local retrieval records and lint output reside under `audit/pr1343-compliant-scenarios-20261020/` outside Git.
