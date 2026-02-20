Step 4. STP Convergence Time
=============================

Goals
-----

This step focuses on the time STP takes to converge — the period from
when the simulation starts until all ports are in a stable Forwarding or
Blocking state. Understanding convergence time is important because hosts
cannot communicate reliably until the tree is stable.

STP Port States
~~~~~~~~~~~~~~~

STP ports transition through four states:

- **Blocking**: The port discards all frames except BPDUs. This prevents loops
  during the convergence process.
- **Listening**: The port participates in BPDU exchange to determine its role
  but still discards data frames. Duration: ``forwardDelay`` (default 15 s).
- **Learning**: The port begins building its MAC forwarding table but still
  discards data frames. Duration: ``forwardDelay`` (default 15 s).
- **Forwarding**: The port forwards data frames normally.

The total time from startup to forwarding for a designated port is therefore
at least ``2 × forwardDelay = 30 s``. In practice, root election and path
cost calculation add several more seconds, bringing the total to approximately
50 s.

During convergence, the port role and state are shown as a label on each
switch interface in the INET canvas (when ``visualize = true``).

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step4]
   :end-before: ------

The simulation runs for 200 s, giving time to observe the full convergence
cycle and traffic flowing afterward. Traffic starts at t=60 s, after
convergence is complete.

The ``forwardDelay`` and ``helloTime`` timers can be adjusted on the :ned:`Stp`
module:

- ``helloTime`` (default 2 s): interval between Hello BPDUs sent by the root
- ``forwardDelay`` (default 15 s): time spent in Listening and Learning states
- ``maxAge`` (default 20 s): maximum age of saved BPDU information

Results
~~~~~~~

When the simulation starts, you can observe the port state labels on each
switch interface cycling through Blocking → Listening → Learning → Forwarding.
Root ports and designated ports reach the Forwarding state after approximately
30 s. Non-designated ports remain in Blocking.

.. figure:: media/step4convergence.png
   :width: 90%
   :align: center

By t≈50 s the tree is fully converged and traffic from ``host2`` reaches
``host1`` without issues. The next step shows how RSTP reduces this 50 s
convergence time to under 10 s.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
