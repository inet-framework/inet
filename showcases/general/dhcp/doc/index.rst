Dynamic Host Configuration Protocol (DHCP)
===========================================

Goals
-----

Before a host can communicate over IP, it needs an IP address, subnet mask,
and default gateway. Assigning these parameters manually on every host does
not scale well: administrators must track which addresses are already in use
to avoid conflicts, update configurations when hosts move between subnets,
and reclaim addresses when hosts are decommissioned. In networks where hosts
join and leave frequently, this becomes impractical.

The Dynamic Host Configuration Protocol (DHCP), defined in :rfc:`2131`,
automates this process. The server manages a pool of addresses
and leases them to clients for a limited time, reclaiming them when the
lease expires.

This showcase demonstrates how to set up DHCP-based address assignment in
INET using the :ned:`DhcpServer` and :ned:`DhcpClient` application modules.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/general/dhcp <https://github.com/inet-framework/inet/tree/master/showcases/general/dhcp>`__

About DHCP
----------

DHCP is an application-layer protocol that operates over UDP (server on
port 67, client on port 68). Because a client has no IP address when it
first joins the network, DHCP messages are sent as link-layer broadcasts.
This means DHCP operates within a single LAN segment — a router does not
forward DHCP broadcasts unless a DHCP relay agent is configured.

The DHCP server is configured with a pool of IP addresses that it can
hand out. It derives the available range from its own interface address and
subnet mask, and maintains a table of which addresses are currently in use
and by which client (identified by MAC address).

A DHCP address assignment is called a *lease* — the server does not give
the address to the client permanently, but grants it for a limited duration
(the *lease time*). In a real network this ensures that addresses are eventually returned to
the pool if a client leaves without explicitly releasing them.

The initial address acquisition uses a four-message exchange known as
**DORA**:

1. **Discover** — The client broadcasts a DHCPDISCOVER message to locate
   available servers.
2. **Offer** — The server responds with a DHCPOFFER containing an available
   IP address, the subnet mask, the default gateway, and the lease duration.
3. **Request** — The client broadcasts a DHCPREQUEST indicating which offer
   it accepts. (Broadcasting rather than unicasting informs any other servers
   that their offers were not chosen.)
4. **Acknowledge** — The server confirms the lease with a DHCPACK.

After receiving the DHCPACK, the client enters the **BOUND** state and
configures its interface with the leased address.

Before the lease expires, the client must extend it. This happens in two
stages, providing a fallback in case the original server becomes
unreachable:

- **Renewing** (at time T1, typically half the lease time) — The client
  sends a unicast DHCPREQUEST directly to the server that granted the
  lease. Under normal conditions, the server replies with a DHCPACK and
  the lease is extended. This is the common case.

- **Rebinding** (at time T2, typically 87.5% of the lease time) — If the
  client received no response during the renewing phase (e.g. the original
  server is down), it falls back to *broadcasting* a DHCPREQUEST so that
  any available DHCP server on the LAN can respond and extend the lease.

If neither succeeds and the lease expires, the client loses its address and
must start over with a new Discover.

The full client state machine maps directly onto these steps: the client
starts in **INIT**, sends the Discover and enters **SELECTING** while
waiting for offers, transitions to **REQUESTING** after choosing an offer
and sending the Request (the client accepts the first offer it receives),
and enters **BOUND** once the Acknowledge is received. From there,
renewal timers drive the **RENEWING** and
**REBINDING** states. A separate path exists for rebooting clients: a
client that still holds a lease from before a restart enters
**INIT-REBOOT**, broadcasts a DHCPREQUEST for its old address, and moves
to **REBOOTING** while waiting for the server's response.

| INIT → SELECTING → REQUESTING → BOUND → RENEWING → REBINDING
| INIT-REBOOT → REBOOTING → BOUND

This showcase includes five configurations that illustrate different
aspects of the protocol:

- **BasicDHCP** — The standard four-message DORA exchange where clients
  obtain addresses from a server.
- **LeaseRenewal** — A short lease time triggers the renewal mechanism
  during the simulation.
- **ClientReboot** — A client is shut down and restarted mid-simulation,
  demonstrating DHCP re-acquisition after a reboot.
- **ServerReboot** — The DHCP server is shut down and restarted, losing
  its lease database. When clients attempt to renew, the server rejects
  the request and clients must re-acquire addresses.
