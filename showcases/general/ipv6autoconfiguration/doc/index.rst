IPv6 Address Autoconfiguration
==============================

Goals
-----

A host that joins a network cannot do anything until it has an address. In IPv4,
someone has to supply that address: an administrator configures it by hand, or a
DHCP server hands it out. Both answers need something on the network besides the
host itself.

IPv6 offers a way that needs neither. A host builds its own addresses from two
things: the identifier of its own network interface, and a prefix that the local
router announces to everyone on the link. This is called Stateless Address
Autoconfiguration (SLAAC). No server keeps any state, and nothing is configured
by hand. IPv6 also has a stateful alternative, DHCPv6, which is widely used where
an operator wants central control; this showcase is about the stateless method,
and INET does not implement DHCPv6.

Building your own address raises an obvious risk. If two hosts pick the same
address, both break. IPv6 answers this with Duplicate Address Detection (DAD):
before a host uses an address, it asks the link whether anyone already has it.

This showcase demonstrates both mechanisms. In the first simulation, four hosts
and a server start with no addresses and end up fully configured. In the second, a
host carrying a duplicated hardware address joins the same network, and Duplicate
Address Detection refuses the address it tried to claim.

| Verified with INET version: ``4.7``
| Source files location: `inet/showcases/general/ipv6autoconfiguration <https://github.com/inet-framework/inet/tree/master/showcases/general/ipv6autoconfiguration>`__

About IPv6 Address Autoconfiguration
------------------------------------

Stateless Address Autoconfiguration (SLAAC) is part of a larger protocol called
Neighbor Discovery (ND), defined in RFC 4861. Address autoconfiguration itself is
defined in RFC 4862.

The interface identifier
~~~~~~~~~~~~~~~~~~~~~~~~

Every IPv6 address is 128 bits. On an Ethernet link the lower 64 bits are the
*interface identifier*, and a host derives it from the 48-bit MAC address of its
network interface. The derivation is called Modified EUI-64, and it takes three
steps:

1. Split the MAC address in half and insert the two bytes ``FF:FE`` between the
   halves. This stretches 48 bits to 64.
2. Invert the second-lowest bit of the first byte. This bit distinguishes a
   globally unique identifier from a locally assigned one.
3. Write the result as four groups of 16 bits.

For the MAC address ``0A-AA-00-00-00-09``, which one of the hosts in this
showcase uses, the steps give:

.. code-block:: none

   0A-AA-00-00-00-09          the MAC address
   0A-AA-00-FF-FE-00-00-09    after inserting FF:FE
   08-AA-00-FF-FE-00-00-09    after inverting the bit (0A becomes 08)
   08aa:00ff:fe00:0009        the interface identifier

Leading zeros are dropped when the address is printed, so this identifier appears
as ``8aa:ff:fe00:9`` in every figure and log excerpt below. Recognizing this
pattern makes the addresses in this showcase readable: the ``ff:fe`` in the middle
is the inserted marker, and the last digits come straight from the MAC address.

Two hosts with different MAC addresses therefore produce different interface
identifiers. This is what makes autoconfiguration work without a server: the host
already owns a value that is supposed to be unique on the link.

The link-local address
~~~~~~~~~~~~~~~~~~~~~~

The first address a host builds is a *link-local address*. It combines the fixed
prefix ``fe80::/64`` with the interface identifier. A link-local address works
only on the local link, and routers never forward packets that carry one. Its
purpose is to let the host talk to its neighbors and to its router before it has
any routable address.

Router discovery
~~~~~~~~~~~~~~~~

To build a routable address, the host needs a prefix, and only the router knows
it. Two messages carry this information:

- A **Router Solicitation** is sent by a host that wants the prefix now. It goes
  to the all-routers multicast address ``ff02::2``, so only routers process it.
- A **Router Advertisement** is sent by a router. It goes to the all-nodes
  multicast address ``ff02::1``. Routers send Router Advertisements periodically
  on their own, and also in answer to a Router Solicitation.

