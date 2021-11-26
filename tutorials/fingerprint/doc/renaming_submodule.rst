Renaming a Submodule
====================

Renaming submodules can cause the fingerprints to change, because the default ingredients (``tplx``) contain the full module path, thus the submodule name as well. Renaming submodules can cause regression in some cases, e.g. when functionality depends on submodule names (e.g. submodules referring to each other).

To go around the problem of fingerprints failing due to renaming, the fingerprints need to be calculated without the full path:

- Before making the change, rerun fingerprint tests without the full path ingredient
- Update fingerprints; the new values can be accepted because the model didn't change
- Perform renaming
- Run fingerprint tests again; the new fingerprints are not sensitive to changes in submodule names

.. note:: To run fingerprints with ``tlx``, delete the ``p`` from the ingredient in the .csv file. When the fingerprint test is run again, the fingerprints will be calculated with the new ingredients. The tests will fail, as the values are still the ones calculated with ``tplx``, but it is safe to overwrite them with the updated values, as the model didn't change.

Here is the workflow demonstrated using a simplistic example:

We set the fingerprint ingredients to ``tlx`` in the .csv file:

.. code-block:: text

  .,        -f omnetpp.ini -c Ethernet -r 0,           5s,         a92f-8bfe/tlx, PASS,
  .,        -f omnetpp.ini -c Wifi -r 0,               5s,         5e6e-3064/tlx, PASS,

Then we run the fingerprint tests:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingSubmodule
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED (should be PASS)
  . -f omnetpp.ini -c Wifi -r 0  ... : FAILED (should be PASS)

As expected, they fail because the values are still the ones calculated with ``tplx``.
We can update the .csv file with the new values:

.. code-block:: fp

   $ mv baseline.csv.UPDATED baseline.csv

.. **TODO** show that if we run the tests again they pass -> just for now, not needed later

When we rerun the fingerprint tests, now they PASS:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingSubmodule
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS

As a simplisic example, we rename the ``eth`` module vector to ``ethernet`` in :ned:`LinkLayerNodeBase` and :ned:`NetworkLayerNodeBase`. This change affects all host-types such as :ned:`StandardHost` and :ned:`AdhocHost` since they are derived modules:

.. literalinclude:: ../sources/LinkLayerNodeBase.ned.modified
   :diff: ../sources/LinkLayerNodeBase.ned.orig

.. literalinclude:: ../sources/NetworkLayerNodeBase.ned.modified
   :diff: ../sources/NetworkLayerNodeBase.ned.orig

We run the fingerprint tests again:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingSubmodule
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS
  . -f omnetpp.ini -c Wifi -r 0  ... : PASS

As expected, the fingerprints didn't change, so we can assume the model is correct.

.. TODO: change to tplx, rerun, and accept (show all steps)

Now, we can change the ingredients back to ``tplx``, rerun the tests, and accept the new values (described in more detail in the :doc:`accepting` step).

.. **TODO** or show it here as well

However/In other cases, renaming submodules can lead to ERROR in the fingerprint tests, e.g. when modules cross-reference each other and look for other modules by name. Also, renaming can lead to FAILED fingerprint tests, because there might be no check in the model on cross-referencing module names.

.. **TODO** before doing the example, reset the previous change

.. .. note:: Before trying the following example, reset the previous change in the model.

.. note:: Before trying the following example, reset the model changes of the previous example.

.. **TODO**: show an example for FAIL and ERROR

Here is an example change which results in ERROR in the tests when using ``tlx`` fingerprints:

In :ned:`Ipv4NetworkLayer`, we rename the ``routingTable`` submodule to ``ipv4RoutingTable``:

.. literalinclude:: ../sources/Ipv4NetworkLayer.ned.routingtable.modified
   :diff: ../sources/Ipv4NetworkLayer.ned.orig

Here are the results of the fingerprint tests:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingSubmodule
  . -f omnetpp.ini -c Wifi -r 0  ... : ERROR (should be PASS)
  . -f omnetpp.ini -c Ethernet -r 0  ... : ERROR (should be PASS)

The simulations finished with an error, because the :ned:`Ipv4NetworkConfigurator` module was looking for the routing table module by the original name, and couldn't find it.

Note that we omitted the error messages from the above output to make it more concise.
The simulations give the following error message:

.. code-block:: text

  Error: Module not found on path 'RegressionTestingTutorialWireless.host1.ipv4.routingTable'
  defined by par 'RegressionTestingTutorialWireless.host1.ipv4.configurator.routingTableModule'
  -- in module (inet::Ipv4NodeConfigurator) RegressionTestingTutorialWireless.host1.ipv4.configurator
  (id=83), during network initialization

.. **TODO** example for FAILED -> later

.. When the model is validated, we can change the ingredients back to ``tplx`` (or some other default), re-run the tests, and accept the new values. This process is described in more detail in the :doc:`accepting` step.
