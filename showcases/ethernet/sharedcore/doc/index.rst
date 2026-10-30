Q-in-Q: IEEE 802.1ad Shared Core Network
=========================================

Goals
-----

IEEE 802.1ad, also known as Q-in-Q or provider bridging, lets a network carry
the VLANs of several customers without mixing them, even if the customers use
the same VLAN ids. The provider's bridges put an extra VLAN tag, the S-tag, in
front of the customer's own tag, and forward the frames by the S-tag only.

This showcase presents the wholesale case: a core network shared by two
service providers. Each service provider runs C-VLANs for its own customers,
and both of them use the same C-VLAN ids. To make the point even clearer, the
hosts of the second service provider use the same MAC addresses as the hosts
of the first one. The showcase demonstrates that the core still keeps the two
service providers apart, because it relays the frames by the S-VLAN of the
service provider and never looks at the C-tags.

| Verified with INET version: ``4.8``
| Source files location: `inet/showcases/ethernet/sharedcore <https://github.com/inet-framework/inet/tree/master/showcases/ethernet/sharedcore>`__

About Q-in-Q
------------

An IEEE 802.1Q VLAN tag is four bytes long, and it is inserted in an Ethernet
frame between the source MAC address and the EtherType field. It consists of
a 16-bit Tag Protocol Identifier (TPID), a 3-bit Priority Code Point (PCP), a
Drop Eligible Indicator (DEI) bit, and a 12-bit VLAN identifier (VID). The TPID
takes the place of the EtherType, so a receiver recognizes a tagged frame by
it.

IEEE 802.1ad, now part of IEEE 802.1Q, defines two kinds of tags with the same
layout:

- The *C-tag* (customer tag, TPID ``0x8100``) is the ordinary VLAN tag. A
  network that uses C-tags is built from *C-VLAN Bridges*.
- The *S-tag* (service tag, TPID ``0x88a8``) is the tag of the provider. A
  network that uses S-tags is built from *S-VLAN Bridges*.

When a frame enters the provider's network, a bridge of the provider pushes an
S-tag in front of the C-tag, if the frame has one. Inside the provider's
network, the frame looks like this on the wire:

.. code-block:: text

   | destination MAC | source MAC | S-tag 0x88a8 | C-tag 0x8100 | EtherType | payload | FCS |

An S-VLAN Bridge reads and writes only the S-tag. The C-tag is part of the
payload for it, so the C-VLAN ids of the customers are completely invisible to
the provider's network; they can overlap between customers.

The interface where the provider takes over a customer's frames is called a
*service interface*. In the *port-based service interface*, the bridge of the
provider puts all frames that arrive on the port of a customer into the S-VLAN
of that customer, whatever C-tag they have. The port is then a *Customer
Network Port*. (IEEE 802.1Q also defines a *Provider Edge Bridge*, which has
C-VLAN components besides the S-VLAN component and can select the S-VLAN by
the C-VLAN of the frame; this showcase does not need one.)

VLAN-aware bridges learn MAC addresses per VLAN: a MAC forwarding table entry
is a (VLAN id, MAC address) pair. The same MAC address can thus appear in
several VLANs, on different ports, without the entries interfering. This is
why the reused MAC addresses in this showcase cause no harm in the core: the
two copies of each address are learned in different S-VLANs.

The Model
---------

The Network
~~~~~~~~~~~

The network contains two service providers, each with a network in the west
and one in the east, and the shared core between them:

.. figure:: media/network.png
   :width: 100%

On the canvas, the networks of service provider 1 are blue, those of service
provider 2 orange, and the core network green; each customer site has a
color of its own. The labels on the links between bridges name the VLANs that
the link carries tagged.

The nodes play the following roles:

- **Hosts** (``host*``) are :ned:`StandardHost`\ s without an 802.1Q layer.
  They send and receive untagged frames.
- **Customer switches** (``switchB1``, ``switchC1``, ``switchA2``,
  ``switchD2``) are VLAN-unaware :ned:`EthernetSwitch`\ es.
- **Service provider bridges** (``sp1w``, ``sp1e``, ``sp2w``, ``sp2e``) are
  C-VLAN Bridges. They put each customer into a C-VLAN, 10 for customers A and
  C and 20 for customers B and D, and send the frames C-tagged to the core.
