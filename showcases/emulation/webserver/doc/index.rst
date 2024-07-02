Webserver Emulation Showcase
============================

Goals
-----

The goal of this showcase is to demonstrate the integration and operation of a
real-world application within an OMNeT++/INET simulation environment.
Specifically, it involves running a Python-based webserver and executing several
`wget` commands from host operating system environments. These are interfaced with
simulated network components, illustrating the capabilities of OMNeT++ in hybrid
simulations that involve both emulated and simulated network elements.

| INET version: ``4.6``
| Source files location: `inet/showcases/emulation/webserver <https://github.com/inet-framework/inet/tree/master/showcases/emulation/webserver>`__


The Model
---------

The showcase uses the following network:

.. figure:: media/Network.png

Here is the NED definition of the network:

.. literalinclude:: ../WebserverShowcase.ned
   :start-at: WebserverShowcase
   :language: ned

This network consists of a server connected to an Ethernet switch, along with a
configurable number of clients also connected to the switch. The server will run
a real Python webserver, and the clients will perform HTTP GET requests using
`wget`.


Configuration
-------------

The behavior of the model is controlled by the following INI file configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

This configuration sets up the real-time scheduler to synchronize the simulation
time with the wall clock time, allowing interaction between the simulated model
and real applications. It also specifies the number of clients in the simulation
and various other network settings.


Results
-------

Upon running the simulation, the webserver on the host OS will respond to HTTP
GET requests initiated by the `wget` commands from the simulated clients. The
interaction can be observed in the simulation's event log, and network
performance metrics such as response time and throughput can be analyzed based
on the simulation results.

The following terminal screenshot shows the running webserver and `wget` processes:

.. figure:: media/ps.png

The output of the processes appears in the Qtenv log window. Here is the output of one of the `wget` commands:

.. figure:: media/wget_module_log.png

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`WebserverShowcase.ned <../WebserverShowcase.ned>`,

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.