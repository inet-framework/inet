MPLS Traffic Engineering with RSVP-TE
=====================================

Goals
-----

MPLS Traffic Engineering (MPLS-TE) uses RSVP-TE to set up label-switched
paths (LSPs) that follow explicit, engineered routes with reserved
bandwidth, rather than the shortest path an IGP would otherwise choose. This
lets a network operator steer traffic engineering-aware flows away from
congested links, guarantee bandwidth to selected traffic, and protect
high-priority traffic from lower-priority traffic when links are scarce.

This showcase demonstrates three RSVP-TE traffic engineering mechanisms
using INET's MPLS/RSVP-TE models:

(a) setting up explicit-route LSPs with bandwidth reservation,
(b) setup/holding priority preemption, where a higher-priority LSP evicts a
    lower-priority one when bandwidth becomes scarce, and
(c) Hello-based failure detection and recovery, where a transit router
    failure is detected, the affected LSP is torn down, and (once the
    router recovers) the permanent LSP is automatically re-signaled.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/mpls/te <https://github.com/inet-framework/inet/tree/master/showcases/mpls/te>`__

The Model
---------

The Network
~~~~~~~~~~~

This showcase uses the ``MplsTeShowcase`` network, defined in
:download:`MplsTeShowcase.ned <../MplsTeShowcase.ned>`. It has 8 label
switching routers (:ned:`RsvpMplsRouter`) and 4 hosts:

- **LSR1** is the ingress router. It carries the classifier and RSVP-TE
  traffic configuration for both tunnels, and is connected to client hosts
  ``host1`` and ``host2``.
- **LSR2**/**LSR4** and **LSR3**/**LSR7** form two node-disjoint paths from
  LSR1 towards the egress side; **LSR6** provides spare bypass capacity
  around LSR4 that neither tunnel's hand-written route uses (this model has
  no CSPF to route around a failure automatically -- see the note under
  "Failure Detection and Recovery" below).
- **LSR5** is the merge point where all backbone paths converge before the
  egress.
- **LSR8** is the egress router, connected to server hosts ``host3`` and
  ``host4``.

Router IDs are derived automatically from each router's highest interface
address (``routerId = "auto"``), RSVP-TE Hello peers are derived
automatically from the TED (the ``peers = "auto"`` default), and
:ned:`Ipv4NetworkConfigurator` assigns all IPv4 addresses and computes the
initial IPv4 routes -- no manual routing table files are needed.

Two RSVP-TE tunnels are configured on LSR1 (in
:download:`LSR1_rsvp.xml <../LSR1_rsvp.xml>`, referenced by the classifier's
FEC table in :download:`LSR1_fec.xml <../LSR1_fec.xml>`):

- **Tunnel 1** (``host1`` to ``host3``), high priority (setup/holding
  priority 1), 400 kbit/s, explicit route LSR1-LSR2-LSR4-LSR5-LSR8.
- **Tunnel 2** (``host2`` to ``host4``), default priority (7, the lowest),
  400 kbit/s, explicit route LSR1-LSR3-LSR7-LSR5-LSR8.

Both tunnels are marked ``permanent``, so :ned:`RsvpTe` automatically retries
signaling them if a setup attempt fails.

(a) Explicit-Route LSPs with Bandwidth Reservation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Neither LSP's route is computed by the model: this codebase does not
implement CSPF (Constrained Shortest Path First), so both tunnels' explicit
routes above are hand-written, hop by hop, in the traffic XML. This is
exercised by every configuration in this showcase, and demonstrated in
isolation by the ``ExplicitRouteBandwidth`` configuration: LSR1 signals both
Path messages along their fixed EROs, each downstream router runs admission
control (CAC) against the requested bandwidth and installs the reservation,
and the ingress receives a ``Resv`` back confirming the LSP and installing
its ingress label.

.. literalinclude:: ../LSR1_rsvp.xml
   :language: xml
   :start-at: <route>
   :end-at: </route>

(b) Priority Preemption
~~~~~~~~~~~~~~~~~~~~~~~

