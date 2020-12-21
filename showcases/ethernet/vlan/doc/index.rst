:orphan:

Ethernet Virtual LAN
====================

Goals
-----

This showcase demonstrates the Virtual Local Area Network (VLAN) support of INET's modular Ethernet model.

A physical Ethernet network can be separated into virtual networks at the data link layer, so that these virtual LANs behave as if they were physically separate networks. (Virtual LANs are described in `IEEE 802.1Q <https://en.wikipedia.org/wiki/IEEE_802.1Q>`__.)

VLAN Overview
-------------

Ethernet VLANs are created by assigning VLAN IDs to Ethernet frames. Each frame can only be assigned to one VLAN. The VLAN-aware parts of the network can filter traffic at interfaces, based on VLAN ID.

.. **TODO** 802.1q nem header hanem tag...link a megfelelo message fileba 8021qtagheader.msg -> itt le van dokumentalva

VLAN IDs are assigned by adding a VLAN tag (or 802.1Q tag) to Ethernet frames. This tag contains the VLAN ID. (For more information, see the Ieee8021qTagHeader.msg file). **TODO** link

.. note:: VLAN connections are a subset of the physical connections in the network.  

INET's VLAN support
-------------------

Overview
~~~~~~~~

   In INET, various modules (such as apps, interfaces and policy submodules) can add a VLAN tag request (:cc:`VlanReq`) to packets. The actual VLAN tag is then added by the 802.1q submodule (:ned:`Ieee8021qProtocol`) in the same network node. 

   Some interfaces of the network need to be VLAN-aware, i.e. be able to understand 802.1q headers.
   These are `outgoing` interfaces that either filter packets based on VLAN ID, or map VLAN IDs on packets.
   Interfaces in the network without VLAN-awareness can still forward packets with 802.1q headers.

   The requirements for an interface to be VLAN-aware are the following:

   - The containing network node needs to be set to have a 802.1q submodule:

   ``*.switch.ieee8021q.typename = "Ieee8021qProtocol"``

   - The interface needs to be set to connect to the 802.1q submodule:

   ``*.switch.eth[0].protocol = "ieee8021qctag"``

   Specifying the :par:`protocol` parameter configures the interface to use the protocol services of the 802.1q module, i.e. to send packets coming from higher layers to the 802.1q module. The module can add a 802.1q header if the packet doesn't already have one.

   Several VLAN schemes/configurations are possible with INET's Ethernet model. For example, as demonstrated in this showcase, VLAN IDs can be assigned by a host's interface or in an Ethernet switch. Some other possibilities are the following:

   - Assigning VLAN IDs in a network node's 802.1q submodule
   - Different VLAN IDs for different traffic directions
   - VLAN Double tagging

   For more examples, check out ``inet/examples/ethernet/vlan``.

 **TODO** this shouldnt work like this

VLAN use cases
~~~~~~~~~~~~~~

**TODO** :cc:`VlanReq` everywhere (no VLAN req)

from the perspective of the use cases:

- simplest case: packet receives VLAN ID in application, travels the VLAN to its destination (has the same VLAN ID)
- second: packet is sent by application (app doesnt know about VLAN). Virtual interface adds VLAN ID, travels the VLAN to its destination (has the same VLAN ID)
- third: host sends packet (host doesnt know about VLAN). Arrives at first switch, switchs adds VLAN ID. Last switch removes VLAN ID -> need VLAN policy in switch
- forth: host sends packet (host doesnt know about VLAN). complex network with several subnetworks. VLAN ID changes (packet goes from one VLAN to another) -> need VLAN policy in switch

**TODO** Introduce VLAN policy

   Typically applications and virtual interfaces can add a certain VLAN ID :cc:`VlanReq` to packets 
   Various modules in hosts can assign a specific VLAN ID to packets, such as apps and virtual interfaces.
   Howeever, for VLANs to be more complex, we can set up policies that govern how packets are assigned to VLANs.
   In Ethernet switches, VLAN policies specify how to filter and re-map VLAN IDs on packets, based on incoming and outgoing interfaces.

Creating VLAN policies
~~~~~~~~~~~~~~~~~~~~~~

VLAN policies can be configured using the :ned:`VlanPolicyLayer` submodule, which is an optional submodule of the bridging layer in :ned:`EthernetSwitch`. (The bridging layer contains optional submodules such as **TODO**)