- **Edge bridges of the core** (``pebW``, ``pebE``) are S-VLAN Bridges whose
  ports toward the service providers are Customer Network Ports: they put all
  frames of service provider 1 into S-VLAN 100, and all frames of service
  provider 2 into S-VLAN 200. Despite their names, they are not Provider Edge
  Bridges in the IEEE 802.1Q sense, as they have no C-VLAN components.
- **The core bridge** (``pb``) is an S-VLAN Bridge that relays the frames by
  S-VLAN.

Note the point of view: for the core network, the customers are the two
service providers. Service provider 1 has customers A and B, service provider
2 has customers C and D. Each customer has a site in the west and one in the
east. A customer site is either two hosts on access ports of the service
provider's bridge, or a customer switch with two hosts, connected to one access
port; every customer has one site of each kind.

The configuration below refers to the ports of the bridges by their interface
names. The ports are numbered in the order of the connections in the NED file:

- A customer switch has its hosts on ``eth0`` and ``eth1``, and the uplink to
  the service provider on ``eth2``.
- A service provider bridge has its customer sites on ``eth0`` to ``eth2``,
  and the uplink to the core on ``eth3``.
- ``pebW`` and ``pebE`` have service provider 1 on ``eth0``, service provider 2
  on ``eth1``, and ``pb`` on ``eth2``.
- ``pb`` has ``pebW`` on ``eth0``, and ``pebE`` on ``eth1``.

Inside the Bridges
~~~~~~~~~~~~~~~~~~

All VLAN-aware bridges of the network, the service provider bridges and the
bridges of the core, have the same internal structure. The following image
shows the inside of ``pebW``:

.. figure:: media/pebW.png
   :width: 100%

The network interfaces (``eth[*]``) are at the bottom, and the protocol layers
are connected to each other through the ``li`` and ``bl`` message dispatchers.
A frame received by an interface goes up to the ``ethernet`` layer, which
checks and removes the Ethernet MAC header. The dispatchers then send the frame
on by the protocol of its payload. If the payload starts with a tag that the
``ieee8021q`` layer handles, the frame goes there; the layer removes the tag,
and passes its contents (VLAN id, PCP and DEI) up as packet tags. Every other
frame goes directly to the ``bridging`` layer. On the way down, the
``ieee8021q`` layer inserts a tag if the bridging layer requested one, and the
``ethernet`` layer adds the MAC header.

The 802.1Q layer handles one kind of tag only, the C-tag or the S-tag; this is
selected by its :par:`vlanTagType` parameter. In ``pebW``, it handles the
S-tag. A frame that arrives from a service provider has a C-tag but no S-tag,
so it bypasses the 802.1Q layer, and the C-tag stays in the payload of the
frame.

The ``bridging`` layer is a :ned:`BridgingLayer`:

.. figure:: media/pebW_bridging.png
   :width: 60%
   :align: center

From the top down, the submodules that this showcase uses are the following:

- ``directionReverser`` decides whether a frame is delivered locally or
  forwarded. For a forwarded frame, it turns the indication tags of the
  received frame into request tags for the outgoing frame, for example the
  received VLAN id into the requested one.
- ``interfaceRelay`` learns the source MAC address of each received frame in
  the MAC forwarding table, per VLAN, and selects the outgoing port of each
  frame by its destination MAC address and VLAN. If the destination is unknown,
  it sends the frame to all other ports.
- ``vlanPolicy`` applies the VLAN configuration of the ports.

:ned:`BridgingLayer` also has slots for stream identification, stream relay,
stream filtering, stream coding and processing delay, which the Time-Sensitive
Networking features of INET use. They are empty here, so they do not appear in
the image.

The ``vlanPolicy`` module is a :ned:`VlanPolicyLayer`:

.. figure:: media/pebW_vlanpolicy.png
   :width: 60%
   :align: center

On the way in, the ``inboundFilter`` drops the frames whose VLAN id the
incoming port does not accept, and then the ``inboundMapper`` can replace the
VLAN id, for example to put the untagged frames of a port into a VLAN. On the
way out, the ``outboundMapper`` can replace or remove the VLAN id per outgoing
port, and then the ``outboundFilter`` drops the frames whose VLAN id the
outgoing port does not accept. Both filters check the VLAN id as it is on the
wire: ``-1`` stands for an untagged frame.

