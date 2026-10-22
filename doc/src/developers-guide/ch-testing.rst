.. _dg:cha:testing:

Testing
=======

INET contains a comprehensive test suite. The test suite can be found under the
:file:`tests` folder. Testing is usually done from the command line after setting
up the environment with:

.. code-block:: shell

   $ cd ~/workspace/omnetpp
   $ . setenv
   $ cd ~/workspace/inet
   $ . setenv

Regression Testing
------------------

The most often used tests are the so-called fingerprint tests, which are generally
useful for regression testing. Fingerprint tests are a low-cost but effective tool
for regression testing of simulation models during development. For more details
on fingerprint testing, please refer to the `OMNeT++ manual
<https://omnetpp.org/doc/omnetpp/manual/#sec:testing:fingerprint-tests>`__.

The INET fingerprint baseline is the JSON file :file:`tests/fingerprint/store.json`.
Each entry of the file describes one fingerprint of one simulation run: the working
directory, the ini file, the config, the run number, the simulation time limit, the
fingerprint ingredients, the expected fingerprint and the expected test result. The
store holds a run-0 fingerprint of the ingredients ``tplx``, ``~tNl`` and ``~tND``
for each concrete config that can run without user input, and some more runs.

The fingerprint tests run with the :command:`opp_run_fingerprint_tests` command of
`opp_repl <https://github.com/omnetpp/opp_repl>`__. Set up the OMNeT++ and INET
environment first. Then run all fingerprint tests from anywhere in the INET tree:

.. code-block:: shell

   $ opp_run_fingerprint_tests --load @opp -p inet

The ``--load @opp`` option loads the project descriptions that come with opp_repl,
and ``-p inet`` selects INET. The INET description sets the store, the ingredients
that CI checks (``tplx``, ``~tNl`` and ``~tND``) and a CPU time limit of 600s for
each simulation. The command shows its options with ``-h``.

Running all INET fingerprint tests takes several minutes even on a modern
computer. The command runs the simulations in parallel on all CPUs.

The simplest way to make the tests run faster is to run only a subset of all
fingerprint tests. The ``-f`` option takes a regular expression that must match the
working directory, the ini file or the config. The ``-w``, ``-i`` and ``-c`` options
filter on one of them only.

Another less commonly used technique is running the fingerprint tests in release
mode. One disadvantage of release mode is that certain assertions, which are
conditionally compiled, are not checked. The default mode is debug.

For example, the following command runs all wireless tests in release mode:

.. code-block:: shell

   $ opp_run_fingerprint_tests --load @opp -p inet -m release -f wireless

While the tests are running, the command prints a line for each simulation. The
test result of each fingerprint is PASS, FAIL or ERROR, and a FAIL line shows the
calculated and the correct value:

.. code-block:: shell

   examples/bgpv4/BgpAndOspfSimple -c config1 for 100s PASS
   examples/bgpv4/BgpCompleteTest -c config1 for 100s FAIL (unexpected) (Fingerprint mismatch) calculated 1234-abcd/tplx (correct 5678-ef01/tplx)

When testing is finished, the command prints a summary of the results. Its exit code
is 0 only if each result is the expected one.

To accept new fingerprints, run the update command with the same filter:

.. code-block:: shell

   $ opp_update_fingerprint_test_results --load @opp -p inet -f wireless

The update runs each stored run at its stored limit, and writes the new values into
the store. It reports KEEP, UPDATE or INSERT for each simulation, and
:command:`git diff tests/fingerprint/store.json` shows the changed values. A new
config gets its run-0 fingerprints with ``--no-stored-only``, for example
``opp_update_fingerprint_test_results --load @opp -p inet --no-stored-only -c '^MyConfig$' -r 0``.
The new entries take the ``sim-time-limit`` of the ini file.

Usually, when a simulation model is being developed, the fingerprint tests should
be run every now and then to make sure there are no regressions (i.e. all tests
run with the expected result). How often tests should be run depends on the kind
of development; tests may be run from several times a day to a few times a week.

For example, if a completely new simulation model is developed with new examples,
then the new fingerprint tests can be run as late as when the first version is
pushed into the repository. In contrast, during an extensive model refactoring,
it's advisable to run the affected tests more often (e.g. after each small step).
Running the tests more often helps avoid getting into a situation where it's
difficult to tell if the new fingerprints are acceptable or not.

In any case, certain correct changes in the simulation model break fingerprint
tests. When this happens, there are two ways to validate the changes:

-  One is when the changes are further divided until the smallest set of changes
   is found, which still break fingerprints. In this case, this set is carefully
   reviewed (potentially by multiple people), and if it is found to be correct,
   then the fingerprint changes are simply accepted.

-  The other is to actually change the fingerprint calculation in a way that
   ignores the effects of the set of changes. To this end, you can change the
   fingerprint ingredients, use filtering, or change the fingerprint calculation
   algorithm in C++. These options are covered in the OMNeT++ manual.
   
   With this method, record the fingerprints with the modified calculation before you
   apply the changes, and check them after you apply the changes. These values are
   temporary: restore the store with :command:`git checkout tests/fingerprint/store.json`
   before you record the real values. This method is often time-consuming but may be
   worth the effort for complex changes because it allows having far greater confidence
   in correctness.

