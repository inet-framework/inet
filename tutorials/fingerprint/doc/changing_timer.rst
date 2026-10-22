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

We run the simulation and filter the events of the timer. We use the ``inet`` command in the tutorial directory.
The fingerprint ``0000-0000/tplx`` is a dummy value. It makes the run fail, and the run prints the calculated value:

.. code-block:: bash

  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=0000-0000/tplx --fingerprint-events='"not name=~removeNonInterferingTransmissions"'

.. code-block:: fp

  Fingerprint mismatch! calculated: XXXX-XXXX/tplx, expected: 0000-0000/tplx

Note the calculated value. The model did not change. The filter changed the set of events that the fingerprint uses.
The command does not change the store, so the stored ``tplx`` values stay as they are.

Now, we change the timer:

.. literalinclude:: ../sources/RadioMedium.cc.mod
   :diff: ../sources/RadioMedium.cc.orig

We run the simulation again with the same filter and the calculated fingerprint:

.. code-block:: bash

  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tplx --fingerprint-events='"not name=~removeNonInterferingTransmissions"'

.. code-block:: fp

  Fingerprint successfully verified

The model is verified. The fingerprints can be calculated without filtering. Then the update command records the new values in the store. This process is described in more detail in the :doc:`accepting` step.
