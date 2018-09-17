:orphan:

Scenario Scripting
==================

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
   :scale: 100%
   :align: center

The network contains ``destinationNode`` and a vector of ``sourceNodes``'s. The number of ``sourceNodes``
is initially set to zero, because the nodes are created with the help of the scenario scripting
during the simulation.

The layout of the wired simulation looks like the following:

.. figure:: Wired_layout.png
   :scale: 100%
   :align: center

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
   :end-at: *.scenarioManager.script = xmldoc("wireless.xml")

The ``scenario.xml`` file contains the script for the wireless simulation.

Note that is life-cycle modeling is required, the following line must be added to the
ini file to ensure that nodes have status modules:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # lifecycle
   :end-at: **.hasStatus = true

Two :ned:`DynamicHost`:s, ``sourceNode[0]`` and ``sourceNode[1]`` are created at
the beginning of the simulation. This is achieved as the following in the XML file:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="6.0">

As you can see, the ``<create-module>`` element has four attributes, one for the type,
one for the submodule name, one for the parent module and one for indicating that it is a vector of modules.
We create a vector of modules named ``sourceNode`` with the type of the previously mentioned :ned:`DynamicHost`.
The created modules will have a random position inside the parent module.
The parent module in this case is the playground, because the ``parent`` attribute is set to is ``"."``.

The ``destinationNode`` is shut down at ``t=6.0`` and remains so for two seconds. During this, the ping request messages
can not reach it. To achieve this behavior, two life-cycle operations need to be used. With the ``<shutdown>`` element
a module, given in the ``module`` attribute, can be shut down. This operation represents the process of orderly
shutting down a network node.
It is recommended to first initiate the shut down operation
with the ``<initiate>`` attribute. The ``destinationNode`` is then started after two seconds using the ``<startup>``
element. The ``<startup>`` operation represents the process of turning on a network node after a shutdown or
crash. Here you can see the parts in question of  the scrip in the XML file:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="6.0">
   :end-before: <at t="10.0">

Another life-cycle operation to be mentioned is the crash of a node, which can be achieved with the
``<crash>`` element. This operation represents the process of crashing a network node. The
difference between this operation and shutdown operation is that the
network node will not do a graceful shutdown (e.g. routing protocols will
not have chance of notifying peers about broken routes). In this simulation,
``sourceNode[0]`` crashes due to a theoretical malfunction, so that it
does not send ping request messages anymore:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="10.0">
   :end-before: <at t="12.0">

At the end of the simulation, both ``sourceNode[0]`` and ``sourceNode[1]`` are
destroyed using the ``<delete-module>`` element:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="12.0">
   :end-before: </scenario>

We can simply confirm that these operations truely took place during the simulation
if we take a look at the statistics. The following image shows the
``radioMode`` of the destination node:

.. figure:: RadioMode.png
   :scale: 100%
   :align: center

We can clearly see for example that ``destinationNode`` was indeed shut down
for two seconds between ``t=6.0s`` and ``t=8.0s``, and that ``sourceNode[0]`` truly
crashed at ``t=10.0s``.

Wired
~~~~~

In this example simulation, there are neither dynamically created nor dynamically destroyed
modules, therefore the :ned:`Ipv4NetworkConfigurator` can be used.

All of the connections between the nodes (even the later dynamically created ones)
is set in the ned file. This needs to be done because the number of gates of a module can not
be modified after the initialization.

.. literalinclude:: ../scenMan.ned
   :language: ned
   :start-at: connections:
   :end-before: backboneRouter.ethg++ <--> Eth10M <--> destinationNode.ethg++;

At the beginning of the simulation, ``sourceNode2`` is
disconnected from the network. At ``t=3.0s`` the connection between the ``backboneRouter``
and the ``destinationNode`` modules is also destroyed. This can be done with the ``<disconnect>``
element. Note that if the ``src-gate`` parameter names and inout gate, both directions will be disconnected.

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="4.0">

While the ``destinationNode`` is disconnected from the network, the ping request
messages sent by ``sourceNode1`` can not reach it. A connection can be established (or
re-established in our case) between ``desstinationNode`` and ``backboneRouter`` using the ``<connect>`` element:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="4.0">
   :end-before: <at t="6.0">

After six second, a connection between ``sourceNode2`` and the network is established
and it starts to send the ping requests to ``destinationNode`` as well:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="6.0">
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

We can also change the parameters of the nodes during the simulation with the help of the
<set-param> element. In this case, the :par:`sendInterval` parameter of ``sourceNode1`` is modified:

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

We can see that the desired actions were indeed executed by the :ned:`ScenarioManager`
during the simulation, if we take a look at the ``transmissionState`` statistic
of ``backboneRouter``'s PPP interface:

.. figure:: TransmissionState.png
   :scale: 100%
   :align: center

The plot clearly shows the time intervals when the ping request messages (for some reason)
could not be forwarded to ``destinationNode``. From the density of the state changes we can also see
when the :par:`sendInterval` parameter of ``sourceNode1`` was modified to a smaller value than it was before.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