The filters and the mappers treat a port without an entry differently. A
filter drops all frames on a port that it has no entry for, so its list must
name every port of the bridge. A mapper passes the frames of such a port
unchanged, and also the frames whose VLAN id has no entry in the map of the
port, so it needs entries only where it changes the VLAN id.

Whether a bridge is a C-VLAN Bridge or an S-VLAN Bridge depends only on the
:par:`vlanTagType` parameter of its modules, and the VLAN ids in the mappers
and filters differ. The customer switches, in contrast, have no 802.1Q layer,
and their ``bridging`` layer is the default :ned:`Ieee8021dRelay`.

Addresses and Traffic
~~~~~~~~~~~~~~~~~~~~~

Each customer is an IP subnet of its own, configured by the
:ned:`Ipv4NetworkConfigurator`:

.. literalinclude:: ../addresses.xml
   :language: xml

The MAC addresses are set explicitly: customer C uses the MAC addresses of
customer A, and customer D those of customer B. Every host pings its
counterpart at the other site of the same customer five times, 10 ms apart.
``hostA1a`` starts at 1 ms, so that its first ping crosses the network alone
(see `The First Ping`_), and the other hosts start at 2 ms. This is 80 ping
requests altogether within the 100 ms of simulated time.

Configuration
~~~~~~~~~~~~~

The hosts have no 802.1Q layer, so they send and receive untagged frames. The
MAC addresses of the hosts of service provider 2 are the same as those of
service provider 1:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The hosts send and receive untagged frames
   :end-at: *.hostD2b.eth[0].address

The service provider bridges and the bridges of the core are VLAN-aware. Each
of them gets an 802.1Q layer, a :ned:`BridgingLayer`, and a
:ned:`VlanPolicyLayer` in it. The customer switches keep their defaults, so they
remain VLAN-unaware:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The customer switches are VLAN-unaware
   :end-at: *.pb.bridging.vlanPolicy.typename

The direction reverser forwards the PCP of a received frame by default. A
frame that arrives tagged and leaves on a port that sends its VLAN untagged
would then keep a tag with VLAN id 0, a so-called priority tag. Neither the
hosts nor the service provider bridges expect such tags, so PCP forwarding is
turned off:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # A forwarded PCP would keep the tag
   :end-at: *.pb.bridging.directionReverser.reverser.forwardPcp

The service provider bridges are C-VLAN Bridges, as the :par:`vlanTagType`
parameter is ``"c"`` by default. Their access ports accept untagged frames
only, and the inbound mapper puts them into the C-VLAN of the customer. The
outbound mapper removes the C-tag on the access ports. The uplink to the core
(``eth3``) is a trunk that carries both C-VLANs tagged, so the mappers have no
entry for it:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The bridges of the service providers are C-VLAN bridges
   :end-at: *.sp2e.bridging.vlanPolicy.outboundMapper.mappedVlanIds

The frames that a service provider bridge forwards from an access port to
another access port in the same C-VLAN stay untagged; frames toward a port of
another C-VLAN are dropped by the outbound filter.

A single line per node makes the bridges of the core S-VLAN Bridges: setting
:par:`vlanTagType` to ``"s"`` on all modules of the node makes the 802.1Q layer
insert and remove S-tags, and makes the VLAN policy and the relay work with the
S-VLAN of the frames:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The bridges of the core are S-VLAN bridges
   :end-at: *.pb.**.vlanTagType

The edge bridges of the core implement the port-based service interface. A
frame from a service provider has no S-tag, so the inbound filter sees it as
untagged (``-1``), whatever C-tag it carries. The inbound mapper puts all such
frames into the S-VLAN of the service provider, and the outbound mapper removes
the S-tag toward the service provider:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # An edge bridge of the core accepts frames
   :end-at: *.peb*.bridging.vlanPolicy.outboundMapper.mappedVlanIds

The core bridge carries both S-VLANs on both of its ports:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The core bridge relays by S-VLAN only.

Finally, ``pebW`` records the traffic of its port toward service provider 1
and of its port toward the core bridge into PCAP files:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: # The port of pebW toward service provider 1
   :end-at: pcapRecorder[1].pcapFile

A Frame's Path through an Edge Bridge
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section follows a ping request of customer A through ``pebW``, from
service provider 1 toward the core bridge, using the module structure shown in
`Inside the Bridges`_.

