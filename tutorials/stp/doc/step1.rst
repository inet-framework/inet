Step 1. The Problem: Broadcast Loops Without STP
================================================

Goals
-----

Before learning how STP solves the problem, it is important to understand what
happens in a bridged Ethernet network with redundant links but *without* STP.
This step demonstrates that loops in a switched network cause frames to
circulate indefinitely, creating a broadcast storm that prevents reliable
communication.

Background
----------

Ethernet switches (also called *bridges* in the IEEE 802.1D standard) forward
frames based on destination MAC addresses. When a switch receives a frame, it
looks up the destination address in its *MAC forwarding table* (also called
*filtering database*). If the address is found, the frame is sent out the
corresponding port; if not, the frame is *flooded* — sent out all ports except
the one it arrived on. Broadcast and multicast frames are always flooded.

Switches build their forwarding tables automatically through *MAC learning*:
when a frame arrives on a port, the switch records the frame's source MAC
address and the port it arrived on. Over time, the table is populated with
entries that map each known host address to a specific port.

In enterprise and data-center networks, redundant links between switches are
common — they protect against individual link or switch failures that would
otherwise partition the network. However, redundant links inevitably create
*loops* in the network topology, and as this step demonstrates, loops and the
flood-and-learn mechanism are a dangerous combination.

The Network
-----------

The simulation uses the ``LoopNetwork`` network, which contains three Ethernet
switches (``switch1``, ``switch2``, ``switch3``) connected in a ring, with two
hosts (``host1`` and ``host2``) attached to ``switch1`` and ``switch3``
respectively.

.. figure:: media/LoopNetwork_nostp.png
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
arrives at both ``switch2`` and ``switch3``. Neither switch knows ``host2``'s
location either, so both flood the frame further — creating two copies
circulating in opposite directions around the ring.

Each time a switch receives one of these circulating copies, it floods it again,
causing the number of frame copies in the network to grow exponentially. This is
a *broadcast storm*.

The loop also causes *MAC table instability*. Consider the path of
``host1``'s frame: when it first arrives at ``switch3`` directly from
``switch1``, the switch learns that ``host1`` is reachable via the port
connected to ``switch1``. Moments later, the *same* frame arrives at
``switch3`` from ``switch2`` (having gone the long way around the ring), and
the switch overwrites its table entry to say ``host1`` is reachable via the
``switch2`` port. This oscillation repeats continuously with each circulating
copy, making the forwarding table useless.

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