A host does not have to wait for the next periodic Router Advertisement. It sends
a Router Solicitation as soon as its link-local address is ready. If nothing
answers after three attempts, four seconds apart, the host concludes that there is
no router on the link. It keeps its link-local address and can still reach
neighbors on the same link. It does not stop listening, though: a Router
Advertisement that arrives later is still processed, and the host configures itself
then.

The standard also allows a router to answer a solicitation with a Router
Advertisement addressed to the soliciting host alone. INET always sends it to the
all-nodes multicast address, which is what lets one answer serve several hosts in
the results below.

A router must not flood the link with Router Advertisements. Each advertising
interface keeps its own record of when it last sent one, and defers each solicited
Router Advertisement to at least three seconds after that, plus a small random
delay. Two consequences matter for reading the results. A router with two
interfaces runs two independent timers, so an advertisement on one link says
nothing about the timing on the other. And when several solicitations are pending
at once, each deferral is computed on its own, so two advertisements can still end
up closer together than three seconds.

The Prefix Information option
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Router Advertisement carries a *Prefix Information option* for each prefix on
the link. The two fields this showcase depends on are the prefix itself, which is
64 bits long for autoconfiguration, and the *autonomous* flag, which tells hosts
they may build an address from this prefix. The option also carries an *on-link*
flag, and two different lifetimes: the *preferred lifetime*, after which the
address should no longer be used to start new communication, and the longer *valid
lifetime*, after which the address stops existing. This showcase does not run long
enough for either to matter.

A host that receives a Prefix Information option with the autonomous flag set
combines the prefix with its own interface identifier. The result is its global
address.

Duplicate Address Detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A newly built address is *tentative*. The host must not send ordinary traffic from
a tentative address. First it runs Duplicate Address Detection (DAD).

The host sends a **Neighbor Solicitation** that names the tentative address as its
target. The source address of this message is the unspecified address ``::``,
because the host has no address it is allowed to use yet. Two answers mean
failure:

- another host answers with a **Neighbor Advertisement**, which means it already
  holds the address;
- another host sends a Neighbor Solicitation for the same target, which means it
  is testing the same address at the same time.

Either way, the address is refused and is never assigned. Failure is therefore
immediate: it takes one round trip on the link. Success, by contrast, can only be
concluded from silence, so the host has to wait out a timer before the address
becomes usable.

The solicited-node multicast address
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Neighbor Solicitation is not broadcast. It is sent to the *solicited-node
multicast address* of the target: the prefix ``ff02::1:ff00:0/104`` with the low 24
bits of the target address filling the remainder. A host testing
``fe80::8aa:ff:fe00:9`` therefore sends to ``ff02::1:ff00:9``. On Ethernet this maps
to the multicast MAC address ``33-33-FF-00-00-09``, so network cards whose address
does not match discard the frame in hardware. IPv4 Address Resolution Protocol
(ARP) instead broadcasts, and every node has to inspect the packet.

The saving is at the receiving node, not on the wire. An Ethernet switch that does
not track multicast membership still forwards the frame to every port, as the
switch in this showcase does. What changes is that only the intended target spends
any effort on it.

The order of events
~~~~~~~~~~~~~~~~~~~

Putting the pieces together, a host that joins a link performs these steps:

1. Build the link-local address from the interface identifier.
2. Run Duplicate Address Detection (DAD) on the link-local address.
3. Send a Router Solicitation.
4. Receive a Router Advertisement and read its Prefix Information option.
5. Build the global address from the prefix and the interface identifier.
6. Run Duplicate Address Detection (DAD) on the global address.

Each address is therefore checked separately, and a host runs Duplicate Address
Detection twice before it is fully configured.

Two points about this ordering are specific to INET rather than required by the
standard. INET waits for step 2 to finish before starting step 3, while RFC 4861
allows a host to send its Router Solicitation earlier, from the unspecified source
address, without waiting for the check to pass. And while a host is still in step 2
it ignores any Router Advertisement that arrives. Both have a visible effect in the
results below: a host that is still checking its link-local address when an
advertisement passes by does not use it, and has to ask for one of its own
afterwards.

