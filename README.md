[![Build: Linux](https://github.com/inet-framework/inet/actions/workflows/build-linux.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/build-linux.yml)
[![Build: Mac OS](https://github.com/inet-framework/inet/actions/workflows/build-macos.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/build-macos.yml)
[![Build: Windows](https://github.com/inet-framework/inet/actions/workflows/build-windows.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/build-windows.yml)

[![Test: chart](https://github.com/inet-framework/inet/actions/workflows/chart-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/chart-tests.yml)
[![Test: coverage](https://github.com/inet-framework/inet/actions/workflows/coverage-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/coverage-tests.yml)
[![Test: feature](https://github.com/inet-framework/inet/actions/workflows/feature-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/feature-tests.yml)
[![Test: fingerprint](https://github.com/inet-framework/inet/actions/workflows/fingerprint-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/fingerprint-tests.yml)
[![Test: module](https://github.com/inet-framework/inet/actions/workflows/module-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/module-tests.yml)
[![Test: packet](https://github.com/inet-framework/inet/actions/workflows/packet-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/packet-tests.yml)
[![Test: protocol](https://github.com/inet-framework/inet/actions/workflows/protocol-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/protocol-tests.yml)
[![Test: queueing](https://github.com/inet-framework/inet/actions/workflows/queueing-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/queueing-tests.yml)
[![Test: speed](https://img.shields.io/badge/Test%3A%20speed-disabled-lightgrey?logo=github)](https://github.com/inet-framework/inet/blob/master/.github/workflows/speed-tests.yml.disabled)
[![Test: statistical](https://github.com/inet-framework/inet/actions/workflows/statistical-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/statistical-tests.yml)
[![Test: unit](https://github.com/inet-framework/inet/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/unit-tests.yml)
[![Test: validation](https://github.com/inet-framework/inet/actions/workflows/validation-tests.yml/badge.svg)](https://github.com/inet-framework/inet/actions/workflows/validation-tests.yml)

<!-- A badge above shows the result of the last completed run of its workflow. The collapsed table shows the state of each job on the current master head: queued, pending, passing or failing. The label of a small badge is the matrix part of the job name. A small badge finds its job by the exact name, so a change of a job name or of a matrix needs the same change here. The three build workflows all have the jobs main (debug) and main (release), so on a head with a weekly build, the small build badges also show the macOS and Windows jobs. The weekly workflows do not run on each head, so they have no row. The speed test needs CPU performance counters, which GitHub runners do not have. -->

<details>
<summary>State of each job on the master head</summary>

| Workflow | Jobs |
| --- | --- |
| [Build: Linux][build-linux-runs] | [![Build: Linux: job main (debug) on the master head][build-linux-head-debug]][build-linux-runs][![Build: Linux: job main (release) on the master head][build-linux-head-release]][build-linux-runs] |
| [Test: chart][chart-tests-runs] | [![Test: chart: job chart-tests (release) on the master head][chart-tests-head-release]][chart-tests-runs] |
| [Test: fingerprint][fingerprint-tests-runs] | [![Test: fingerprint: job fingerprint-tests (debug, 0) on the master head][fingerprint-tests-head-debug-0]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (debug, 1) on the master head][fingerprint-tests-head-debug-1]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (debug, 2) on the master head][fingerprint-tests-head-debug-2]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (debug, 3) on the master head][fingerprint-tests-head-debug-3]][fingerprint-tests-runs]<br>[![Test: fingerprint: job fingerprint-tests (release, 0) on the master head][fingerprint-tests-head-release-0]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (release, 1) on the master head][fingerprint-tests-head-release-1]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (release, 2) on the master head][fingerprint-tests-head-release-2]][fingerprint-tests-runs][![Test: fingerprint: job fingerprint-tests (release, 3) on the master head][fingerprint-tests-head-release-3]][fingerprint-tests-runs] |
| [Test: module][module-tests-runs] | [![Test: module: job module-tests (debug) on the master head][module-tests-head-debug]][module-tests-runs] |
| [Test: packet][packet-tests-runs] | [![Test: packet: job packet-tests (debug) on the master head][packet-tests-head-debug]][packet-tests-runs] |
| [Test: protocol][protocol-tests-runs] | [![Test: protocol: job protocol-tests (debug) on the master head][protocol-tests-head-debug]][protocol-tests-runs] |
| [Test: queueing][queueing-tests-runs] | [![Test: queueing: job queueing-tests (debug) on the master head][queueing-tests-head-debug]][queueing-tests-runs] |
| [Test: statistical][statistical-tests-runs] | [![Test: statistical: job statistical-tests (release) on the master head][statistical-tests-head-release]][statistical-tests-runs] |
| [Test: unit][unit-tests-runs] | [![Test: unit: job unit-tests (debug) on the master head][unit-tests-head-debug]][unit-tests-runs] |
| [Test: validation][validation-tests-runs] | [![Test: validation: job validation-tests (release) on the master head][validation-tests-head-release]][validation-tests-runs] |

</details>

[build-linux-runs]: https://github.com/inet-framework/inet/actions/workflows/build-linux.yml?query=branch%3Amaster
[build-linux-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=main%20%28debug%29&label=debug
[build-linux-head-release]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=main%20%28release%29&label=release
[chart-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/chart-tests.yml?query=branch%3Amaster
[chart-tests-head-release]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=chart-tests%20%28release%29&label=release
[fingerprint-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/fingerprint-tests.yml?query=branch%3Amaster
[fingerprint-tests-head-debug-0]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28debug%2C%200%29&label=debug%200
[fingerprint-tests-head-debug-1]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28debug%2C%201%29&label=debug%201
[fingerprint-tests-head-debug-2]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28debug%2C%202%29&label=debug%202
[fingerprint-tests-head-debug-3]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28debug%2C%203%29&label=debug%203
[fingerprint-tests-head-release-0]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28release%2C%200%29&label=release%200
[fingerprint-tests-head-release-1]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28release%2C%201%29&label=release%201
[fingerprint-tests-head-release-2]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28release%2C%202%29&label=release%202
[fingerprint-tests-head-release-3]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=fingerprint-tests%20%28release%2C%203%29&label=release%203
[module-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/module-tests.yml?query=branch%3Amaster
[module-tests-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=module-tests%20%28debug%29&label=debug
[packet-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/packet-tests.yml?query=branch%3Amaster
[packet-tests-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=packet-tests%20%28debug%29&label=debug
[protocol-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/protocol-tests.yml?query=branch%3Amaster
[protocol-tests-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=protocol-tests%20%28debug%29&label=debug
[queueing-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/queueing-tests.yml?query=branch%3Amaster
[queueing-tests-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=queueing-tests%20%28debug%29&label=debug
[statistical-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/statistical-tests.yml?query=branch%3Amaster
[statistical-tests-head-release]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=statistical-tests%20%28release%29&label=release
[unit-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/unit-tests.yml?query=branch%3Amaster
[unit-tests-head-debug]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=unit-tests%20%28debug%29&label=debug
[validation-tests-runs]: https://github.com/inet-framework/inet/actions/workflows/validation-tests.yml?query=branch%3Amaster
[validation-tests-head-release]: https://img.shields.io/github/check-runs/inet-framework/inet/master?nameFilter=validation-tests%20%28release%29&label=release

INET Framework for OMNEST/OMNeT++
=================================

The [INET framework](https://inet.omnetpp.org) is an open-source communication networks
simulation package, written for the [OMNEST/OMNeT++](https://omnetpp.org) simulation system.
The INET framework contains models for numerous wired and wireless protocols, a detailed
physical layer model, application models and more. See the CREDITS file for the
names of people who have contributed to the INET Framework.

IMPORTANT: The INET Framework is continuously being improved: new parts
are added, bugs are corrected, and so on. We cannot assert that any protocol
implemented here will work fully according to the specifications. YOU ARE
RESPONSIBLE YOURSELF FOR MAKING SURE THAT THE MODELS YOU USE IN YOUR SIMULATIONS
WORK CORRECTLY, AND YOU'RE GETTING VALID RESULTS.

Contributions are highly welcome. You can make a difference!

See the WHATSNEW file for recent changes, and the doc/whatsnew folder for the
changes of the version in development.


GETTING STARTED
---------------
You may start by downloading and installing the INET framework. Read the INSTALL
file for further information.

Then you can gather initial experience by starting some examples or following a
tutorial or showcase (see the /examples, /showcases or /tutorials folder).
After that, you can learn the NED language from the OMNeT++ manual & sample
simulations.

After that, you may write your own topologies using the NED language. You may
assign some of the submodule parameters in NED files. You may leave some of
them unassigned.

Then, you may assign unassigned module parameters in omnetpp.ini of your
simulation. (You can refer to sample simulations & manual for the content of
omnetpp.ini)

Finally, you will be ready to run your simulation. As you see, you may use
the INET framework without writing any C++ code, as long as you use the
available modules.

To implement new protocols or modify existing ones, you'll need to add your
code somewhere under the src directory. If you add new files under the 'src/inet'
directory you will need to regenerate the makefiles (using the 'make makefiles'
command).

If you want to use external interfaces in INET, enable the "Emulation" feature
either in the IDE or using the inet_featuretool then regenerate the INET makefile
using 'make makefiles'.