- **Roaming** — A mobile wireless client moves between two access points,
  each served by a separate DHCP server on a different subnet.

The Model
---------

DHCP in INET
~~~~~~~~~~~~

INET provides two application modules for DHCP:

- :ned:`DhcpServer` — Listens on a network interface, manages the address
  pool, and responds to client requests. Key parameters:
  :par:`interface` (interface to serve DHCP on; if omitted, uses the only
  non-loopback interface),
  :par:`numReservedAddresses` (number of addresses to skip counting from
  the network address; with a server on 192.168.1.0/24 and
  ``numReservedAddresses=10``, the pool starts at 192.168.1.10),
  :par:`maxNumClients` (maximum number of concurrent leases),
  :par:`gateway` (default gateway announced to clients; if left empty,
  defaults to the server's own interface address), and
  :par:`leaseTime`.

  .. note::

     The INET DHCP server does not expire leases on its own — once an
     address is leased, it remains marked as in-use until the server is
     restarted. Lease expiration relies on the client performing timely
     renewals. Additionally, the client does not send a DHCPRELEASE
     message on shutdown; addresses are reclaimed only when the server
     restarts.

- :ned:`DhcpClient` — Runs the DHCP client state machine on a host
  interface. Its main parameter is :par:`startTime`, which controls when
  the client begins the DORA exchange.

The T1 and T2 renewal timers are not client-side defaults — the server
sets them explicitly in every DHCPACK message (T1 = 0.5 × lease time,
T2 = 0.875 × lease time).

Both modules are added to a :ned:`StandardHost` as applications
(``app[*]``). The server needs a statically configured IP address on its
interface (assigned by :ned:`Ipv4NetworkConfigurator`); client interfaces
are left unconfigured so they receive their addresses via DHCP.

The Network
~~~~~~~~~~~

The network consists of a DHCP server (``dhcpServer``), an Ethernet switch
(``switch``), and three DHCP clients (``client[0..2]``), all connected via
100 Mbps Ethernet links. An :ned:`IntegratedCanvasVisualizer` displays the
acquired addresses on the canvas.

.. figure:: media/network.png
   :width: 80%
   :align: center

The :ned:`Ipv4NetworkConfigurator` assigns a static IP address only to the
server — client interfaces are left unconfigured so they obtain addresses
via DHCP:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [General]
   :end-at: netmask

BasicDHCP
~~~~~~~~~

This configuration sets up a straightforward DHCP scenario. The server is
configured with :par:`numReservedAddresses` = 10, :par:`maxNumClients` = 50,
:par:`gateway` = "192.168.1.1", and :par:`leaseTime` = 3600s. Each client
starts its DHCP process at a random time within the first 2 seconds.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config BasicDHCP]
   :end-before: [Config LeaseRenewal]

After running this configuration, all three clients should obtain IP
addresses starting from 192.168.1.10: ``numReservedAddresses=10`` skips
the first 10 addresses from the network address (.0–.9), so the pool
starts at .10 and spans 50 addresses (.10–.59) as limited by
``maxNumClients=50``. The server's own address (.1) falls within the
reserved range. The interface table visualizer displays the acquired
address and prefix length next to each host.

LeaseRenewal
~~~~~~~~~~~~

This configuration extends ``BasicDHCP`` and reduces the lease time to 60
seconds. Since the T1 timer fires at half the lease time (30s), clients
will attempt to renew their leases during the 200-second simulation. This
allows observation of the BOUND → RENEWING transition and the subsequent
unicast DHCPREQUEST/DHCPACK renewal exchange.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config LeaseRenewal]
   :end-before: [Config ClientReboot]

ClientReboot
~~~~~~~~~~~~

This configuration demonstrates how a DHCP client re-acquires its address
after a reboot. It uses a :ned:`ScenarioManager` to shut down ``client[0]``
at t=30s and restart it at t=60s. When the client comes back up, it still
holds its lease object in memory (the application module's internal state
survives the stop/start lifecycle) and enters the INIT-REBOOT state:
instead of a full DORA exchange, it broadcasts a DHCPREQUEST for its
previously held address and the server responds with a DHCPACK (or a
DHCPNAK if the address is no longer valid). The lease time is set to 120
seconds.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ClientReboot]
   :end-before: [Config ServerReboot]

The scenario script:

.. literalinclude:: ../scenario.xml
   :language: xml

ServerReboot
~~~~~~~~~~~~

