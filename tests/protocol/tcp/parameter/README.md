# The parameter tests of TCP

A parameter test checks that one NED parameter of one TCP module operates as its NED comment
says, for one class of values. A class is a set of values that take the same code path and
give the same documented behavior: `Tcp.delayedAckFrameCount` has the classes `one` (1),
`many` (2 and more) and `invalid` (0 and less). The catalog
[`parameters.md`](../../../../doc/project/evidence/model/tcp/parameters.md) lists the classes
of each parameter, the test of each class, its verdict, the findings, and the coverage. The
procedure, with its principles and steps, is
[`derive-tests-from-parameters.md`](../../../../doc/project/guide/derive-tests-from-parameters.md);
this file gives what is special to this folder.

## The groups

The 339 tests cover the 116 parameters of the modules of `transportlayer/tcp` and of
`TcpSpoof`. Each folder is a group of parameters, in the order of the NED comment of `Tcp`:

| Folder | Parameters | Tests |
| --- | --- | ---: |
| `ack/` | acknowledgments | 14 |
| `setup/` | connection setup and header options | 37 |
| `send/` | sending data | 18 |
| `rto/` | retransmission timeout | 13 |
| `congestion/` | congestion control and the algorithm modules | 82 |
| `recovery/` | loss detection and recovery | 26 |
| `receive/` | receive window and buffer | 26 |
| `close/` | connection close, the checks of arriving segments, ICMP errors | 15 |
| `ecn/` | ECN and AccECN | 20 |
| `pmtu/` | path MTU | 15 |
| `keepalive/` | keepalive | 13 |
| `fastopen/` | TCP Fast Open | 34 |
| `module/` | the checksum and the stop of the module `Tcp` | 11 |
| `spoof/` | `TcpSpoof` | 15 |

## The names of the classes

The scripts read some parts of the name of a class:

- `invalid`, or another name for a value that stops the run: an error test, which is no
  alternative in the cross-class check.
- `changed`: an action changes a `@mutable` parameter during the run.
- `<class>-<part>`, a facet: one part of the behavior of a class, with the same value as the
  class, for example `on-ipv6` or `dctcp-reduction`.
- `<name>-override`: a socket option takes the place of the parameter, so the test passes with
  each value of the parameter by design, and the cross-class check skips it.
- `<name>-alias`: another spelling of the value of another class; the cross-class check skips
  it too.

A failing test stays: its row in the catalog says FAIL and names its finding (`PF-n`). The
work that wrote these tests changed nothing in `src/`; a later plan repairs the findings.

## The form of a test file

The file name is `<Module><Parameter><Class>.test` in PascalCase, unique in the whole TCP
suite: `TcpDelayedAckFrameCountOne.test`. A parameter that starts with the module name does not
repeat it: `TcpAlgorithmClassVegas.test`. A facet adds its part: `TcpPmtudEnabledOnIpv6.test`.

The `%#` lines say what the test is about; opp_test ignores them, and the two checks read them:

```
%# parameter: Tcp.delayedAckFrameCount
%# class: one
%# value: 1
%# key: *.host2.tcp.delayedAckFrameCount
%# expected-result: PASS
```

The ini section sets the key to the value, then the lines of the scenario, and it includes
the shared files last, because in an ini file the first line that matches a key wins:

```ini
[General]
*.host2.tcp.delayedAckFrameCount = 1    # the parameter under test
include ../../parameter/ini/_bulk.ini
include ../../parameter/ini/_base.ini
```

[`ini/_base.ini`](ini/_base.ini) selects the shared network `TapPairNetwork` and `Tcp`;
[`ini/_bulk.ini`](ini/_bulk.ini) is the scenario in which host1 opens a connection to host2 at
0.1 s and sends 10000 bytes at 0.2 s. A test of an invalid value has no program: it checks the
exit code and the error message.

## The shared networks and helpers

The networks are in [`../../lib`](../../lib); the letter is the one of the column `Net` of the
catalog.

| Letter | Network | Settings of the test |
| --- | --- | --- |
| P | `TapPairNetwork`: host1, tap, host2 | — (the default of `_base.ini`) |
| P6 | the same, with IPv6 hosts | `StandardHost6`, `Ipv6FlatNetworkConfigurator`, an open at 3 s after the address configuration |
| R | `TapRouterNetwork`: host1, tap, router, host2 | `network = inet.protocoltest.TapRouterNetwork`; a bottleneck by `bottleneckDatarate` and `bottleneckDelay` |
| R6 | the same, with IPv6 nodes | `StandardHost6`, `Router6`, `Ipv6NetworkConfigurator` and `**.ipv6.configurator.networkConfiguratorModule = "configurator"`: the flat configurator gives the hosts no route through the router |
| F | `TapSwitchNetwork`: more hosts behind a switch | `numPeers` |

[`../TcpMutations.h`](../TcpMutations.h) holds the helpers of the tap rules and of the lambdas:
the checksum of RFC 9293, the TCP options, the Fast Open cookie, and the fields of a frame.
`TcpTestApp` of the library makes the socket calls, and the action word `stopNode` stops a node
with the lifecycle operation of INET.

## Run the tests and the checks

From the INET root, in an environment of INET:

```sh
opp_run_protocol_tests -m debug -w tcp -f parameter/ack     # one group
opp_run_protocol_tests -m debug -w tcp -f parameter/        # all parameter tests
python3 tests/protocol/tcp/parameter/check_catalog.py       # catalog, NED files and tests agree
python3 tests/protocol/tcp/parameter/cross_class.py ack     # each test fails with the other values
```

`check_catalog.py --pending` also lists the parameters without a class yet, and `--strict`
makes them errors. `cross_class.py` reruns the binaries of the last test run, so run the tests
first. opp_test writes the run script of a test only when the test passes, so the check reports
a failing test as `NOT BUILT`; run it by hand with the run script of another test.
