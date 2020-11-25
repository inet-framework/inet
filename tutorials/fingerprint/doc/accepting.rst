Accepting Fingerprint Changes
=============================

After successfully validating a set of changes using the methods demonstrated in this tutorial, the baseline fingerprints should be updated. The new baseline will be used in the future to validate new changes by comparing against.

For example, if the fingerprint ingredients have been changed, then it's advisable to revert the changes, because the default ingredients were selected to have the right amount of sensitivity to a broad range of changes, thus may be more suitable for detecting regressions.

To do that, replace the ingredients with ``tplx`` (or other chosen set of baseline ingredients):

.. code-block:: text

  .,        -f omnetpp.ini -c Ethernet -r 0,               5s,         a92f-8bfe/tplx, PASS,
  .,        -f omnetpp.ini -c EthernetShortPacket -r 0,    5s,         879f-5956/tplx, PASS,
  .,        -f omnetpp.ini -c Wifi -r 0,                   5s,         5e6e-3064/tplx, PASS,
  .,        -f omnetpp.ini -c WifiShortPacket -r 0,        5s,         7678-3e16/tplx, PASS,

Then run the fingerprint tests:

.. code-block:: fp

  $ inet_fingerprinttest
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED
  . -f omnetpp.ini -c WifiShortPacket -r 0  ... : FAILED
  . -f omnetpp.ini -c EthernetShortPacket -r 0  ... : FAILED

The tests fail, because the fingerprint values in the .csv file correspond to the previous set of ingredients or filtering. Since there was no change in the model since validation, it is safe to accept the new values/the new values can be accepted/it is safe to overwrite the .csv file with the new values:

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

It is advisable to re-run the tests to check whether the fingerprints are stable; i.e. it might happen that each run gives different values, e.g. when the simulation trajectory depends on memory layout (caused by iteration on std::map of object pointers, for example).

When we re-run the tests, they pass:

.. code-block:: fp

  $ inet_fingerprinttest fingerprintshowcase.csv
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS
  . -f omnetpp.ini -c WifiShortPacket -r 0  ... : PASS
  . -f omnetpp.ini -c EthernetShortPacket -r 0  ... : PASS

If the fingerprints are not stable, it indicates that something's wrong with the model.
