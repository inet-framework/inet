Renaming a Submodule
====================

Renaming submodules can cause the fingerprints to change, because the default ingredients (``tplx``) contain the full module path, thus the submodule name as well. Renaming submodules can cause regression in some cases, e.g. when functionality depends on submodule names (e.g. submodules referring to each other).

To go around the problem of fingerprints failing due to renaming, the fingerprints need to be calculated without the full path:

- Before making the change, run the simulations without the full path ingredient
- Note the new fingerprints; they are correct because the model didn't change
- Perform renaming
- Run the simulations again with the new fingerprints; the new fingerprints are not sensitive to changes in submodule names

.. note:: To run fingerprints with ``tlx``, delete the ``p`` from the ingredients. Use the ``inet`` command in the tutorial directory. The command does not change the store.

Here is the workflow demonstrated using a simplistic example:

We run the simulations with the ``tlx`` ingredients. The fingerprint ``0000-0000/tlx`` is a dummy value.
It makes the run fail, and the run prints the calculated value:

.. code-block:: bash

  $ inet -u Cmdenv -c Ethernet -r 0 --sim-time-limit=0.2s --fingerprint=0000-0000/tlx
  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=0000-0000/tlx

.. code-block:: fp

  Fingerprint mismatch! calculated: XXXX-XXXX/tlx, expected: 0000-0000/tlx
  Fingerprint mismatch! calculated: XXXX-XXXX/tlx, expected: 0000-0000/tlx

As expected, they fail because the dummy value is wrong.
Note the two calculated values. There was no change in the model, so they are correct.

.. **TODO** show that if we run the tests again they pass -> just for now, not needed later

When we run the simulations again with the calculated values, they pass:

.. code-block:: bash

  $ inet -u Cmdenv -c Ethernet -r 0 --sim-time-limit=0.2s --fingerprint=XXXX-XXXX/tlx
  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tlx

.. code-block:: fp

  Fingerprint successfully verified
  Fingerprint successfully verified

As a simplisic example, we rename the ``eth`` module vector to ``ethernet`` in :ned:`LinkLayerNodeBase` and :ned:`NetworkLayerNodeBase`. This change affects all host-types such as :ned:`StandardHost` and :ned:`AdhocHost` since they are derived modules:

.. literalinclude:: ../sources/LinkLayerNodeBase.ned.modified
   :diff: ../sources/LinkLayerNodeBase.ned.orig

.. literalinclude:: ../sources/NetworkLayerNodeBase.ned.modified
   :diff: ../sources/NetworkLayerNodeBase.ned.orig

We run the simulations again with the same fingerprints:

.. code-block:: bash

  $ inet -u Cmdenv -c Ethernet -r 0 --sim-time-limit=0.2s --fingerprint=XXXX-XXXX/tlx
  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tlx

.. code-block:: fp

  Fingerprint successfully verified
  Fingerprint successfully verified

As expected, the fingerprints didn't change, so we can assume the model is correct.

.. TODO: change to tplx, rerun, and accept (show all steps)

Now, we can run the update command with the ``tplx`` ingredients to record the new values in the store (described in more detail in the :doc:`accepting` step).

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

We run the simulation with the ``tlx`` fingerprint:

.. code-block:: bash

  $ inet -u Cmdenv -c Wifi -r 0 --sim-time-limit=5s --fingerprint=XXXX-XXXX/tlx

The simulation finished with an error, because the :ned:`Ipv4NetworkConfigurator` module was looking for the routing table module by the original name, and couldn't find it.

The simulation gives the following error message:

.. code-block:: text

  Error: Module not found on path 'RegressionTestingTutorialWireless.host1.ipv4.routingTable'
  defined by par 'RegressionTestingTutorialWireless.host1.ipv4.configurator.routingTableModule'
  -- in module (inet::Ipv4NodeConfigurator) RegressionTestingTutorialWireless.host1.ipv4.configurator
  (id=83), during network initialization

.. **TODO** example for FAILED -> later

.. When the model is validated, we can change the ingredients back to ``tplx`` (or some other default), re-run the tests, and accept the new values. This process is described in more detail in the :doc:`accepting` step.
