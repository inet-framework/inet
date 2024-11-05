Getting Started
===============

During the development of simulation models, it is important to detect
regressions, i.e. to notice that a change in a correctly working model led to
incorrect behavior.

Regressions can be detected by manually examining simulations in Qtenv. However,
some behavioral changes might be subtle and go unnoticed, but still would
invalidate the model. Regression testing automates this process, so the model
doesn't have to be manually checked as thoroughly and as often. It might also
detect regressions that would have been missed during manual examination.

Fingerprint testing is a useful and low-cost tool that can be used for this
purpose. A fingerprint is a hash value, which is calculated during a simulation
run and is characteristic of the simulation's trajectory. After the change in
the model, a change in the fingerprint can indicate that the model no longer
works along the same trajectory. This may or may not mean that the model is
incorrect. When the fingerprints stay the same, the model can be assumed to be
still working correctly (thus protecting against regressions). Fingerprints can
be calculated in different ways, using so called 'ingredients' that specify
which aspects of the simulation to be taken into account when calculating a hash
value.

The steps in this tutorial contain examples for typical changes in the model
during development, and describe the model validation workflow using fingerprint
tests.

Information about OMNeT++'s fingerprint support can be found in the
`corresponding section <https://doc.omnetpp.org/omnetpp/manual/#sec:testing:fingerprint-tests>`_
of the OMNeT++ Simulation Manual. Also, the :doc:`Testing </developers-guide/ch-testing>`
section in the INET Developer's Guide describes regression testing.


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/tutorials/fingerprint`` folder in the `Project Explorer`. There, you can view
and edit the tutorial files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/tutorials/fingerprint && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.


Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1001>`__ in
the GitHub issue tracker for commenting on this tutorial.
