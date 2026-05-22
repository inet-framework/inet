Comparing PIM-DM and PIM-SM Multicast Routing
=============================================

Goals
-----

Multicast routing allows efficient delivery of data to multiple recipients
by sending packets only once over each link. PIM (Protocol Independent Multicast)
is the dominant multicast routing protocol family, with two main modes:
PIM-DM (Dense Mode) and PIM-SM (Sparse Mode).

In this showcase, we compare these two modes using the same network topology
to highlight their fundamental behavioral differences. We'll see how PIM-DM
floods traffic everywhere then prunes back, while PIM-SM builds explicit
distribution trees via Join messages.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/routing/pim <https://github.com/inet-framework/inet/tree/master/showcases/routing/pim>`__

About PIM
---------

PIM is called "Protocol Independent" because it relies on an underlying
unicast routing protocol (like OSPF or static routes) to determine the
Reverse Path Forwarding (RPF) neighbor. PIM itself does not maintain
unicast routing tables.

PIM-DM (Dense Mode) — RFC 3973
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PIM-DM assumes receivers are densely distributed throughout the network.
It uses a **flood-and-prune** approach:

1. **Flood**: Multicast traffic initially floods to all PIM-enabled routers
2. **Prune**: Routers without interested receivers send Prune messages upstream
3. **Re-flood**: After a timeout, flooding resumes (in case new receivers joined)
4. **Graft**: Receivers can rejoin quickly using Graft messages

PIM-DM is simple but can waste bandwidth due to periodic flooding,
especially in networks with few receivers.

PIM-SM (Sparse Mode) — RFC 4601
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PIM-SM assumes receivers are sparsely distributed. It uses a
**rendezvous point (RP)** architecture:

1. **Join**: Receivers explicitly request multicast traffic by sending Join
   messages toward the RP, building a shared distribution tree (*,G)
2. **Register**: Sources send multicast data encapsulated in Register messages
   to the RP, which decapsulates and forwards down the shared tree
3. **Register-Stop**: The RP stops Register messages once traffic flows
4. **KeepAlive**: Soft-state timers maintain the tree (expired state is removed)

PIM-SM avoids flooding entirely. Traffic only goes where Join messages
have established state.

The Model
---------

The showcase uses the `PimComparisonShowcase` network with 4 MulticastRouters,
1 source, and 2 receivers:

.. figure:: media/network.png
   :align: center

The network topology features two paths from source to receivers.
Receiver1 joins multicast group 239.0.0.1; Receiver2 does not.
In PIM-SM mode, R2 acts as the Rendezvous Point (RP).

PIM in INET
~~~~~~~~~~~

INET implements both PIM-DM (`PimDm`) and PIM-SM (`PimSm`) as simple modules
inside the compound `Pim` module. The `PimSplitter` dispatches incoming packets
to the appropriate mode based on interface configuration.

Interface mode is configured via XML in `pimConfig`:

- Dense mode: ``<interface mode="dense"/>``
- Sparse mode: ``<interface mode="sparse"/>``

For PIM-SM, the RP address must also be configured:

.. code-block:: ini

   **.RP.**.routerId = "10.0.0.4"
   **.RP = "10.0.0.4"

The `MulticastRouter` node type has `hasPim = true` and `multicastForwarding = true`
by default, making it ready for multicast routing.

Configuration
-------------

The `omnetpp.ini` file contains two configurations demonstrating the same
scenario with different PIM modes:

.. literalinclude:: ../omnetpp.ini
   :language: ini

The source starts sending multicast traffic at 10s. Receiver1 joins the
group at 5s (before traffic starts). Receiver2 never joins.

Results
-------

PIM-DM Results
~~~~~~~~~~~~~~

In PIM-DM mode, the initial multicast packets flood to all routers.
R4 (the router toward Receiver2) sends a Prune message upstream because
Receiver2 did not join. However, traffic still arrives at R4 briefly
before the prune takes effect.

.. figure:: media/dm-flood.png
   :align: center

The prune state times out after the default 180s, causing a re-flood.
This periodic flooding continues throughout the simulation.

PIM-SM Results
~~~~~~~~~~~~~~

In PIM-SM mode, traffic only flows where explicitly requested.
Receiver1's Join message creates state in R3 and R2 toward the RP.
When the source sends traffic, it reaches the RP via Register messages,
then flows down the established tree to Receiver1.

.. figure:: media/sm-tree.png
   :align: center

R4 never receives the multicast traffic because no Join was sent
and no flooding occurs.

Comparison
~~~~~~~~~~

.. list-table::
   :header-rows: 1

   * - Aspect
     - PIM-DM
     - PIM-SM
   * - Initial behavior
     - Flood everywhere
     - No traffic until Join
   * - Traffic at R4 (no receiver)
     - Briefly, then pruned
     - Never arrives
   * - Overhead
     - Periodic re-flooding
     - Soft-state Join refresh
   * - Best for
     - Dense receivers
     - Sparse receivers

The packet statistics confirm the difference:
PIM-DM shows more total packets due to flooding and periodic re-flooding,
while PIM-SM shows targeted delivery only to interested receivers.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PimShowcase.ned <../PimShowcase.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project, then navigate to
``inet/showcases/routing/pim`` in the Project Explorer.

Otherwise, install using `opp_env <https://omnetpp.org/opp_env>`__:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/routing/pim && inet'

Run the PIM-DM configuration:

.. code-block:: bash

    $ inet -c PimDm

Run the PIM-SM configuration:

.. code-block:: bash

    $ inet -c PimSm

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/XXX>`__
in the GitHub issue tracker for commenting on this showcase.
