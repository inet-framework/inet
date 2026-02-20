Step 1. The Problem: Broadcast Loops Without STP
================================================

Goals
-----

Before learning how STP solves the problem, it is important to understand what
happens in a bridged Ethernet network with redundant links but *without* STP.
This step demonstrates that loops in a switched network cause frames to
circulate indefinitely, creating a broadcast storm that prevents reliable
communication.

The Network
-----------

The simulation uses the ``LoopNetwork`` network, which contains three Ethernet
switches (``switch1``, ``switch2``, ``switch3``) connected in a ring, with two
hosts (``host1`` and ``host2``) attached to ``switch1`` and ``switch3``
respectively.

.. figure:: media/step1network.png
   :width: 70%
   :align: center

The ring topology provides redundant paths between the hosts (``host1`` can reach
``host2`` via ``switch1→switch2→switch3`` or directly via ``switch1→switch3``),
but without STP, both paths are active simultaneously, causing loops.

Configuration
~~~~~~~~~~~~~

The configuration is in the ``Step1`` section of :download:`omnetpp.ini <../omnetpp.ini>`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step1]
   :end-before: ------

No STP is enabled (there is no ``hasStp`` setting). ``host1`` sends Ethernet
frames to ``host2`` every second.

Results
~~~~~~~

When ``host1`` sends a frame to ``host2``, ``switch1`` does not yet know which
port leads to ``host2``, so it floods the frame out all other ports. The frame
arrives at ``switch2`` and ``switch3`` simultaneously. Both switches also don't
know ``host2``'s location and flood the frame further — creating two copies of
the frame circulating in opposite directions around the ring.

Each time a switch receives one of these circulating copies, it floods it again,
causing the number of frame copies to grow exponentially. This is a *broadcast
storm*. The MAC forwarding tables in each switch become inconsistent, as the same
MAC address appears on different ports depending on which direction the looping
frames arrive from.

As a result, ``host2`` receives many duplicate copies of each frame sent by
``host1``, and communication is completely unreliable.

.. figure:: media/step1result.png
   :width: 80%
   :align: center

The next step shows how STP solves this problem by logically breaking the ring.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`StpTutorial.ned <../StpTutorial.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
