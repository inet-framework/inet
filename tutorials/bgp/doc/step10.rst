Step 10. LOCAL_PREF Attribute
=============================

Goals
-----

Step 10 demonstrates the **LOCAL_PREF** (Local Preference) BGP attribute. Local
Preference is a discretionary attribute used to influence the selection of the
preferred exit point from an Autonomous System (AS). When an AS has multiple
connections to an external AS, the Local Preference attribute allows internal
routers to agree on which border router should be used for outbound traffic.

Key features demonstrated:

- **Inbound Influence on Outbound Traffic**: Setting Local Preference on routes
  learned from external peers to control how traffic leaves the local AS.
- **Path Selection Hierarchy**: Local Preference is evaluated early in the BGP
  best-path selection algorithm, typically before AS_PATH length.
- **AS-Wide Consistency**: Local Preference is a transitive attribute within an
  AS, ensuring all internal routers (I-BGP peers) share the same preference
  information.

Configuration
~~~~~~~~~~~~~

This step uses the following network (``BGP_LOCAL_PREF.ned``):

.. figure:: media/BGP_LOCAL_PREF.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_LOCAL_PREF.ned
   :start-at: network BGP
   :language: ned

The topology features AS 64600 (RB) with two border routers (RB1, RB2) and one
interior router (RB3). Both border routers are connected to RA (AS 64500).

The configuration in ``omnetpp.ini`` defines the setup:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10
   :end-before: ------

In the BGP configuration (``BGPConfig_LOCAL_PREF.xml``), RB1 and RB2 are 
configured to assign different ``localPreference`` values to their sessions 
with RB3:

.. literalinclude:: ../BGPConfig_LOCAL_PREF.xml
   :language: xml
   :start-at: <Neighbor address='20.0.0.2'
   :end-at: localPreference='600' />

- **RB1** sets ``localPreference='100'`` for the route to RA.
- **RB2** sets ``localPreference='600'`` for the same route.

Results
~~~~~~~

The simulation results in ``step10.rt`` demonstrate how RB3 chooses its path
to RA's network (10.0.0.0/30):

1. **E-BGP Discovery**: Both RB1 and RB2 learn the route to 10.0.0.0/30 from RA
   via their respective E-BGP sessions.
2. **I-BGP Propagation**: RB1 and RB2 advertise this route to RB3 via I-BGP.
3. **Initial Route Selection**: Initially, RB3 installs the route learned from
   RB1 (next hop 20.0.0.1) at approximately 60.0s.
4. **Best Path Update**: Shortly after (at 62.0s), RB3 receives the update from
   RB2. Because RB2's advertisement carries a higher Local Preference (600 vs
   100), RB3 **updates its routing table** to use RB2 (next hop 20.0.0.5) as the
   preferred exit point.

You can verify this in ``step10.rt`` by observing the route change at the 62s
mark for router RB3.

This step illustrates how network administrators can use Local Preference to
enforce routing policies, such as preferring a high-bandwidth link or a specific
upstream provider for outbound traffic.

Sources: :download:`BGP_LOCAL_PREF.ned <../BGP_LOCAL_PREF.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_LOCAL_PREF.xml <../BGPConfig_LOCAL_PREF.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