IPv6 Address Autoconfiguration in INET
--------------------------------------

Node types
~~~~~~~~~~

The showcase uses three node types. :ned:`StandardHost6` is a host with an IPv6
network layer and IPv4 disabled. :ned:`Router6` is the same arrangement for a
router; it forwards packets and sends Router Advertisements.
:ned:`EthernetSwitch` is a layer-2 switch, so all nodes attached to it share one
link and one prefix.

Leaving the addresses to the hosts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, INET's :ned:`Ipv6NetworkConfigurator` assigns a global address to
every interface in the network before the simulation starts. That would leave
Stateless Address Autoconfiguration (SLAAC) with nothing to do.

Setting :par:`assignAddressesToHosts` to ``false`` changes this. The configurator
then addresses only router interfaces, and sets up the prefixes those interfaces
advertise. Host interfaces are left without addresses, so every host address seen
in this showcase is one the host worked out for itself. The router is the
exception: its global addresses come from the configurator, and only its link-local
addresses are built and checked by the router itself.

The parameter withholds *addresses* from hosts, not routes. The configurator still
installs an on-link route for the link's prefix and a default route via the router
in every host before the simulation starts. Neighbor Discovery would install a
default router entry anyway once the first Router Advertisement arrives, so nothing
here depends on the pre-installed routes — but a model built on this pattern should
not assume the hosts learned their routing table from the protocol.

The prefixes are chosen in an XML configuration:

.. literalinclude:: ../configurator.xml
   :language: xml

Each link gets its own ``/64`` prefix, which Stateless Address Autoconfiguration
(SLAAC) requires. The ``among`` attribute names the nodes that share a link. The
switch is not named, because it works at layer 2 and has no IPv6 address of its
own. The same XML configuration can override the advertised lifetimes, the Router
Advertisement intervals, the Prefix Information flags and the number of Duplicate
Address Detection probes per interface. This showcase keeps the defaults: one
Duplicate Address Detection probe per address, a valid lifetime of 30 days, a
preferred lifetime of 7 days, and both Prefix Information flags set.

Neighbor Discovery parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :ned:`Ipv6NeighbourDiscovery` module inside each node's network layer
implements router discovery, Stateless Address Autoconfiguration (SLAAC) and
Duplicate Address Detection (DAD). The parameters that matter for reading the
results are:

- :par:`dupAddrDetectTransmits` — how many Neighbor Solicitations to send for
  Duplicate Address Detection (DAD), one by default. Setting it to ``0`` skips the
  probing: the address is accepted at once, and is counted as a completed check.
- :par:`retransTimer` — the wait before the address is accepted, one second by
  default. A random delay of up to one more second is added, to account for the
  time a node needs to join the solicited-node multicast group. A successful check
  therefore takes between one and two seconds.
- :par:`minIntervalBetweenRAs` and :par:`maxIntervalBetweenRAs` — how often a
  router sends unsolicited Router Advertisements, 200 and 600 seconds by default.
  These defaults are far longer than the simulations here, so every Router
  Advertisement seen below was triggered by a Router Solicitation.
- :par:`hostBootupTime` and :par:`routerBootupTime` — when a node assigns its
  link-local address. Both are random: ``uniform(0.4s, 1s)`` for hosts and
  ``uniform(0s, 0.3s)`` for routers, so a router starts before the hosts it serves.
  The randomness is why identically configured hosts do not start together in the
  results below.

The module records three statistics, written to the ``.sca`` result file:
``startDad`` counts the Duplicate Address Detection runs a node begins,
``dadCompleted`` counts those that succeed, and ``dadFailed`` counts those that
find a duplicate.

Seeing the addresses appear
~~~~~~~~~~~~~~~~~~~~~~~~~~~