1. The frame arrives on ``eth0`` from ``sp1w``, C-tagged with C-VLAN 10. The
   ``ethernet`` layer removes the MAC header, and the dispatchers send the frame
   directly to the ``bridging`` layer, because it has no S-tag.
2. In the ``vlanPolicy``, the ``inboundFilter`` sees no S-VLAN, that is,
   ``-1``, which ``eth0`` accepts. The ``inboundMapper`` puts the frame into
   S-VLAN 100.
3. The ``learner`` of the ``interfaceRelay`` learns the source MAC address of
   the frame on ``eth0`` in S-VLAN 100.
4. The ``directionReverser`` forwards the frame, and turns its S-VLAN
   indication into an S-VLAN request.
5. The ``selector`` of the ``interfaceRelay`` looks up the destination MAC
   address in S-VLAN 100, and selects ``eth2``, the port toward ``pb``.
6. The ``outboundMapper`` has no entry for ``eth2``, so the frame keeps S-VLAN
   100, which the ``outboundFilter`` of ``eth2`` accepts.
7. Because the frame has an S-VLAN request, the dispatchers send it to the
   ``ieee8021q`` layer, which inserts an S-tag with VLAN id 100. The C-tag is
   now behind the S-tag. The ``ethernet`` layer adds the MAC header with the
   S-tag's TPID, ``0x88a8``, and the frame leaves on ``eth2``.

The ping reply takes the opposite path. It arrives on ``eth2`` with an S-tag,
so the dispatchers send it to the ``ieee8021q`` layer, which removes the S-tag
and indicates S-VLAN 100. The ``outboundMapper`` of ``eth0`` maps S-VLAN 100 to
``-1``, that is, it removes the S-VLAN request, so the frame bypasses the
802.1Q layer on the way down, and leaves ``eth0`` with its C-tag only.

Note that the C-tag is never touched in ``pebW``: neither the filters nor the
relay look at it.

Results
-------

The First Ping
~~~~~~~~~~~~~~

``hostA1a`` starts pinging 1 ms before the other hosts, so its first exchange
crosses the network alone. The following video shows it: ``hostA1a`` first
resolves the MAC address of ``hostA2a`` with ARP, then sends the ping request,
and ``hostA2a`` answers:

.. video:: media/ping.mp4
   :width: 100%

   <!--internal video recording, run until t=0.999ms, then run with animation until the ping reply reaches hostA1a; playback speed 3x-->

The ARP request is a broadcast, so the bridges send it to all ports of its
VLAN. It never reaches service provider 2: ``sp1w`` sends it only in C-VLAN 10,
and ``pebW`` and ``pebE`` send it only in S-VLAN 100, that is, toward ``pb``
and toward service provider 1. The frames on the links between ``pebW``,
``pb`` and ``pebE`` carry both tags.

Frames on the Wire
~~~~~~~~~~~~~~~~~~

The PCAP files of ``pebW`` show the ping request of ``hostA1a`` on both sides
of the edge bridge. On ``eth0``, toward service provider 1, the frame carries
only the C-tag of customer A:

.. code-block:: text

   $ tshark -r results/General-#0-pebW_eth0.pcap -V -O eth,vlan,ieee8021ad \
       -Y 'icmp.type==8 && ip.src==10.1.10.1 && icmp.seq==0'
   Frame 3: 106 bytes on wire (848 bits), 106 bytes captured (848 bits) on interface eth0, id 0 (inbound)
   Ethernet II, Src: 02:00:00:00:0a:1a (02:00:00:00:0a:1a), Dst: 02:00:00:00:0a:2a (02:00:00:00:0a:2a)
       Type: 802.1Q Virtual LAN (0x8100)
   802.1Q Virtual LAN, PRI: 0, DEI: 0, ID: 10
       Type: IPv4 (0x0800)
       Trailer: 7e3c659c
   Internet Protocol Version 4, Src: 10.1.10.1, Dst: 10.1.10.3
   Internet Control Message Protocol

On ``eth2``, toward the core bridge, the same frame has an S-tag with S-VLAN
100 in front of the unchanged C-tag:

