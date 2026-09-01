# Run the gates

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [enforcement/README.md](../enforcement/README.md), [testing.md](../rule/testing.md)

What to run during focused development and before a push, in order, and what each result must say.
The full inventory and the tier ladder are
[enforcement/README.md](../enforcement/README.md). Run everything from the repository root unless a
test command explicitly changes directory.

## Keep the tested library current

After compiled INET source or generated-code inputs change, build the INET library in the mode the
test will load before running the test. This prevents a green result from describing an older
`libINET_dbg` or `libINET` than the changed source tree:

```bash
make -j$(nproc) MODE=debug
# Before a release-mode test:
make -j$(nproc) MODE=release
```

Debug mode is the primary mode for local behavioral validation because it retains runtime
assertions. Release mode remains a required compilation check before a contributor pushes; run
behavioral tests in release mode as well when the changed contract or defect is mode-sensitive.
A test-definition, script, configuration or documentation-only change does not require rebuilding
the INET library unless it also changes compiled support inputs. Always record the mode and any
required build status with the test result.

## During focused development

Map each changed path, symbol or behavioral contract to the directly related cases in the category
required by [TR-CAT-MATCH](../rule/testing.md#tr-cat-match). Run those cases through an explicit
case, tag or filter; for example:

```bash
doc/project/enforcement/check-architecture.sh src/inet/<affected-subtree>
( cd tests/fingerprint && ./fingerprinttest -m '<affected-case-or-tag-regex>' )
# Use the category runner's explicit selector for unit, module, protocol and other tests.
```

Keep the working directory, exact build and test commands, configuration, run and seed where
applicable, filter, exit status and artifact paths. An unfiltered category or repository-wide suite
may add integration coverage, but it cannot replace a directly related case. If no such case exists,
report the coverage gap instead of broadening the command until something green appears
([TR-FOCUSED-EVIDENCE](../rule/testing.md#tr-focused-evidence)).

## Before every push

The final pass keeps the focused evidence and adds the project-wide compilation and mechanical
gates. It does not turn every local validation into an unfiltered test campaign.

```bash
# 1. compile the source in both supported modes
make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release

# 2. run the rules a script can check
doc/project/enforcement/check-architecture.sh
doc/project/enforcement/check-naming.sh --base origin/master
doc/project/enforcement/check-commits.sh origin/master..HEAD
doc/project/enforcement/check-source-seals.sh --base origin/master

# 3. rerun each recorded, explicitly filtered test command against a fresh matching library
```

CI, integration and release campaigns may run broader suites and build matrices. Their result is
additional evidence; it does not erase the focused change-to-test mapping.

## When the change touches this document set

```bash
doc/project/enforcement/check-seals.sh          # the seal flags and the generated index
doc/project/enforcement/check-seals.sh --write  # ... and rewrite the index
```

## When the change is large or touches C++ broadly

```bash
doc/project/enforcement/check-cpp.sh src/inet/<subtree>
```

It needs a compilation database; build first, or set `INET_COMPILE_DB`.

## Scope a gate to your subtree

A scoped run is the one worth doing often while developing:

```bash
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ethernet
doc/project/enforcement/check-naming.sh src/inet/linklayer/ethernet
```

The naming command scans declaration names in every NED and MSG file under the subtree. The scoped
run is not the final project-wide architecture and naming pass before a push.

## What none of them cover

The gates cover dependency direction, C++ identifiers, file names and the commit series. They do not
cover the **semantic** rules: visualization logic inside a protocol, a zero-time message standing in
for a call, prose that duplicates a NED declaration, a test whose category does not match its claim.
Those are tier 4, and they need the [agent-review checklist](../enforcement/checklist/general.md).

The pull-request `enforcement-tests.yml` workflow runs the enforcement checker tests.
`check-sealing.yml` evaluates paths sealed at the branch merge base and, when necessary, waits for
the protected approval required by [SR-PR-APPROVAL](../rule/sealing.md#sr-pr-approval). The remaining
rule gates above still depend on contributors running them by hand, and are the next CI coverage gap
to close.
