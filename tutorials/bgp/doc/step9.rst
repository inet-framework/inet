Step 9. Using Network attribute to advertise specific networks
==============================================================

Goals
-----

The goal of this step is to demonstrate explicit network advertisement in BGP using
the **Network** statement instead of relying on IGP redistribution.

In previous steps, BGP routes were created by redistributing routes from IGPs (OSPF
or RIP). However, BGP also supports explicitly defining which networks to advertise
using the Network statement. This approach offers several advantages:

*   **Control**: Precise control over which networks are advertised via BGP
*   **Independence**: BGP advertisements don't depend on IGP configuration
*   **Stability**: Networks can be advertised even if temporarily absent from IGP
*   **Policy**: Different networks can have different BGP policies applied
*   **Aggregation**: Allows advertising summary routes while IGP has specific routes

The **Network statement** in BGP:

*   Tells BGP to originate a route for a specific prefix
*   The network must exist in the routing table (learned via IGP, static, or connected)
*   BGP doesn't redistribute all IGP routes, only explicitly configured networks
*   This is the recommended approach for controlled BGP advertisement

Configuration
~~~~~~~~~~~~~

This step uses the same topology as Step 6:

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step9]
   :end-before: ------

Key differences from previous steps:

*   OSPF is used as the IGP in all ASes
*   **No redistribution** is configured (no ``redistributeOspf`` or ``redistributeRip``)
*   BGP configuration explicitly lists networks to advertise using Network statements

The BGP configuration:

.. literalinclude:: ../BGPConfig.xml
   :language: xml

In this configuration, each router explicitly declares which networks it will advertise.
For example, if the BGP configuration includes:

.. code-block:: xml

    <Router interAddr="10.0.0.1">
        <Network address='10.0.0.0/24' />
    </Router>

This tells the router to advertise the 10.0.0.0/24 network via BGP, but only if that
network exists in the router's routing table (learned via OSPF, connected, or static).

Results
~~~~~~~

The simulation demonstrates:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF establishes routing in all three ASes
    *   All routers learn internal routes via OSPF
    *   OSPF operates completely independently of BGP

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP and I-BGP sessions establish as in previous steps
    *   Border routers establish sessions with their peers

3.  **Selective Network Advertisement**:

    *   Only networks explicitly listed in the Network statements are advertised
    *   Each router checks its routing table for the configured networks
    *   If a network exists in the routing table, BGP originates a route for it
    *   If a network doesn't exist, BGP doesn't advertise it (even if other routes exist)

4.  **No Automatic Redistribution**:

    *   Unlike previous steps, not all OSPF routes appear in BGP
    *   Only explicitly configured networks are advertised
    *   This provides fine-grained control over what's shared between ASes
    *   Internal networks can remain internal (not advertised externally)

5.  **Origin Attribute**:

    *   Routes advertised via Network statement have ORIGIN = IGP
    *   This indicates the route was originated within the AS
    *   Different from redistributed routes which may have ORIGIN = incomplete

**Comparison: Network Statement vs Redistribution**

*Network Statement:*

*   Explicit control over advertised prefixes
*   Manually configured for each network
*   Recommended for production networks
*   Prevents accidental advertisement of internal routes
*   Better for security and policy control

*Redistribution:*

*   Automatic advertisement of all IGP routes (or filtered subset)
*   Less manual configuration
*   Easier to misconfigure (advertising too much)
*   Can overwhelm BGP with many routes
*   Useful for dynamic environments

**Best Practices:**

*   Use Network statements in production for controlled advertisement
*   Use redistribution only with careful filtering
*   Don't mix Network statements and redistribution without clear policy
*   Document which networks are advertised and why
*   Implement route filtering at AS boundaries

This step demonstrates the recommended approach for BGP deployment: explicit network
advertisement gives operators full control over inter-domain routing while maintaining
separation between IGP (internal) and BGP (external) routing domains.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig.xml <../BGPConfig.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
