# Run the gates

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [enforcement/README.md](../enforcement/README.md)

What to run before you push, in order, and what each one covers. The full inventory and the tier
ladder are [enforcement/README.md](../enforcement/README.md).

Run everything from the repository root.

## Before every push

```bash
# 1. does it build, in both modes
make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release

# 2. the rules a script can check
doc/project/enforcement/check-architecture.sh              # dependency direction
doc/project/enforcement/check-naming.sh                    # file, package and asset names
doc/project/enforcement/check-commits.sh origin/master..HEAD   # the commit series

# 3. the tests that match the claim  (rule/testing.md, TR-CAT-MATCH)
cd tests/<category> && ./runtest
```

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

Every check takes a path, and a scoped run is the one worth doing often:

```bash
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ethernet
```

## What none of them cover

The gates cover dependency direction, C++ identifiers, file names and the commit series. They do not
cover the **semantic** rules: visualization logic inside a protocol, a zero-time message standing in
for a call, prose that duplicates a NED declaration, a test whose category does not match its claim.
Those are tier 4, and they need the [agent-review checklist](../enforcement/checklist/general.md).

**Nothing in `enforcement/` runs in CI yet.** `.github/workflows/` holds twelve test workflows and no
rule gate, so every check above is one a person must remember to run. That is the weakest form of
every rule it covers, and it is the first thing worth repairing.
