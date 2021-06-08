:orphan:

Ethernet Virtual LAN
====================

Goals
-----

.. **V1** Ethernet Virtual Local Area Networks (VLANs) separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs. The Ethernet Virtual LAN (VLAN) feature is supported by the IEEE 802.1Q standard. 

This showcase demonstrates the Virtual Local Area Network (VLAN) support of INET's modular Ethernet model.

A physical Ethernet network can be separated into virtual networks at the data link layer, so that these virtual LANs behave as if they were physically separate networks. (Virtual LANs are described in `IEEE 802.1Q <https://en.wikipedia.org/wiki/IEEE_802.1Q>`__.)

..    so

    - VLANs are created by adding VLAN tags to packets
    - the tags are added by adding a 802.1q header to ethernet packets
    - the parts of the network that supports VLANs filter traffic based on the VLAN tags

    - Various modules can add VLAN request tags to packets
    - The tags are added by the 802.1q submodule / in VLAN-aware network nodes
    - As such, the presence of the 802.1q module makes network nodes (typically switches?) VLAN-aware

    - So hosts can add VLAn request tags to packets
    - So that when the packets enter the VLAN-aware part of the network, the VLAN tags are added based on the request
    - Also, the policy submodule can do that. It also filters packets based on VLAN tags, and can modify VLAN tags
    - The whole thing supports double tagging (where 802.1q headers are nested) and virtual interfaces as well

VLAN Overview
-------------

.. **V1** Ethernet VLANs are created by assigning VLAN IDs to Ethernet frames. This is done by adding an extra header (802.1q header) containing a VLAN tag that stores the VLAN ID. The VLAN-aware parts of the network (typically Ethernet switches) filter traffic at interfaces, based on this VLAN ID. Packets without VLAN tags are considered to be part of the native VLAN **(which can be allowed/disallowed as well).** 
   **Interfaces can also remap VLAN IDs.** The VLAN mechanism removes connections from a physical network by filtering packets at the data link layer, but cannot create new connections. 

.. **TODO** "which can be allowed/disallowed..." too INET specific, not here; "remap", might be INET specific; wikipedia links?

.. **V2** Ethernet VLANs are created by assigning VLAN IDs to Ethernet frames. This is done by adding an extra header (802.1q header) containing a VLAN tag that stores the VLAN ID. Each frame can only be within one VLAN. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN ID. Packets without VLAN tags are considered to be part of the native VLAN. The VLAN mechanism removes connections from a physical network by filtering packets at the data link layer, but cannot create new connections. 

Ethernet VLANs are created by assigning VLAN IDs to Ethernet frames. Each frame can only be assigned to one VLAN. The VLAN-aware parts of the network can filter traffic at interfaces, based on VLAN ID.

.. **TODO** 802.1q nem header hanem tag...link a megfelelo message fileba 8021qtagheader.msg -> itt le van dokumentalva

VLAN IDs are assigned by adding a VLAN tag (or 802.1Q tag) to Ethernet frames. This tag contains the VLAN ID. (For more information, see the Ieee8021qTagHeader.msg file).

.. note:: The VLAN mechanism removes connections from a physical network (by filtering packets at the data link layer), but cannot create new connections. 

.. Portions of the network which are VLAN-aware (i.e., IEEE 802.1Q conformant) can include VLAN tags. When a frame enters the VLAN-aware portion of the network, a tag is added to represent the VLAN membership.Each frame must be distinguishable as being within exactly one VLAN. A frame in the VLAN-aware portion of the network that does not contain a VLAN tag is assumed to be flowing on the native VLAN. (from Wikipedia). **TODO** is this needed?

.. **TODO** "which can be allowed/disallowed..." too INET specific, not here; "remap", might be INET specific; wikipedia links?

.. Interfaces can also remap VLAN IDs. TODO place

INET's VLAN support
-------------------

Overview
~~~~~~~~

.. **V1** Ethernet VLANs are created by adding an extra header to Ethernet frames. The header contains VLAN tags. The VLAN-aware parts of the network filter traffic based on these VLAN tags. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN tags and VLAN ID. Each interface has a set of allowed VLAN IDs. Packets without VLAN tags are considered to be part of the native VLAN and are not filtered.

.. and are not filtered.

.. **V1** This basically takes away connections from a physical network by filtering; it obviously cannot add new connections.

.. **V3** This mechanism restricts connections in an existing physical network, but cannot add new connections.

