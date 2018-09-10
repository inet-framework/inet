:orphan:

Simulating Dynamic Networks
===========================

Goals
-----

In real life, networks are rarely static. In dynamic networks, the
topology changes over time, nodes may come and go, edges
may crash and recover. Regarding the simulation of a real
network, it is essential to be able to simulate the dynamics of the network.

This showcase demonstrates how dynamic networks can be simulated
using INET components.

INET version: ``4.0``

Source files location: `inet/showcases/general/dynamic <https://github.com/inet-framework/inet-showcases/tree/master/general/??>`__

About Scenario Scripting
------------------------

The INET :doc:`User's Guide </users-guide/ch-scenario-scripting>`
contains a good overview of Scenario Scripting in INET.
We recommend that you read
that first, because here we provide a brief summary only.

Scenario Scripting helps the user to simulate dynamic networks. It allows
to schedule actions to be carries out at specified simulation times. The
following built-in actions are supported by INET: creating or deleting modules
and connections, setting the parameters of modules and channels,
initiating life-cycle operations on a network node or part of it.
Life-cycle operations include startup, shutdown and crash.

The model
---------

The network
~~~~~~~~~~~

The showcase presents two example simulations:

-  Wireless: Simulation of a dynamic ad hoc network. Introduction of the creation and
   destruction of nodes, and of the usage of life-cycle operations.
-  Wired: Simulation of a dynamic wired network. Introduction of parameter setting and
   of the creation and deletion of connections.

The following image shows the layout of the network for the wireless simulation:

.. figure:: Wireless_layout.png
   :width: 100%

The network contains ``destinationNode`` and a vector of ``sourceNodes``'s. The number of ``sourceNodes``
is initially set to zero, because the nodes are created with the help of the scenario scripting
during the simulation.

The layout of the wired simulation looks like the following:

.. figure:: Wired_layout.png
   :width: 100%

It contains two ``sourceNode``'s and one ``destinationNode`` connected through two routers and a switch.

Both networks contain a :ned:`ScenarioManager` module, which executes a script specified in XML.
The XML script describes the actions to be executed during the course of the simulation,
i.e. which parameter should be changed and when, what nodes should be created or deleted and when, etc.

In both networks, the ``sourceNode`` modules send ping
request messages to the ``destinationNode`` modules. This way we can
compare the statistics with the desired actions.

Configuration and Results
-------------------------

Wireless
~~~~~~~~

The :ned:`DynamicHost` type is defined in the NED file, ``DynamicShowcase.ned``. It is basically an :ned:`AdhocHost`,
but it is configured to use a per-host :ned:`HostAutoConfigurator` module instead of the global :ned:`IPv4NetworkConfigurator`.
The reason for this is that :ned:`Ipv4NetworkConfigurator` doesn’t support IP address assignment
in scenarios, where modules are dynamically created and/or destroyed. Here is the NED definition of :ned:`DynamicHost`:

.. literalinclude:: ../scenMan.ned
   :language: ned
   :start-at: module DynamicHost extends AdhocHost
   :end-before: network DynamicShowcase

In the ini file, the :ned:HostAutoConfigurator`'s :par:`interface` parameter needs to be set to the hosts'
wlan interface.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # dynamic node IP address autoconfigurator
   :end-at: *.*Node*.autoConfigurator.interfaces = "wlan0"

The parameters of the dynamically created modules can be set from the ini file, just as with other modules.
The configuration of the dynamically created modules will take effect when they are spawn.
The key part of the configuration regarding this showcase is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # scenario script to create and destroy nodes dynamically
   :end-at: *.scenarioManager.script = xmldoc("scenario.xml")

The ``scenario.xml`` file contains the script for the wireless simulation. Two :ned:`DynamicHost`:s,
``sourceNode[0]`` and ``sourceNode[1]`` are created at the beginning of the simulation:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="6.0">

The created modules will have a random position inside the parent module.
The parent module in this case is the playground, because the parent module is ``"."``.

The ``destinationNode`` is shut down for four seconds, and during this, the ping request messages
can not reach it:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="6.0">
   :end-before: <at t="10.0">

After that, ``sourceNode[0]`` crashes before both modules are destroyed:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="10.0">
   :end-before: </scenario>

We can simply see how these operations took place during the simulation
if we take a look at specific statistics. The following image shows the
``radioMode`` of the destination node:

.. figure:: RadioMode.png
   :width: 100%

Wired
~~~~~

In this example simulation, there are neither dynamically created nor dynamically destroyed
modules, therefore the :ned:`Ipv4NetworkConfigurator` can be used.

All of the connections between the nodes (even the later dynamically created ones)
is set in the ned file:

.. literalinclude:: ../scenMan.ned
   :language: ned
   :start-at: connections:
   :end-before: backboneRouter.ethg++ <--> Eth10M <--> destinationNode.ethg++;

This needs to be done because the number of gates of a module can not
be modified after the initialization. At the beginning, ``sourceNode2`` is
disconnected from the network, and the connection between the ``backboneRouter``
and the ``destinationNode`` modules is also destroyed during the simulation:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="4.0">

While the ``destinationNode`` is disconnected from the network, the ping request
messages sent by ``sourceNode1`` can not reach it. After the connection between them
is re-established, the request can reach their destination again:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="4.0">
   :end-before: <at t="6.0">

After six second, a connection between ``sourceNode2`` and the network is established
and it starts to send the ping requests to ``destinationNode`` as well:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="4.0">
   :end-before: <at t="8.0">

Not only the malfunction of ``destinationNode`` or its connection can cause that the
ping request can not reach their target, but the routers can crash as well.

The malfunction of the routers has the same consequence as when ``destinationNode`` was
disconnected from the network. In this simulation, the router crashes for two seconds before
it is restarted:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="8.0">
   :end-before: <at t="10.0">

We can also change the parameters of the nodes during the simulation. In this case, the
:par:`sendInterval` parameter of ``sourceNode1`` is modified:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <set-param t="12.0" module="sourceNode1.app[0]" par="sendInterval" value="0.02s"/>
   :end-before: <at t="14.0">

At the end of the simulation, both ``sourceNode1`` and ``sourceNode2`` are shut down
and disconnected from the network:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="14.0">
   :end-before: </scenario>

We can see that the desired actions were actually executed by the ``ScenarioManager``
during the simulation, if we take a look at the ``transmissionState`` statistic
of ``backboneRouter``'s PPP interface:

.. figure:: TransmissionState.png
   :width: 100%

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
