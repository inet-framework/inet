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

-  :ned:`ExtInterface` is an INET network interface that represents a
   real interface (an interface of the host OS) in the simulation.
   Packets sent to an :ned:`ExtInterface` will be sent out on the host
   OS interface, and packets received by the host OS interface (or
   rather, the appropriate subset of them) will appear in the simulation
   as if received on an :ned:`ExtInterface`. The code uses the pcap
   library for capturing packets, and raw sockets for sending.

-  :cpp:`RealTimeScheduler`, a socket-aware real-time scheduler class.



.. note::

   It is probably needless to say, but the simulation must be fast enough
   to be able to keep up with real time. That is, its relative speed compared
   to real time (the simsec/sec value) must be >>1.  (Under Qtenv, this
   can usually only be achieved in Express mode.)

The simulation is run under Qtenv,

.. _ug:sec:emulation:preparation:

Preparation
-----------

There are a few things that need to be arranged before you can
successfully run simulations in network emulation mode.

First, network emulation is a separate *project feature* that needs to
be enabled before it can be used. (Project features can be reviewed and
changed in the *Project \| Project Features...* dialog in the IDE.)

The network emulation code makes use of the pcap library, and therefore
it must be available on your system. On Ubuntu, for example, pcap can be
installed with the following command:



::

   $ sudo apt install libpcap-dev

Also, when running a simulation, make sure you have the necessary
permissions. Sending uses raw sockets (type ``SOCK_RAW``), which, on
many systems, is only allowed for processes that have root
(administrator) privileges.

.. _ug:sec:emulation:configuring:

Configuring
-----------

INET nodes such as :ned:`StandardHost` and :ned:`Router` can be
configured to have :ned:`ExtInterface`’s. The simulation may contain
several nodes with external interfaces, and one node may also have
several external interfaces.

A network node can be configured to have an external interface in the
following way:



.. code-block:: ini

   **.host1.numExtInterfaces = 1

Also, the simulation must be configured to run under control the of the
appropriate real-time scheduler class:



.. code-block:: ini

   scheduler-class = "inet::RealTimeScheduler"

:ned:`ExtInterface` has two important parameters which need to be
configured. The :par:`device` parameter should be set to the name of the
real interface on the host OS, and :par:`filterString` should contain a
packet filter expression that selects which packets captured on the real
interface should be relayed into the simulation via this
:ned:`ExtInterface`. (:par:`filterString` is simply passed to the pcap
library, so it should follow the *tcpdump* filter expressions syntax
that pcap understands.)

An example configuration:



.. code-block:: ini

   **.numExtInterfaces = 1
   **.ext[0].ext.filterString = "(sctp or icmp) and ip dst host 10.1.1.1"
   **.ext[0].ext.device = "eth0" # or "en0" on macOS, or something
   **.ext[0].ext.mtu = 1500B

The filter string ``"(sctp or icmp) and ip dst host 10.1.1.1"`` means
that the protocol must be SCTP or ICMP, and the destination host must be
10.1.1.1.



.. note::

   Why is filtering of incoming packets done at packet capture (in pcap),
   and not in :ned:`ExtInterface`? The reason is performance: it costs
   much fewer CPU cycles to discard unnecessary packets right where
   they come in, and not send them up into the simulation for the
   same decision. And, given that the simulation needs to keep up with
   real time, saving CPU cycles is important.

Let us examine the paths outgoing and incoming packets take, and the
necessary configuration requirements to make them work. We assume IPv4
as network layer protocol, but the picture does not change much with
other protocols. We assume the external interface is named
``ext[0]``.

Outgoing path
~~~~~~~~~~~~~

The network layer of the simulated node routes datagrams to its
``ext[0]`` external interface.

For that to happen, the routing table needs to contain an entry where
the interface is set to ``ext[0]``. Such entries are not created
automatically, one needs to add them to the routing table explicitly,
e.g. by using an :ned:`Ipv4NetworkConfigurator` and an appropriate XML
file.

Another point is that if the packet comes from a local app (and from
another simulated node), it needs to have a source IP address assigned.
There are two ways for that to happen. If the sending app specified a
source IP address, that will be used. Otherwise, the IP address of the
``ext[0]`` interface will be used, but for that, the interface needs
to have an IP address at all.

Once in ``ext[0]``, the datagram is serialized. Serialization is a
built-in feature of INET packets. (Packets, or rather, packet chunks
have multiple alternative representations, i.e. C++ object and
serialized form, and conversion between them is transparent.)

The result of serialization is a byte string, which is written into a
raw socket with a ``sendto`` system call.

The packet will then travel normally in the real network to the
destination address.

Incoming path
~~~~~~~~~~~~~

First of all, packets intended to be received by the simulation need to
find their way to the correct interface of the host that runs the
simulation. For that, IP addresses of simulated hosts must be routable
in the real network, and routed to the captured interface of the host
OS. (On Linux, for example, this can be achieved by adding static routes
with the command.)

As packets are received by the interface of the host OS, they are
examined by the pcap library to find out whether they match the filter
expression. If the filter matches, pcap hands the packet over to the
simulation, and after deserialization it pops out of ``ext[0]`` and
sent up to the network layer. After that, it is routed to the simulated
destination host in the normal way.

The pcap filter expression must be crafted so that it matches the
packets destined to simulated hosts, and does not match any other
packet.

Moreover, if the simulation contains several external interfaces that
map to the same real interface, care must be taken so that filter
expressions are disjunct. Otherwise, a packet may be matched by more
than one filter, and then it will be inserted into the simulation in
multiple copies (once for each matching :ned:`ExtInterface`.) This is
usually not what is wanted.