.. Ethernet VLANs are created by assigning VLAN IDs to ethernet frames, by adding an extra header containing VLAN tags which store the VLAN ID.

.. In INET, several modules...

In INET, various modules (such as apps, interfaces and policy submodules) can add a VLAN tag request to packets. The actual VLAN tag is then added by a 802.1q submodule (:ned:`Ieee8021qProtocol`). Its possible that a VLAN tag request is added in one network node and the VLAN tag is added in another (e.g. an app in a host adds request, the 802.1q submodule in a switch adds tag).

.. **TODO** example for this process somewhere (for example, host's app adds request, connected switch's 802.1q module adds tag)

.. **TODO** (even if its in another host which is vlan aware/has 802.1q module)

.. As such, the presence of the 802.1q submodule makes the network node VLAN-aware. **TODO** meaning what ?
   that the network node can read VLAN tags on packets ?
   -> the presence of the 802.1q submodule makes the network node able to filter packets based on VLAN tags ?

.. - some parts of the network/some interfaces need to be VLAN-aware...that is be able to understand
   802.1q headers because it either manipulates or does something based on that
   - that is set in the Ethernet interface

.. - interfaces which are supposed to add VLAN tags/802.1q headers to packets should be VLAN-aware, i.e.
   be able to understand 802.1q headers, by setting the protocol parameter to 802.1qtag
   so that if a packet going out the interface doesnt have a 802.1q header is sent to the 802.1q module, which adds it if it doesnt have one

.. - if an outgoing interface has to filter packets based on VLAN tag or map VLAN tags if needs to be VLAN-aware
   - other interfaces don't but they still can forward packets with VLAN tags 802.1q headers

Some interfaces of the network need to be VLAN-aware, i.e. be able to understand 802.1q headers.
These are `outgoing` interfaces that either filter packets based on VLAN ID, or map VLAN IDs on packets.
Interfaces in the network without VLAN-awareness can still forward packets with 802.1q headers.

The requirements for an interface to be VLAN-aware are the following:

- The containing network node needs to be set to have a 802.1q submodule:

  ``*.switch.ieee8021q.typename = "Ieee8021qProtocol"``

- The interface needs to be set to connect to the 802.1q submodule:

  ``*.switch.eth[0].protocol = "ieee8021qctag"``

.. This configures the interface to send packets coming from higher layers to the 802.1q module, so that the module can add a 802.1q header if the packet doesn't already has one./Use the protocol services of the 802.1q module

Specifying the :par:`protocol` parameter configures the interface to use the protocol services of the 802.1q module, i.e. to send packets coming from higher layers to the 802.1q module. The module can add a 802.1q header if the packet doesn't already have one.

.. **V1** Several configurations are possible, such as the following: 

  - Assigning VLAN IDs in a network node's 802.1q submodule
  - Different VLAN IDs for different traffic directions
  - VLAN Double tagging

Several VLAN schemes/configurations are possible with INET's Ethernet model. For example, as demonstrated in this showcase, VLAN IDs can be assigned by a host's interface or in an Ethernet switch. Some other possibilities are the following:

- Assigning VLAN IDs in a network node's 802.1q submodule
- Different VLAN IDs for different traffic directions
- VLAN Double tagging

.. For more possibilities, see the VLAN example (``inet/examples/ethernet/vlan``).

.. For such examples, see ``inet/examples/ethernet/vlan``.

For more examples, check out ``inet/examples/ethernet/vlan``.

.. **v2** You can find more examples in ``inet/examples/ethernet/vlan``.

.. **TODO** protocol = 8021qtag

.. **TODO** 802.1q submodule NED type

.. The VlanPolicyLayer submodule
   -----------------------------

Creating VLAN policies
~~~~~~~~~~~~~~~~~~~~~~

.. **TODO** what are VLAN policies?

.. **TODO** how to add policy submodule?

.. **TODO** what it is, where it is and how to add it

.. Various modules in hosts can assign VLAN IDs with static mapping.
   However, we can set up policies with more complex mappings, such as .
   In Ethernet switches, VLAN policies specify this, based on incoming and outgoing interfaces, and also how packets are filtered on interfaces **TODO**
   VLAN policies can be created using the :ned:`VlanPolicyLayer` submodule, which is an optional submodule of the bridging layer in :ned:`EthernetSwitch`. (The bridging layer contains optional submodules such as **TODO**)

