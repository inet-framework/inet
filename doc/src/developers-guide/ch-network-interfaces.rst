:orphan:

.. _dg:cha:network-interfaces:

Network Interafces
==================

Overview
--------

TODO

The Interface Table
-------------------

The :ned:`InterfaceTable` module holds one of the key data structures in
the INET Framework: information about the network interfaces in the
host. The interface table module does not send or receive messages;
other modules access it using standard C++ member function calls.

TODO

Accessing the Interface Table
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a module wants to work with the interface table, first it needs to
obtain a pointer to it. This can be done with the help of the
:cpp:`InterfaceTableAccess` utility class:



.. code-block:: c++

   IInterfaceTable *ift = InterfaceTableAccess().get();

:cpp:`InterfaceTableAccess` requires the interface table module to be a
direct child of the host and be called ``"interfaceTable"`` in order
to be able to find it. The :fun:`get()` method never returns
``NULL``: if it cannot find the interface table module or cannot cast
it to the appropriate C++ type (:cpp:`IInterfaceTable`), it throws an
exception and stop the simulation with an error message.

For completeness, :cpp:`InterfaceTableAccess` also has a
:fun:`getIfExists()` method which can be used if the code does not
require the presence of the interface table. This method returns
``NULL`` if the interface table cannot be found.

Note that the returned C++ type is :cpp:`IInterfaceTable`; the initial
"``I``" stands for "interface". :cpp:`IInterfaceTable` is an abstract
class interface that :cpp:`InterfaceTable` implements. Using the
abstract class interface allows one to transparently replace the
interface table with another implementation, without the need for any
change or even recompilation of the INET Framework.

Interface Entries
~~~~~~~~~~~~~~~~~

Interfaces in the interface table are represented with the
:cpp:`NetworkInterface` class. :cpp:`IInterfaceTable` provides member
functions for adding, removing, enumerating and looking up interfaces.

Interfaces have unique names and interface IDs; either can be used to
look up an interface (IDs are naturally more efficient). Interface IDs
are invariant to the addition and removal of other interfaces.

Data stored by an interface entry include:

-  *name* and *interface ID* (as described above)

-  *MTU*: Maximum Transmission Unit, e.g. 1500 on Ethernet

-  several flags:

   -  *down*: current state (up or down)

   -  *broadcast*: whether the interface supports broadcast

   -  *multicast* whether the interface supports multicast

   -  *pointToPoint*: whether the interface is point-to-point link

   -  *loopback*: whether the interface is a loopback interface

-  *datarate* in bit/s

-  *link-layer address* (for now, only IEEE 802 MAC addresses are
   supported)

-  *network-layer gate index*: which gate of the network layer within
   the host the NIC is connected to

-  *host gate IDs*: the IDs of the input and output gate of the host the
   NIC is connected to

Extensibility: You have probably noticed that the above list does not
contain data such as the IPv4 or IPv6 address of the interface. Such
information is not part of :cpp:`NetworkInterface` because we do not want
:ned:`InterfaceTable` to depend on either the IPv4 or the IPv6 protocol
implementation; we want both to be optional, and we want
:ned:`InterfaceTable` to be able to support possibly other network
protocols as well.

Thus, extra data items are added to :cpp:`NetworkInterface` via extension.
Two kinds of extensions are envisioned: extension by the link layer
(i.e. the NIC), and extension by the network layer protocol:

-  NICs can extend interface entries via C++ class inheritance, that is,
   by simply subclassing :cpp:`NetworkInterface` and adding extra data and
   functions. This is possible because NICs create and register entries
   in :ned:`InterfaceTable`, so in their code one can just write
   ``new MyExtendedNetworkInterface()`` instead of ``new NetworkInterface()``.

-  **Network layer protocols** cannot add data via subclassing, so
   composition has to be used. :cpp:`NetworkInterface` contains pointers
   to network-layer specific data structures. For example, there are
   pointers to IPv4 specific data, and IPv6 specific data. These objects
   can be accessed with the following :cpp:`NetworkInterface` member
   functions: :fun:`ipv4Data()`, :fun:`ipv6Data()`, and
   :fun:`getGenericNetworkProtocolData()`. They return pointers of the
   types :cpp:`Ipv4InterfaceData`, :cpp:`Ipv6InterfaceData`, and
   :cpp:`GenericNetworkProtocolInterfaceData`, respectively. For
   illustration, :cpp:`Ipv4InterfaceData` is installed onto the
   interface entries by the :ned:`Ipv4RoutingTable` module, and it
   contains data such as the IP address of the interface, the netmask,
   link metric for routing, and IP multicast addresses associated with
   the interface. A protocol data pointer will be ``NULL`` if the
   corresponding network protocol is not used in the simulation; for
   example, in IPv4 simulations only :fun:`ipv4Data()` will return a
   non-``NULL`` value.

Interface Registration
~~~~~~~~~~~~~~~~~~~~~~

Interfaces are registered dynamically in the initialization phase by
modules that represent network interface cards (NICs). The INET
Framework makes use of the multi-stage initialization feature of
OMNeT++, and interface registration takes place in the first stage (i.e.
stage ``INITSTAGE_LINK_LAYER``).

Example code that performs interface registration:

.. code-block:: c++

   void PPP::initialize(int stage)
   {
       if (stage == INITSTAGE_LINK_LAYER) {
           ...
           networkInterface = registerInterface(datarate);
       ...
   }

   NetworkInterface *PPP::registerInterface(double datarate)
   {
       NetworkInterface *e = new NetworkInterface(this);

       // interface name: NIC module's name without special characters ([])
       e->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

       // data rate
       e->setDatarate(datarate);

       // generate a link-layer address to be used as interface token for IPv6
       InterfaceToken token(0, simulation.getUniqueNumber(), 64);
       e->setInterfaceToken(token);

       // set MTU from module parameter of similar name
       e->setMtu(par("mtu"));

       // capabilities
       e->setMulticast(true);
       e->setPointToPoint(true);

       // add
       IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
       ift->addInterface(e);

       return e;
   }

TODO

Interface Change Notifications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:ned:`InterfaceTable` has a change notification mechanism built in, with
the granularity of interface entries.

Clients that wish to be notified when something changes in
:ned:`InterfaceTable` can subscribe to the following notification
categories in the host’s :ned:`NotificationBoard`:

-  ``NF_INTERFACE_CREATED``: an interface entry has been created and
   added to the interface table

-  ``NF_INTERFACE_DELETED``: an interface entry is going to be
   removed from the interface table. This is a pre-delete notification
   so that clients have access to interface data that are possibly
   needed to react to the change

-  ``NF_INTERFACE_CONFIG_CHANGED``: a configuration setting in an
   interface entry has changed (e.g. MTU or IP address)

-  ``NF_INTERFACE_STATE_CHANGED``: a state variable in an interface
   entry has changed (e.g. the up/down flag)

In all those notifications, the data field is a pointer to the
corresponding :cpp:`NetworkInterface` object. This is even true for
``NF_INTERFACE_DELETED`` (which is actually a pre-delete
notification).
