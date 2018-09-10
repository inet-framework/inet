:orphan:

Scenario Scripting
==================

Goals
-----

In real life, networks are rarely static. In dynamic networks, the
topology changes over time, nodes may come and go, edges
may crash and recover. Regarding the simulation of a real
network, it is essential to be able to simulate the dynamics of the network.

This showcase demonstrates how scenario scripting can be used to simulate dynamic networks.

INET version: ``4.0``

Source files location: `inet/showcases/general/dynamic <https://github.com/inet-framework/inet-showcases/tree/master/general/??>`__

About Scenario Scripting
------------------------

The INET :doc:`User's Guide </users-guide/ch-scenario-scripting>`
contains a good overview of Scenario Scripting in INET.
We recommend that you read
that first, because here we provide a brief summary only.

Scenario Scripting allows
to schedule actions to be carried out at specified simulation times. The
following built-in actions are supported by INET: creating or deleting modules
and connections, setting the parameters of modules and channels,
initiating lifecycle operations on a network node or part of it.
lifecycle operations include startup, shutdown and crash.

The model
---------

The network
~~~~~~~~~~~

The showcase presents two example simulations:

-  Wireless: Simulation of a dynamic ad hoc network. Introduction of the creation and
   destruction of nodes, and of the usage of lifecycle operations.
-  Wired: Simulation of a dynamic wired network. Introduction of parameter setting and
   of the creation and deletion of connections.

The following image shows the layout of the network for the wireless simulation:

.. figure:: media/Wireless_layout.png
   :scale: 100%
   :align: center

The network contains only a ``destinationNode`` module, because the other nodes, the
``sourceNode`` modules, are created during the simulation with the help of scenario scripting.

The layout of the wired simulation looks like the following:

.. figure:: media/Wired_layout.png
   :scale: 100%
   :align: center

It contains two ``sourceNode`` and one ``destinationNode`` modules connected through two routers and a switch.

Both networks contain a :ned:`ScenarioManager` module, which executes a script specified in XML.
The XML script describes the actions to be carried out during the course of the simulation,
i.e. which parameter should be changed and when, what nodes should be created or deleted and when, etc.

In both networks, the ``sourceNode`` modules send ping
request messages to the ``destinationNode`` modules. This way we can
compare the statistics with the desired actions.

Configuration and Results
-------------------------

Wireless
~~~~~~~~

The :ned:`Ipv4NetworkConfigurator` doesn’t support IP address assignment
in scenarios, where modules are dynamically created and/or destroyed.
Therefore, the hosts in this example simulation are of the type :ned:`DynamicHost`.
It is basically an :ned:`AdhocHost`, but it is configured to use a per-host
:ned:`HostAutoConfigurator` module instead of the global :ned:`IPv4NetworkConfigurator`.
Here is the NED definition of :ned:`DynamicHost`:

.. literalinclude:: ../scenarioScripting.ned
   :language: ned
   :start-at: module DynamicHost extends AdhocHost
   :end-before: network DynamicShowcase

The :ned:`HostAutoConfigurator`'s :par:`interface` parameter defines the interfaces of the containing module,
to which IP addresses should be automatically assigned. In this example, it is the hosts' wlan interface:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # dynamic node IP address autoconfigurator
   :end-at: *.*Node*.autoConfigurator.interfaces = "wlan0"

The parameters of the dynamically created modules can be set from the ini file, just as if they would already exist.
The configuration of the dynamically created modules will take effect when they are spawn.
Here you can see the configuration of the ``sourceNode`` modules for sending ping request messages:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # ping app (host[0] pinged by others)
   :end-at: *.sourcenode1.app[0].startTime = 3.25s

Note that if lifecycle modeling is required, the following line must be added to the
ini file to ensure that nodes have status modules:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # lifecycle
   :end-at: **.hasStatus = true

This module keeps track of the status of network node (up, down, etc.) for other modules, and also displays it.

The key part of the configuration regarding this showcase is the configuration of the :ned:`ScenarioManager` module:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # scenario script for wireless example
   :end-at: *.scenarioManager.script = xmldoc("wireless.xml")

The ``wireless.xml`` file contains the script for the wireless simulation.
Two :ned:`DynamicHost` modules, ``sourceNode0`` and ``sourceNode1`` are created at
the beginning of the simulation. This is specified as the following in the XML file:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="6.0">

