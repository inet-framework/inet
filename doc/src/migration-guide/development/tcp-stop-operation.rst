The Stop of TCP
~~~~~~~~~~~~~~~

When a node stops, :ned:`Tcp` closes each connection that its application
left open with a FIN, and its stage of the stop waits until each connection
has its data and its FIN acknowledged, at most :par:`stopOperationTimeout`
(default 2s). Before, it deleted all connections at once, without a segment.
A simulation that stops nodes with open TCP connections can therefore give
other results: the peers receive the FINs, and the layers below TCP stay up
during the wait.

To keep the old behavior, delete the connections at once:

.. code-block:: ini

   **.tcp.stopOperationTimeout = 0s

:par:`stopOperationTimeout` now bounds only the wait for the connections, and
:par:`stopOperationExtraTime` follows the wait in full. A crash
(``ModuleCrashOperation``) does not change: it deletes the connections at
once.
