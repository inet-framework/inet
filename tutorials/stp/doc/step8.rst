Step 8. Link Reconnect with RSTP
=================================

Goals
-----

This step demonstrates another type of topology change: a link that is
disconnected and then reconnected. Unlike a full switch failure, a link
event affects only the two switches at each end of the link. We observe
how RSTP handles this gracefully.

Configuration
~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step8]

The ``Step8`` configuration extends ``Step7`` but uses a different scenario
script, :download:`reconnect.xml <../reconnect.xml>`:

.. literalinclude:: ../reconnect.xml
   :language: xml

At t=60 s, the link between ``switch1`` and ``switch3`` is disconnected (the
gate ``switch1.ethg[1]`` is the port connected to ``switch3``). At t=120 s,
the same link is reconnected using a ``connect`` action.

The ``CarrierBasedLifeTimer`` (``lifetimer.typename = "CarrierBasedLifeTimer"``)
on the interface causes the port to detect the carrier loss/gain and trigger
RSTP's rapid state transitions.

How RSTP Handles Link Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Carrier-based vs. timer-based detection.** By default, RSTP detects a
neighbor's disappearance by noticing missed BPDUs — if 3 consecutive hellos
are missed (6 s), the port's stored information expires. However, when the
physical link itself goes down (cable unplugged, interface shut down), the
Ethernet interface can report *carrier loss* to the upper layers immediately.

The ``CarrierBasedLifeTimer`` (configured via
``lifetimer.typename = "CarrierBasedLifeTimer"``) enables this fast detection
path in INET: the RSTP module is notified of link-down events as soon as the
physical layer detects them, rather than waiting for BPDU timeouts. This
reduces failure detection time from 6 s to effectively zero.

**Link down.** When a link goes down and the port detects carrier loss:

1. If the port was a *Root port*, the switch promotes its best *Alternate port*
   to Root port immediately (no timer wait).
2. If the port was a *Designated port*, the switch marks it as *Discarding* and
   sends topology change notifications to flush MAC tables across the network.

**Link up.** When a link comes back up and carrier is detected:

1. The port starts in Discarding state and begins sending BPDUs to its new
   neighbor.
2. RSTP runs the *proposal/agreement* handshake with the neighbor to determine
   port roles and safely transition to Forwarding.
3. If the restored link offers a better path cost than the current tree paths,
   the tree may restructure to incorporate it. Otherwise, the port becomes an
   Alternate or Backup port in Discarding state.

Results
~~~~~~~

**At t=60 s** — The link ``switch1↔switch3`` goes down. This link was part
of the active tree, so RSTP reacts within one hello interval (~2 s) and reroutes
traffic over the alternate path between ``switch4`` and ``switch7``.

.. video_noloop:: media/step8failure2.mp4
   :width: 100%
   :align: center

**At t=120 s** — The link is reconnected. RSTP detects the new link and runs
the proposal/agreement handshake. If the link's path cost makes it preferable
to the current tree path, it may become part of the active tree. Otherwise it
becomes an Alternate (backup) port in Discarding state.

.. video_noloop:: media/step8recovery2.mp4
   :width: 100%
   :align: center

In both cases, re-convergence completes in a few seconds, and all hosts
maintain connectivity with minimal interruption — demonstrating RSTP's
resilience to link-level topology changes.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`,
:download:`reconnect.xml <../reconnect.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