The ``PriorityPreemption`` configuration adds a second path to tunnel 1 at
t=2s, via a :ned:`ScenarioManager` ``add-session`` command
(:download:`scenario_preemption.xml <../scenario_preemption.xml>`). The new
path inherits tunnel 1's high setup/holding priority (1) and is routed over
the exact same links tunnel 2's low-priority (7) reservation already uses
(LSR1-LSR3-LSR7-LSR5-LSR8). Those links only have 600 kbit/s of capacity, so
the combined demand (400 + 400 = 800 kbit/s) cannot be satisfied; because
the new path's setup priority is better than tunnel 2's holding priority,
RSVP-TE preempts tunnel 2's reservation (rather than rejecting the new
path), sends a ResvErr downstream towards the egress for the evicted
reservation, and the egress removes its own reservation state upon
receiving it.

.. literalinclude:: ../scenario_preemption.xml
   :language: xml
   :start-at: <add-session
   :end-at: </add-session>

(c) Failure Detection and Recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``FailureReroute`` configuration
(:download:`scenario_failure.xml <../scenario_failure.xml>`) shuts LSR4 down
at t=2s and restarts it at t=4s. LSR4 is a transit hop of tunnel 1's route,
so the LSP breaks: LSR2 (LSR4's Hello neighbor on the path) notices the dead
adjacency once its Hello timer expires, and raises a PathErr that
propagates back to the ingress. LSR1 tears the broken path down and, since
it is permanent, schedules a retry.

Because the route is a fully explicit, hand-written ERO rather than a
CSPF-computed one, the model has no way to route around LSR4 automatically:
the retry keeps re-signaling the *same* broken route and keeps failing
until LSR4 itself comes back up at t=4s, at which point the retried path
succeeds again. (The ``LSR6`` bypass mentioned earlier could, in principle,
carry this traffic around LSR4 -- but only a CSPF-capable ingress, which
this model does not have, could discover and use it automatically.)
Meanwhile, tunnel 2 -- which does not use LSR4 -- is unaffected throughout.

Configuration
-------------

The simulation is configured in :download:`omnetpp.ini <../omnetpp.ini>`
with three named configurations, one per behavior demonstrated:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ExplicitRouteBandwidth]

Results
-------

Running ``ExplicitRouteBandwidth`` shows both tunnels' Path/Resv exchanges
completing and both hosts' UDP traffic being delivered end to end, with the
ingress logging ``Path successfully established`` for each tunnel.

Running ``PriorityPreemption`` additionally shows, at t=2s, the ingress
receiving the ``add-session`` command (``adding new path into an existing
session``), a downstream router preempting tunnel 2's reservation
(``preempting RSB ... releasing 400000`` followed by ``sending ResvErr``),
and the egress logging the resulting ``reservation failed`` for tunnel 2.

Running ``FailureReroute`` shows the shutdown/startup commands being
processed, LSR2 logging ``hello timeout, considering ... failed``, the
ingress logging ``error reached ingress router`` (repeated, since retries
over the same broken route keep failing while LSR4 is down), and finally a
second ``Path successfully established`` for tunnel 1 once LSR4 restarts;
tunnel 2's traffic keeps flowing throughout.

If recording vector/scalar results is enabled, the following signals (added
to :ned:`RsvpTe` and :ned:`LibTable` for this kind of use case) are of particular
interest for charting these behaviors: ``lspEstablished`` (LSP setup
latency, emitted at the ingress), ``psbCount``/``rsbCount`` (the number of
active path/reservation state blocks on a router over time -- useful for
visualizing the preemption and the failure/recovery cycle), and
``libEntryCount`` (the number of active LIB entries on a router, i.e. how
much of the data plane is currently programmed).

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`MplsTeShowcase.ned <../MplsTeShowcase.ned>`, :download:`LSR1_fec.xml <../LSR1_fec.xml>`, :download:`LSR1_rsvp.xml <../LSR1_rsvp.xml>`, :download:`scenario_preemption.xml <../scenario_preemption.xml>`, :download:`scenario_failure.xml <../scenario_failure.xml>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/mpls/te`` folder in the `Project Explorer`. There, you can
view and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/mpls/te && inet'

This command creates an ``inet-workspace`` directory, installs the
appropriate versions of INET and OMNeT++ within it, and launches the
``inet`` command in the showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET
project, then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this showcase.