For a simple example, a timing change that neither changes the order of events
nor where the events happen can be easily validated by removing time from the
ingredients (default is tplx). Before the change, record ``plx`` fingerprints for
the affected runs. After the change, check them:

.. code-block:: shell

   $ opp_update_fingerprint_test_results --load @opp -p inet --no-stored-only -f <filter> -r 0 --ingredients plx
   $ opp_run_fingerprint_tests --load @opp -p inet -f <filter> --ingredients plx

Another common example is when a submodule is renamed. Since the default fingerprint
ingredients contain the full module path of all events, this change will break
fingerprint tests. If the module is not optional or not accessed by other modules
using its name, then removing the module path from the fingerprint ingredients
(i.e., using tlx) can be used to validate this change.

For a more complicated example, when the IEEE 802.11 MAC (very complex) model is
refactored to a large extent, then it's really difficult to validate the changes.
If despite the changes, the model is expected to keep its external behavior, then
running the fingerprint tests can be used to prove this to some extent. The easy
way to do this is to actually ignore all events executed by the old and the new
IEEE 802.11 MAC models. Give the module filter to both the update before the change
and the test after the change. ``--append-args`` passes the rest of the line to
``opp_run``, so it must be the last option:

.. code-block:: shell

   # please note the quoting uses both ' and " in the right order
   $ opp_update_fingerprint_test_results --load @opp -p inet -f <filter> --append-args --fingerprint-modules='"not(fullPath(**.wlan[*].mac.**))"'
   $ opp_run_fingerprint_tests --load @opp -p inet -f <filter> --append-args --fingerprint-modules='"not(fullPath(**.wlan[*].mac.**))"'

INET uses continuous integration on Github. Fingerprint and other tests are run
automatically for changes on the master and integration branches. Moreover, all
submitted pull requests are automatically tested the same way. The result of the
test suite is clearly marked on the pull request with a check mark or a red cross.

In general, contributors are expected to take care of not breaking the fingerprint
tests. In the case of a necessary fingerprint change, update the store in the commit
that makes the change, and say in the commit message which behavior changes.

INET Fingerprint Ingredients
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to the standard OMNeT++ fingerprint ingredients (covered in the
`OMNeT++ manual <https://omnetpp.org/doc/omnetpp/manual/#sec:testing:fingerprint-tests>`__),
INET provides 5 additional fingerprint ingredients that offer additional control
over what aspects of network simulation are considered significant for reproducibility
testing.

These ingredients are handled by the ``inet::FingerprintCalculator`` class. The
fingerprint test commands automatically add the corresponding ``--fingerprintcalculator-class``
option to the simulation command lines of each ingredient string with ``~``.

**Filter Ingredients (Exclusion Filters)**

These ingredients act as filters that remove certain events from fingerprint calculation
entirely, before other ingredients are processed:

``~`` (NETWORK_COMMUNICATION_FILTER)
   Excludes intra-node communication from fingerprint calculation. When enabled,
   only includes packet arrival events that cross network node boundaries (where
   sender node and arrival node are different); all other events are excluded
   from the fingerprint. This focuses fingerprinting on actual network
   communication rather than internal node processing.

``U`` (PACKET_UPDATE_FILTER)
   Excludes packet update arrival events from fingerprint calculation, all other
   events are unaffected. When enabled, filters out packets that are marked as
   "transmission update" packets. This causes packet transmission updates such
   as truncations to be ignored. Packets will only be taken into account with
   their originally intended content, regardless of what was actually
   transmitted. This flag is formally independent of the '~' flag, but in
   practice it only makes sense together with it.

**Path Information Ingredients**

These ingredients add path information to the fingerprint hash for packet arrival events:

``N`` (NETWORK_NODE_PATH)
   For packet arrival events, includes the full path of both sender and arrival
   network nodes in the fingerprint. Ignores non-packet events and all events
   that happen outside network nodes. Makes the fingerprint sensitive to which
   specific network nodes are involved in communication. For each packet arrival
   event, adds the full path of the containing network node for both the sender
   and arrival modules, even inside the same network node. This is mostly useful when
   used together with the `~` ingredient.

``I`` (NETWORK_INTERFACE_PATH)
   For packet arrival events, includes the full interface path of both sender
   and arrival network interfaces in the fingerprint. Ignores non-packet events
   and all events that happen outside network interfaces. Makes the fingerprint
   sensitive to which specific network interfaces are used for communication.
   For each packet arrival event, adds the interface full path of the containing
   NIC modules, even inside the same network interface. This is mostly useful when
   used together with the `~` ingredient.

**Content Ingredient**

``D`` (PACKET_DATA)
   Includes the actual packet data in the fingerprint. For packet arrival events
   with non-zero length, adds the raw packet content to the hash. This makes the
   fingerprint sensitive to actual packet content, not just packet structure or
   timing. Note that this ingredient requires packet serialization, so it will
   not work for simulations that contain protocol headers, payload, etc. that
   have no serializer implemented.

**Usage Notes**

- Filter ingredients (``~`` and ``U``) work as exclusion filters and are processed
  before other ingredients
- Path and content ingredients (``N``, ``I``, ``D``) only apply to packet arrival events