Various modules in hosts can assign a specific VLAN ID to packets, such as apps and interfaces.
Howeever, for VLANs to be more complex, we can set up policies that govern how packets are assigned to VLANs.
In Ethernet switches, VLAN policies specify how to filter and re-map VLAN IDs on packets, based on incoming and outgoing interfaces.

VLAN policies can be created using the :ned:`VlanPolicyLayer` submodule, which is an optional submodule of the bridging layer in :ned:`EthernetSwitch`. (The bridging layer contains optional submodules such as **TODO**)

.. In Ethernet switches, VLAN policies specify this, based on incoming and outgoing interfaces, and also how packets are filtered on interfaces **TODO**

.. /how packets are assigned to VLANs

.. **TODO** also, VLAN IDs can be assigned in hosts' interfaces as well (static mapping?)

.. In Ethernet switches, VLAN policies specify how packets are assigned to VLANs, and how they are filtered on interfaces.

.. In Ethernet switches, VLAN policies specify the set of allowed VLAN IDs at various incoming and outgoing interfaces, and can add/alter VLAN tags.

.. In Ethernet switches, VLAN policies specify the set of allowed VLAN IDs at various incoming and outgoing interfaces, and can map vlan ids

.. **TODO** what is the bridging layer and defaults ?

.. To add it, the ``vlanPolicy`` submodule's type needs to be specified.

.. :ned:`EthernetSwitch` does not contain a VLAN policy layer by default. 

.. It is an optional submodule of :ned:`EthernetSwitch`.

.. Here is the VLAN policy layer submodule inside an :ned:`EthernetSwitch`:

To add a :ned:`VlanPolicyLayer` to :ned:`EthernetSwitch`, set the ``bridging`` submodule's type in the switch to :ned:`Bridging`, then the ``vlanPolicy`` submodule's type to :ned:`VlanPolicyLayer` in the briding module.

.. **TODO** example

.. .. figure:: media/switch2.png
   :align: center

.. **V1** The module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (incoming/outgoing). **TODO** how do they become optional? -> omitted type

   By default, the module has the following modules:

   .. figure:: media/policy.png
      :align: center

The :ned:`VlanPolicyLayer` module can filter packets based on VLAN tags, and modify VLAN tags on packets. The policy module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (inbound/outbound). By default, the module has the following submodules:

.. figure:: media/policy.png
   :align: center

   Figure X. Submodules present by default in :ned:`VlanPolicyLayer`
   
.. The default submodules of the Vlan Policy Layer module**

.. .. note:: (we set it to an omitted type, so that the module is not there in the simulation, but the other modules are connected properly) **TODO** omitted thing not here but INET's VLAN support section

.. note:: The submodules of :ned:`VlanPolicyLayer` can be disabled by setting their type to an omitted type. Omitted-type modules are not there in the simulation, but the other modules are connected properly. To disable mapper modules, set their type to :ned:`OmittedPacketFlow`; to disable filter modules, set the type to :ned:`OmittedPacketFilter`.

.. **TODO** should be called no-op filter; it doesnt appear at runtime

In the filter submodules, the set of allowed VLAN IDs can be specified with the :par:`acceptedVlanIds` parameter, using the following syntax (VLAN ID -1 means the packet has no VLAN tag):

.. code-block:: text

   **.acceptedVlanIds = {"interface" : [vlanid1, vlanid2, ... ], ... } 

.. For example, allowing VLAN IDs 1 and 2 on interface ``eth2``:

   .. literalinclude:: ../omnetpp.ini
      :start-at: switch1.bridging.vlanPolicy.outboundFilter.acceptedVlanIds
      :end-at: switch1.bridging.vlanPolicy.outboundFilter.acceptedVlanIds
      :language: ini

For example, allowing VLAN IDs 1 and 2 on interface ``eth1``, and VLAN ID 3 on interface ``eth2``:

.. code-block:: ini

   **.acceptedVlanIds = {"eth1" : [1, 2], "eth2" : [3]}

In the mapper submodules, VLAN ID mappings can be specified with the :par:`mappedVlanIds` parameter, using the following syntax:

.. code-block:: text

   **.mappedVlanIds = {"interface" : {"orig vlan id" : mapped vlan id}, ... } 
   
.. For example: **TODO**

For example, mapping untagged packets (VLAN ID -1) of eth1 to VLAN ID 1, and those of eth2 to VLAN ID 2:

.. code-block:: ini

   mappedVlanIds = {"eth1" : {"-1" : 1}, "eth2" : {"-1" : 2}}

.. .. literalinclude:: ../omnetpp.ini
      :start-at: switch1.policy.outboundFilter.acceptedVlanIds
      :end-at: switch1.policy.outboundFilter.acceptedVlanIds
      :language: ini

.. Some explanation on the syntax for the mapper:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: switch1.bridging.vlanPolicy.inboundMapper.mappedVlanIds
   :end-at: switch1.bridging.vlanPolicy.inboundMapper.mappedVlanIds
   :language: ini

.. **TODO** adds vlan id request/indication tags

.. **TODO** vlan tags in qtenv ?

Here is a VLAN tag in Qtenv (VLAN ID highlighted):

.. figure:: media/vlantag.png
   :align: center

   Figure X. VLAN tag in Qtenv

.. **TODO** [-1] is [none]

.. so

    - The VlanPolicyLayer module can filter incoming and outgoing packets based on VLAN ID
    - It can also re-map VLAN tags/IDs
    - The VlanPolicyLayer has four optional submodules: a mapper and a filter module for each direction

   .. figure:: media/vlanpolicylayer2.png
      :align: center

.. so

    VLAN:

    - Create Virtual LANs by filtering traffic at the data link layer
    - Acts as physically separate LANs
    - Implemented by 802.1Q
    - Extra header to Ethernet frames containing VLAN ID
    - Actually filters physical connections
    - Works on interfaces
    - Set of allowed VLAN IDs on the interface

    - Several modules can attach VLAN tag requests to packets
    - The VLAN tag/VLAN header will be put on by the 802.1q module

    - native VLAN
    - VLAN aware part of the network (switches typically)
    - several configurations, double tagging, etc

.. the structure:

   - what is VLAN and how is it implemented (data link layer filtering, act like different physical networks, vlan tags, 802.1q, extra header, interface filtering)

