About Fingerprint Testing
=========================

.. About fingerprints in more detail
   ---------------------------------

.. Fingerprint testing is a useful and low-cost tool for regression testing during model development.
   A fingerprint is a hash value, which is calculated during a simulation run from certain properties of the simulation, such as time of events, module names, packet data, etc.

.. Fingerprint testing is a useful and low-cost tool for regression testing during model development.

A fingerprint is a hash value, which is calculated during a simulation run from specified "ingredients", i.e. properties of the simulation such as time of events, module names, packet data, etc.
The hash is updated at each event which takes part in the fingerprint calculation (as defined by the ingredients, filtering, etc) until the end of the simulation (or some defined time limit),
resulting in a fingerprint value.
This value is characteristic of the simulation's trajectory; a fingerprint change indicates a change in the trajectory.

.. **TODO** it is updated at each event/each event which takes part in the fingerprint calculation (as defined by the ingredients, filtering, etc)

.. **V2** This value is characteristic of the simulation's trajectory. It is useful for regression testing because a change in the fingerprint after a change in the model means the trajectory changed.
   It is useful during development to see if some change in the model breaks the model's correct behavior.

  TODO: the workflow is calculate the fingerprint when the model is assumed to work correctly,
  before introducing a change. After the change, run the fingerprint test again, and see if the
  fingerprints match. if not, the change likely broke something in the model.
  One needs to examine if the change in fact did break something, or was expected.
  Running a fingerprint with other ingredients might help verify that the model is correct.
  -> later

.. **- what are the various ingredients? and where to find them**

  .. TODO: maybe this shouldnt be here but in one of the steps

  The fingerprint ingredients are actually can be specified...

  the fingerprint is a 32 bit value, and also it contains the ingredients, which are denoted by a letter

  such as abcd-ef01-1248-abcd/tplx -> more like 32 bits

  the list of ingredients are in the manual

.. The fingerprint is a 32-bit hexadecimal value and upended by the ingredients used for calculating it, denoted by letters:

The fingerprint contains a 32-bit hexadecimal hash value and the ingredients used for calculating it, denoted by letters. Here is an example:

.. TODO: example fingerprint

.. code-block:: text

   53de-64a7/tplx

In this example, ``t`` means time of event, ``p`` the full path of modules, ``l`` the message length, and ``x`` the extra data added programatically. Other possible ingredients include various properties of messages, modules and results. For the complete list, check out the `Fingerprint section <https://doc.omnetpp.org/omnetpp/manual/#sec:testing:fingerprint-tests>`_ in the OMNeT++ Simulation Manual.

.. TODO tplx is a default

The fingerprints can be specified in the ini file for each configuration. Specifying any fingerprint enables fingerprint calculation for the simulation. For example:

.. if a fingerprint is specified, it will be calculated. so if you wanna calculate the fingerprint for a config X,
   you just say:

  | [Config X]
  | network = Y
  | fingerprint = 0000-0000/tplx

.. code-block:: ini

   [Config Ethernet]
   network = Wired
   fingerprint = 0000-0000/tplx

.. you just specify a value and the ingredients and it will be calculated and the 0000-0000 can be replaced by the resulting value.

The fingerprint will be calculated for that simulation when run in Qtenv or Cmdenv.
Qtenv and Cmdenv indicate whether the fingerprint is verified, and print the calculated fingerprint value. The original value can be replaced with the resulting one in the ini file.

.. the calculated value is printed. The original value can be replaced with the resulting one in the ini file.

.. TODO: qtenv prints the fingerprint

.. also, the fingerprint is calculated whether the simulation is run with qtenv or from the command line.

.. TODO fingerprint successfully verified

.. However, there is a more convenient way in INET, the fingerprint tool.

.. It automates the process to help with development.

INET's fingerprint test commands automate this process, making it easier to work with many simulations and fingerprints.

The fingerprint tests
---------------------

