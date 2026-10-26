# Step I2: the coverage jobs in the GitHub-job container

The measurement runs the jobs of the planned workflow on one coverage build: the 4 fingerprint slices and the suites job (unit, module, packet, queueing, protocol) at the same time, each on its own 4 cores, like 5 GitHub runners. Each job has its own profile pool and exports its own lcov file. Then one report merges the 9 lcov files.

| Item | Value |
| --- | --- |
| INET | `c8afbf7a96`, with the packet Makefile repair of this branch |
| OMNeT++ | `omnetpp-6.x` at `29f713c321`, `WITH_OSG=yes WITH_OSGEARTH=no`, all INET features |
| opp_repl | `main` at `944790f`, mounted over the installed package |
| Container | [Containerfile](Containerfile): the image of the earlier plans (ubuntu 24.04, clang 16.0.6 as `clang`) plus `llvm-16`, `libclang-rt-16-dev` and `lcov` |
| Jobs | [measure.sh](measure.sh) with [crun.sh](crun.sh) and [job.sh](job.sh); `taskset` gives each job 4 cores, and [sitecustomize.py](sitecustomize.py) makes Python's `cpu_count()` report them, as on a runner |
| Commands | `opp_run_fingerprint_tests --load @opp -p inet -m coverage --no-build --split-count 4 --split-index <i>`; `opp_run_<kind>_tests --load @opp -p inet -m coverage`; then `opp_export_coverage --load @opp -p inet -t <suite>` |

## Build

The build of OMNeT++ and INET in coverage mode took 144 s on 32 cores with an empty ccache. The first attempt failed: the coverage link needs the profile runtime `libclang_rt.profile-x86_64.a`, which is in `libclang-rt-16-dev`. The workflows install with `--no-install-recommends`, so the workflow must install that package itself.

| Output | Size |
| --- | --- |
| `src/libINET_coverage.so` | 989 MB (debug: 915 MB) |
| `inet/out/clang-coverage` | 5.2 GB |
| `omnetpp/out/clang-coverage` | 426 MB |
| ccache | 873 MB |

## Times, pools and exports (run 1, 2026-10-26)

Times are on 4 cores of this machine. Its cores are faster than those of a GitHub runner, so the times show the relations, not the CI times.

| Job | Test time | Pool | Pool files | lcov | Export |
| --- | ---: | ---: | ---: | ---: | ---: |
| fingerprint 0 | 759 s | 131 MB | 28 | 11.6 MB | 6 s |
| fingerprint 1 | 589 s | 131 MB | 28 | 11.6 MB | 5 s |
| fingerprint 2 | 570 s | 131 MB | 28 | 11.6 MB | 4 s |
| fingerprint 3 | 585 s | 131 MB | 28 | 11.6 MB | 3 s |
| unit | 195 s | 135 MB | 130 | 11.2 MB | 6 s |
| module | 344 s | 137 MB | 120 | 11.5 MB | 7 s |
| packet | 2 s | 33 MB | 7 | 11.2 MB | 6 s |
| queueing | 32 s | 131 MB | 47 | 11.2 MB | 5 s |
| protocol | 417 s | 167 MB | 528 | 11.3 MB | 4 s |

The packet suite of run 1 did not link (`-lINET_dbg`); the table gives its time from the run after the repair. A pool holds at most 4 files for each instrumented binary: 7 binaries for a fingerprint slice, and one more for each test program of an opp_test kind. The pool size does not grow with the number of runs.

Coverage mode against debug mode: at the same load, fingerprint slice 0 took 998 s in coverage mode (run 2) and 1084 s in debug mode. The instrumentation costs about as much time as the difference between two runs; the CI time of a coverage job is therefore about the time of a debug job.

No fingerprint run reached the CPU time limit of 600 s: all runs passed, except the expected ERROR entries.

## Report

`opp_coverage_report --load @opp -p inet <9 lcov files> --html` took 54 s on 4 cores.

| Output | Size |
| --- | --- |
| `coverage.lcov` (merged) | 11.8 MB |
| `summary.json` | 573 KB |
| `history.json` (one history line) | 3.5 KB |
| `html/` of `genhtml` | 167 MB on disk, 139 MB of content |

