TCP Algorithms
~~~~~~~~~~~~~~

The algorithm of a TCP connection is a module now: each connection creates it as its submodule
``algorithm``, of the NED type that the ``tcpAlgorithmClass`` parameter names. An algorithm
implemented outside INET needs these changes:

- Register the class with ``Define_Module()`` instead of ``Register_Class()``.
- Add a simple module type that extends ``TcpAlgorithm``, with the C++ class in its ``@class``
  property. The value of ``tcpAlgorithmClass`` names this type: a name in the package
  ``inet.transportlayer.tcp.flavours``, or the qualified name of a type in another package. Without
  the type, the connection stops with an error.
- Rename the override of ``initialize()`` to ``initializeAlgorithm()``. The connection calls it in
  its own context, so the timers that it creates belong to the connection. ``initialize()`` is now
  the initialization of the module.
- Rename the calls of the ``cancelEvent()`` helper of ``TcpAlgorithmBase`` to ``cancelTimer()``.
  Delete a timer with ``conn->deleteTimer()``, because the connection owns it.
- Move the parameters of the algorithm from ``Tcp`` to the new type, and read them with ``par()``.

For example:

.. code-block:: ned

   import inet.transportlayer.tcp.flavours.TcpAlgorithm;

   simple MyTcpAlgorithm extends TcpAlgorithm
   {
       parameters:
           @class(MyTcpAlgorithm);
           double gain = default(0.5);
   }

The parameters of CUBIC, HyStart and DCTCP moved to ``TcpCubic`` and ``DcTcp``, so their ini keys
change. For example, ``**.tcp.cubicBeta`` is now ``**.tcp.conn-*.algorithm.beta``, and
``**.tcp.dctcpGamma`` is now ``**.tcp.conn-*.algorithm.g``. WHATSNEW lists all of them.
