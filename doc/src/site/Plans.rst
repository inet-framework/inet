=====
Plans
=====

This page describes potential ideas for INET in the future. For the work that is
already under way, see :doc:`Progress`.

After INET 4.7
--------------

1. Standards-based protocol tests

   - continue deriving protocol tests from the standards, at the level of the core
     requirements, for Mobile IPv6 and Proxy Mobile IPv6, SCTP, OSPFv2 and OSPFv3,
     PIM-SM and PIM-DM, BGP-4, AODV, DiffServ, RTP/RTCP, and the IPv6-in-IPv6 tunnel
   - extend the QUIC tests (RFC 9002 loss detection and congestion control, RFC 8899
     packetization layer path MTU discovery)
   - repair the protocol model defects that these tests find

2. IPv6

   - finish the review and repair of Neighbor Discovery, IPv6 tunneling, Mobile IPv6
     and Proxy Mobile IPv6
   - add Path MTU Discovery (RFC 8201), with a path MTU cache in the IPv6 layer that
     IPsec can also use

3. IPsec

   - add tunnel mode (RFC 4301), next to the existing transport mode

4. MPLS and PPP

   - add the PPP Link Control Protocol (RFC 1661) and the MPLS Control Protocol
     (RFC 3032) on top of it
   - add the Path Computation Element Protocol (PCEP, RFC 5440 and RFC 8231), the
     advertisement of IPv6 FECs in LDP, and a 6PE example

5. IEEE 802.11

   - support delayed Block Ack, Multi-TID Block Ack, and A-MPDU in the QoS data service
   - make association in mixed (ERP and non-ERP) BSSs work, and expose the selection
     of the supported PHY modes
   - separate LDPC negotiation, code selection and coding behavior
   - prepare noncontiguous (80+80 MHz) VHT channels, reverse direction, and MU-MIMO
   - airtime fairness

6. TCP

   - continue bringing the TCP model to feature parity with Linux, verified by the
     packetdrill scripts of the Linux kernel

7. Routing protocols

   - IS-IS, Babel, LISP and VRRP

8. Visualization

   - a new 3D visualizer based on VulkanSceneGraph, with terrain from LIDAR point
     clouds and geospatial scenes

9. Physical layer

   - support capturing a stronger signal while receiving a weaker one
   - add the simulation of the synchronization process (pilot/preamble)
   - add the missing Weibull and Jakes fading
   - add multipath support
   - add real CUDA and multi-core parallel execution

10. Mobility models

    - separate initial positioning from positioning over time
    - make them composable sequentially
    - extend the interface with future movement predictions

11. Showcases

    - new showcases for Mobile IPv6, Proxy Mobile IPv6, IPv6 autoconfiguration,
      IPv6 tunneling, 10BASE-T1S, the IEEE 802.11 rate anomaly, and geographic
      mobility

As for the not-so-near future (potentially in a later major version), we are considering some nontrivial changes:

1. Synchronous sends

   - introducing synchronous (immediate) message sends to reduce communication overhead between protocol layers

2. Factor out the base infrastructure from INET so we would have a subset of components that could be separately supported.
   Protocol-specific code should be avoided in this codebase.

3. Based on the common infrastructure create a framework where generic network models could be built using only the
   combination of existing (very simple) modules. This could be useful for modeling abstract network concepts.

4. Parallel simulation of INET models
