Step 11. Multi-hop E-BGP
========================

Goals
-----

Step 11 introduces **Multi-hop E-BGP**. By default, External BGP (E-BGP) assumes
that peers are directly connected (on the same physical link) and uses an IP
Time-To-Live (TTL) of 1 for its BGP packets. If border routers are separated by
one or more intermediate routers, the default E-BGP session closure will fail.

This scenario (``Multihop_EBGP.ned``) demonstrates how to establish an E-BGP
session between two border routers (RA and RB) that are separated by an
intermediate router (R) belonging to a different infrastructure or acting as a
simple forwarder.

Key features demonstrated:

- **E-BGP Multihop**: Using the ``ebgpMultihop`` attribute in the BGP
  configuration to increase the packet TTL, allowing the BGP session to traverse
  intermediate hops.
- **Peering Reachability**: In a multi-hop setup, peers must know how to reach
  each other's IP addresses *before* BGP can start. This is often achieved
  through static routes or an IGP.
- **Transit Routing**: Verifying that the intermediate router (R) successfully
  forwards BGP control traffic and subsequent data traffic between ASes.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/Multihop_EBGP.png
   :width: 100%
   :align: center

.. literalinclude:: ../Multihop_EBGP.ned
   :start-at: network Multihop_EBGP
   :language: ned

The topology is a simple chain: ``RA <--> R <--> RB``.

- **RA** belongs to AS 64520.
- **RB** belongs to AS 64530.
- **R** is an intermediate node.

The ``omnetpp.ini`` configuration sets up static routes so RA and RB can reach each other's peering addresses:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step11
   :end-before: ------

The BGP configuration (``BGPConfig_MultiHopEBGP.xml``) specifies ``ebgpMultihop='2'`` for both neighbors:

.. literalinclude:: ../BGPConfig_MultiHopEBGP.xml
   :language: xml
   :start-at: <AS id="64520">
   :end-before: <!--bi-directional

Results
~~~~~~~

The simulation results in ``step11.rt`` show the successful establishment of the multi-hop session:

1. **Static Convergence**: RA and RB use the configured static routes to reach each other via R.

2. **BGP Session Establishment**: Because ``ebgpMultihop='2'`` is set, the BGP
   packets sent by RA can reach RB (and vice-versa) even though they pass
   through R. The TCP connection is established successfully.

3. **Route Exchange**:

   - RA advertises its 10.0.0.0/30 network to RB.
   - RB advertises its 20.0.0.0/30 network to RA.

4. **Data Reachability**: Once BGP converges, hosts behind RA can reach hosts
   behind RB. ``step11.rt`` confirms that RA eventually has a BGP route to
   20.0.0.0/30 with RB's address as the next hop.

This step demonstrates a common real-world scenario where border routers might
not be physically adjacent, requiring multihop capabilities to form the BGP
adjacency.

Sources: :download:`Multihop_EBGP.ned <../Multihop_EBGP.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_MultiHopEBGP.xml <../BGPConfig_MultiHopEBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
