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

| Since INET version: ``4.6``
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
:download:`WebserverShowcase.ned <../WebserverShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/emulation/webserver`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-latest --options=inet:full
    $ cd inet-workspace
    $ opp_env shell

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and opens an interactive shell. (The
``--options=inet:full`` argument is required to enable the Emulation feature in
opp_env.)

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

To experiment with the emulation examples, navigate to the
``inet/showcases/emulation/webserver`` directory. From there, you can execute the
commands outlined in the previous sections.

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.