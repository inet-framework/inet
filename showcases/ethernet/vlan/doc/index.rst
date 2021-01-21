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
~~~~~~~~~~~~~~~~~~~~~~

The :ned:`VlanPolicyLayer` module can filter packets based on VLAN tags, and modify VLAN tags on packets. It is an optional submodule of :ned:`EthernetSwitch`. Here is the VLAN policy layer submodule inside an :ned:`EthernetSwitch`:

.. figure:: media/switch2.png
   :align: center

.. **V1** The module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (incoming/outgoing). **TODO** how do they become optional? -> omitted type

   By default, the module has the following modules:

   .. figure:: media/policy.png
      :align: center

The module has four optional submodules: a `mapper` and a `filter` module, for each traffic direction (inbound/outbound). By default, the module has the following submodules:

.. figure:: media/policy.png
   :align: center

In the filter submodules, the set of allowed VLAN IDs can be specified with the :par:`acceptedVlanIds` parameter. In the mapper submodules, VLAN ID mappings can be specified with the :par:`mappedVlanIds` parameter.

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

.. figure:: media/network4.png
   :align: center

It contains a network of two switches (:ned:`EthernetSwitch`). A host (:ned:`StandardHost`) is connected to each switch.

- the configs

**V1** There are two configurations in this showcase:

- Config ``BetweenSwitches``: All packets between the switches are assigned to VLAN 42.
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

The first three lines configure all network nodes to use the composable Ethernet model. Then, all network nodes are configured to have a :ned:`Ieee8021qLayer` submodule. Finally, the connection speed is set.

The example simulation is defined in the ``BetweenSwitches`` config in omnetpp.ini:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config BetweenSwitches
   :end-at: outboundMapper
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



Here is a TCP packet displayed in Qtenv's packet inspector:

.. figure:: media/inspector2.png
   :align: center