Renaming a Module Parameter
===========================

.. 1. change or action
   2. phenomenon / effect
   3. solution
   4. example

The renaming of NED parameters can also cause regression; the parameter might be used by derived modules;
a parameter setting in a derived module might not have the effect it had before; forgetting to update ini keys can also cause problems.

The following is a simplistic example for module parameter renaming causing a regression. The :ned:`Router` module sets the :par:`forwarding` parameter to ``true`` which it inherits from :ned:`NetworkLayerNodeBase`. The latter uses the parameter to enable forwarding in its various submodules, such as :ned:`Ipv4` and :ned:`Ipv6`:

.. code-block:: ned
   :emphasize-lines: 4

   module Router extends ApplicationLayerNodeBase
   {
    parameters:
        forwarding = true;

.. code-block:: ned
   :emphasize-lines: 4

   module NetworkLayerNodeBase extends LinkLayerNodeBase
   {
    parameters:
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;

In :ned:`Ipv4NetworkLayer`, we rename the ``forwarding`` parameter to ``unicastForwarding`` to make it similar to ``multicastForwading``:

.. literalinclude:: ../sources/Ipv4NetworkLayer.ned.forwarding.modified
   :diff: ../sources/Ipv4NetworkLayer.ned.orig

The :ned:`Ipv4NetworkLayer` module sets the ``forwarding`` parameters in its :ned:`Ipv4` submodule to the value of ``unicastForwarding``.
Now, the ``forwarding = true`` key in :ned:`Router` doesn't take effect in the router's submodules,
and the router doesn't forward packets. This change causes the fingerprint tests/``tplx`` fingerprint tests to fail:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingParameter
  . -f omnetpp.ini -c Ethernet -r 0  ... : FAILED

To correct the model, the renaming needs to be followed everywhere. When we change the deep parameter assignment of the ``forwarding`` parameter in :ned:`NetworkLayerNodeBase` to ``unicastForwarding``, the fingerprint tests pass:

.. When we rewrite the parameter nesting? in networklayernodebase...

.. **TODO**

.. literalinclude:: ../sources/NetworkLayerNodeBase.ned.forwarding
   :diff: ../sources/NetworkLayerNodeBase.ned.orig

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingParameter
  . -f omnetpp.ini -c Ethernet -r 0  ... : PASS

This time we don't have to accept the fingerprint changes, because they didn't change.

.. note:: Before trying the following example, reset the model changes of the previous example.

Renaming module parameters can also lead to ERROR in the fingerprint tests. For example, we rename the ``queue`` submodule to ``txQueue`` in `EtherMacFullDuplex.ned`:

.. literalinclude:: ../sources/EtherMacFullDuplex.ned.mod
   :diff: ../sources/EtherMacFullDuplex.ned.orig

When run the ``tplx`` fingerprint tests, they result in ERROR:

.. code-block:: fp

  $ inet_fingerprinttest -m RenamingParameter
  . -f omnetpp.ini -c Ethernet -r 0  ... : ERROR

This error message was omitted from the above output for simplicity:

.. code-block:: text

  Error: check_and_cast(): Cannot cast nullptr to type 'inet::queueing::IPacketQueue *'
  -- in module (inet::EtherMacFullDuplex) RegressionTestingTutorialWired.router1.eth[0].mac (id=58),
  during network initialization

.. **TODO** network name!

.. When the model is verified, we can change the ingredients back to ``tplx`` (or some other default), re-run the tests, and accept the new values. This process is described in more detail in the :doc:`accepting` step.

.. ...

.. ha átkapcsolod unicastForwarding = false in Router -> it keeps working

   -> because -> Ipv4RoutingTable has a default for forwarding -> but we set the unicastForwarding

   command lineból is elég