:ned:`InterfaceTableVisualizer` writes an interface's IPv6 address next to its
node on the canvas. The ``format = "%a"`` setting selects one address per
interface: the *preferred* one, which is the address the node would use to reach
an off-link destination. A host therefore shows its link-local address first, and
that label is then **replaced** by the global address once the host has one. The
figures in this page show the end state, so the link-local addresses are no longer
visible in them; the video shows the change happening.

What INET does not model
~~~~~~~~~~~~~~~~~~~~~~~~

Three limitations bound what this showcase can claim:

- There is no DHCPv6 implementation. A Router Advertisement can carry a *Managed*
  flag, which tells hosts to obtain addresses from a DHCPv6 server instead, but
  since no such server exists here the flag has no effect. Stateless Address
  Autoconfiguration (SLAAC) is the only way a host obtains an address.
- Temporary privacy addresses are not implemented. Every address is derived from
  the MAC address, so addresses are stable and predictable. That is what makes the
  duplicate-address simulation below possible; real hosts often use randomized
  identifiers instead, precisely so that their addresses cannot be traced back to
  their hardware.
- The lifetimes of an address that is already configured are not refreshed by
  later Router Advertisements. Lifetime expiry is therefore not demonstrated, and
  both simulations are kept well short of any lifetime.

The Model
---------

All simulations use the following network:

.. figure:: media/network.png
   :align: center

Four hosts and a router are attached to a switch, so they share one link and one
prefix. A server is attached to the router over a separate link, which therefore
has a different prefix. The ``configurator`` sets up the router's addresses and
advertised prefixes, the ``visualizer`` displays interface addresses, and the
``scenarioManager`` starts a node during the second simulation.

Every host address in the figure was built by the host itself. The four hosts
share the prefix ``2001:db8:1:1::/64``, while the server, being on the other link,
uses ``2001:db8:1:2::/64`` — which is the only visible sign that the second prefix
in the XML configuration is real. The router's two global addresses come from the
configurator, as described above.

Each interface in the figure carries two addresses, a link-local one and a global
one, but the label shows only the preferred address, which is the global one.

The general configuration is:

.. literalinclude:: ../omnetpp.ini
   :start-at: [General]
   :end-before: [Config Autoconfiguration]
   :language: ini

The first setting points each node's own IPv6 configurator submodule at the
network-level :ned:`Ipv6NetworkConfigurator`, so that the two agree on which
addresses and prefixes to use.

Autoconfiguration Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config Autoconfiguration]
   :end-before: [Config DuplicateAddress]
   :language: ini

There is nothing else to configure, and that is the point of the simulation. No
host is given an address, and the addresses are the result.

DuplicateAddress Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../omnetpp.ini
   :start-at: [Config DuplicateAddress]
   :language: ini

This configuration gives ``host[0]`` and ``host[3]`` the same MAC address. Both
therefore derive the same interface identifier, and both would build the same
link-local address ``fe80::8aa:ff:fe00:10``.

Setting :par:`hasStatus` to ``true`` gives every node a status submodule, which is
what allows a node to be started and stopped during the run. ``host[3]`` begins in
the ``DOWN`` state, and the ``scenarioManager`` starts it after 15 seconds:

.. literalinclude:: ../clone.xml
   :language: xml

The delay separates the two nodes in time. By the time ``host[3]`` joins,
``host[0]`` has held its addresses for more than ten seconds. This is the situation
an operator meets after cloning a virtual machine without changing its hardware
address, and it exercises the first of the two failure conditions described
earlier: an established owner answers with a Neighbor Advertisement. The second
condition, two hosts testing the same address at the same moment, would need them
to start together; it is not shown here.

Because assigning explicit MAC addresses to ``host[0]`` and ``host[3]`` also
shifts the addresses INET generates automatically for the other nodes, the host
addresses in this simulation differ from those in the first one. Only the
identifiers change; the mechanism does not.

Results
-------

Autoconfiguration
~~~~~~~~~~~~~~~~~