This configuration demonstrates what happens when the DHCP server reboots
and loses its lease database. The server is shut down at t=30s and
restarted at t=40s via :ned:`ScenarioManager`. When the server comes back
up, it has no record of previously granted leases.

The lease time is set to 100 seconds, so the T1 renewal timer fires at
t≈50s. When a client sends a unicast DHCPREQUEST to renew its lease, the
server does not recognize it and responds with a DHCPNAK. The client then
falls back to the INIT state and performs a full DORA exchange to obtain
a new address.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ServerReboot]
   :end-before: [Config Roaming]

The scenario script:

.. literalinclude:: ../scenario_server_reboot.xml
   :language: xml

Roaming
~~~~~~~

This configuration uses a different network (:ned:`DhcpRoaming`) to
demonstrate DHCP in a wireless roaming scenario. Two access points
(``ap1``, ``ap2``) are each connected to a dedicated DHCP server on a
separate subnet (192.168.1.0/24 and 192.168.2.0/24). A wired
``server`` is reachable through both DHCP servers, which act as
gateways (``forwarding = true``). Static routes on the ``server``
ensure both subnets are reachable.

.. figure:: media/roaming_network.png
   :width: 80%
   :align: center

The wireless client uses :ned:`RectangleMobility` to move back and forth
between the two access points at 20 m/s. As it moves out of range of one access
point and into range of the other, it associates with the new AP. The
:ned:`DhcpClient` subscribes to the link-layer association signal; when a
new association is detected it unbinds the current lease and restarts the
DORA exchange, obtaining an address from the DHCP server on the new
subnet.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Roaming]

Results
-------

BasicDHCP
~~~~~~~~~

The following sequence chart shows the DORA exchange between one of the
clients and the server:

.. figure:: media/dora_sequence_chart.png
   :width: 100%
   :align: center

   DHCP DORA exchange between client and server.

The interface table visualizer displays the addresses acquired by each
client:

.. figure:: media/interface_tables.png
   :width: 80%
   :align: center

   Interface table visualization after DHCP address assignment.

LeaseRenewal
~~~~~~~~~~~~

During the simulation, the T1 timer fires at t≈30s for each client,
triggering a unicast DHCPREQUEST to the server. The server responds with
a DHCPACK, extending the lease. This renewal cycle repeats throughout
the simulation.

.. figure:: media/lease_renewal.png
   :width: 100%
   :align: center

   Lease renewal exchange visible in the sequence chart.

ClientReboot
~~~~~~~~~~~~

At t=30s, ``client[0]`` is shut down and its interface is deconfigured.
At t=60s, the client restarts. Because the client still holds its lease
internally, it enters the INIT-REBOOT state and broadcasts a DHCPREQUEST
for its previously held address — skipping the Discover and Offer steps.
The server confirms with a DHCPACK and the client receives the same IP
address as before the reboot. The other two clients remain unaffected
throughout.

.. figure:: media/client_reboot.png
   :width: 100%
   :align: center

   Client shutdown and restart with DHCP re-acquisition.

ServerReboot
~~~~~~~~~~~~

The server shuts down at t=30s and restarts at t=40s. When the clients'
T1 timers fire around t≈50s, the server no longer recognizes their
leases. The sequence chart shows the server responding with a DHCPNAK,
followed by the client performing a complete DORA exchange to obtain a
new address.

.. figure:: media/server_reboot.png
   :width: 100%
   :align: center

   Server reboot causes lease rejection and full re-acquisition.

Roaming
~~~~~~~

As the client moves from one access point's coverage area to the other,
it disassociates from the old AP and associates with the new one. The
DHCP client detects the interface change and initiates a new DORA
exchange with the DHCP server on the new subnet, obtaining an address
from a different address range.

.. figure:: media/roaming.png
   :width: 100%
   :align: center

   Client roaming between two DHCP servers on different subnets.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`DhcpShowcase.ned <../DhcpShowcase.ned>`, :download:`scenario.xml <../scenario.xml>`, :download:`scenario_server_reboot.xml <../scenario_server_reboot.xml>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/general/dhcp`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/general/dhcp && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TODO>`__ in
the GitHub issue tracker for commenting on this showcase.

.. TODO: The DhcpServer does not expose a parameter for the DNS server address
   announced to clients (the field defaults to 0.0.0.0). Consider adding a
   ``dns`` parameter and documenting it in the parameter list above.
