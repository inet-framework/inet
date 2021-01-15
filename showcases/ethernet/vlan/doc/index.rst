:orphan:

Ethernet VLAN
=============

Goals
-----

.. Ethernet VLANs are supported by the IEEE 802.1Q standard. Virtual LANs separate physical networks into virtual networks at the date link layer level, so that virtual LANs behave as if they were physically separate LANs.

.. The Ethernet Virtual LAN (VLAN) feature is supported by the IEEE 802.1Q standard. Virtual LANs separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs.

**V1** Ethernet Virtual Local Area Networks (VLANs) separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs. The Ethernet Virtual LAN (VLAN) feature is supported by the IEEE 802.1Q standard. 

**V2** Ethernet Virtual Local Area Networks (VLANs) separate physical networks into virtual networks at the data link layer level, so that virtual LANs behave as if they were physically separate LANs. 

This showcase demonstrates the VLAN support of INET's modular Ethernet model.

The Model
---------

    so

    - VLANs are created by adding VLAN tags to packets
    - the tags are added by adding a 802.1q header to ethernet packets
    - the parts of the network that supports VLANs filter traffic based on the VLAN tags

    - the Policy module

    - actually, VLAN tags can be added in hosts

VLANs are created by adding VLAN tags to Ethernet packets in a 802.1q header. The VLAN-aware parts of the network filter traffic based on the VLAN tags. Packets without VLAN tags are considered to be part of the native VLAN.

In INET, various modules (such as hosts and policy submodules) can add a VLAN tag request to packets. The actual VLAn tags are then added by the 802.1q submodule. 

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

The Model
---------

**V1** Ethernet VLANs are created by adding an extra header to Ethernet frames. The header contains VLAN tags. The VLAN-aware parts of the network filter traffic based on these VLAN tags. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN tags and VLAN ID. Each interface has a set of allowed VLAN IDs. Packets without VLAN tags are considered to be part of the native VLAN and are not filtered.

**V2** Ethernet VLANs are created by assigning VLAN IDs to ethernet frames, by adding an extra header containing VLAN tags which store the VLAN ID. The VLAN-aware parts of the network filter traffic at interfaces, based on VLAN tags and VLAN ID. Each interface has a set of allowed VLAN IDs. Packets without VLAN tags are considered to be part of the native VLAN and are not filtered.

This basically takes away connections from a physical network by filtering; it obviously cannot add new connections.

.. Ethernet VLANs are created by assigning VLAN IDs to ethernet frames, by adding an extra header containing VLAN tags which store the VLAN ID.

.. In INET, several modules...

In INET, various modules (such as hosts and policy submodules) can add a VLAN tag request to packets. The actual VLAn tags are then added by the 802.1q submodule. **TODO** (even if its in another host which is vlan aware/has 802.1q module)

As such, the presence of the 802.1q submodule makes the network node VLAN-aware.

Several configurations are possible...**TODO**

.. The VlanPolicyLayer submodule
   -----------------------------

Creating VLAN policies
----------------------

The :ned:`VlanPolicyLayer` module can filter packets based on VLAN tags, and modify VLAN tags on packets. It is an optional submodule of :ned:`EthernetSwitch`. Here is the VLAN policy layer submodule inside an :ned:`EthernetSwitch`:

.. figure:: media/switch2.png
   :align: center

so

    - The VlanPolicyLayer module can filter incoming and outgoing packets based on VLAN ID
    - It can also re-map VLAN tags/IDs
    - The VlanPolicyLayer has four optional submodules: a mapper and a filter module for each direction

.. figure:: media/vlanpolicylayer2.png
   :align: center

so

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

the structure:

    - what is VLAN?

      data link layer filtering, act like different physical networks, VLAN tags, extra header, interface filtering (allowed VLAN tags on an interface, implemented by 802.1q, wlan aware part mostly switches, native vlan, filtering only (no routing or creating new physical connections obviously)

    - how is it in INET?

      several modules can attach VLAN tag requests to packets (for example what)
      the tags are attached by the 802.1q module (even if its in another host which is vlan aware/has 802.1q module

    - some configurations thats possible

      double tagging, virtual interfaces, etc 

    - the vlan policy module