| Metric | Covered | Total | Coverage |
| --- | ---: | ---: | ---: |
| lines | 138345 | 207142 | 66.79% |
| functions | 26420 | 39089 | 67.59% |
| branches | 59335 | 114124 | 51.99% |

`genhtml` gives the same line and branch values. It counts 124 fewer functions (38965); it probably counts template instances in a different way.

| Suite | Lines | Functions | Branches |
| --- | ---: | ---: | ---: |
| fingerprint | 59.26% | 60.54% | 43.70% |
| module | 41.57% | 42.83% | 31.85% |
| protocol | 24.27% | 31.66% | 18.71% |
| unit | 6.14% | 13.03% | 5.20% |
| queueing | 2.42% | 4.56% | 1.68% |
| packet | 1.20% | 3.52% | 0.88% |

| Area | Lines | Area | Lines |
| --- | ---: | --- | ---: |
| applications | 64.97% | physicallayer | 70.09% |
| clock | 69.77% | power | 75.39% |
| common | 57.96% | protocolelement | 77.61% |
| emulation | 0.00% | queueing | 73.15% |
| environment | 70.24% | routing | 71.55% |
| linklayer | 76.56% | transportlayer | 69.85% |
| mobility | 72.45% | visualizer | 3.26% |
| networklayer | 72.16% | networks, node | 100.00% |

`emulation` needs real network interfaces and `visualizer` mostly needs Qtenv, so CI cannot execute them.

## Verdicts

The suites ran in coverage mode and in debug mode in the same container with the same opp_repl runner. [verdicts.py](verdicts.py) compares the verdict of each test.

| Suite | Coverage mode | Debug mode | Different verdicts |
| --- | --- | --- | --- |
| fingerprint | 1840 tests: 1838 PASS, 2 expected ERROR | slice 0: 459 PASS, 1 expected ERROR, as in coverage mode | counts of slice 0 only |
| unit | 126 PASS, 2 FAIL | the same | 0 |
| module | 751 PASS, 5 FAIL, 3 ERROR | 752 PASS, 4 FAIL, 3 ERROR | 1 |
| protocol | 395 PASS, 306 SKIP, 47 expected FAIL, 76 FAIL | the same | 0 |
| queueing | 77 PASS | not run | — |
| packet | PASS | not run | — |

The only different verdict is `Ieee80211BlockAckVariant_1.test`: its check script starts `inet_dbg`, which a coverage build does not have. The other unexpected results are the same in debug mode, so coverage mode does not cause them: the 4 `ExternalProcess` tests need `ping` and `killall`, the 3 module errors need `gawk`, and none of these is in the image. The 2 unit failures (`EV_1`, `MathFunction_1`) and the 76 protocol failures also occur in debug mode with the opp_repl runner; steps S1 and S2 of the plan `tests-on-opp-repl` compare that runner with INET's own runner. The 306 SKIPs are the packetdrill wrappers without inet-gpl (decision D11).

## Noise

Run 2 repeated all jobs on the same build. [noise.py](noise.py) compares the covered sets of the two runs.

| Suite | Covered lines | Lines in one run only | Functions in one run only | Branches in one run only |
| --- | ---: | ---: | ---: | ---: |
| fingerprint | 122762 | 0 | 0 | 0 |
| unit | 12709 | 0 | 0 | 0 |
| module | 86105 | 0 | 0 | 0 |
| packet | 2485 | 0 | 0 | 0 |
| queueing | 5023 | 0 | 0 | 0 |
| protocol | 50265 | 0 | 0 | 0 |
| all | 138345 | 0 | 0 | 0 |

The covered sets are the same. The fingerprint lcov files of the two runs are also the same bytes; the module lcov files differ in some counts only. Run 2 was slower (fingerprint slices 828 to 998 s), because a debug slice ran at the same time.

## What the measurement does not show

- The times on a GitHub runner, whose cores are slower; step I3 measures them.
- The coverage of the packetdrill wrappers (decision D11).
- Whether a test checks the code that it executes.
