Step 14. Hierarchical OSPF topology and summary LSA
===================================================

Goals
-----

The goal of this step is to demonstrate hierarchical OSPF with multiple areas
and to examine how Area Border Routers (ABRs) generate and flood Summary LSAs to
provide inter-area reachability.

OSPF uses a hierarchical design to improve scalability. The network is divided
into areas, with Area 0 (the backbone area) connecting all other areas. Routers
with interfaces in multiple areas are called ABRs. ABRs generate Summary LSAs
(Type-3 LSAs) to advertise networks from one area into another area, allowing
routers to learn about networks outside their own area without needing to know
the detailed topology of other areas.

Configuration
~~~~~~~~~~~~~

This step uses the ``OSPF_AreaTest`` network with three OSPF areas:

*   **Area 0.0.0.0** (backbone): Contains the link between **R3** and **R4**
*   **Area 0.0.0.1**: Contains **R1**, **R2**, **R3**, **host1**, and **host2**
*   **Area 0.0.0.2**: Contains **R4**, **R5**, and **host3**

**R3** and **R4** are ABRs because they have interfaces in multiple areas.

.. figure:: media/OSPF_AreaTest.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_AreaTest.ned
   :start-at: network OSPF_AreaTest
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step14
   :end-before: ------

The OSPF configuration in ``ASConfig_Area.xml`` assigns interfaces to areas:

.. literalinclude:: ../ASConfig_Area.xml
   :language: xml

Results
~~~~~~~

After OSPF convergence, the network exhibits the following behavior:

1.  **Intra-area routing**: Within Area 0.0.0.1, routers **R1**, **R2**, and
    **R3** exchange Router LSAs to build a complete view of the area topology.
    Similarly, **R4** and **R5** exchange Router LSAs within Area 0.0.0.2.

2.  **Summary LSA generation**: 
    
    *   **R3** (ABR between Area 0.0.0.1 and Area 0.0.0.0) generates Summary
        LSAs to advertise networks from Area 0.0.0.1 into Area 0.0.0.0, and vice
        versa. For example, R3 advertises the 192.168.11.x/30 networks into the
        backbone.
    *   **R4** (ABR between Area 0.0.0.2 and Area 0.0.0.0) generates Summary
        LSAs to advertise the 192.168.22.x/30 networks from Area 0.0.0.2 into
        Area 0.0.0.0.
    *   Both ABRs then propagate Summary LSAs from the backbone into their
        respective non-backbone areas.

3.  **Inter-area routing**: Routers use Summary LSAs to install inter-area
    routes in their routing tables. For example:
    
    *   **R1** learns about the 192.168.22.x networks via Summary LSAs from **R3**
    *   **R5** learns about the 192.168.11.x networks via Summary LSAs from **R4**

The routing table shows inter-area routes (learned via Summary LSAs) alongside
intra-area routes.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_AreaTest.ned <../OSPF_AreaTest.ned>`,
:download:`ASConfig_Area.xml <../ASConfig_Area.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
