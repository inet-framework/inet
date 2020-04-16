.. _ug:cha:network-nodes:

Network Nodes
=============

.. _ug:sec:nodes:overview:

Overview
--------

Hosts, routers, switches, access points, mobile phones, and other
network nodes are represented in INET with compound modules. The
previous chapter has introduced a few node types like
:ned:`StandardHost`, :ned:`Router`, and showed how to put together
networks from them. In this chapter, we look at the internals of such
node models, in order to provide a deeper understanding of their
customization possibilities and to give some guidance on how custom
nodes models can be assembled.

.. _ug:sec:nodes:ingredients:

Ingredients
-----------

Node models are assembled from other modules which represent
applications, communication protocols, network interfaces, routing
tables, mobility models, energy models, and other functionality. These
modules fall into the following broad categories:

-  *Applications* often model the user behavior as well as the
   application program (e.g., browser), and the application layer
   protocol (e.g., :protocol:`HTTP`). Applications typically use
   transport layer protocols (e.g., :protocol:`TCP` and/or
   :protocol:`UDP`), but they may also directly use lower layer
   protocols (e.g., :protocol:`IP` or :protocol:`Ethernet`) via sockets.

-  *Routing protocols* are provided as separate modules:
   :protocol:`OSPF`, :protocol:`BGP`, or :protocol:`AODV` for MANET
   routing. These modules use :protocol:`TCP`, :protocol:`UDP`, and
   :protocol:`IPv4`, and manipulate routes in the
   :ned:`Ipv4RoutingTable` module.

-  *Transport layer protocols* are connected to applications and network
   layer protocols. They are most often represented by simple modules,
   currently :protocol:`TCP`, :protocol:`UDP`, and :protocol:`SCTP` are
   supported. :protocol:`TCP` has several implementations: :ned:`Tcp` is
   the OMNeT++ native implementation; :ned:`TcpLwip` module wraps the
   lwIP :protocol:`TCP` stack; and :ned:`TcpNsc` module wraps the
   Network Simulation Cradle library.

-  *Network layer protocols* are connected to transport layer protocols
   and network interfaces. They are usually modeled as compound modules:
   :ned:`Ipv4NetworkLayer` for :protocol:`IPv4`, and
   :ned:`Ipv6NetworkLayer` for :protocol:`IPv6`. The
   :ned:`Ipv4NetworkLayer` module contains several protocol modules:
   :ned:`Ipv4`, :ned:`Arp`, :ned:`Icmp` and :ned:`Icmpv6`.

-  *Network interfaces* are represented by compound modules which are
   connected to the network layer protocols and other network interfaces
   in the wired case. They are often modeled as compound modules
   containing separate modules for queues, classifiers, MAC, and PHY
   protocols.

-  *Link layer protocols* are usually simple modules sitting in network
   interface modules. Some protocols, for example :protocol:`IEEE 802.11 MAC`,
   are modeled as a compound module themselves due to the complexity of the
   protocol.

-  *Physical layer protocols* are compound modules also being part of
   network interface modules.

-  *Interface table* maintains the set of network interfaces (e.g.
   ``eth0``, ``wlan0``) in the network node. Interfaces are registered
   dynamically during initialization of network interfaces.

-  *Routing tables* maintain the list of routes for the corresponding
   network protocol (e.g., :ned:`Ipv4RoutingTable` for :ned:`Ipv4`).
   Routes are added by automatic network configurators or routing
   protocols. Network protocols use the routing tables to find out the
   best matching route for datagrams.

-  *Mobility modules* are responsible for moving around the network node
   in the simulated scene. The mobility model is mandatory for
   wireless simulations even if the network node is stationary. The
   mobility module stores the location of the network node which is
   needed to compute wireless propagation and path loss. Different
   mobility models are provided as different modules. Network nodes
   define their mobility submodule with a parametric type, so the
   mobility model can be changed in the configuration.

-  *Energy modules* model energy storage mechanisms, energy consumption
   of devices and software processes, energy generation of devices, and
   energy management processes which shutdown and startup network nodes.

-  *Status* (:ned:`NodeStatus`) keeps track of the status of the network
   node (up, down, etc.)

-  *Other modules* with particular functionality such as
   :ned:`PcapRecorder` are also available.

.. _ug:sec:nodes:node-architecture:

Node Architecture
-----------------