.. the structure:

    - what is VLAN?

      data link layer filtering, act like different physical networks, VLAN tags, extra header, interface filtering (allowed VLAN tags on an interface, implemented by 802.1q, wlan aware part mostly switches, native vlan, filtering only (no routing or creating new physical connections obviously)

    - how is it in INET?

      several modules can attach VLAN tag requests to packets (for example what)
      the tags are attached by the 802.1q module (even if its in another host which is vlan aware/has 802.1q module

    - some configurations thats possible

      double tagging, virtual interfaces, etc 

    - the vlan policy module

.. The Model/Configuration/The example simulations
   -----------------------------------------------

The Model and Results
---------------------

.. Ethernet VLANs are demonstrated with two configurations:

.. structure:

  - The model
    
     - Config x
     - Config y
    
  - Results

.. Overview
   ~~~~~~~~

We'll demonstrate Ethernet VLANs with the following configurations:

- ``VLAN Between Switches``: Packets are assigned to VLANs by Ethernet switches
- ``VLAN Between Hosts Using Virtual Interfaces``: Packets are assigned to a VLAN by a host's virtual interface

Config: VLAN Between Switches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As mentioned above, packets are assigned to VLANs in Ethernet switches in this simulation. The switches also divide the network into two VLANs by filtering packets based on VLAN ID.

The simulation uses the following network (also reused for the next example simulation), containing six hosts (:ned:`StandardHost`) and two switches (:ned:`EthernetSwitch`):

.. .. figure:: media/network.png
      :align: center

.. .. figure:: media/network5.png
   :align: center

.. .. figure:: media/network6.png
   :align: center

.. .. figure:: media/network7.png
   :align: center

.. figure:: media/network10.png
   :align: center

   Figure X. The network and the upper and lower VLANs

.. **TODO** crop eth0

.. It contains a network of two switches (:ned:`EthernetSwitch`). A host (:ned:`StandardHost`) is connected to each switch.

.. The network contains six hosts (:ned:`StandardHost`) and two switches (:ned:`EthernetSwitch`). 

.. **TODO** the scenario...two VLANs, upper and lower

Our goal is to separate the network into two virtual networks (upper and lower), with three hosts each, indicated on the image with red and blue tint of the hosts. We assign VLAN ID 1 to the upper VLAN, and VLAN ID 2 to the lower VLAN.
To demonstrate that the two VLANs are separate, we broadcast packets from host1 and host2 to the entire network, and observe that they are only delivered to hosts in their originating VLAN. 

Note that IP addresses are assigned by default so that all hosts are in the same subnet (addresses are sequential from 10.0.0.1 to 10.0.0.6). We can use the 10.0.0.7 address to broadcast packets. 

.. **TODO** not here?

.. We just need to broadcast packets using the 10.0.0.7 address. **TODO** not here?

.. /so that we can broadcast packets using the 10.0.0.7 address. **TODO** actually this is the default

.. **TODO** why is it important that we assign sequential IPs?

.. The two switches are configured to be VLAN aware, but the hosts are not. (Thus the switches are responsible for creating the VLANs and adding VLAN tags to packets. **TODO** redundant ?

.. VLAN IDs are assigned by switch one, depending on a packet's incoming interface, so that packets from host1 have VLAN ID 1, and packets from host2 have VLAN ID 2. Furthermode, switch2 sends VLAN ID 1 packets only to the blue tinted hosts, and VLAN ID 2 packets to the red ones. **TODO** redundant ?; merge with "UDP application..."

Let's see some of the settings that are shared by the two simulations. Here is part of the ``General`` configuration with Ethernet settings:

.. we need to configure 

.. literalinclude:: ../omnetpp.ini
   :start-at: encap
   :end-at: bitrate
   :language: ini

The first three lines configure all network nodes to use the composable Ethernet model, and the last one sets the Ethernet connection speed on all interfaces.

.. The VLANs are created/managed by the switches, so the other parts of the network don't even need to be "VLAN-aware", i.e. understand 802.1q protocol headers in Ethernet frames.
   Since the VLAN awareness is only between the two switches, we configure the two interfaces between the two switches to understand 802.1q protocol headers:

.. **TODO** how does that work? or not even here

.. The VLANs are managed by the switches, so the only part of the network that needs to be "VLAN-aware", i.e. understand 802.1q protocol headers in Ethernet frames, is the two interfaces that are between the two switches.
   We configure these two interfaces to understand 802.1q protocol headers:

**TODO** so the switches dont need a socket to connect to the 8021q module ?

.. **TODO** VLANBetweenVirtualInterfaces: where is the 8021q module?

.. **TODO** do switches need 802.1q modules in the virtual interfaces config ?

.. The switches are configured to have a :ned:`Ieee8021qLayer` submodule. 


.. - the configs

   **TODO** traffic



.. For traffic, host1 in the upper VLAN broadcasts UDP packets to host4 in the same VLAN. Similarly, host4 in the lower VLAN send packets to hostX.

.. ----------------------------------------------

   A UDP application in host1 and host4 generate packets, which are broadcast to the entire LAN (to address 10.0.0.7). However, the switches are configured to only deliver packets in the same VLAN they originate from (host1 -> host2 and host3, host4 -> host5 and host6).

   .. (host1 to host2 and host3, host4 to host5 and host6)

   .. **TODO** each is a different subnet -> how about they weren't ? can they have the same address ?

   .. For traffic, one of the hosts in each VLAN send UDP packets to another host in the same VLAN. The switches separate the packets.

   .. **V1** There are two configurations in this showcase:

      - Config ``BetweenSwitches``: All packets between the5switches are assigned to VLAN 42.
      - Config ``VirtualInterface``: All packets between the hosts are assigned to VLAN 42 and use virtual interfaces in the hosts.

   .. Example: VLAN between the Switches
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   .. **TODO** config

   Let's see the relevant parts of configuring VLAN in the switches. In the ``VLANBetweenSwitches`` configuration, the switches are configured to have a VLAN policy layer submodule:

   .. literalinclude:: ../omnetpp.ini
      :start-at: VlanPolicyLayer
      :end-at: VlanPolicyLayer
      :language: ini

   **TODO** not here

   ----------------------------------------------

.. In the ``VLANBetweenSwitches`` configuration, the switches are configured to be VLAN-aware. Since the VLAN awareness is only between the two switches, the interfaces which are between the two switches are configured to understand the 802.1q protocol. **TODO** rewrite

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

.. The configured VLAN policies are illustrated on the following image:

.. figure:: media/12.png
   :align: center

   Figure X. The configured VLAN policies

.. **TODO** this could be smaller

.. So/According to this policy, switch1 assigns VLAN IDs to packets based on incoming interface. Switch2 only sends packets to the appropriate hosts, so that packets stay in their own VLAN.

.. The summary of the policy is the following:

To summarize:

- Switch1 assigns VLAN IDs to packets based on incoming interface
- Switch2 only sends packets to the appropriate hosts, so that packets stay in their own VLAN

VLAN IDs are assigned by switches1, so the only part of the network that needs to be "VLAN-aware", i.e. understand 802.1q protocol headers in Ethernet frames, is the switch1's outgoing interface.
We configure this interface to understand 802.1q protocol headers/use 802.1q protocol services:

.. **TODO** not here -> this is actually config specific

.. The VLAN-aware part of the network is between the two switches, so the two interfaces that are between the who switches need to be vlan aware...

.. the interfaces that are between the two switches are configured to understand the 802.1q protocol.

.. literalinclude:: ../omnetpp.ini
   :start-at: *.switch1.eth[2].protocol = "ieee8021qctag"
   :end-at: *.switch1.eth[2].protocol = "ieee8021qctag"
   :language: ini

How is the above policy configured in :ned:`VlanPolicyLayer`? We configure the switches to have a VLAN policy layer, and specify the policies themselves in each switch:

.. Let's see this VLAN policy configuration in omnetpp.ini/

.. literalinclude:: ../omnetpp.ini
   :start-at: VlanPolicyLayer
   :end-at: switch2.bridging.vlanPolicy.outboundFilter.acceptedVlanIds
   :language: ini

Let's explain what this does:

..  - we map the VLAN IDs in switch1
  - we filter the packets by vlan id in switch2
  - this is only for this one direction

.. **TODO** explain this

- In switch1, we don't want to filter inbound packets, so we disable the inbound filter
- We need the optional inbound mapper module to assign VLAN IDs to packets based on which interface they come in on, so we set the inbound submodule type to :ned:`VlanIndMapper`
- The mapper assigns VLAN ID 1 to untagged packets incoming on eth0, and VLAN ID 2 to those incoming on eth2
- The outbound mapper isn't needed either, so we disable it
- In switch2, we only need the outbound filter. The filter only allows packets towards their own VLAN.
- The other submodules of switch2 (inbound filter and both mappers) are disabled

.. **TODO** omitted modules mean the modules are not there (but the others are connected properly)

.. **TODO** we could have set the disabled modules to allow all packets for the same effect

.. note:: This VLAN policy configuration is only for one direction (enough for UDP). Policies for the other direction can be set up similarly if needed (e.g. for TCP). Also, instead of disabling modules, we could have set them to allow all packets or not map and VLAN IDs at all, for the same effect.

.. However, the return direction can be configured similarly if needed

.. .. figure:: media/schematic.png
   :align: center

.. so

   - in switch one, map the incoming packets to VLANs based on incoming interface
   - in switch two, outgoing interfaces filter packets based on VLAN IDz

Results
+++++++

Here is a video of the simulation:

.. video:: media/vlan3.mp4
   :align: center
   :width: 100%

As expected, the broadcast packets stay within their VLANs.

Config: VLAN Between Hosts Using Virtual Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. In this configuration, host1 sends UDP packets to host2 via a virtual interface in each host.

.. **V1** In this configuration, VLAN tags are added to packets in a host. 

.. **V2** In this configuration, a host adds VLAN tags to packets; (as opposed to switches).

.. - in this configuration, a host's interface adds a VLAN tag to packets (actually no but a request)
   - in this configuration, a host adds VLAN tags to packets
   - the switches filter packets according to configured VLAN policies

In this configuration, a host interface adds VLAN tags to packets, and switches filter packets according to configured VLAN policies.
The configuration uses the same network as the previous one (the host tints are irrelevant here). However, in addition to the Ethernet interfaces (``eth``), host1 and host2 each contain a virtual interface (``virt``) as well:

.. **TODO** image

.. figure:: media/virt.png
   :align: center

   **Figure X. Virtual interface in host**

.. .. note:: a virtual interface (:ned:`VirtualInterface`) acts as a real network interface, i.e. it has its own IP address. Packets sent via the virtual interface are transmitted via real network interfaces, and have the virtual interface's address as source IP address. 

.. note:: A virtual interface (:ned:`VirtualInterface`) acts as a real network interface, i.e. it has its own IP address, and packets can be sent to and from it. The virtual interface uses a real interface to transmit packets on. These packets have the virtual interface's address as source or destination IP address. The interface's functionality is implemented by its tunnel submodule (:ned:`VirtualTunnel`). 

.. These packets are transmitted via real network interfaces, and have the virtual interface's address as source or destination IP address. 

.. VirtualInterface contains a :ned:`VirtualTunnel` submodule.

Our goal is to make host2 accessible from host1 only through a virtual tunnel, i.e. only for traffic going between the two hosts' virtual interfaces. To that end, we configure the VLAN policy submodule in switch2 to forward only VLAN-tagged packets on its interface connected to host2). 

Two UDP applications in host1 send packets to host2: one of them via the two hosts' Ethernet interfaces, the other via the virtual interfaces. The virtual interfaces use the hosts' Ethernet interfaces to carry the packets. The packets sent between the virtual interfaces are configured to be VLAN tagged and the ones between the Ethernet interfaces are not.

.. **TODO** host1 and host2 contains a virtual interface


.. Thus, packets sent by host1's Ethernet interface are dropped by switch2, but those sent by the virtual interface are forwarded to host2.

.. **TODO** not here The virtual interface in host1 adds a VLAN tag request to outgoing packets, and VLAN tags are attached by host1's 802.1q submodule.

.. **TODO** Ieee8021q in switches only ?

.. The example simulation is defined in the ``BetweenSwitches`` config in omnetpp.ini:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config VLANBetweenSwitches
   :end-at: switch2.policy.outboundFilter.acceptedVlanIds
   :language: ini

.. The configuration adds a :ned:`VlanPolicyLayer` module to the switches. **TODO** qtag

   so

   - the accepted vlan ids are defined
   - eth0 interfaces in both switches face the connected hosts
   - eth1 interfaces face the other switch
   - on eth1, add vlan tag 42
   - on eth0, remove tag (or dont add)(we dont want tags)

   - so the syntax replaces an accepted vlan id to another one on an interface
   (replace -1 with 42, that is if there is no tag add tag 42 -> if there is tag 1, does it get replaced?)

.. x

   To separate the VLANs, the policy submodule's mapper in ``switch1`` is configured to add VLAN tags to packets based on the incoming interface (VLAN ID 1 on eth0 and VLAN ID 2 on eth1). The policy submodule's mapper in ``switch2`` is configured to send packets with VLAN ID 1 to the upper hosts, and those with VLAN ID 2 to the lower hosts.

   This line maps incoming packets on eth0 from VLAN -1 to VLAN 1 (-1 meaning no VLAN tag present on the packet).
   Similarly, it maps incoming packets on eth1 from VLAN -1 to VLAN 2.

   Note that this configurations only separates traffic going in one direction; this is not a problem now, since we're just using UDP in one direction. Policies for the other direction can be set up similarly if needed (e.g. TCP).



.. Here is a TCP packet displayed in Qtenv's packet inspector:

   .. figure:: media/inspector2.png
      :align: center

   .. video:: media/vlan3.mp4
      :width: 100%

.. VLAN Between Virtual Interfaces
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. This configuration demonstrates how to use VLANs with virtual interfaces. It uses the following network:

   .. figure:: media/network7.png
      :align: center

.. The network contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet. Both hosts have a regular Ethernet interface (``eth0``) and a virtual interface (``virt0``), which tunnels packets through the Ethernet interface.

.. .. figure:: media/virt.png
      :align: center

The simulation is defined in the ``VirtualInterfaces`` configuration in omnetpp.ini. Let's see some excerpts from the configuration below. 

Virtual Interface Settings
++++++++++++++++++++++++++

First, we configure host1 and host2 to have a virtual interface:

.. First, the virtual interfaces are configured:

.. .. literalinclude:: ../omnetpp.ini
      :start-at: Config VirtualInterfaces2
      :end-at: vlanId
      :language: ini

.. The virtual interfaces connect to the 802.1q submodule via a socket, thus the 802.1q submodule is configured to have a socket in both host1 and host2. The tunnel submodules in the virtual interfaces are configured to use the real Ethernet interface (eth0) of the host. The protocol parameter needs to be specified so that the interface is VLAN-aware???? We also set the VLAN ID.


.. literalinclude:: ../omnetpp.ini
   :start-at: Config VirtualInterfaces2
   :end-at: numVirtInterfaces
   :language: ini

.. Virtual interface basically has a VirtualTunnel in it.

Some parameters of the virtual tunnel need to be configured, such as which real interface to send packets on, and how to connect to the 802.1q submodule of the host. 
The tunnel submodule of the virtual interface accesses the 802.1q module via a socket. (In INET, sockets are used by modules, such as applications, to access protocol services in a network node.) We need to enable the 802.1q module's socket so that the tunnel can use the module's protocol service:

.. the 802.1q is configured to have one:

.. Sockets are used by modules, such as applications, to access protocol services in a network node. In this case, the tunnel submodule of the virtual interface accesses the 802.1q module via a socket; hence the 802.1q is configured to have one:

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

.. VLAN tags with the specified VLAN ID are added later by the 8021q module.

.. VLAN tags themselves are added later by the 8021q submodule

.. The Ipv4NetworkConfigurator module cannot autoconfigure addresses and routes for the 

.. **TODO** routes

The virtual interfaces require manual routing configuration. The Ipv4NetworkConfigurator module can assign addresses to virtual interfaces automatically, but cannot set up routes for them because it does not know where the virtual interfaces are in the network topology (i.e. which real interface they use). Thus routes for the virtual interfaces need to be added to the configator's xml config file:

.. We need to do some manual route configuration because of the virtual interfaces. 

.. /. Here is the xml configuration for the configurator:

.. Virtual interfaces need manual route configuration. We need to configure the routes to the virtual interfaces manually. Some routing configuration is required/We need to configure some of the routing manually. 

.. literalinclude:: ../config_virt.xml
   :language: xml

.. The xml configuration adds routes for the virtual interfaces in both hosts.

.. note:: We use ``globalArp``, but the simulation works with the default ARP as well. The arp packets that originate in the virtual interface are VLAN-tagged, so they can reach host2.

.. **TODO** traffic

Traffic Settings
++++++++++++++++

The only traffic in the simulation is generated by two UDP apps in host1, one targetting host2's eth interface, the other host2's virt interface:

.. literalinclude:: ../omnetpp.ini
   :start-after: *.host1.app[*].typename = "UdpBasicApp"
   :end-at: destAddresses = "host2%virt0"
   :language: ini

The apps are also configured to include the outgoing interface in the packet name (eth and virt), so that we can tell the packets apart in Qtenv.

.. Our goal is to make host2 only accessible via the two virtual interfaces, so we configure the VLAN policy like so:

VLAN Settings
+++++++++++++

Our goal is to make host2 only accessible via the two virtual interfaces, so we configure the following VLAN policy:

.. The configured VLAN policies are illustrated on the following image: The configured VLAN policies are illustrated on the following image: 

.. .. figure:: media/11.png
      :align: center

.. figure:: media/virt_schematic.png
   :align: center

   Figure 4: The configured VLAN policies

.. **TODO** protocol = 802.1qtag

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

.. The other, unneeded submodules are disabled.

.. - two UDP apps, one targetting host2's eth interface, the other host2's virt interface
   - include just the relevant part (destaddresses)
   - mention packetname
   - our goal is to make host2 only accessible via the virtual interface
   - so we need VLAN policy
   - would work without globalarp

.. **TODO** VLAN policy

   For traffic, there is just two UDP apps in host1, one sending packets via the ethernet interface, the other via the virtual interface:

   .. literalinclude:: ../omnetpp.ini
      :start-at: Config VirtualInterfaces2
      :end-at: acceptedVlanIds
      :language: ini

   Two UDP apps in host1 generate traffic, one of them for host2's eth0 and the other for host2's virt0 interface.
   The virtual interface is configured to assign packets to VLAN 1.

   **TODO** note that the host tint has no relevance in this configuration (it just uses the same network as the previous one)

   so

   - two hosts connected with 100Mbps Ethernet
   - both have a regular Ethernet interface and a virtual interface which uses the Ethernet interface
   - host1 has two udp apps
   - one of them sends packets to host2's Ethernet interface
   - the other to host2's virt interface
   - the virtual interface puts vlan tags on outgoing (?) packets
   - so the packets sent to the ethernet interface doesn't have VLAN tags
   - the ones sent to the virt interface do
   - the configurator can assign addresses to virtual interfaces automatically, but can't set up routes for them, because it doesn't know the topology (which real interface they use) so routes for the virtual interfaces need to be added manually
   - it doesnt know the topology...i.e. how the virtual interfaces are connected
   - this is about how to use vlans with virtual interfaces
   - the application doesnt know anything about the vlan the packets are assigned to (they could)

Results
+++++++

Here is a video of the simulation:

.. video:: media/virt2.mp4
   :align: center
   :width: 100%

As expected, the ``eth`` packets are filtered in switch2, but the ``virt`` packets are forwarded to host2.