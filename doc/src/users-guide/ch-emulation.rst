.. _ug:cha:emulation:

Network Emulation
=================

.. _ug:sec:emulation:motivation:

Motivation
----------

There are several projects that may benefit from the network emulation
capabilities of INET, that is, from the ability to mix simulated
components with real networks.

Some example scenarios:

-  Run a simulated component, such as an app or a routing protocol, on
   nodes of an actual ad-hoc network. This setup would allow testing the
   component’s behavior under real-life conditions.

-  Test the interoperability of a simulated protocol with its real-world
   counterparts. Several setups are possible: simulated node in a real
   network; a simulated subnet in real network; real-world node in
   simulated network; etc.

-  As a means of implementing hybrid simulation. The real network (or a
   single host OS) may contain several network emulator devices or
   simulations running in emulation mode. Such a setup provides a
   relatively easy way for connecting heterogenous simulators/emulators
   with each other, sparing the need for HLA or a custom
   interoperability solution.

.. _ug:sec:emulation:overview:

Overview
--------

To act as a network emulator, the simulation must run in real time, and
must be able to communicate with the real world.

This is achieved with two components in INET:

-  :ned:`ExtLowerEthernetInterface` is an INET network interface that
   represents a real interface (an interface of the host OS) in the simulation.
   Packets sent to an :ned:`ExtLowerEthernetInterface` will be sent out on the
   host OS interface, and packets received by the host OS interface (or
   rather, the appropriate subset of them) will appear in the simulation
   as if received on an :ned:`ExtLowerEthernetInterface`. The code uses
   raw sockets for sending and receiving packets.

-  :cpp:`RealTimeScheduler`, a socket-aware real-time scheduler class.



.. note::

   It is probably needless to say, but the simulation must be fast enough
   to be able to keep up with real time. That is, its relative speed compared
   to real time (the simsec/sec value) must be >>1.  (Under Qtenv, this
   can usually only be achieved in Express mode.)

.. _ug:sec:emulation:preparation:

Preparation
-----------

There are a few things that need to be arranged before you can
successfully run simulations in network emulation mode.

First, network emulation is a separate *project feature* that needs to
be enabled before it can be used. (Project features can be reviewed and
changed in the *Project \| Project Features...* dialog in the IDE.)

Also, when running a simulation, make sure you have the necessary
permissions. Sending and receiving packets rely on raw sockets
(type ``SOCK_RAW``), which, on many systems, is only allowed for
processes that have root (administrator) privileges.

.. _ug:sec:emulation:configuring:

Configuring
-----------

INET nodes such as :ned:`StandardHost` and :ned:`Router` can be
configured to have :ned:`ExtLowerEthernetInterface`’s. The simulation
may contain several nodes with external interfaces, and one node may
also have several external interfaces.

A network node can be configured to have an external interface in the
following way:



.. code-block:: ini

   **.host1.numEthInterfaces = 1
   **.host1.eth[0].typename = "ExtLowerEthernetInterface"

Also, the simulation must be configured to run under control the of the
appropriate real-time scheduler class:



.. code-block:: ini

   scheduler-class = "inet::RealTimeScheduler"

:ned:`ExtLowerEthernetInterface` has two important parameters which need
to be configured. The :par:`device` parameter should be set to the name
of the real (or virtual) interface on the host OS. The :par:`namespace`
parameter can be set to utilize the network namespace functionality of
linux operating systems.

An example configuration:



.. code-block:: ini

   **.numEthInterfaces = 1
   **.eth[0].device = "veth0" # or "eth0" for example
   **.eth[0].namespace = "host0" # optional
   **.eth[0].mtu = 1500B


.. note::

Let us examine the paths outgoing and incoming packets take, and the
necessary configuration requirements to make them work. We assume IPv4
as network layer protocol, but the picture does not change much with
other protocols. We assume the external interface is named
``eth[0]``.

Outgoing path
~~~~~~~~~~~~~

The network layer of the simulated node routes datagrams to its
``eth[0]`` external interface.

For that to happen, the routing table needs to contain an entry where
the interface is set to ``eth[0]``. Such entries are not created
automatically, one needs to add them to the routing table explicitly,
e.g. by using an :ned:`Ipv4NetworkConfigurator` and an appropriate XML
file.

Another point is that if the packet comes from a local app (and from
another simulated node), it needs to have a source IP address assigned.
There are two ways for that to happen. If the sending app specified a
source IP address, that will be used. Otherwise, the IP address of the
``eth[0]`` interface will be used, but for that, the interface needs
to have an IP address at all. The MAC and IP address of external interfaces
are automatically copied between the real and simulated counterparts.

Once in ``eth[0]``, the datagram is serialized. Serialization is a
built-in feature of INET packets. (Packets, or rather, packet chunks
have multiple alternative representations, i.e. C++ object and
serialized form, and conversion between them is transparent.)

The result of serialization is a byte array, which is written into a
raw socket with a ``sendto`` system call.

The packet will then travel normally in the real network to the
destination address.

Incoming path
~~~~~~~~~~~~~

First of all, packets intended to be received by the simulation need to
find their way to the correct interface of the host that runs the
simulation. For that, IP addresses of simulated hosts must be routable
in the real network, and routed to the selected interface of the host
OS. (On Linux, for example, this can be achieved by adding static routes
with the command.)

As packets are received by the interface of the host OS, they are handed
over to the simulation. The packets are received from the raw socket with a
``recv`` system call. After deserialization they pop out of ``eth[0]`` and
they are sent up to the network layer. The packets are routed to the simulated
destination host in the normal way.