Within network nodes, OMNeT++ connections are used to represent
communication opportunities between protocols. Packets and messages sent
on these connections represent software or hardware activity.

Although protocols may also be connected to each other directly, in most
cases they are connected via *dispatcher modules*. Dispatchers
(:ned:`MessageDispatcher`) are small, low-overhead modules that allow
protocol components to be connected in one-to-many and many-to-many
fashion, and ensure that messages and packets sent from one component
end up being delivered to the correct component. Dispatchers need no
manual configuration, as they use discovery and peek into packets.

In there pre-assembled node models, dispatchers allow arbitrary protocol
components to talk directly to each other, i.e. not only to ones in
neighboring layers.

.. _ug:sec:nodes:customizing-nodes:

Customizing Nodes
-----------------

The built-in network nodes are written to be as versatile and
customizable as possible. This is achieved in several ways:

Submodule and Gate Vectors
~~~~~~~~~~~~~~~~~~~~~~~~~~

One way is the use of gate vectors and submodule vectors. The sizes of
vectors may come from parameters or derived by the number of external
connections to the network node. For example, a host may have an
arbitrary number of wireless interfaces, and it will automatically have
as many :protocol:`Ethernet` interfaces as the number of
:protocol:`Ethernet` devices connected to it.

For example, wireless interfaces for hosts are defined like this:

.. code-block:: ned

   wlan[numWlanInterfaces]: <snip> // wlan interfaces in StandardHost etc al.

Where :par:`numWlanInterfaces` is a module parameter that defaults to
either 0 or 1 (this is different for e.g. :ned:`StandardHost` and
:ned:`WirelessHost`.) To configure a host to have two interfaces, add
the following line to the ini file:

.. code-block:: ini

   **.hostA.numWlanInterfaces = 2

Conditional Submodules
~~~~~~~~~~~~~~~~~~~~~~

Submodules that are not vectors are often conditional. For example, the
:protocol:`TCP` protocol module in hosts is conditional on the
``hasTcp`` parameter. Thus, to disable :protocol:`TCP` support in a
host (it is enabled by default), use the following ini file line:

.. code-block:: ini

   **.hostA.hasTcp = false

Parametric Types
~~~~~~~~~~~~~~~~

Another often used way of customization is parametric types, that is,
the type of a submodule (or a channel) may be specified as a string
parameter. Almost all submodules in the built-in node types have
parametric types. For example, the :protocol:`TCP` protocol module is
defined like this:

.. code-block:: ned

   tcp: <default("Tcp")> like ITcp if hasTcp;

The ``typename`` parameter of the tcp submodule defaults to the default
implementation, :ned:`Tcp`. To use another implementation instead, add the
following line to the ini file:

.. code-block:: ini

   **.host*.tcp.typename = "TcpLwip"  # use lwIP's TCP implementation

Submodule vectors with parametric types are defined without the use of a
module parameter to allow elements have different types. An example is
how applications are defined in hosts:

.. code-block:: ned

   app[numApps]: <> like IApp;  // applications in StandardHost et al.

And applications can be added in the following way:

.. code-block:: ini

   **.hostA.numApps = 2
   **.hostA.apps[0].typename = "UdpBasicApp"
   **.hostA.apps[1].typename = "PingApp"

Inheritance
~~~~~~~~~~~

Inheritance can be use to derive new, specialized node types from
existing ones. A derived NED type may add new parameters, gates,
submodules or connections, and may set inherited unassigned parameters
to specific values.

For example, :ned:`WirelessHost` is derived from :ned:`StandardHost` in
the following way:

.. code-block:: ned

   module WirelessHost extends StandardHost
   {
       @display("i=device/wifilaptop");
       numWlanInterfaces = default(1);
   }

.. _ug:sec:nodes:custom-network-nodes:

Custom Network Nodes
--------------------

Despite the many pre-assembled network nodes and the several available
customization options, sometimes it is just easier to build a network
node from scratch. The following example shows how easy it is to build a
simple network node.

This network node already contains a configurable application and
several standard protocols. It also demonstrates how to use the packet
dispatching mechanism which is required to connect multiple protocols in
a many-to-many relationship.

.. literalinclude:: lib/Snippets.ned
   :language: ned
   :start-after: !NetworkNodeExample
   :end-before: !End
   :name: Network node example
