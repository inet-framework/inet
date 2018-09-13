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

The most often used tests are the so called fingerprint tests, which are generally
useful for regression testing. Fingerprint tests are a low-cost but effective tool
for regression testing of simulation models during development. For more details
on fingerprint testing, please refer to the `OMNeT++ manual
<https://omnetpp.org/doc/omnetpp/manual/#sec:testing:fingerprint-tests>`__.

The INET fingerprint tests can be found under the :file:`tests/fingerprint` folder.
Each test is basically a simulation scenario described by one line in a CSV file.
INET contains several such CSV files for different groups of tests. One CSV line
describes a simulation by specifying the working directory, command line arguments,
simulation time limit, expected fingerprint and test result (e.g. pass or fail),
and several user defined tags to help filtering.

Fingerprint tests can be run using the :command:`fingerprinttest` script found
in the above folder. To run all fingerprint tests, simply run the script without
any arguments:

.. code-block:: shell

   $ ./fingerprinttest

The script also has various command line arguments, which can be understood by
asking for help with:

.. code-block:: shell

   $ ./fingerprinttest -h

Running all INET fingerprint tests takes several minutes even on a modern
computer. By default, the script utilizes all available CPUs, the tests are
run in parallel (non-deterministic order).

The simplest way to make the tests run faster, is to run only a subset of all
fingerprint tests. The script provides a filter parameter (-m) which takes a
regular expression that is matched against all information of the test.

Another less commonly used technique is running the fingerprint tests in release
mode. One disadvantage of release mode is that certain assertions, which are
conditionally compiled, are not checked.

For example, the following command runs all wireless tests in release mode:

.. code-block:: shell

   $ ./fingerprinttest --release -m wireless

While the tests are running, the script prints some information about each test
followed by the test result. The test result is either PASS, FAIL or ERROR (plus
some optional details):

.. code-block:: shell

   /examples/bgpv4/BgpAndOspfSimple/ -f omnetpp.ini -c config1 -r 0 ... : PASS
   /examples/bgpv4/BgpCompleteTest/ -f omnetpp.ini -c config1 -r 0 ... : FAIL
   
When testing is finished, the script also prints a short summary of the results
with the total number of tests, failures and errors:

.. code-block:: shell

   Failures:
     /examples/bgpv4/BgpCompleteTest/ -f omnetpp.ini -c config1 -r 0
   ----------------------------------------------------------------------
   Ran 2 tests in 3.615s

   FAILED (failures=1)

Apart from the information printed on the console, fingerprint testing also
produces three CSV files. One for the updated fingerprints, one for the failed
tests, and one for the test which couldn't finish without an error. The updated
file can be used to overwrite the original CSV file to accept the new fingerprints.

Usually, when a simulation model is being developed, the fingerprint tests should
be run every now and then, to make sure there are no regressions (i.e. all tests
run with the expected result). How often tests should be run depends on the kind
of development, tests may be run from several times a day to a few times a week.

For example, if a completely new simulation model is developed with new examples,
then the new fingerprint tests can be run as late as when the first version is
pushed into the repository. In contrast, during an extensive model refactoring,
it's adivsable to run the affected tests more often (e.g. after each small step).
Running the tests more often helps avoiding getting into a situation where it's
difficult to tell if the new fingerprints is acceptable or not.

In any case, certain correct changes in the simulation model break fingerprint
tests. When this happens, there are two ways to validate the changes:

-  One is when the changes are further divided until the smallest set of changes
   are found, which still break fingerprints. In this case, this set is carefully
   reviewed (potentially by multiple people), and if it is found to be correct,
   then the fingerprint changes are simply accepted.

-  The other is to actually change the fingerprint calculation in a way that
   ignores the effects of the set of changes. To this end, you can change the
   fingerprint ingredients, use filtering, or change the fingerprint calculation
   algorithm in C++. These options are covered in the OMNeT++ manual.
   
   With this method, the fingerprint tests must be re-run before the changes are
   applied using the modified fingerprint calculation. The updated fingerprint
   CSV files must be used to run the fingerprint tests after applying the changes.
   This method is often time consuming, but may be worth the efforts for complex
   changes, because it allows having a far greater confidence in correctness.

For a simple example, a timing change which neither changes the order of events
nor where the events happen, can be easily validated by changing the fingerprint
and removing time from the ingredients (default is tplx) as follows:

.. code-block:: shell

   ./fingerprinttest -a --fingerprint=0 --fingerprint-ingredients=plx

Another common example is when a submodule is renamed. Since the default fingerprint
ingredients contain the full module path of all events, this change will break
fingerprint tests. If the module is not optional or not accessed by other modules
using its name, then removing the module path from the fingerprint ingredients
(i.e. using tlx) can be used to validate this change.

For a more complicated example, when the IEEE 802.11 MAC (very complex) model is
refactored to a large extent, then it's really difficult to validate the changes.
If despite the changes, the model is expected to keep its external behavior, then
running the fingerprint tests can be used to prove this to some extent. The easy
way to do this is to actually ignore all events executed by the old and the new
IEEE 802.11 MAC models:
 
.. code-block:: shell

   # please note the quoting uses both ' and " in the right order
   ./fingerprinttest -a --fingerprint-modules='"not(fullPath(**.wlan[*].mac.**))"'

INET uses continuous integration on Github. Fingerprint and other tests are run
automatically for changes on the master and integration branches. Moreover, all
submitted pull requests are automatically tested the same way. The result of the
test suite is clearly marked on the pull request with check mark or a red cross.

In general, contributors are expected to take care of not breaking the fingerprint
tests. In case of a necessary fingerprint change, the CSV files should be updated
in separate patches.