The following video covers the first eight seconds of the ``Autoconfiguration``
simulation. It starts just after the six nodes have assigned their link-local
addresses, so each host is labelled with an ``fe80::`` address at the beginning.
Watch each label change as the host obtains its global address: ``host[0]`` and
``host[1]`` change first, then the server, then ``host[2]`` and ``host[3]``.

The label changes the moment the global address is *assigned*, which is before
Duplicate Address Detection (DAD) has confirmed it. A tentative global address is
already the preferred one, because a link-local address could not be used to reach
an off-link destination anyway. So the label showing a global address does not yet
mean the host may use it; the log excerpts below give the times at which each
address was actually accepted.

.. video:: media/autoconfiguration.mp4
   :width: 100%
   :align: center

The log shows the order described earlier. Link-local addresses are built first,
at times drawn from :par:`hostBootupTime` and :par:`routerBootupTime`:

.. code-block:: none

   0.289098  router:  Assigning Link Local Address
   0.563593  host[0]: Assigning Link Local Address
   0.578520  server:  Assigning Link Local Address
   0.630064  host[1]: Assigning Link Local Address
   0.686599  host[2]: Assigning Link Local Address
   0.875035  host[3]: Assigning Link Local Address

Each of them is then checked by Duplicate Address Detection (DAD), which takes
between one and two seconds. Once a host's link-local address is accepted, the
host starts router discovery, and the router answers:

.. code-block:: none

   2.022849  host[1]: DAD completed for address fe80::8aa:ff:fe00:a, address is unique
   2.043570  host[0]: DAD completed for address fe80::8aa:ff:fe00:9, address is unique
   2.093885  host[1]: Initiating Router Discovery
   2.137463  router:  Create and send RA invoked!
   2.137484  host[0]: Assigning new address to: eth0
   2.137484  host[1]: Assigning new address to: eth0

One detail here is worth noting. ``host[0]`` obtains the prefix from the Router
Advertisement that ``host[1]`` asked for, because that Router Advertisement is sent
to the all-nodes multicast address. A single answer serves every host that is
ready to use it.

``host[2]`` and ``host[3]`` were also on the link at 2.137 s, and their network
cards did receive that same Router Advertisement. They did not use it, because
neither had finished checking its own link-local address yet — that happened at
2.61 s and 2.71 s. As described above, a host in the middle of Duplicate Address
Detection ignores Router Advertisements. Each therefore had to send a Router
Solicitation of its own once its check finished:

.. code-block:: none

   2.612195  host[2]: DAD completed for address fe80::8aa:ff:fe00:b, address is unique
   2.711113  host[3]: DAD completed for address fe80::8aa:ff:fe00:c, address is unique
   2.851464  host[3]: Initiating Router Discovery
   3.390352  host[2]: Initiating Router Discovery

The answer to those solicitations did not come immediately. The router had already
sent a Router Advertisement to the all-nodes multicast address on this interface at
2.137 s, so it deferred each answer to at least three seconds after that, plus a
random delay. For ``host[2]``'s solicitation the delay came out at 0.237 s, giving
5.374 s:

.. code-block:: none

   5.374267  router:  Create and send RA invoked!
   5.374288  host[2]: Assigning new address to: eth0
   5.374288  host[3]: Assigning new address to: eth0
   7.053168  host[2]: DAD completed for address 2001:db8:1:1:8aa:ff:fe00:b, address is unique
   7.094921  host[3]: DAD completed for address 2001:db8:1:1:8aa:ff:fe00:c, address is unique

Because this advertisement goes to the all-nodes multicast address, it serves both
hosts, exactly as the 2.137 s one served ``host[0]`` and ``host[1]``. A further
advertisement follows at 5.572 s, deferred from ``host[3]``'s earlier solicitation;
by then both hosts are already configured, so it changes nothing. This is the case
mentioned earlier, where deferrals computed separately for two pending
solicitations end up closer together than three seconds.