.. code-block:: text

   $ tshark -r results/General-#0-pebW_eth2.pcap -V -O eth,vlan,ieee8021ad \
       -Y 'icmp.type==8 && ip.src==10.1.10.1 && icmp.seq==0'
   Frame 3: 110 bytes on wire (880 bits), 110 bytes captured (880 bits) on interface eth2, id 0 (outbound)
   Ethernet II, Src: 02:00:00:00:0a:1a (02:00:00:00:0a:1a), Dst: 02:00:00:00:0a:2a (02:00:00:00:0a:2a)
       Type: 802.1ad Provider Bridge (Q-in-Q) (0x88a8)
   IEEE 802.1ad, ID: 100
       Type: 802.1Q Virtual LAN (0x8100)
   802.1Q Virtual LAN, PRI: 0, DEI: 0, ID: 10
       Type: IPv4 (0x0800)
       Trailer: b6086a6b
   Internet Protocol Version 4, Src: 10.1.10.1, Dst: 10.1.10.3
   Internet Control Message Protocol

The ping request of ``hostC1a``, a customer of service provider 2, crosses the
same link. Its MAC addresses and its C-tag are identical to those of
``hostA1a``'s frame; only the S-VLAN differs:

.. code-block:: text

   $ tshark -r results/General-#0-pebW_eth2.pcap -V -O eth,vlan,ieee8021ad \
       -Y 'icmp.type==8 && ip.src==10.2.10.1 && icmp.seq==0'
   Frame 23: 110 bytes on wire (880 bits), 110 bytes captured (880 bits) on interface eth2, id 0 (outbound)
   Ethernet II, Src: 02:00:00:00:0a:1a (02:00:00:00:0a:1a), Dst: 02:00:00:00:0a:2a (02:00:00:00:0a:2a)
       Type: 802.1ad Provider Bridge (Q-in-Q) (0x88a8)
   IEEE 802.1ad, ID: 200
       Type: 802.1Q Virtual LAN (0x8100)
   802.1Q Virtual LAN, PRI: 0, DEI: 0, ID: 10
       Type: IPv4 (0x0800)
       Trailer: ce5b75be
   Internet Protocol Version 4, Src: 10.2.10.1, Dst: 10.2.10.3
   Internet Control Message Protocol

The ``Trailer`` lines are the frame check sequences of the frames, which
Wireshark does not recognize as such behind the VLAN tags.

Over the whole simulation, ``pebW``'s port toward service provider 1 carries
only C-tagged frames of the subnets of service provider 1. Its port toward the
core bridge carries S-VLAN 100 with the subnets of service provider 1, and
S-VLAN 200 with those of service provider 2, each over C-VLAN 10 and 20.

MAC Learning in the Core
~~~~~~~~~~~~~~~~~~~~~~~~

At the end of the simulation, the MAC forwarding table of the core bridge
``pb`` holds 16 entries for the 8 different MAC addresses of the hosts: each
address is learned once in S-VLAN 100 and once in S-VLAN 200. Interface 100 is
``eth0``, toward ``pebW``, and interface 101 is ``eth1``, toward ``pebE``:

.. figure:: media/pb_mactable.png
   :width: 70%
   :align: center

The two copies of each address do not interfere, because they are learned in
different S-VLANs.

Ping Results
~~~~~~~~~~~~

Every host sent 5 ping requests and received 5 replies, so all 80 pings were
answered, with 0% loss:

.. code-block:: text

   host      sent  received  loss
   hostA1a   5     5         0%
   hostA1b   5     5         0%
   hostA2a   5     5         0%
   hostA2b   5     5         0%
   hostB1a   5     5         0%
   hostB1b   5     5         0%
   hostB2a   5     5         0%
   hostB2b   5     5         0%
   hostC1a   5     5         0%
   hostC1b   5     5         0%
   hostC2a   5     5         0%
   hostC2b   5     5         0%
   hostD1a   5     5         0%
   hostD1b   5     5         0%
   hostD2a   5     5         0%
   hostD2b   5     5         0%

The numbers come from the ``Pings sent``, ``pingRxSeq:count`` and ``ping loss
rate (%)`` scalars of the ping applications.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`SharedCoreShowcase.ned <../SharedCoreShowcase.ned>`, :download:`addresses.xml <../addresses.xml>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/ethernet/sharedcore`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.8 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.8.*/showcases/ethernet/sharedcore && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.8
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.
