Removing Events
===============

.. When a change results in events being removed, the fingerprints change

  so

  - some changes cause events to be removed
  - this changes fingerprints, just like with the new events
  - cos the set of events changes
  - so need to do the same as described in the previous sections
  - we'll remove the removeInterferingTransmissions message
  - we'll use in the example the filtering of the event which is eventually removed
  - for that we need new fingerprints that doesn't contain the event eventually removed
  - the workflow is
  - create the fingerprints without the event
  - make the change
  - run the tests again

Some changes in the model removes events from the simulation, resulting in changed fingerprints, but doesn't necessarily invalidate the model. The methods described in the previous steps involving new events are applicable in this case as well.
The workflow is the following:

- Create fingerprints which are not sensitive to the removal of the event/doesnt contain the event (such as filter the module the event takes place in, use ingredients which only take into account network communication, or filter only the event itself)
- Make the change in the model
- Run the fingerprint tests again

As a simplistic example, we'll delete the ``removeNonInterferingTransmissions`` self message from the :ned:`RadioMedium` module. This message schedules a timer to remove those transmissions from the radio medium which can no longer cause any interference. This change doesn't alter the simulation, but deletes events from it, thus it would change the baseline fingerprints.

.. **TODO** this wont alter the model

.. **TODO** we'll use the event filtering

First, we create alternate fingerprints by filtering out the ``removeNonInterferingTransmissions`` events:

.. code-block:: fp

  $ inet_fingerprinttest -m RemovingEvent -a --fingerprint-events='"not fullName=~removeNonInterferingTransmissions"'
  . -f omnetpp.ini -c Wifi -r 0 ... : FAILED

Now, we can update the .csv file with the new fingerprints:

.. code-block:: fp

  $ mv baseline.csv.UPDATED baseline.csv

We remove the ``nonInterferingTransmissions`` message from the code of ``RadioMedium.cc``; we delete the declaration, and all instances of its scheduling.
We build INET and run the fingerprint tests again with the filter expression:

.. code-block:: fp

  $ inet_fingerprinttest -m RemoveEvent -a --fingerprint-events='"not fullName=~removeNonInterferingTransmissions"'
  . -f omnetpp.ini -c Wifi -r 0 ... : PASS

.. **TODO** accepting

We can restore the baseline fingerprints by re-running the tests without filtering and accept the new values. This process is described in more detail in the :doc:`accepting` step.

.. .. literalinclude:: ../sources/RadioMedium.cc.removeevent
   :diff: ../sources/RadioMedium.cc.orig

.. **TODO** this is too long, not sure its needed

.. user@legendre:~/Integration/inet/tutorials/fingerprint$ inet_fingerprinttest removeevent.csv -a --fingerprint-events='"not fullName=~removeNonInterferingTransmissions"'
  . -f omnetpp.ini -c Ethernet -r 0 ... : PASS
  . -f omnetpp.ini -c EthernetShortPacket -r 0 ... : PASS
  . -f omnetpp.ini -c Wifi -r 0 ... : PASS
  . -f omnetpp.ini -c WifiShortPacket -r 0 ... : PASS
