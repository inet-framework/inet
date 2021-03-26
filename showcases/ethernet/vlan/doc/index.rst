:orphan:

Ethernet VLAN
=============

Goals
-----

.. Ethernet VLANs are supported by the IEEE 802.1Q standard. Virtual LANs separate physical networks into virtual networks at the date link layer level, so that virtual LANs behave as if they were physically separate LANs.

.. The Ethernet Virtual LAN (VLAN) feature is supported by the IEEE 802.1Q standard. Virtual LANs separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs.

.. **V1** Ethernet Virtual Local Area Networks (VLANs) separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs. The Ethernet Virtual LAN (VLAN) feature is supported by the IEEE 802.1Q standard. 

.. Ethernet Virtual Local Area Networks (VLANs) separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs.

Ethernet Virtual Local Area Networks (VLANs) are physical Ethernet networks separated into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs.

This showcase demonstrates the VLAN support of INET's modular Ethernet model.

.. The Model
   ---------

    so

    - VLANs are created by adding VLAN tags to packets
    - the tags are added by adding a 802.1q header to ethernet packets
    - the parts of the network that supports VLANs filter traffic based on the VLAN tags

    - the Policy module

    - actually, VLAN tags can be added in hosts

.. VLANs are created by adding VLAN tags to Ethernet packets in a 802.1q header. The VLAN-aware parts of the network filter traffic based on the VLAN tags. Packets without VLAN tags are considered to be part of the native VLAN.

   In INET, various modules (such as hosts and policy submodules) can add a VLAN tag request to packets. The actual VLAn tags are then added by the 802.1q submodule. 
   **V2** 
   As such, the presence of the 802.1q submodule makes the network node VLAN-aware.

    so

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

INET's VLAN support
-------------------

Overview
~~~~~~~~

.. **V1** Ethernet VLANs are created by adding an extra header to Ethernet frames. The header contains VLAN tags. The VLAN-aware parts of the network filter traffic based on these VLAN tags. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN tags and VLAN ID. Each interface has a set of allowed VLAN IDs. Packets without VLAN tags are considered to be part of the native VLAN and are not filtered.

Ethernet VLANs are created by assigning VLAN IDs to ethernet frames, by adding an extra header containing VLAN tags which store the VLAN ID. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN ID. Each interface has a set of allowed VLAN IDs. Packets without VLAN tags are considered to be part of the native VLAN (which can be allowed/disallowed as well). This mechanism removes connections from a physical network by filtering packets at the data link layer, but cannot create new connections.

.. and are not filtered.

.. **V1** This basically takes away connections from a physical network by filtering; it obviously cannot add new connections.

.. **V3** This mechanism restricts connections in an existing physical network, but cannot add new connections.

.. Ethernet VLANs are created by assigning VLAN IDs to ethernet frames, by adding an extra header containing VLAN tags which store the VLAN ID.

.. In INET, several modules...

In INET, various modules (such as hosts and policy submodules) can add a VLAN tag request to packets. The actual VLAN tags are then added by 802.1q submodules. Its possible that a VLAN tag request is added in one network node and the VLAN tag is added in another (e.g. an app in a host adds request, the 802.1q submodule in a switch adds tag).

.. **TODO** example for this process somewhere (for example, host's app adds request, connected switch's 802.1q module adds tag)

.. **TODO** (even if its in another host which is vlan aware/has 802.1q module)

As such, the presence of the 802.1q submodule makes the network node VLAN-aware. **TODO** meaning what ?
that the network node can read VLAN tags on packets ?
-> the presence of the 802.1q submodule makes the network node able to filter packets based on VLAN tags ?

Several configurations are possible...**TODO** such as double tagging

.. The VlanPolicyLayer submodule
   -----------------------------

Creating VLAN policies
~~~~~~~~~~~~~~~~~~~~~~

The :ned:`VlanPolicyLayer` module can filter packets based on VLAN tags, and modify VLAN tags on packets. It is an optional submodule of :ned:`EthernetSwitch`. Here is the VLAN policy layer submodule inside an :ned:`EthernetSwitch`:

.. figure:: media/switch2.png
   :align: center

.. **V1** The module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (incoming/outgoing). **TODO** how do they become optional? -> omitted type

   By default, the module has the following modules:

   .. figure:: media/policy.png
      :align: center

The policy module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (inbound/outbound). By default, the module has the following submodules:

.. figure:: media/policy.png
   :align: center

In the filter submodules, the set of allowed VLAN IDs can be specified with the :par:`acceptedVlanIds` parameter. In the mapper submodules, VLAN ID mappings can be specified with the :par:`mappedVlanIds` parameter.

The syntax for the acceptedVlanIds parameter is 

.. code-block:: text

   **.acceptedVlanIds = {"interface" : [vlanid1, vlanid2, ... ]} 

