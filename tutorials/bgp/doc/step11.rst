Step 11. Multi-hop E-BGP
=========================

Goals
-----

The goal of this step is to demonstrate **multi-hop E-BGP**, where BGP peers in different
autonomous systems are not directly connected but establish sessions over intermediate
routers.

By default, E-BGP sessions are established only between directly connected routers
(TTL=1 for BGP packets). This is a security feature that prevents BGP sessions from
being hijacked or spoofed from remote locations. However, there are legitimate scenarios
where E-BGP peers need to establish sessions across intermediate routers:

**Use Cases for Multi-hop E-BGP:**

*   **Internet Exchange Points (IXPs)**: Multiple ASes connect through a shared switch
    or Layer 2 network, but BGP sessions may use loopback addresses
*   **DMZ configurations**: A firewall or intermediate router sits between BGP peers
*   **Load balancing**: Multiple physical links between ASes, but BGP uses loopbacks
    for session stability
*   **Redundancy**: BGP session survives failure of individual physical links

**Multi-hop E-BGP Configuration:**

*   Requires explicit configuration with ``ebgpMultihop`` parameter
*   Sets BGP packet TTL higher than 1 (commonly 2-255)
*   Static routes or IGP needed to reach the peer's address
*   Should be used carefully due to security implications

Configuration
~~~~~~~~~~~~~

This step uses the following network (``Multihop_EBGP.ned``):

.. figure:: media/Multihop_EBGP.png
   :width: 100%
   :align: center

The network topology shows:

*   **AS 64520** (RA): BGP router with host0
*   **Router R**: Non-BGP intermediate router (acts as a transit)
*   **AS 64530** (RB): BGP router with host1

RA and RB are **not directly connected**. An intermediate router R sits between them.
The E-BGP session must be established across this intermediate router.

.. literalinclude:: ../Multihop_EBGP.ned
   :start-at: network Multihop
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step11]
   :end-before: ------

Key configuration elements:

*   Static routes configured for RA and RB to reach each other through R:
    
    - RA has a route to RB's address (192.168.0.5) via R
    - RB has a route to RA's address (192.168.0.1) via R

*   Only RA and RB run BGP; router R does not

The BGP configuration:

.. literalinclude:: ../BGPConfig_MultiHopEBGP.xml
   :language: xml

Critical parameters in the configuration:

*   ``ebgpMultihop='2'``: Allows the BGP session to span 2 hops
*   Session uses addresses 192.168.0.1 (RA) and 192.168.0.5 (RB)
*   Without ebgpMultihop, the BGP session would fail (TTL=1 by default)

Results
~~~~~~~

The simulation demonstrates:

1.  **Static Route Configuration** (t=0s):

    *   RA has a route to 192.168.0.5 (RB) via R
    *   RB has a route to 192.168.0.1 (RA) via R
    *   R routes packets between its interfaces
    *   IP connectivity exists between RA and RB through R

2.  **BGP Session Attempt** (t≈5s):

    *   RA attempts to establish TCP connection to RB on port 179
    *   Without ebgpMultihop, this would fail (BGP packets with TTL=1 expire at R)
    *   With ebgpMultihop=2, BGP packets can reach the peer through R

3.  **Multi-hop E-BGP Session Establishment**:

    *   TCP connection established between RA and RB through R
    *   BGP OPEN messages exchanged
    *   RA and RB negotiate BGP parameters
    *   Session enters ESTABLISHED state

4.  **Route Advertisement**:

    *   RA advertises network 10.0.0.0 to RB
    *   RB advertises network 20.0.0.0 to RA
    *   Routes are installed in the routing tables
    *   NEXT_HOP is set to the advertising router's address

5.  **End-to-End Connectivity**:

    *   host0 (in AS 64520) can reach host1 (in AS 64530)
    *   Traffic flows: host0 → RA → R → RB → host1
    *   R forwards packets based on IP routing (not BGP-aware)
    *   BGP provides the routing information; IP forwarding delivers the packets

**Security Considerations:**

Multi-hop E-BGP has security implications:

*   **Spoofing risk**: Attackers further than 1 hop could potentially send BGP packets
*   **Mitigation**: Use BGP authentication (MD5 or TCP-AO)
*   **Best practice**: Set ebgpMultihop to the minimum required value (e.g., 2 not 255)
*   **Filtering**: Implement strict ACLs on intermediate routers
*   **Monitoring**: Log and monitor BGP session establishment

**Comparison: Direct E-BGP vs Multi-hop E-BGP**

*Direct E-BGP (default):*

*   Routers must be directly connected
*   TTL = 1 for BGP packets
*   More secure (harder to spoof)
*   Simpler configuration
*   Preferred when possible

*Multi-hop E-BGP:*

*   Routers can be multiple hops apart
*   TTL > 1 (configured via ebgpMultihop)
*   Requires static routes or IGP for reachability
*   More flexible for complex topologies
*   Requires additional security measures

This step demonstrates that BGP is flexible enough to accommodate various network
topologies, including scenarios where direct connectivity between BGP peers is not
feasible or desirable. However, this flexibility must be balanced with security
considerations and proper configuration.

Sources: :download:`Multihop_EBGP.ned <../Multihop_EBGP.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_MultiHopEBGP.xml <../BGPConfig_MultiHopEBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
