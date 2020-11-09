.. _ug:cha:lifecycle:

Modeling Node Failures
======================

.. _ug:sec:lifecycle:overview:

Overview
--------

Simulation is often used to study the effects of unexpected events like
a router crash on the network. In order to accommodate such scenarios,
INET supports *lifecycle modeling* of network nodes. The up/down status
of a node is changed via lifecycle operations.

INET supports the following lifecycle operations:

-  *Startup* represents the process of booting up or starting a network
   node after a shutdown or crash operation.

-  *Shutdown* represents the process of orderly shutting down a network
   node.

-  *Crash* represents the process of crashing a network node. The
   difference between *crash* and *shutdown* is that for a crash, the
   network node will not do a graceful shutdown (e.g. routing protocols
   will not have a chance of notifying peers about broken routes).

In a real-life router or other network node, a crash or shutdown and
subsequent restart affects all parts of the system. All non-persistent
information is lost. Protocol states are reset, various tables are
cleared, connections are broken or torn down, applications restart, and
so on.

Mimicking this behavior in simulation does not come for free, it needs
to be explicitly programmed into each affected component. Here are some
examples how INET components react to a *crash* lifecycle event:

-  :ned:`Tcp` forgets all open connections and sockets

-  :ned:`Ipv4` clears the fragmentation reassembly buffers and pending
   packets

-  :ned:`Ipv4RoutingTable` clears the route table

-  :ned:`EthernetCsmaMac` and other MAC protocols clear their queues and reset
   their state associated with the current transmission(s)

-  :ned:`Ospfv2` clears its full state

-  :ned:`UdpBasicApp`, :ned:`TcpSessionApp` and other applications reset
   their state and stop/restart their timers

-  :ned:`EthernetSwitch`, :ned:`AccessPoint`, and other L2 bridging devices
   clear their MAC address tables

While down, network interfaces, and components in general, ignore
(discard) messages sent to them.

Lifecycle operations are currently instanteneous, i.e. they complete in
zero simulation time. The underlying framework would allow for modeling
them as processes that take place in some finite (nonzero) simulation
time, but this possibility is currently not in use.

It also is possible to simulate a crash or shutdown of part of a node
(certain protocols or interfaces only). Such scenarios would correspond
to e.g. the crash of an OSPF daemon on a real OS.

Some energy-related INET components trigger node shutdown or crash under
certain conditions. For example, a node will crash when it runs out of
power (e.g. its battery depletes); see the chapter on power consumption
modeling :doc:`ch-power` for details.

In the following sections we outline the INET components that
participate in lifecycle modeling, and show a usage example.

.. _ug:sec:lifecycle:nodestatus:

NodeStatus
----------

Node models contain a :ned:`NodeStatus` module that keeps track of the
status of the node (up, down, etc.) for other modules, and also displays
it in the GUI as a small overlay icon.

The :ned:`NodeStatus` module is declared conditionally (so that it is
only created in simulations that need it), like this:

.. code-block:: ned

   status: NodeStatus if hasStatus;

If lifecycle modeling is required, the following line must be added to
the ini file to ensure that nodes have status modules:

.. code-block:: ini

   **.hasStatus = true

.. _ug:sec:lifecycle:scripting:

Scripting
---------

Lifecycle operations can be triggered from C++ code, or from scripts.
INET supports scripting via the :ned:`ScenarioManager` NED type,
described in chapter :doc:`ch-scenario-scripting`. Here is an
example script that shuts down a router at simulation time 2s, and
starts it up a again at time 8s:

.. code-block:: xml

   <scenario>
     <initiate t="2s" module="Router2" operation="shutdown"/>
     <initiate t="8s" module="Router2" operation="startup"/>
   </scenario>

The ``module`` attribute should point to the module (host, router,
network interface, protocol, etc.) to be operated on. The
``operation`` attribute should contain the operation to perform:
``"shutdown"``, ``"crash"``, or ``"startup"``. ``t`` is the
simulation time the operation should be initiated at.

An alternative, shorter form is to use ``<shutdown>`` /
``<crash>`` / ``<startup>`` elements instead of the
``operation`` attribute:

.. code-block:: xml

   <scenario>
     <shutdown t="2s" module="Router2"/>
     <startup  t="8s" module="Router2"/>
   </scenario>
