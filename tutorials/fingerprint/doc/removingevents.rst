Removing Events
===============

.. When a change results in events being removed, the fingerprints change

  so

  - some changes cause events to be removed
  - this changes fingerprints, just like with the new events
  - because the set of events changes
  - so need to do the same as described in the previous sections
  - we'll remove the removeInterferingTransmissions message
  - we'll use in the example the filtering of the event which is eventually removed
  - for that we need new fingerprints that don't contain the event eventually removed
  - the workflow is
  - create the fingerprints without the event
  - make the change
  - run the tests again

Some changes in the model remove events from the simulation, resulting in changed fingerprints, but don't necessarily invalidate the model. The methods described in the previous steps involving new events are applicable in this case as well.
The workflow is the following:

- Create fingerprints which are not sensitive to the removal of the event/doesn't contain the event (such as filter the module the event takes place in, use ingredients which only take into account network communication, or filter only the event itself)
- Make the change in the model
- Run the fingerprint tests again

As a simplistic example, we'll delete the ``removeNonInterferingTransmissions`` self message from the :ned:`RadioMedium` module. This message schedules a timer to remove those transmissions from the radio medium which can no longer cause any interference. This change doesn't alter the simulation, but deletes events from it, thus it would change the baseline fingerprints.

.. **TODO** this won't alter the model

.. **TODO** we'll use the event filtering

First, we create alternate fingerprints by filtering out the ``removeNonInterferingTransmissions`` events.
The fingerprint ``0000-0000/tplx`` is a dummy value. It makes the run fail, and the run prints the calculated value:

.. code-block:: bash

  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=0000-0000/tplx --fingerprint-events='"not fullName=~removeNonInterferingTransmissions"'

.. code-block:: fp

  Fingerprint mismatch! calculated: XXXX-XXXX/tplx, expected: 0000-0000/tplx

Note the calculated value. The command does not change the store.

We remove the ``nonInterferingTransmissions`` message from the code of ``RadioMedium.cc``; we delete the declaration, and all instances of its scheduling.
We build INET and run the simulation again with the same filter expression and the calculated fingerprint:

.. code-block:: bash

  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tplx --fingerprint-events='"not fullName=~removeNonInterferingTransmissions"'

.. code-block:: fp

  Fingerprint successfully verified

.. **TODO** accepting

We can restore the baseline fingerprints. Run the update command without filtering to record the new values in the store. This process is described in more detail in the :doc:`accepting` step.

.. .. literalinclude:: ../sources/RadioMedium.cc.removeevent
   :diff: ../sources/RadioMedium.cc.orig

.. **TODO** this is too long, not sure it's needed