INET keeps the correct fingerprints in the JSON file ``tests/fingerprint/store.json``. This file is the fingerprint store.
A store entry defines one simulation run. It has the working directory, the configuration, the run number, the sim time limit, and the fingerprint with its ingredients.
The store file is part of the INET repository. The tests of the tutorial use the entries of the ``tutorials/fingerprint`` working directory.

Two commands of ``opp_repl`` work with the store:

- ``opp_run_fingerprint_tests`` runs the simulations. It compares the calculated fingerprints with the stored ones.
- ``opp_update_fingerprint_test_results`` runs the simulations. It records the calculated fingerprints in the store.

Each test has one of these results: ``PASS``, ``FAIL`` or ``ERROR``.
A test results in ``FAIL`` if the calculated fingerprint differs from the stored one.
A test results in ``ERROR`` if there is an error during the simulation.

The commands run the tests at the same time. They use all CPU cores.
To select tests, use the ``-w`` option for the working directory, the ``-c`` option for the configuration, and the ``-f`` option for a general filter.
Each option takes a regular expression. The ``--ingredients`` option selects the fingerprint ingredients.
The ``-m`` option selects the build mode, ``debug`` or ``release``. The default is ``debug``.
Use the ``--help`` option to see all options.

The update command prints one line for each simulation:

- ``KEEP`` means that the stored fingerprint is correct.
- ``UPDATE`` means that the fingerprint is different. The command replaces the stored value.
- ``INSERT`` means that the store has no value for these ingredients. The command adds the new value.

The update command changes the file ``tests/fingerprint/store.json``.
Use ``git diff tests/fingerprint/store.json`` to see the changes.
To accept new fingerprints, run the update command and keep the changes of the store file.
To reject them, restore the file with ``git checkout tests/fingerprint/store.json``.

The configurations
------------------

.. all this fingerprint testing depends on repeatable deterministic simulations, random number generation, etc.
  if it's not repeatable, e.g. emulation, than this whole stuff doesn't apply

  default ingredients is good because there's always a fingerprint sensitive to changes ready-to-be-used for regression testing

  - the configs in the tutorial that will be used to demonstrate the fingerprint tests
  - the csv file
  - they are correct fingerprints
  - the changes are no contained in the tutorial
  - the user is expected to make them and run the fingerprint test to see the results

The tutorial contains several simulations, which will be used to demonstrate how various changes affect fingerprints. Here is an overview of the simulations:

.. **TODO**: a list of the configs (just an overview)

- ``Ethernet``: Contains an Ethernet network with wired hosts and routers. The server host sends UDP packets to two of the hosts:

  .. figure:: media/EthernetNetwork.png
     :width: 90%
     :align: center

- ``Wifi``: Contains two ad-hoc hosts; one of them sends UDP packets to the other:

  .. figure:: media/WifiNetwork.png
     :width: 80%
     :align: center

- ``EthernetShortPacket``: The same as the ``Ethernet`` configuration, but the server sends 10-Byte UDP packets.

- ``WifiShortPacket``: The same as the ``Wifi`` configuration, but the host sends 10-Byte UDP packets.

The fingerprint store ``tests/fingerprint/store.json`` contains the fingerprints of these four configurations.
All four configurations have run number 0. The ``Ethernet`` and ``EthernetShortPacket`` configurations run for 0.2 s.
The ``Wifi`` and ``WifiShortPacket`` configurations run for 5 s. The stored fingerprints use the ``tplx`` ingredients.

The store contains the correct fingerprints for the latest INET version, i.e. they all pass.
To try the examples, make the changes that the tutorial describes, or try other changes.
Then run the fingerprint tests to see how the changes affect the fingerprints.

.. **TODO** resetting

.. note:: Before trying the example in a step, reset the model changes of the previous step.

.. or something similar/something else/some other changes/different changes

.. TODO its for experimentation

Running the tests
-----------------

Run the fingerprint tests from the command line. Make sure to run ``. setenv`` in both the ``omnetpp`` and ``inet`` folders:

