# Derive tests from the parameters of a module

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [test-anatomy.md](../design/test-anatomy.md), [testing.md](../rule/testing.md), [derive-tests-from-a-standard.md](derive-tests-from-a-standard.md)

How to turn the NED parameters of a module into tests: one test for each class of values of each parameter. The comment of a NED parameter is a promise to the user: it says what each value does. A value that no test sets can stop to work, or never work, and nobody sees it. For example, before this procedure ran on TCP, no test set `seqNo = -1` of `TcpSpoof`, and that value stops each run with an error of the simulation time (finding PF-13). The worked example is TCP: the catalog [parameters.md](../evidence/model/tcp/parameters.md) and the tests in [tests/protocol/tcp/parameter/](../../../tests/protocol/tcp/parameter/README.md), 330 tests for the 116 parameters of the modules of `transportlayer/tcp` and of `TcpSpoof`.

"Parameter test" names a derivation, as "standards test" does, and not a category of [test-anatomy.md](../design/test-anatomy.md). The tests of a protocol module use the protocol test library, so they stay in the `protocol` category under [TR-CAT-MATCH](../rule/testing.md#tr-cat-match). For a module that is not a protocol, the same procedure puts the tests in `tests/module/<subject>/parameter/`.

## Principle: the claim is the NED comment

A test checks what the comment of its parameter says, for one class of values. When the code does something that the comment does not say, the test states the behavior from the code or from the standard that the code cites, and the catalog records a documentation gap: its source column says "code" and not "NED". For example, the comment of `Tcp.keepAliveProbeCount` names no special value, but with 0 the code aborts the connection at the idle time, without a probe. The class `zero` comes from the code, and its row says so.

## Principle: a test separates its class

A test must fail when its parameter has the value of another class. A test that passes for every value adds coverage, but no evidence. For example, a test of `tcpAlgorithmClass = "DcTcp"` that only checks that the window falls after an ECE also passes with TcpNewReno. The DcTcp test therefore checks that the slow-start threshold is 3/4 of the flight, which only DcTcp gives.

The cross-class check runs each test again with the value of each other class of its parameter, and expects the verdict FAIL. Three kinds of class are no alternative: a class whose test checks an error, a class whose name ends with `-override` (a socket option takes the place of the parameter, so its test passes with each value by design), and a class whose name ends with `-alias` (another spelling of the value of another class). The check also skips the tests of the last two.

## Principle: a failing test is a finding

Keep a test that fails, and record the defect in the findings of the catalog. The pass that writes the tests changes no model code; a later plan repairs the findings. Whether a failure is declared expected follows [the class of a failure](derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected).

A defect must not hide the verdicts of other classes. For example, `TcpSpoof` wrote no checksum mode into its header, so the TCP of a receiver stopped the run at the first spoofed segment (PF-11). A spoof test that waits for the answer of the receiver failed for this one defect. So the spoof tests checked the segment on the wire, and a tap rule dropped it before the receiver. Only the facet `isSYN:true-answer` let the segment through, and it failed until the repair of PF-11.

## The steps

| Step | What | Artifact |
| --- | --- | --- |
| 1 | Choose the scope, the groups and the shared files | the plan, the folders, the shared networks and ini files |
| 2 | Find the classes of each parameter: the code pass | the rows of the catalog |
| 3 | Write the tests | one `.test` file for each class or facet |
| 4 | Run the tests and calibrate the expected values | the assertions |
| 5 | Run the cross-class check | its report |
| 6 | Record the verdicts and the findings, and check the other tests | the catalog |
| 7 | After the last group: measure the coverage and close its gaps | new facets, the coverage section of the catalog |

Steps 2 to 6 run once for each group. Report and commit each group by itself: a group of 10 to 30 tests is a size that a reviewer can read.

### Step 1 — choose the scope, the groups and the shared files

The scope is all parameters of all modules of one folder. A module that only changes the defaults of another module, such as `LinuxTcp` and `LegacyTcp`, gets no tests of its own: each of its defaults falls into a class, and a column of the catalog names the presets of each class.

The groups follow the sections of the NED comment: for TCP, `ack`, `setup`, `send`, `rto`, `congestion`, `recovery`, `receive`, `close`, `ecn`, `pmtu`, `keepalive`, `fastopen`, the parameters of the module itself (`module`), and `TcpSpoof` (`spoof`). Each group has a folder.

Prepare the shared files before the first group, so that a test holds only what its class needs:

- a shared network, or a small set of them: for TCP, two hosts with a tap between them, a router with a bottleneck link, and a switch with more hosts;
- a shared base ini file and a shared scenario ini file, which each test includes last: in an ini file the first line that matches a key wins, so the lines of the test come first;
- the catalog, and a script that checks that the catalog, the NED files and the tests agree.

### Step 2 — find the classes of each parameter

Do these steps for each parameter, one group at a time, just before the tests of the group are written.

1. Read the NED comment. Write down each value or range that it names: the special values (`-1`, `0`, `""`), the enum values, the bounds, the errors, the default, the preconditions ("needs receiveBufferSize"), and a socket option that takes the place of the parameter.
2. Read the code that reads the parameter, and each use of the member that keeps its value. Each condition on the value is a boundary, also a condition in a shared helper. The values between two boundaries are one class. A check that stops the run gives the class `invalid`.
3. A `@mutable` parameter gets the class `changed`: an action changes the value during the run, the next connection uses the new value, and the open connection keeps the old one.
4. A `volatile` parameter gets a class for a distribution, if the code reads it more than one time.
5. A class whose behavior has more than one independent part gets one test for each part, a facet: the quick ACKs, the ATO and the pingpong mode of `adaptiveDelayedAcks = true`, or the IPv4 and the IPv6 path of `checksumMode = "computed"`.
6. A combination of values of the peer is not a class. Select the peer that makes the class visible.
7. Choose one representative value for each class: the default when it is in the class, else a typical value, and a boundary value only when the code treats the boundary specially.
8. Write the classes into the catalog with their source: "NED", or "code" with the function. A class with the source "code" and no NED statement is a documentation gap.

A value that the code treats in a branch of its own is a class, also when the comment does not name it. For example, `OperationalMixin::startActiveOperationExtraTimeOrFinish()` has a branch for a negative extra time, and 28 other modules of INET have `-1s` as their default. So `Tcp.stopOperationExtraTime` has the class `negative-alias`, and its test found the defect PF-9: a negative value stopped the run. A value that only reaches a check of another module, such as a negative delay that reaches the scheduler, is not a class: the catalog records it as a finding.

### Step 3 — write the tests

A test sets its parameter on the host under test, also when the class holds the default, and it sets the preconditions of its class. Everything else keeps the defaults of the module. The description names the claim, the classes of the parameter, the code that separates them, the host under test, the scenario, and the observation.

Observe the class at the first level that can show it:

| Level | Channel | Example |
| --- | --- | --- |
| 1 | the packets on the wire, or what the application receives | the MSS option of the SYN; the time of a retransmission |
| 2 | the declared signals of the module | `ssthresh` after a loss |
| 3 | the internal state, read through its class descriptor | the size of the receive buffer |
| — | the error at the start of the run | `mss = 87`: "mss must be -1 or at least 88" |

Use level 3 only when levels 1 and 2 cannot show the class, and give the reason in the description. The words of the protocol test library are in [AUTHORING.md](../../../tests/protocol/lib/AUTHORING.md): tap rules make the losses, marks and changed fields that a class needs, an action changes a `@mutable` parameter or stops a node, and a test application makes socket calls.

### Step 4 — run the tests and calibrate the expected values

Run a new test first with a tolerant assertion that prints the value, then write the assertion with a range that the other classes miss. A value that the code computes is often not the value that a reader expects. For example, the reduction of TcpVegas computes its window with the MSS of the handshake (1460 bytes), not with the payload of a segment (1448 bytes), and the window before the reduction is the last value of the signal, not the largest one.

A test must reach the code of its claim. A test can pass through another path, and then it gives no evidence for its class. For example, the first test of the clamp growth of `windowAutoTuning` passed, but the receive buffer grew through the growth for out-of-order data, which the hole in the data started. A coverage run of the single test showed it. When the claim names a code path, check that the test reaches it.

A default of another parameter can hide a class. Find the defaults that act in the scenario: Tail Loss Probe sends a probe before a timeout, the delayed ACK combines two segments, and DcTcp negotiates ECN only when `tcpEcnMode` asks for it. Set such a parameter as a precondition, and name it in the description.

### Step 5 — run the cross-class check

The check reruns the binary of the last run of each test, with the value of each other class on the command line. A run that stops with an error gives no verdict, and the check reports it. Often the cause is a defect of the other value; record it in the description of the test. A run can also stop because the scenario lets a defect of another part act: then change the scenario so that the run continues, as the tap does in the spoof tests.

A test that passes with the value of another class needs a stronger observation. For example, the test of the DcTcp receiver checked an immediate ACK after the marked segment, and DumbTcp, which acknowledges each segment at once, passed it too. The test now also checks that the unmarked segments get the delayed ACK. In the same way, a class can fall back to the behavior of another class with the default peer: `tcpEcnMode = "accecn"` uses classic ECN with a peer that does not answer AccECN, so the test of `rfc3168` uses a peer that answers both.

opp_test writes the run script of a test only when the test passes, so the check cannot rerun a failing test. Run such a test by hand with the run script of another test of the folder.

### Step 6 — record the verdicts and the findings, and check the other tests

Write the verdict of each test into its row. A row with FAIL names its finding. A finding states the defect or the documentation gap, the place in the code or the NED file, and the test or the code pass that found it.

Run all other tests before and after the group, and compare the verdicts. A new test must not change the verdict of another test.

### Step 7 — measure the coverage and close its gaps

After the last group, measure the line coverage of the sources of the module for each pool of tests: the tests that existed before, the parameter tests, and the other suites. Use a build in coverage mode. When the OMNeT++ tree is shared, use a copy of it: a coverage build writes its tools into `bin/`, where the other build modes use them too.

Sort each line that no pool reaches into one of four kinds:

| Kind | The line | What to do |
| --- | --- | --- |
| (a) | a parameter value decides it, and a test can show a behavior that separates the class | add a facet, and write its test |
| (b) | a socket command, a socket option, or another path of the API | record it for a test class of the API |
| (c) | the segments of the peer or a rare state decide it, and no parameter does | record it for the standards tests |
| (d) | a model error, a string of the inspectors, or dead code | record dead code as a finding |

Both conditions of kind (a) matter. A line behind a value whose behavior equals that of another class, such as the timeout of TcpTahoe and that of TcpReno, gets no facet, because no test could separate the two classes. A line that also needs a rare state, such as a delayed-ACK timeout that grows above the RTO, is kind (c).

Write the facets, run steps 4 to 6 for them, and measure again. Repeat until no line of kind (a) is left. The catalog records the numbers of the pools and the kinds of the lines that remain. For TCP, the INET tests went from 47.2 % to 77.9 % of the lines, and all pools together from 73.3 % to 86.0 %.
