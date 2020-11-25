Changing a Timer
================

..   TODO: find a module where we can change the interval of a timer
  e.g. RadioMedium removeNonInterfering...
  to change interval: change maxTransmissionDuration in mediumLimitCache

  TODO: if you filter out this message, recompute fingerprints before changing, do the change and the tests PASS, then accept, etc.

  filter out how ? by message name or by radio medium module

Some changes in a model's timers might break fingerprints, but otherwise wouldn't invalidate the model, such as timers which don't affect the model's behavior.
However, the change in the timers might break the default ingredient fingerprint tests.
In such case, the events or modules that contain the timer need to be filtered:

.. **TODO** we know that it doesnt affect the model

- Before making the change, filter the events or modules which involve the affected timer, and calculate the fingerprints
- Accept the new fingerprint values
- Make the change
- Rerun the fingerprint tests with the same filtering

The tests should pass.

As a simplistic example, we'll change how the ``removeNonInterferingTransmissionsTimer`` is scheduled in ``RadioMedium.cc``.
The timer deletes transmissions that no longer cause any interference (e.g. they have already left the vicinity of network nodes).

We run the fingerprint tests and filter the events in the ``RadioMedium`` module:

.. code-block:: fp

  $ inet_fingerprinttest -m ChangingTimer -a --fingerprint-events='"not name=~removeNonInterferingTransmissions"'
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED

The tests fail because the set of events used to calculate them changed.
Since there was no change in the model, just in how the fingerprints are calculated, the .csv file can be updated with the new values:

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

Now, we change the timer:

.. literalinclude:: ../sources/RadioMedium.cc.mod
   :diff: ../sources/RadioMedium.cc.orig

We re-run the fingerprint tests:

.. code-block:: fp

  $ inet_fingerprinttest -m ChangingTimer -a --fingerprint-events='"not name=~removeNonInterferingTransmissions"'
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS

The model is verified. The fingerprints can be calculated without filtering, and the .csv file can be updated with the new values. This process is described in more detail in the :doc:`accepting` step.
