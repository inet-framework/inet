Step 7. Topology Change with RSTP: Faster Recovery
===================================================

Goals
-----

This step runs the same switch-failure scenario as Step 6 but with RSTP
enabled instead of STP. The goal is to observe the dramatically faster
re-convergence that RSTP provides after a topology change.

Configuration
~~~~~~~~~~~~~

The ``Step7`` configuration extends ``Step6`` with only one change:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step7]
   :end-before: ------

The same ``LargeNet`` network and ``failure.xml`` script are used. The only
difference is ``spanningTreeProtocol = "Rstp"``.

How RSTP Handles Topology Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a switch detects that a neighbor is no longer sending BPDUs (link failure),
RSTP reacts immediately instead of waiting for ``maxAge`` to expire:

1. The switch that loses a root or designated port immediately initiates a new
   election using its stored *alternate port* as the new root port — no waiting
   required.
2. The topology change (TC) flag in BPDUs propagates quickly through the network,
   causing all switches to flush their MAC forwarding tables on non-edge ports.
3. The *proposal/agreement* mechanism allows newly designated ports to transition
   to Forwarding within one or two hello intervals (≈ 2–4 s).

Total re-convergence time after a failure with RSTP: approximately **6 s**.

Results
~~~~~~~

**Before failure (t=0–60 s)**: RSTP converges in ~6 s. All hosts communicate
normally. The active tree is identical to the STP case (same root, same paths),
but it was established much faster.

**At t=60 s** — ``switch4`` shuts down. RSTP detects the failure within one
hello interval (2 s) and immediately begins re-election using alternate ports.
The tree re-stabilizes in approximately **6 s**, compared to **~50 s** for STP.
Hosts experience only a brief connectivity interruption.

**At t=120 s** — ``switch4`` restarts. RSTP's edge-port and proposal/agreement
mechanisms allow ``switch4`` to rejoin the tree quickly without going through
the long Listening/Learning cycle.

.. figure:: media/step7result.png
   :width: 90%
   :align: center

The comparison between Step 6 and Step 7 clearly illustrates why RSTP is
preferred over STP in modern networks: the ~50 s reconvergence of STP causes
noticeable outages, while RSTP's ~6 s recovery is nearly transparent.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`failure.xml <../failure.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