To add a :ned:`VlanPolicyLayer` to :ned:`EthernetSwitch`, set the ``bridging`` submodule's type in the switch to :ned:`Bridging`, then the ``vlanPolicy`` submodule's type to :ned:`VlanPolicyLayer` in the briding module.

bridgingben amikor visszakanyarodik a csomag -> ind becomes req (incoming -> outgoing)(typically ind -> req)

Policy introduction: Policy filters incoming packets based on VLAN ID.... -> incoming mapper -> change the VLAN ID so it appears to have come from that VLAN ID

-> it can change and filter VLAN IDs

VLAN Policy can filter out incoming and outgoing packets based on their VLAN ID, and optionally can also change the packets' VLAN ID. (VLAN ID in this case is some metainformation attached to the packet, in contrast to the actual 802.1Q tag VLAN ID field).

**TODO** What is VLanInd and VlanReq; who puts it on the packets (ind: 802.1Q tag processing, e.g. :ned:`Ieee8021qTagEpdHeaderChecker`).

VlanReq is typically added by an applications, a virtual interface, or the bridging (forwarding component in the bridging layer (which makes ind -> req turn-around);
but in general any component/protocol can.

VlanInd is typically added by the header...

The :ned:`VlanPolicyLayer` module can filter incoming packets based on :cc:`VlanInd`, and outgoing packets based on :cc:`VlanReq`. It can also modify :cc:`VlanInd` and :cc:`VlanReq` on packets. The policy module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (inbound/outbound). By default, the module has the following submodules:

.. figure:: media/policy.png
   :align: center

   Figure X. Submodules present by default in :ned:`VlanPolicyLayer`
   
.. note:: The submodules of :ned:`VlanPolicyLayer` can be disabled by setting their type to an omitted type. Omitted-type modules are not there in the simulation, but the other modules are connected properly. To disable mapper modules, set their type to :ned:`OmittedPacketFlow`; to disable filter modules, set the type to :ned:`OmittedPacketFilter`.

In the filter submodules, the set of allowed VLAN IDs can be specified with the :par:`acceptedVlanIds` parameter, using the following syntax (VLAN ID -1 means the packet has no VLAN tag):

.. code-block:: text

   **.acceptedVlanIds = {"interface" : [vlanid1, vlanid2, ... ], ... } 

For example, allowing VLAN IDs 1 and 2 on interface ``eth1``, and VLAN ID 3 on interface ``eth2``:

.. code-block:: ini

   **.acceptedVlanIds = {"eth1" : [1, 2], "eth2" : [3]}

In the mapper submodules, VLAN ID mappings can be specified with the :par:`mappedVlanIds` parameter, using the following syntax:

.. code-block:: text

   **.mappedVlanIds = {"interface" : {"orig vlan id" : mapped vlan id}, ... } 
   
For example, mapping untagged packets (VLAN ID -1) of eth1 to VLAN ID 1, and those of eth2 to VLAN ID 2:

.. code-block:: ini

   mappedVlanIds = {"eth1" : {"-1" : 1}, "eth2" : {"-1" : 2}}

Here is a VLAN tag in Qtenv (VLAN ID highlighted):

.. figure:: media/vlantag.png
   :align: center

   Figure X. VLAN tag in Qtenv

The Model and Results
---------------------

We'll demonstrate Ethernet VLANs with the following configurations:

- ``VLAN Between Switches``: Packets are assigned to VLANs by Ethernet switches
- ``VLAN Between Hosts Using Virtual Interfaces``: Packets are assigned to a VLAN by a host's virtual interface

Config: VLAN Between Switches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As mentioned above, packets are assigned to VLANs in Ethernet switches in this simulation. The switches also divide the network into two VLANs by filtering packets based on VLAN ID.

The simulation uses the following network (also reused for the next example simulation), containing six hosts (:ned:`StandardHost`) and two switches (:ned:`EthernetSwitch`):

.. figure:: media/network10.png
   :align: center

   Figure X. The network and the upper and lower VLANs

Our goal is to separate the network into two virtual networks (upper and lower), with three hosts each, indicated on the image with red and blue tint of the hosts. We assign VLAN ID 1 to the upper VLAN, and VLAN ID 2 to the lower VLAN.
To demonstrate that the two VLANs are separate, we broadcast packets from host1 and host2 to the entire network, and observe that they are only delivered to hosts in their originating VLAN. 

Note that IP addresses are assigned by default so that all hosts are in the same subnet (addresses are sequential from 10.0.0.1 to 10.0.0.6). We can use the 10.0.0.7 address to broadcast packets. 

Let's see some of the settings that are shared by the two simulations. Here is part of the ``General`` configuration with Ethernet settings:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap
   :end-at: bitrate
   :language: ini

The first three lines configure all network nodes to use the composable Ethernet model, and the last one sets the Ethernet connection speed on all interfaces.

**TODO** so the switches dont need a socket to connect to the 8021q module ?

As mentioned above, the switches need a bridging module, and a 802.1Q submodule:

.. literalinclude:: ../omnetpp.ini
   :start-at: bridging
   :end-at: ieee8021q
   :language: ini

Let's see some of the VLANBetweenSwitches .ini configuration/Let's see the traffic and VLAN settings in the ``VLANBetweenSwitches`` .ini configuration. 

Traffic Settings
++++++++++++++++

The traffic generator UDP apps in host1 and host4 are configured to broadcast UDP packets to the entire subnet. The VLAN the packets belong to (lower or upper) is added to the packet name, so we can tell them apart in Qtenv's animations:

.. literalinclude:: ../omnetpp.ini
   :start-at: destAddresses
   :end-at: UdpBasicAppData-lower
   :language: ini

VLAN Settings
+++++++++++++

As mentioned above, our goal is to divide the hosts in the network into two VLANs, upper and lower. To do that, we configure the following VLAN policies in the two switches:

.. figure:: media/12.png
   :align: center

   Figure X. The configured VLAN policies

.. **TODO** this could be smaller

To summarize:

- Switch1 assigns VLAN IDs to packets based on incoming interface
- Switch2 only sends packets to the appropriate hosts, so that packets stay in their own VLAN

VLAN IDs are assigned by switches1, so the only part of the network that needs to be "VLAN-aware", i.e. understand 802.1q protocol headers in Ethernet frames, is the switch1's outgoing interface.
We configure this interface to understand 802.1q protocol headers/use 802.1q protocol services:

.. literalinclude:: ../omnetpp.ini
   :start-at: *.switch1.eth[2].protocol = "ieee8021qctag"
   :end-at: *.switch1.eth[2].protocol = "ieee8021qctag"
   :language: ini

How is the above policy configured in :ned:`VlanPolicyLayer`? We configure the switches to have a VLAN policy layer, and specify the policies themselves in each switch:

.. literalinclude:: ../omnetpp.ini
   :start-at: VlanPolicyLayer
   :end-at: switch2.bridging.vlanPolicy.outboundFilter.acceptedVlanIds
   :language: ini

Let's explain what this does:

- In switch1, we don't want to filter inbound packets, so we disable the inbound filter
- We need the optional inbound mapper module to assign VLAN IDs to packets based on which interface they come in on, so we set the inbound submodule type to :ned:`VlanIndMapper`
- The mapper assigns VLAN ID 1 to untagged packets incoming on eth0, and VLAN ID 2 to those incoming on eth2
- The outbound mapper isn't needed either, so we disable it
- In switch2, we only need the outbound filter. The filter only allows packets towards their own VLAN.
- The other submodules of switch2 (inbound filter and both mappers) are disabled

.. note:: This VLAN policy configuration is only for one direction (enough for UDP). Policies for the other direction can be set up similarly if needed (e.g. for TCP). Also, instead of disabling modules, we could have set them to allow all packets or not map and VLAN IDs at all, for the same effect.

Results
+++++++

Here is a video of the simulation:

.. video:: media/vlan3.mp4
   :align: center
   :width: 100%

As expected, the broadcast packets stay within their VLANs.

Config: VLAN Between Hosts Using Virtual Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this configuration, a host interface adds VLAN tags to packets, and switches filter packets according to configured VLAN policies.
The configuration uses the same network as the previous one (the host tints are irrelevant here). However, in addition to the Ethernet interfaces (``eth``), host1 and host2 each contain a virtual interface (``virt``) as well:

.. figure:: media/virt.png
   :align: center

   **Figure X. Virtual interface in host**

.. note:: A virtual interface (:ned:`VirtualInterface`) acts as a real network interface, i.e. it has its own IP address, and packets can be sent to and from it. The virtual interface uses a real interface to transmit packets on. These packets have the virtual interface's address as source or destination IP address. The interface's functionality is implemented by its tunnel submodule (:ned:`VirtualTunnel`). 

Our goal is to make host2 accessible from host1 only through a virtual tunnel, i.e. only for traffic going between the two hosts' virtual interfaces. To that end, we configure the VLAN policy submodule in switch2 to forward only VLAN-tagged packets on its interface connected to host2). 

Two UDP applications in host1 send packets to host2: one of them via the two hosts' Ethernet interfaces, the other via the virtual interfaces. The virtual interfaces use the hosts' Ethernet interfaces to carry the packets. The packets sent between the virtual interfaces are configured to be VLAN tagged and the ones between the Ethernet interfaces are not.

The simulation is defined in the ``VirtualInterfaces`` configuration in omnetpp.ini. Let's see some excerpts from the configuration below. 

Virtual Interface Settings
++++++++++++++++++++++++++

First, we configure host1 and host2 to have a virtual interface:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config VirtualInterfaces2
   :end-at: numVirtInterfaces
   :language: ini

Some parameters of the virtual tunnel need to be configured, such as which real interface to send packets on, and how to connect to the 802.1q submodule of the host. 
The tunnel submodule of the virtual interface accesses the 802.1q module via a socket. (In INET, sockets are used by modules, such as applications, to access protocol services in a network node.) We need to enable the 802.1q module's socket so that the tunnel can use the module's protocol service:

.. literalinclude:: ../omnetpp.ini
   :start-at: *.host{1..2}.ieee8021q.hasSocketSupport = true
   :end-at: *.host{1..2}.ieee8021q.hasSocketSupport = true
   :language: ini

Here is the configuration for the virtual interface's tunnel submodule:

.. literalinclude:: ../omnetpp.ini
   :start-at: realInterface
   :end-at: vlanId
   :language: ini

The tunnel/virtual interface is set to use the real Ethernet interface (eth0) for sending packets. The :par:`protocol` parameter selects that the tunnel should connect to the 802.1q protocol module via a socket. Finally, the VLAN ID is specified; vlan request tags are inserted by the tunnel to outgoing packets (the 802.1q module adds the VLAN tags with the specified VLAN ID when the packets get to it).

The virtual interfaces require manual routing configuration. The Ipv4NetworkConfigurator module can assign addresses to virtual interfaces automatically, but cannot set up routes for them because it does not know where the virtual interfaces are in the network topology (i.e. which real interface they use). Thus routes for the virtual interfaces need to be added to the configator's xml config file:

.. literalinclude:: ../config_virt.xml
   :language: xml

.. note:: We use ``globalArp``, but the simulation works with the default ARP as well. The arp packets that originate in the virtual interface are VLAN-tagged, so they can reach host2.

Traffic Settings
++++++++++++++++

The only traffic in the simulation is generated by two UDP apps in host1, one targetting host2's eth interface, the other host2's virt interface:

.. literalinclude:: ../omnetpp.ini
   :start-after: *.host1.app[*].typename = "UdpBasicApp"
   :end-at: destAddresses = "host2%virt0"
   :language: ini

The apps are also configured to include the outgoing interface in the packet name (eth and virt), so that we can tell the packets apart in Qtenv.

VLAN Settings
+++++++++++++

Our goal is to make host2 only accessible via the two virtual interfaces, so we configure the following VLAN policy:

.. figure:: media/virt_schematic.png
   :align: center

   Figure 4: The configured VLAN policies

The outgoing interfaces of switch1 filter packets based on VLAN ID, so they need to be VLAN-aware:

.. literalinclude:: ../omnetpp.ini
   :start-at: switch1.eth[2].protocol = "ieee8021qctag"
   :end-at: switch1.eth[2].protocol = "ieee8021qctag"
   :language: ini

This policy is configured in omnetpp.ini as follows:

.. literalinclude:: ../omnetpp.ini
   :start-after: GlobalArp
   :end-at: outboundFilter
   :language: ini

Packets sent by host1's virtual interface have a VLAN ID of 1. We only need to have an outbound filter submodule in the VLAN policy layer module in switch2, which we configure to only allow packets with VLAN ID 1 on its eth1 interface (the one towards host2). Packets without VLAN tags are allowed on the other interfaces. 
/The other interfaces of switch2 only allow packets without VLAN tags.
The inbound filter and both mapper submodules of VlanPolicy are not needed, so they are disabled.

Results
+++++++

Here is a video of the simulation:

.. video:: media/virt2.mp4
   :align: center
   :width: 100%

As expected, the ``eth`` packets are filtered in switch2, but the ``virt`` packets are forwarded to host2.