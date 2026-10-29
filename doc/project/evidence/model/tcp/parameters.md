# TCP — the catalog of the parameter tests

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [Tcp.ned](../../../../../src/inet/transportlayer/tcp/Tcp.ned), [the plan](../../../../../plan/pending/tcp-parameter-tests.md)

The classes of values of each NED parameter of the TCP modules, and the parameter test of each class. A class is a set of values that take the same code path and give the same documented behavior. The tests are in [`tests/protocol/tcp/parameter/`](../../../../../tests/protocol/tcp/parameter); each test sets its parameter to the value of its row, and the cross-class check makes sure that it fails with the value of each other row of its parameter.

This document is a ledger: each group of parameters writes its rows when its tests are written, and the verdict column changes with each run. [`check_catalog.py`](../../../../../tests/protocol/tcp/parameter/check_catalog.py) compares the rows with the NED files and the test files, and lists the parameters that have no rows yet.

## How to read a row

- **Class**: `<Module>.<parameter>:<class>`. A class with more than one test has one row for each facet: `linux-quickack`, `linux-ato`, `linux-pingpong` are three facets of the value `true`.
- **Value**: the NED expression text that the test sets, the representative value of the class.
- **Preset**: `LinuxTcp` or `LegacyTcp` when that module sets its default to a value of this class; — when no preset does. A preset has no tests of its own: each of its defaults falls into one class.
- **Claim**: the behavior that separates the class from the other classes of the parameter.
- **Source**: where the class comes from: "NED" (the comment of the parameter), "code" (a condition in the code, with the function), or both.
- **Obs**: how the test observes the class: W the wire, A the application, S a signal, I the internal state (`assertModule`), E the error at the open.
- **Net**: P `TapPairNetwork`, R `TapRouterNetwork`, F `TapSwitchNetwork`; T a tap rule, K a socket call of `TcpTestApp`, M a change of a parameter during the run.
- **Verdict**: the result of the last run: PASS, FAIL, or FAIL (expected) with its reason in the findings.

## Findings

Defects and gaps that the parameter tests found. This pass changes nothing in `src/`; a later plan repairs them.

| Id | Finding | Where | Found by |
| --- | --- | --- | --- |

## The classes

No group has rows yet; each group adds its section here.
