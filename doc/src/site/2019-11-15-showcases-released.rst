:orphan:
:date: 2019-11-15
:category: news

==================================================================================================================================
New Emulation Showcases: Testing a Linux Routing Daemon in a Simulated Environment; Using Simulated Applications in a Real Network
==================================================================================================================================

Testing a Linux Routing Daemon in a Simulated Environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Testing routing protocols with simulation is often easier than building a test
setup in the real world. INET’s emulation features make it possible to use a
real-world routing protocol implementation in a simulated environment. The simulated
environment and multiple instances of the real-world routing protocol implementation
can run on the same computer. This showcase demonstrates such a test scenario with
the Linux implementation of the Babel routing protocol (Babel daemon).

Using Simulated Applications in a Real Network
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this showcase, we show how a simulated application can be used as a real application
that communicates over the (real) network. Being able to do so opens a lot of possibilities.
For example, you can deploy an application that only exists as a simulation model on real nodes,
and test its behavior over a real network. Or, by letting the model talk to real-world
implementations of the same application, you can test its interoperability and validate its behavior.

Links:

- `Testing a Linux Routing Daemon in a Simulated Environment <https://inet.omnetpp.org/docs/showcases/emulation/babel/doc/>`__
- `Using Simulated Applications in a Real Network <https://inet.omnetpp.org/docs/showcases/emulation/voip/doc/>`__