.. code-block:: bash

   $ cd ~/workspace/omnetpp
   $ . setenv
   $ cd ~/workspace/inet
   $ . setenv
   $ cd ~/workspace/inet/tutorials/fingerprint

The ``opp_run_fingerprint_tests`` command runs the simulations of the tutorial.
Always give the project and the working directory filter. Use ``tplx`` as the ingredients:

.. code-block:: bash

  $ opp_run_fingerprint_tests --load @opp -p inet -w tutorials/fingerprint --ingredients tplx

.. code-block:: fp

  tutorials/fingerprint -c Ethernet for 0.2s PASS
  tutorials/fingerprint -c EthernetShortPacket for 0.2s PASS
  tutorials/fingerprint -c Wifi for 5s PASS
  tutorials/fingerprint -c WifiShortPacket for 5s PASS

.. .. figure:: media/1.png
   :width: 80%
   :align: center

Each step of the tutorial needs only some of the configurations. Use the ``-c`` option to run only those.
For example, the `Changing a Timer` step needs the ``Wifi`` configuration only:

.. code-block:: bash

  $ opp_run_fingerprint_tests --load @opp -p inet -w tutorials/fingerprint --ingredients tplx -c '^Wifi$'

.. code-block:: fp

  tutorials/fingerprint -c Wifi for 5s PASS

Each step shows the ``-c`` option to use.

.. TODO rules of thumb, default fingerprints, workflow, resetting,

.. tags

  all this fingerprint testing depends on repeatable deterministic simulations, random number generation, etc.
  if it's not repeatable, e.g. emulation, than this whole stuff doesn't apply

  default ingredients is good because there's always a fingerprint sensitive to changes ready-to-be-used for regression testing

  The default fingerprints...

  so

  - default/baseline fingerprints are chosen so that they are sensitive to a broad range of changes in the model...but not too sensitive cos then trivial changes would lead to false positives

Baseline ingredients
--------------------

The baseline fingerprint ingredients are chosen because they are sensitive to a broad range of changes in the model (the default ingredients in fingerprint tests in INET is often ``tplx``). If the ingredients are too sensitive, even trivial changes might lead to failed tests; if the ingredients are not sensitive enough, they might fail to detect regressions.

.. The default ingredients in INET is ``tplx``, which is generally good for the baseline purpose;
   ``tplx`` is used frequently in this tutorial. **TODO** there is no default really, because you have to specify something everywhere...

The workflow
------------

.. the workflow is mostly...

  - there is a change in the model
  - the fingerprints fail but you suspect that the model is still correct
  - you change the baseline ingredients to get fingerprints that are not sensitive to the particular changes that the change is about
  - but for others it stays sensitive
  - the fingerprints pass after the change

  the whole thing with a positive workflow:

  - you want to make a change in the model, and you suspect that it doesn't invalidate the model
  - you also know that the default/baseline fingerprints/ingredients are sensitive to that change
  - so you choose other ingredients which are not sensitive to that particular model change, but sensitive to others
  - so it is suitable for detecting regressions
  - so you create alternative fingerprints
  - you make the change in the model
  - the fingerprints pass
  - then baseline fingerprints can be restored

  This, just more concisely.

The tutorial describes the workflow of making typical changes and verifying the model.
Generally, the workflow involves a change in the model, which we suspect will not invalidate it. We also know that the baseline fingerprints are sensitive to this change; we choose other ingredients and create alternative fingerprints which are not sensitive to the particular change, but sensitive to others. Then, after making the model change, the fingerprint tests should pass, and the model should be verified. When the model is verified, the baseline ingredients can be restored.

.. note:: Fingerprint testing depends on repeatable deterministic simulations, random number generation, etc. If the tests are not repeatable, i.e. they don’t result in the same fingerprints when run multiple times (e.g. emulation), fingerprint tests are useless.

.. if it's not repeatable, e.g. emulation, than this whole stuff doesn't apply