So two separate mechanisms delay these two hosts: first their own Duplicate Address
Detection, which stops them using an advertisement that was already on the link,
and then the router's rate limit, which delays the advertisement they asked for. All
four hosts and the server hold a global address by 7.1 s.

The server is served by a Router Advertisement on the other interface, at 3.54 s.
That interface has its own timer, so it is not affected by the 2.137 s
advertisement on the host link. This is why the server obtains
``2001:db8:1:2:8aa:ff:fe00:1``, from the second prefix, while the hosts obtain
addresses from the first.

The statistics in the ``.sca`` result file confirm that every node ran Duplicate
Address Detection (DAD) exactly twice, once for its link-local address and once
for its global address, and that no address was refused:

.. list-table::
   :header-rows: 1
   :widths: 30 25 25 20

   * - Node
     - ``startDad``
     - ``dadCompleted``
     - ``dadFailed``
   * - ``host[0]`` … ``host[3]``
     - 2
     - 2
     - 0
   * - ``router``
     - 2
     - 2
     - 0
   * - ``server``
     - 2
     - 2
     - 0

The router's two runs are for the link-local addresses of its two interfaces. Its
global addresses come from the configurator, so they are not checked.

Duplicate Address
~~~~~~~~~~~~~~~~~

In the second simulation, ``host[3]`` starts after 15 seconds with the same MAC
address as ``host[0]``. It builds the same link-local address, and Duplicate
Address Detection (DAD) refuses it about 30 microseconds later:

.. code-block:: none

   15.712286  host[3]: Assigning Link Local Address
   15.712300  host[0]: Address is duplicate! Inform Sender of duplicate address!
   15.712316  host[3]: Received NA for tentative address fe80::8aa:ff:fe00:10 - Loss of DAD
   15.712316  host[3]: DAD failed for address fe80::8aa:ff:fe00:10 on eth0 --
              Loss of DAD, address will not be assigned

``host[0]`` already holds the address, so it answers the Neighbor Solicitation
with a Neighbor Advertisement, and ``host[3]`` gives up the address. The contrast
with the first simulation is sharp: a successful check waits out a timer for one
to two seconds, while a failed one is settled in the time it takes a frame to
cross the link and come back.

The consequence is visible on the canvas at the end of the simulation:

.. figure:: media/duplicate_address.png
   :align: center

``host[3]`` shows ``<unspec>``: it has no address at all. The failure stops it at
the first step, so it never reaches router discovery and never builds a global
address either. A host in this state cannot communicate.

The statistics show the same result, and show that no other node is affected:

.. list-table::
   :header-rows: 1
   :widths: 30 25 25 20

   * - Node
     - ``startDad``
     - ``dadCompleted``
     - ``dadFailed``
   * - ``host[0]``, ``host[1]``, ``host[2]``
     - 2
     - 2
     - 0
   * - ``host[3]``
     - 1
     - 0
     - 1
   * - ``router``, ``server``
     - 2
     - 2
     - 0

``host[3]`` begins one Duplicate Address Detection run and completes none. This is
the outcome the mechanism exists to produce: a collision is caught before the
address is used, rather than after both hosts are broken.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Ipv6AutoconfigurationShowcase.ned <../Ipv6AutoconfigurationShowcase.ned>`,
:download:`configurator.xml <../configurator.xml>`,
:download:`clone.xml <../clone.xml>`

Try It Yourself
---------------

The showcase contains two configurations, ``Autoconfiguration`` and
``DuplicateAddress``. Both write their Duplicate Address Detection statistics to
``results/``, where the OMNeT++ Analysis tool or ``opp_scavetool`` can read them.

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/general/ipv6autoconfiguration`` folder in the `Project Explorer`.
There, you can view and edit the showcase files, run simulations, and analyze
results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.7 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.7.*/showcases/general/ipv6autoconfiguration && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.7
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

.. TODO: replace the issue-tracker link below once the showcase discussion issue is opened

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues>`__ in
the GitHub issue tracker for commenting on this showcase.
