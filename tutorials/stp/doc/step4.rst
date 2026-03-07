Step 4. STP Convergence Time
=============================

Goals
-----

This step focuses on the time STP takes to converge — the period from
when the simulation starts until all ports are in a stable Forwarding or
Blocking state. Understanding convergence time is important because hosts
cannot communicate reliably until the tree is stable.

STP Port States and Timers
~~~~~~~~~~~~~~~~~~~~~~~~~~

After the root bridge is elected and port roles (Root, Designated, or Blocked)
are determined, ports do not begin forwarding data immediately. Instead, STP
uses a conservative state machine to ensure the *entire* network has agreed on
the new topology before any data flows. This prevents temporary loops that
could form if some switches started forwarding before others had finished
updating their port roles.

STP ports transition through four states:

- **Blocking**: The port discards all data frames and only processes BPDUs. All
  ports start in this state. A port that is neither Root nor Designated remains
  here permanently (until the topology changes).
- **Listening**: The port has been elected Root or Designated and begins
  participating actively in the STP algorithm (sending and processing BPDUs)
  but still discards data frames. This phase lasts ``forwardDelay`` seconds
  (default 15 s) — long enough for the election results to propagate across
  the entire network.
- **Learning**: The port still does not forward data frames, but it begins
  observing incoming frames to populate its MAC forwarding table. This ensures
  that when the port finally starts forwarding, the table already contains
  useful entries, reducing unnecessary flooding. This phase also lasts
  ``forwardDelay`` seconds (default 15 s).
- **Forwarding**: The port forwards data frames normally.

The total time from startup to Forwarding for a Root or Designated port is
therefore at least ``2 × forwardDelay = 30 s``. Adding the time needed for
BPDUs to propagate across the network and for the root election to settle
brings the typical total convergence time to approximately **35 s**.

Three timers govern STP's operation:

- ``helloTime`` (default 2 s): The root bridge sends a BPDU every ``helloTime``
  seconds. Other switches relay these BPDUs on their designated ports.
- ``maxAge`` (default 20 s): If a switch does not receive a BPDU on a port for
  ``maxAge`` seconds, it assumes the topology has changed and re-evaluates its
  port roles. This timer limits how long stale information is trusted.
- ``forwardDelay`` (default 15 s): The duration of the Listening and Learning
  states. It is set conservatively to ensure convergence across networks up to
  a certain diameter (number of hops from root to the farthest switch).
- ``holdTime`` (default 1 s): The minimum interval between successive BPDU
  transmissions on a single port. This prevents a switch from flooding the
  network with BPDUs during rapid topology changes. If a BPDU needs to be sent
  but the hold timer has not yet expired, the transmission is deferred until
  the timer allows it.

These timers are related: the network diameter, ``maxAge``, and ``forwardDelay``
must be chosen so that topology information has time to propagate across the
entire network before any port starts forwarding. The defaults are designed for
networks with a diameter of up to 7 switches.

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

The ``forwardDelay``, ``helloTime``, and ``maxAge`` timers can be adjusted on
the :ned:`Stp` module.

Results
~~~~~~~

When the simulation starts, you can observe the port state labels on each
switch interface cycling through Blocking → Listening → Learning → Forwarding.
Root ports and designated ports reach the Forwarding state after approximately
30 s. Non-designated ports remain in Blocking.

.. video_noloop:: media/step4convergence_realtime.mp4
   :width: 100%
   :align: center

By t≈35 s the tree is fully converged and traffic from ``host2`` reaches
``host1`` without issues. The next step demonstrates how STP handles topology
changes such as switch failures, and how ~50 s convergence delay (including
``maxAge``) applies to failure recovery.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
