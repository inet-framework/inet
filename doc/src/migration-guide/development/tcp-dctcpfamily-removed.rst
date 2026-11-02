The Class DcTcpFamily
~~~~~~~~~~~~~~~~~~~~~

The C++ class ``DcTcpFamily`` and its header ``DcTcpFamily.h`` are removed.
The class had no NED type, and no class of INET derived from it:
:ned:`DcTcp` derives from ``TcpReno``. The state variables
``DcTcpFamilyStateVariables`` stay, in
``inet/transportlayer/tcp/flavours/DcTcpFamilyState_m.h``.

A class that derived from ``DcTcpFamily`` derives from
``TcpClassicAlgorithmBase`` instead, includes ``DcTcpFamilyState_m.h``, and
keeps the alias of the state variables itself, as ``DcTcpFamily`` did:

.. code-block:: c++

   // in the class
   DcTcpFamilyStateVariables *& state; // alias to TcpAlgorithm's 'state'

   // in the constructor
   MyDcTcp::MyDcTcp() : TcpClassicAlgorithmBase(),
       state((DcTcpFamilyStateVariables *&)TcpClassicAlgorithmBase::state)
   {
   }
