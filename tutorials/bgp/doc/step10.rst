Step 10. LOCAL_PREF Attribute
==============================

Goals
-----

The goal of this step is to demonstrate the **LOCAL_PREF** (Local Preference) attribute,
one of the most important BGP path attributes for influencing routing decisions within
an autonomous system.

BGP uses a complex decision process to select the best path when multiple routes to
the same destination exist. The LOCAL_PREF attribute is used early in this process,
making it a powerful tool for traffic engineering.

**LOCAL_PREF Attribute:**

*   Used only within an AS (I-BGP), never sent to E-BGP peers
*   Higher LOCAL_PREF values are preferred (default is typically 100)
*   Allows an AS to prefer certain exit points for outbound traffic
*   Set on routes when received from E-BGP peers or via policy
*   Uniform across the AS - all I-BGP routers see the same LOCAL_PREF

**BGP Decision Process** (simplified):

1.  Prefer route with highest LOCAL_PREF
2.  Prefer route with shortest AS_PATH
3.  Prefer route with lowest ORIGIN type (IGP < EGP < incomplete)
4.  Prefer route with lowest MED (Multi-Exit Discriminator)
5.  Prefer E-BGP over I-BGP routes
6.  Prefer route with lowest IGP metric to next hop
7.  Prefer route from router with lowest router ID

LOCAL_PREF is evaluated first, making it the primary tool for inbound traffic
engineering within an AS.

Configuration
~~~~~~~~~~~~~

This step uses the following network (``BGP_LOCAL_PREF.ned``):

.. figure:: media/BGP_LOCAL_PREF.png
   :width: 100%
   :align: center

The network topology shows:

*   **AS 64500** (RA): Single router connected to both RB1 and RB2
*   **AS 64600** (RB1, RB2, RB3): Three routers with RB1 and RB2 as border routers
*   **AS 64700** (RC): Single router connected to RB2

AS 64600 has two paths to reach AS 64500:

*   Via RB1 (directly connected to RA)
*   Via RB2 (connected to RA)

By setting different LOCAL_PREF values, AS 64600 can control which path is preferred.

.. literalinclude:: ../BGP_LOCAL_PREF.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` defines the setup:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10
   :end-before: ------

The BGP configuration sets different LOCAL_PREF values:

.. literalinclude:: ../BGPConfig_LOCAL_PREF.xml
   :language: xml
   :start-at: <Neighbor address='20.0.0.2'
   :end-at: localPreference='600' />

Key configuration in the XML:

*   RB1 sets LOCAL_PREF=100 for routes learned from RB2
*   RB2 sets LOCAL_PREF=600 for routes learned from RB1
*   Both routers use next-hop-self for I-BGP advertisements

Results
~~~~~~~

The simulation demonstrates LOCAL_PREF in action:

1.  **OSPF Convergence** (t≈0-50s):

    *   OSPF establishes routing within AS 64600
    *   RB1, RB2, and RB3 all learn each other's addresses

2.  **BGP Session Establishment** (t≈50s):

    *   E-BGP sessions: RA-RB1, RA-RB2, RB2-RC
    *   I-BGP sessions: RB1-RB2, RB1-RB3, RB2-RB3 (full mesh)

3.  **Route Advertisement to AS 64600**:

    *   RA advertises network 10.0.0.0/30 to both RB1 and RB2
    *   RB1 receives the route from RA via E-BGP
    *   RB2 receives the route from RA via E-BGP
    *   Both routers advertise this route to their I-BGP peers

4.  **LOCAL_PREF Application**:

    *   RB1 advertises the route with LOCAL_PREF=100 (default) to RB2 and RB3
    *   RB2 advertises the route with LOCAL_PREF=600 to RB1 and RB3
    *   Note: RB2 sets LOCAL_PREF=600 when advertising routes learned from RB1

5.  **Route Selection**:

    *   **RB3** (non-border router) receives two routes to 10.0.0.0/30:
        
        - Via RB1 with LOCAL_PREF=100
        - Via RB2 with LOCAL_PREF=600 (preferred)
        
    *   RB3 selects the route via RB2 because of higher LOCAL_PREF
    *   Traffic from RB3 to AS 64500 goes through RB2, not RB1

6.  **Traffic Engineering Result**:

    *   Even though RB1 may have a shorter IGP path or other advantages
    *   LOCAL_PREF overrides these factors in the decision process
    *   AS 64600 can control its preferred exit point for outbound traffic

**Use Cases for LOCAL_PREF:**

*   **Load balancing**: Distribute traffic across multiple exit points
*   **Cost optimization**: Prefer cheaper or higher-capacity links
*   **Policy routing**: Route certain traffic through specific peering points
*   **Primary/backup paths**: Set high LOCAL_PREF on primary, low on backup
*   **Customer vs peer vs transit**: Prefer customer routes, then peer, then transit

**Example Scenario:**

If AS 64600 has:

*   A high-capacity paid peering with AS 64500 at RB2
*   A low-capacity backup peering at RB1

Setting LOCAL_PREF=600 on RB2 and LOCAL_PREF=100 on RB1 ensures all traffic prefers
the high-capacity link, with automatic failover to RB1 if RB2's link fails.

The LOCAL_PREF attribute is a fundamental tool for BGP traffic engineering, allowing
autonomous systems to implement sophisticated routing policies while maintaining the
consistency required for loop-free routing.

Sources: :download:`BGP_LOCAL_PREF.ned <../BGP_LOCAL_PREF.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_LOCAL_PREF.xml <../BGPConfig_LOCAL_PREF.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
