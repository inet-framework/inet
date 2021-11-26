Adding New Events - Part 2
==========================

.. Using an Alternative Fingerprint Calculator

When a change introduces new events to the simulation and breaks fingerprints, one option is to use an alternative fingerprint calculator instead of the default one.
INET's fingerprint calculator (:cpp:`inet::FingerprintCalculator`) extends the default calculator, and adds new ingredients that can be used alongside the default ones.
This calculator can be used to take into account only the communication between the network nodes when calculating fingerprints.
Thus protocol implementation details and events inside network nodes don't affect the fingerprints,
only the data of the packets sent between network nodes.

The fingerprint calculator has four new ingredients available:

- ``N``: Network node path in the module hierarchy
- ``I``: Network interface path in the module hierarchy; the superset of ``N``
- ``D``: Packet data
- ``~``: Filter events to network communication

.. note:: Both ``d`` and ``D`` are packet data ingredients; ``d`` includes the C++ packet data representation and meta-infos such as annotations, tags, and flags; ``D`` includes only the packet data in network byte order, as seen on the network.

To use the new ingredients, add the following line to the ``General`` configuration:

.. literalinclude:: ../omnetpp.ini
   :start-at: General
   :end-at: class
   :language: ini

Now, the new and the default ingredients can be mixed.

The ``~`` ingredient toggles filtering of events to those that are messages between two different network nodes, effectively limiting the set of events taking part in fingerprint calculation to network communication. Note that this behavior affects the default ingredients as well.

.. note:: If the ingredients contain only D (and not t), the fingerprints are defined/affected only by the order of the packets (and the data they contain), the timings are irrelevant.

To filter out the effects of newly added events, run fingerprints with ingredients which only take into account network communication (e.g. ~tID).

.. workflow: when deciding which fingerprint to use, a general rule of thumb is that, you should use the most sensitive fingerprint that you think will not change because of the updated model. i.e. it is not sensitive to the model change but sensitive to everything else. Should this go into the general section?

.. Note:: When deciding which fingerprint ingredients to use, a general rule of thumb is that you should use the most sensitive fingerprint that you think will not change because of the updated model, i.e. it is not sensitive to the model change but sensitive to everything else. In this step, we chose ``~tID``: this only takes into account messages between different network nodes; it uses the time, the interface full path and the data in network byte order to calculate the fingerprints.

Here is the workflow:

- Before making the change in the model, run the fingerprint with ``~tID`` ingredients only
- Make the changes in the model
- Run the fingerprint tests again

If fingerprint tests pass, there was no change in the communication between network nodes,
and the model can be assumed to be correct with respect to the data of the exchanged packets staying the same.

As a simplistic example, we will make the same change to the :ned:`Udp` module as in the previous step.
We will use only network communication ingredients to calculate fingerprints, and verify the model.

We replace the default ingredients with ``~tID`` in the .csv file:

.. code-block:: text

  .,        -f omnetpp.ini -c Ethernet -r 0,           5s,         aeb0-6fd3/~tID, PASS,
  .,        -f omnetpp.ini -c Wifi -r 0,               5s,         c1f4-8059/~tID, PASS,

Then we run the fingerprint tests:

.. code-block:: fp

  $ inet_fingerprinttest -m AddingNewEvents2
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED

.. **TODO** do we need the new value?

  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED: actual  'b47f-d0db/~NID'

As there was no change in the model, the new fingerprints can be accepted:

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

We make the change:

.. literalinclude:: ../sources/Udp_mod.cc
   :diff: ../sources/Udp_orig.cc

We run the fingerprint tests again:

.. code-block:: fp

  $ inet_fingerprinttest -m AddingNewEvents2
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS

After making the change, the fingerprint tests pass, thus the model can be assumed correct.

.. pittfalls

It might turn out that the selected fingerprint ingredients are too sensitive. For example, the ``~tID`` ingredients take into account the interfaces between which the packets pass. If due to the model change, a packet uses another interface of a host with multiple interfaces, but the data and the source and destination hosts stay the same, the model might be valid, but the fingerprint tests don't pass. Using ``~tND`` instead would not be sensitive to the interfaces, and the tests would pass.

When the model is verified, we can change the ingredients back to ``tplx`` (or some other default), re-run the tests, and accept the new values. This process is described in more detail in the :doc:`accepting` step.
