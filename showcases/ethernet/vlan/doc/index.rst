:orphan:

Ethernet VLAN
=============

Goals
-----

Ethernet VLANs are supported by the IEEE 802.1Q standard. Virtual LANs separate physical networks into virtual networks at the date link layer level, so that virtual LANs behave as if they were physically separate LANs.

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

The VlanPolicyLayer submodule
-----------------------------

Creating VLAN policies
----------------------

The :ned:`VlanPolicyLayer` module can filter packets based on VLAN tags, and modify VLAN tags on packets. It is an optional submodule of :ned:`EthernetSwitch`. Here is the VLAN policy layer submodule inside an :ned:`EthernetSwitch`:

.. figure:: media/switch2.png
   :align: center
   :width: 100%

so

- The VlanPolicyLayer module can filter incoming and outgoing packets based on VLAN ID
- It can also re-map VLAN tags/IDs
- The VlanPolicyLayer has four optional submodules: a mapper and a filter module for each direction

.. figure:: media/vlanpolicylayer2.png
   :align: center