For example, allowing VLAN IDs 1 and 2 on interface ``eth2``:

.. literalinclude:: ../omnetpp.ini
   :start-at: switch1.policy.outboundFilter.acceptedVlanIds
   :end-at: switch1.policy.outboundFilter.acceptedVlanIds
   :language: ini

The syntax for the  mappedVlanIds parameter is 

.. code-block:: text

   **.mappedVlanIds = {"interface" : {"orig vlan id" : mapped vlan id}, ... } 
   
For example: **TODO**

.. literalinclude:: ../omnetpp.ini
   :start-at: switch1.policy.outboundFilter.acceptedVlanIds
   :end-at: switch1.policy.outboundFilter.acceptedVlanIds
   :language: ini

Some explanation on the syntax for the mapper:

.. literalinclude:: ../omnetpp.ini
   :start-at: switch1.policy.inboundMapper.mappedVlanIds
   :end-at: switch1.policy.inboundMapper.mappedVlanIds
   :language: ini

**TODO** adds vlan id request/indication tags

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

The Model/Configuration/The example simulations
-----------------------------------------------

The example simulations use the following network:

.. .. figure:: media/network.png
      :align: center

.. .. figure:: media/network5.png
   :align: center

.. figure:: media/network6.png
   :align: center

.. It contains a network of two switches (:ned:`EthernetSwitch`). A host (:ned:`StandardHost`) is connected to each switch.

It contains six hosts (:ned:`StandardHost`) and two switches (:ned:`EthernetSwitch`). We separate the network into two virtual networks with three hosts each, indicated on the image with red and blue colors. We assign VLAN ID 1 to the upper VLAN, and VLAN ID 2 to the lower VLAN.

The two switches are configured to be VLAN aware, but the hosts are not. (Thus the switches are responsible for creating the VLANs and adding VLAN tags to packets.

- the configs

**TODO** traffic

For traffic, host1 in the upper VLAN broadcasts UDP packets to host4 in the same VLAN. Similarly, host4 in the lower VLAN send packets to hostX.

**TODO** each is a different subnet -> how about they weren't ? can they have the same address ?

For traffic, one of the hosts in each VLAN send UDP packets to another host in the same VLAN. The switches separate the packets.

**V1** There are two configurations in this showcase:

- Config ``BetweenSwitches``: All packets between the5switches are assigned to VLAN 42.
- Config ``VirtualInterface``: All packets between the hosts are assigned to VLAN 42 and use virtual interfaces in the hosts.

**V2** There are two configurations in this showcase:

- **VLAN between the switches**: All packets between the switches are assigned to VLAN 42.
- **VLAN between the hosts using virtual interfaces**: All packets between the hosts are assigned to VLAN 42 and use virtual interfaces in the hosts.

.. Example: VLAN between the Switches
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VLAN between the Switches
~~~~~~~~~~~~~~~~~~~~~~~~~

Before we get to the example simulation, here is part of the ``General`` configuration with Ethernet settings:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap
   :end-at: bitrate
   :language: ini

The first three lines configure all network nodes to use the composable Ethernet model. The switches are configured to have a :ned:`Ieee8021qLayer` submodule. Finally, the connection speed is set on all interfaces.

**TODO** Ieee8021q in switches only ?

The example simulation is defined in the ``BetweenSwitches`` config in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config VLANBetweenSwitches
   :end-at: switch2.policy.outboundFilter.acceptedVlanIds
   :language: ini

The configuration adds a :ned:`VlanPolicyLayer` module to the switches. **TODO** qtag

so

- the accepted vlan ids are defined
- eth0 interfaces in both switches face the connected hosts
- eth1 interfaces face the other switch
- on eth1, add vlan tag 42
- on eth0, remove tag (or dont add)(we dont want tags)

- so the syntax replaces an accepted vlan id to another one on an interface
(replace -1 with 42, that is if there is no tag add tag 42 -> if there is tag 1, does it get replaced?)

To separate the VLANs, the policy submodule's mapper in ``switch1`` is configured to add VLAN tags to packets based on the incoming interface (VLAN ID 1 on eth0 and VLAN ID 2 on eth1). The policy submodule's mapper in ``switch2`` is configured to send packets with VLAN ID 1 to the upper hosts, and those with VLAN ID 2 to the lower hosts.

This line maps incoming packets on eth0 from VLAN -1 to VLAN 1 (-1 meaning no VLAN tag present on the packet).
Similarly, it maps incoming packets on eth1 from VLAN -1 to VLAN 2.

Note that this configurations only separates traffic going in one direction; this is not a problem now, since we're just using UDP in one direction. Policies for the other direction can be set up similarly if needed (e.g. TCP).



Here is a TCP packet displayed in Qtenv's packet inspector:

.. figure:: media/inspector2.png
   :align: center

.. video:: media/vlan1.mp4
   :width: 100%