# The parameter tests of TCP

A parameter test checks that one NED parameter of one TCP module operates as its NED comment
says, for one class of values. A class is a set of values that take the same code path and
give the same documented behavior: `Tcp.delayedAckFrameCount` has the classes `one` (1),
`many` (2 and more) and `invalid` (0 and less). The catalog
[`parameters.md`](../../../../doc/project/evidence/model/tcp/parameters.md) lists the classes
of each parameter and the test of each class, and the plan
[`tcp-parameter-tests.md`](../../../../plan/pending/tcp-parameter-tests.md) gives the design.

## The rules of a test

- **One class, one test.** A class whose behavior has independent parts has one test for each
  part, a facet: `TcpAdaptiveDelayedAcksLinuxQuickack`, `...LinuxAto`, `...LinuxPingpong`.
- **The test must fail with the value of another class.** A test that passes for every value
  gives coverage, but no evidence. `cross_class.py` checks this. The one exception is a class
  whose name ends with `-override`: it claims that a socket option takes the place of the
  parameter, so its test passes with each value of the parameter by design.
- **One parameter changes.** The test sets its parameter on the host under test, also when the
  class holds the default, and it sets the preconditions of the class. Everything else keeps
  the defaults of `Tcp`.
- **Observe from the outside first**: the wire, the application, the signals of the
  connection; read the internal state (`assertModule`) only when they cannot show the class,
  and say why in the description.
- **A failing test is a finding.** Keep it, and record it in the catalog. This pass changes
  nothing in `src/`.

## The form of a test file

The file name is `<Module><Parameter><Class>.test` in PascalCase, unique in the whole TCP
suite: `TcpDelayedAckFrameCountOne.test`. A parameter that starts with the module name does not
repeat it: `TcpAlgorithmClassVegas.test`. The folder is the group of the parameter in
`Tcp.ned`: `ack/`, `setup/`, `congestion/`, ...

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
[`ini/_bulk.ini`](ini/_bulk.ini) is the scenario in which host1 sends data to host2. A test
of an invalid value has no program: it checks the exit code and the error message.

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
first.
