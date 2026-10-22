Accepting Fingerprint Changes
=============================

After you validate a set of changes with the methods of this tutorial, update the stored fingerprints.
The new values are the baseline for later changes. The tests compare new changes against them.

The temporary runs of the previous steps do not change the store.
They use the ``inet`` command with ingredients or event filters of their own.
The store keeps the values for the ``tplx`` ingredients. These ingredients have the right sensitivity to a broad range of changes.
They are suitable for the detection of regressions.

First, run the fingerprint tests with the ``tplx`` ingredients:

.. code-block:: bash

  $ opp_run_fingerprint_tests --load @opp -p inet -w tutorials/fingerprint --ingredients tplx -c '^(Ethernet|Wifi)$'

.. code-block:: fp

  tutorials/fingerprint -c Ethernet for 0.2s FAIL (unexpected) (Fingerprint mismatch) calculated XXXX-XXXX/tplx (correct b7af-fca4/tplx)
  tutorials/fingerprint -c Wifi for 5s FAIL (unexpected) (Fingerprint mismatch) calculated XXXX-XXXX/tplx (correct daf0-4199/tplx)

The tests fail because the stored values are from before the change.
The model is validated, so the new values are correct. Record them in the store:

.. code-block:: bash

  $ opp_update_fingerprint_test_results --load @opp -p inet -w tutorials/fingerprint --ingredients tplx -c '^(Ethernet|Wifi)$'

.. code-block:: fp

  tutorials/fingerprint -c Ethernet for 0.2s UPDATE XXXX-XXXX/tplx
  tutorials/fingerprint -c Wifi for 5s UPDATE XXXX-XXXX/tplx

The command changes the file ``tests/fingerprint/store.json``.
Use ``git diff tests/fingerprint/store.json`` to see the new values.

Run the tests again to check that the fingerprints are stable. Sometimes each run gives different values.
For example, this happens when the simulation trajectory depends on the memory layout.
Iteration on a ``std::map`` of object pointers can cause this.

When we run the tests again, they pass:

.. code-block:: fp

  tutorials/fingerprint -c Ethernet for 0.2s PASS
  tutorials/fingerprint -c Wifi for 5s PASS

If the fingerprints are not stable, something is wrong with the model.
