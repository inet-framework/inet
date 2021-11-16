:orphan:

.. _dg:cha:developing-models:

Developing Protocol and Application Models
==========================================

Overview
--------

This section introduces the most important modeling support features of
INET. These features facilitate the implementation of applications and
communication protocols by providing various commonly used
functionality. Thus modeling support allows rapid implementation of new
models by building on already existing APIs while the implementor can
focus on the research topics. These features differ from the reusable
NED modules introduced earlier, because they are available in the form
of C++ APIs.

The easy usage of protocol services is another essential modeling
support. Applications often need to use several different protocol
services simultaneously. In order to spare the applications from using
the default OMNeT++ message passing style between modules, INET provides
an easy to use C++ socket API.

TODO

NED Conventions
---------------

The @node Property
~~~~~~~~~~~~~~~~~~

By convention, compound modules that implement network devices (hosts,
routers, switches, access points, base stations, etc.) are marked with
the ``@node`` NED property. As node models may themselves be
hierarchical, the ``@node`` property is used by protocol
implementations and other simple modules to determine which ancestor
compound module represents the physical network node they live in.

The @labels Module Property
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``@labels`` property can be added to modules and gates, and it
allows the OMNeT++ graphical editor to provide better editing
experience. First we look at ``@labels`` as a module property.

``@labels(node)`` has been added to all NED module types that may
occur on network level. When editing a network, the graphical editor
will NED types with ``@labels(node)`` to the top of the component
palette, allowing the user to find them easier.

Other labels can also be specified in the ``@labels(...)`` property.
This has the effect that if one module with a particular label has
already been added to the compound module being edited, other module
types with the same label are also brought to the top of the palette.
For example, :ned:`EthernetSwitch` is annotated with
``@labels(node,ethernet-node)``. When you drop an :ned:`EthernetSwitch`
into a compound module, that will bring :ned:`EthernetHost` (which is also
tagged with the ``ethernet-node`` label) to the top of the palette,
making it easier to find.



.. code-block:: ned

   module EthernetSwitch
   {
       parameters:
           @node();
           @labels(node,ethernet-node);
           @display("i=device/switch");
       ...
   }

Module types that are already present in the compound module also appear
in the top part of the palette. The reason is that if you already added
a :ned:`StandardHost`, for example, then you are likely to add more of
the same kind. Gate labels (see next section) also affect palette order:
modules which can be connected to modules already added to the compound
module will also be listed at the top of the palette. The final ordering
is the result of a scoring algorithm.

The @labels Gate Property
~~~~~~~~~~~~~~~~~~~~~~~~~

Gates can also be labelled with ``@labels()``; the purpose is to make
it easier to connect modules in the editor. If you connect two modules
in the editor, the gate selection menu will list gate pairs that have a
label in common.

TODO screenshot

For example, when connecting hosts and routers, the editor will offer
connecting Ethernet gates with Ethernet gates, and PPP gates with PPP
gates. This is the result of gate labelling like this:



.. code-block:: ned

   module StandardHost
   {
       ...
       gates:
           inout pppg[] @labels(PPPFrame-conn);
           inout ethg[] @labels(EtherFrame-conn);
       ...
   }

Guidelines for choosing gate label names: For gates of modules that
implement protocols, use the C++ class name of the packet or acompanying
control info (see later) associated with the gate, whichever applies;
append ``/up`` or ``/down`` to the name of the control info class.
For gates of network nodes, use the class names of packets (frames) that
travel on the corresponding link, with the ``-conn`` suffix. The
suffix prevents protocol-level modules to be promoted in the graphical
editor palette when a network is edited.

Examples:



.. code-block:: ned

   simple TCP like ITCP
   {
       ...
       gates:
           input appIn[] @labels(TCPCommand/down);
           output appOut[] @labels(TCPCommand/up);
           input ipIn @labels(TCPSegment,IPv4ControlInfo/up,IPControlInfo/up);
           output ipOut @labels(TCPSegment,IPv4ControlInfo/down,IPv6ControlInfo/up);
   }



.. code-block:: ned

   simple PPP
   {
       ...
       gates:
           input netwIn;
           output netwOut;
           inout phys @labels(PPPFrame);
   }

Module Initialization
---------------------



.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !ModuleInitializationExample
   :end-before: !End
   :name: Module initialization example

Addresses
---------

Address Types
~~~~~~~~~~~~~

The INET Framework uses a number of C++ classes to represent various
addresses in the network. These classes support initialization and
assignment from binary and string representation of the address, and
accessing the address in both forms. Storage is in binary form, and they
also support the "unspecified" special value (and the
:fun:`isUnspecified()` method) that usually corresponds to the
all-zeros address.

-  :cpp:`MacAddress` represents a 48-bit IEEE 802 MAC address. The
   textual notation it understands and produces is hex string.

-  :cpp:`Ipv4Address` represents a 32-bit IPv4 address. It can parse and
   produce textual representations in the "dotted decimal" syntax.

-  :cpp:`Ipv6Address` represents a 128-bit IPv6 address. It can parse
   and produce address strings in the canonical (RFC 3513) syntax.

-  :cpp:`L3Address` is conceptually a union of a :cpp:`Ipv4Address` and
   :cpp:`Ipv6Address`: an instance stores either an IPv4 address or an
   IPv6 address. :cpp:`L3Address` is mainly used in the transport layer
   and above to abstract away network addresses. It can be assigned from
   both :cpp:`Ipv4Address` and :cpp:`Ipv6Address`, and can also parse
   string representations of both. The :fun:`getType()`,
   :fun:`toIpv4()` and :fun:`toIpv6()` methods can be used to access
   the value.

TODO

Resolving Addresses
~~~~~~~~~~~~~~~~~~~

explain what kind of addresses INET provides for protocols to use:
network and MAC addresses, related protocols: ARP, DHCP, ND, etc.

address lookup by name

node lookup by MAC address

node lookup by L3 address

TODO

Starting and Stopping Nodes
---------------------------



.. literalinclude:: lib/Snippets.cc
   :language: cpp
   :start-after: !LifecycleOperationExample
   :end-before: !End
   :name: Lifecycle operation example
