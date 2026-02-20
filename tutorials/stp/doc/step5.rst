Step 5. RSTP: Faster Convergence
=================================

Goals
-----

This step introduces the Rapid Spanning Tree Protocol (RSTP, IEEE 802.1D-2004),
which dramatically reduces convergence time compared to STP. We will run the
same network as Steps 2–4 with RSTP enabled and observe the difference.

About RSTP
~~~~~~~~~~

RSTP improves on STP in several key ways:

- **Rapid port transitions**: Instead of waiting through Listening/Learning
  states (each 15 s), RSTP uses a *proposal/agreement* handshake between
  neighboring switches to transition ports to Forwarding almost immediately.
  A port can become Forwarding within a few hello intervals (≈ 6 s total).

- **Edge ports**: Ports connected to end hosts (not other switches) are
  configured as *edge ports* (``autoEdge = true`` by default in INET). Edge
  ports transition directly to Forwarding without any delay, since they cannot
  create loops.

- **Port roles**: RSTP defines the same Root and Designated roles as STP, but
  adds *Alternate* (backup path to root) and *Backup* (backup on shared segment)
  roles. Alternate ports can become Root ports quickly when the primary path fails.

- **Active topology change**: Instead of waiting for ``maxAge`` timers, RSTP
  actively notifies neighbors of topology changes using the TC flag in BPDUs.

In INET, RSTP is implemented by the :ned:`Rstp` module. Key parameters:

- ``forwardDelay`` (default 6 s): used only as fallback; rapid transitions bypass this
- ``migrateTime`` (default 3 s): time before an unassigned port becomes Designated
- ``autoEdge`` (default true): automatically detect and configure edge ports

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5]
   :end-before: ------

The only change from the STP configuration is ``spanningTreeProtocol = "Rstp"``.
The simulation runs for only 30 s (versus 200 s for the STP step), because
RSTP converges quickly. Traffic starts at t=15 s.

Results
~~~~~~~

With RSTP, the spanning tree converges in approximately 6 s — roughly eight
times faster than STP. The port state labels on switch interfaces cycle
much more quickly through the RSTP states (Discarding → Learning → Forwarding).

.. figure:: media/step5result.png
   :width: 80%
   :align: center

Edge ports (ports connected to ``host1`` and ``host2``) transition to Forwarding
almost immediately (within one hello interval ≈ 2 s), because they are
automatically identified as edge ports.

At t=15 s, traffic from ``host2`` reaches ``host1`` reliably, demonstrating
that the tree has converged well before the STP equivalent would have.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
