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

When a link goes down, the port connected to that link immediately loses carrier.
RSTP detects this and:

1. If the port was a *Root port*, the switch promotes its best *Alternate port*
   to Root port immediately (no timer wait).
2. If the port was a *Designated port*, the switch marks it as *Discarding* and
   sends topology change notifications.

When a link comes back up, RSTP runs the *proposal/agreement* handshake with
the newly connected neighbor to quickly determine port roles and transition
to Forwarding.

Results
~~~~~~~

**At t=60 s** — The link ``switch1↔switch3`` goes down. If this link was part
of the active tree, RSTP reacts within one hello interval (~2 s) and reroutes
traffic over an alternate path. If it was a blocked alternate path, the topology
is unaffected.

.. figure:: media/step8disconnect.png
   :width: 80%
   :align: center

**At t=120 s** — The link is reconnected. RSTP detects the new link and runs
the proposal/agreement handshake. If the link's path cost makes it preferable
to the current tree path, it may become part of the active tree. Otherwise it
becomes an Alternate (backup) port in Discarding state.

.. figure:: media/step8reconnect.png
   :width: 80%
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
