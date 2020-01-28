Regression Testing and Fingerprints Tutorial
============================================

.. Goals
   -----

..  The introduction

  - what are fingerprints and what they're good for
    #development #regressiontesting #verification

  - what are the various ingredients? and where to find them
  - what other kinds of fingerprint calculators are available, besides the default one ?

  - how to run it? the different possibilities (ini/fingerprint tool)

  - what is this tutorial about ?
  - how is this tutorial structured ?

  The steps

.. ----------------------------------------------------------

  so

  -fingerprint testing is useful and lowcost for regression testing during model development
  -omnetpp has the fingerprint mechanism builtin and inet has the fingerprint test tool
  for convenience
  -a fingerprint is a hash value, which is calculated during a simulation run from certain properties
  of a simulation, such as time of events, module names, packet data, etc.
  -it is constantly updated
  -it is a characteristic of the simulation's trajectory
  -it is good for regression testing because a change in the fingerprint means the trajectory changed
  -useful during development to see if some change breaks the model's correct behavior

.. **- what are fingerprints and what they're good for
   #development #regressiontesting #verification**

  **Introduction**
  Fingerprint testing is a useful and low-cost tool for regression testing during model development.
  A fingerprint is a hash value, which is calculated during a simulation run and is characteristic of the simulation's trajectory. After the change in the model, a change in the fingerprint can indicate that the model no longer works correctly.
  When the fingerprints stay the same after a change in the model, the model can be assumed to be still working correctly. thus protecting against regressions

    - regression testing is about noticing that a change in the model broke something that worked before or that the change is not working as its intended.
    - regression testing is about automating this process, so doesn't have to be manually checked
    - fingerprint testing can be used for this purpose. its a useful tool...
    - fingerprints can be calculated in different ways, using the ingredients which specify which aspects of the simulation to take into account
    - fingerprints can be specified in the ini file and they will be calculated, but the fingerprint tool automates this process
    - the tutorial is about some typical changes in the model during development and their effect on fingerprints, and the verification process
    - the workflow of this

    - also...the fingerprints can be used to assume that a change didn't break the model

During the development of simulation models, it is important to detect regressions, i.e. to notice that a change in a correctly working model led to incorrect behavior.

.. **V2** During the development of simulation models, it is important to detect regressions, i.e. to notice that after a change in a correctly working model, the model (or some aspect of it) no longer works correctly, and that the change didn't work as indended.

.. TODO might be easy to miss? might be hard to notice that something broke ?
   The change in the behavior might be subtle and hard to detect by examining.

.. Regressions can be detected by manually examining simulations, but usually it is less tedious to use automated testing.

.. Regressions can be detected by manually examining simulations, but it can be error prone/but a change in behavior might be hard to detect.

.. in qtenv

.. but it can be error prone/but a change in behavior might be hard to detect.

.. -> so the model no longer works/the change broke something
   This can be detected with manual testing, but

.. Regression testing is about automating this process, so the model doesn't have to be manually checked as thoroughly and as often. and also might detect regressions that would otherwise would have been missed.

.. and might also detect regressions that would have been missed in manual examination.

Regressions can be detected by manually examining simulations in Qtenv. However, some behavioral changes might be subtle and go unnoticed, but still would invalidate the model.
Regression testing automates this process, so the model doesn't have to be manually checked as thoroughly and as often. It might also detect regressions that would have been missed during manual examination.

Fingerprint testing is a useful and low-cost tool that can be used for this purpose.
A fingerprint is a hash value, which is calculated during a simulation run and is characteristic of the simulation's trajectory. After the change in the model, a change in the fingerprint can indicate that the model no longer works along the same trajectory. This may or may not mean that the model is incorrect.
When the fingerprints stay the same, the model can be assumed to be still working correctly (thus protecting against regressions).
Fingerprints can be calculated in different ways, using so called 'ingredients' that specify which aspects of the simulation to be taken into account when calculating a hash value.

.. Fingerprints can be specified in the ini file or at the command line, and will be calculated during the simulation, but INET's fingerprint tool automates this process.

.. **V1** This tutorial is about typical changes in the model during development and their effect on fingerprints, and the verification process.

.. **V2** This tutorial describes typical changes in the model during development, and the workflow of the verification process using fingerprint tests.

.. The steps in this tutorial describe typical changes in the model during development, and the workflow of the model validation process using fingerprint tests. TODO examples

The steps in this tutorial contain examples for typical changes in the model during development, and describe the model validation workflow using fingerprint tests.

.. TODO fingerprints are part of omnetpp

Information about OMNeT++'s fingerprint support can be found in the `corresponding section <https://doc.omnetpp.org/omnetpp/manual/#sec:testing:fingerprint-tests>`_ of the OMNeT++ Simulation Manual. Also, the :doc:`Testing </developers-guide/ch-testing>` section in the INET Developer's Guide describes regression testing.

The tutorial contains the following steps:

..  What the tutorial is about?
  ---------------------------

  Its about the workflow of regression testing. The typical actions done during development and how they might affect fingerprints and how to verify the correctness of the model.
  The different cases.

.. The steps
   ---------

.. toctree::
   :maxdepth: 1

   first
   refactoring
   renaming_submodule
   renaming_module_parameter
   packet
   changing_timer
   newevents_filtering
   newevents_nid
   removingevents
   accepting

.. .. toctree::
      :maxdepth: 1
      :glob:

      *
