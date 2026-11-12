Overriding or calling ``Icmpv6::createParamProblemMsg()``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`Icmpv6::sendErrorMessage()` and :cpp:`Icmpv6::createParamProblemMsg()`
take the Pointer field of an Internet Control Message Protocol for IPv6
(ICMPv6) Parameter Problem message as a new argument:

.. code-block:: c++

   virtual void sendErrorMessage(Packet *datagram, Icmpv6Type type, int code, int mtu = 0, int pointer = 0);
   virtual Packet *createParamProblemMsg(Icmpv6ParameterProblem code, int pointer);

In a subclass of :cpp:`Icmpv6`, an override with the old parameter list no
longer overrides the method: it fails to compile if it is marked ``override``,
and it is never called otherwise. Add the argument to the override. A subclass
that calls :cpp:`createParamProblemMsg()` must pass the pointer; pass 0 to keep
the old message. Callers of :cpp:`sendErrorMessage()` need no change.