As you can see, the ``<create-module>`` element has three attributes, one for the type,
one for the submodule name and one for the parent module.
We create two modules named ``sourceNode0`` and ``sourceNode1``, both with the type of the previously mentioned :ned:`DynamicHost`.
The parent module in this case is the playground, so the ``parent`` attribute is set to is ``"."``.

The ``destinationNode`` is shut down at ``t=6.0`` and remains so for two seconds. During this, the ping request messages
can not reach it. To achieve this behavior, two lifecycle operations need to be used. The ``<shutdown>``
element represents the process of orderly shutting down a network node.
The ``destinationNode`` is then started after two seconds using the ``<startup>`` element.
The ``<startup>`` operation represents the process of turning on a network node after a shutdown or crash.
Note that the ``<startup>`` and ``<shutdown>`` lifecycle operations are equal to the ``<initiate>`` operation used with the
``operation`` attribute set to either ``"startup"`` or ``"shutdown"``.
Here you can see the parts in question of the scrip in the XML file:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="6.0">
   :end-before: <at t="10.0">

Another lifecycle operation to be mentioned is the crash of a node, which can be achieved with the
``<crash>`` element.
The difference between this operation and shutdown operation is that the
network node will not do a graceful shutdown (e.g. routing protocols will
not have chance of notifying peers about broken routes). In this simulation,
``sourceNode0`` crashes due to a theoretical malfunction, so that it
does not send ping request messages anymore:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="10.0">
   :end-before: <at t="12.0">

At the end of the simulation, both ``sourceNode0`` and ``sourceNode1`` are
destroyed using the ``<delete-module>`` element:

.. literalinclude:: ../wireless.xml
   :language: xml
   :start-at: <at t="12.0">
   :end-before: </scenario>

We can simply confirm that these operations truely took place during the simulation
if we take a look at the statistics. The following image shows the
``radioMode`` of the destination node:

.. figure:: media/RadioMode.png
   :scale: 100%
   :align: center

We can clearly see, for example, that ``destinationNode`` was indeed shut down
for two seconds between ``t=6.0s`` and ``t=8.0s``, and that ``sourceNode0`` truly
crashed at ``t=10.0s``.

Wired
~~~~~

In this example simulation, there are neither dynamically created nor dynamically destroyed
modules, therefore the :ned:`Ipv4NetworkConfigurator` can be used.

All of the connections between the nodes (even the later dynamically created ones)
are set in the ned file. This needs to be done because the number of gates of a module can not
be modified after the initialization, if the :ned:`HostAutoConfigurator` is not used.

For this example simulation, the scenario is contained in the ``wired.xml`` file, so the
configuration of the :ned:`ScenarioManager` looks like the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # scenario script for wired example
   :end-at: *.scenarioManager.script = xmldoc("wired.xml")

At the beginning of the simulation, ``sourceNode2`` is
disconnected from the network. At ``t=3.0s`` the connection between the ``backboneRouter``
and the ``destinationNode`` modules is also destroyed. This can be achieved using the ``<disconnect>``
element. Note that if the ``src-gate`` parameter names an inout gate, both directions will be disconnected:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="0.0">
   :end-before: <at t="4.0">

While the ``destinationNode`` is disconnected from the network, the ping request
messages sent by ``sourceNode1`` can not reach it. A connection can be established (or
re-established in our case) between ``destinationNode`` and ``backboneRouter`` using the ``<connect>`` element:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="4.0">
   :end-before: <at t="6.0">

After six second, a connection between ``sourceNode2`` and the network is established
and it starts to send the ping request messages to ``destinationNode`` as well:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="6.0">
   :end-before: <at t="8.0">

Not only the malfunction of ``destinationNode`` or its connection can cause that the
ping request do not reach their target, but the routers can crash as well.
The crash of the routers has the same consequence as when ``destinationNode`` was
disconnected from the network. In this simulation, the router crashes for two seconds before
it is restarted:

.. literalinclude:: ../wired.xml
   :language: xml
   :start-at: <at t="8.0">
   :end-before: <set-param t="12.0" module="sourceNode1.app[0]" par="sendInterval" value="0.02s"/>

We can also change the parameters of the nodes during the simulation with the help of the
``<set-param>`` element. In this case, the :par:`sendInterval` parameter of ``sourceNode1`` is modified:

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

.. figure:: media/TransmissionState.png
   :scale: 100%
   :align: center

The plot clearly shows the time intervals when the ping request messages
could not be forwarded to ``destinationNode``. From the density of the state changes we can also see
when the :par:`sendInterval` parameter of ``sourceNode1`` was modified.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/??>`__ in
the GitHub issue tracker for commenting on this showcase.
