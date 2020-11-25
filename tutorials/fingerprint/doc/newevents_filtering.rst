Adding New Events - Part 1
==========================

..  1. change or action
  2. phenomenon / effect
  3. solution
  4. example

Some changes in the model can add new events to the simulation. These events inevitably change the default fingerprints but they don't necessarily invalidate the model. One option is to filter out the modules in which the new events take place, so they are not included in fingerprint calculation. Taking into account the rest of the modules might show that the simulation trajectory in fact stayed that same.

To filter out modules, run the fingerprint test with the ``--fingerprint-modules`` command line option, e.g.:

.. code-block:: bash

   $ inet_fingerprinttest -a --fingerprint-modules='"not fullPath=~**.wlan[*].mac.**"'

As a simplistic example, we will make changes to the Udp module's code; the changes add new events, but doesn't alter the module's behavior. However, the fingerprints will change; we will filter out the Udp module from the fingerprint calculation to verify the model. Here is the workflow:

We run the fingerprint tests without taking the Udp module into account (the order of the quotes is important):

.. code-block:: fp

  $ inet_fingerprinttest -m AddingNewEvents1 -a --fingerprint-modules='"not fullPath=~**.Udp"'
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED

We can accept the fingerprints because there was no change in the model

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

Then we add a line to one of the functions in :cpp:`Udp.cc` which schedules a self message;
we also add a handleSelfMessage() function which just deletes the message when its received:

.. literalinclude:: ../sources/Udp_mod.cc
   :diff: ../sources/Udp_orig.cc

We run the fingerprint tests again without the Udp module:

.. code-block:: fp

  $ inet_fingerprinttest -m AddingNewEvents1 -a --fingerprint-modules='"not fullPath=~**.Udp"'
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS

The fingerprint tests PASS; the model is verified.

.. note:: The ``-fingerprint-module=""`` command line argument can be added to the .csv file as well.

We can change the ingredients back to ``tplx`` (or some other default), re-run the tests, and accept the new values. This process is described in more detail in the :doc:`accepting` step.

Another option for dealing with the effect of new events is to filter out only the new event
from the fingerprint calculation. This approach is beneficial because alternate fingerprints
with different ingredients are not needed. If only the new event is filtered out, the fingerprints should stay the same.

As a simplistic example, we'll make the same change to the Udp module as before.
We'll run the tests with the baseline ingredients and filter out the new self message by its id:

.. code-block:: fp

  $ inet_fingerprinttest -m AddingNewEvents1 -a --fingerprint-events='"not kind=~42"'
  . -f omnetpp.ini -c Ethernet -r 0 ... : PASS
  . -f omnetpp.ini -c Wifi -r 0 ... : PASS

The tests PASS, and the model is verified.

.. .. note:: As a rule of thumb, it is best to filter out the most specific part of the model possible.

.. .. note:: As a rule of thumb, it is best to filter out specifically the part of the model affected by the change

.. .. note:: As a rule of thumb, it is best to filter out only the specific part of the model affected by the change. The more specific the filtering, the better. The example above illustrates this well; we filtered out only the new event, and thus there was no need for alternate fingerprints. TODO explain this better

.. note:: As a rule of thumb, it is best to filter out specifically those aspects of the simulation which were affected by the change. The more specific the filtering, the better; if you filter specifically, the fingerprints will retain their sensitivity to a broad range of changes, but won't be sensitive to that specific change.

.. so why is this?

  **because if you filter specifically, then the fingerprints will retain their broad sensitivity, but wont be sensitive
  to the specific change**

  what i wanna say is

  - when filtering out something, the best is to filter out very specifically the part of the model which the change affects
  - the more specifically one can filter that, the better
  - because then, only that aspect is filtered, and everything else is not
  - like in the case of the example above...we only filtered out the event which we added. dont need new fingerprints.
