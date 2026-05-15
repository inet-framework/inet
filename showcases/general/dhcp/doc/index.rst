Dynamic Host Configuration Protocol (DHCP)
===========================================

Goals
-----

The Dynamic Host Configuration Protocol (DHCP) allows hosts on an IP network
to obtain their IP address, subnet mask, default gateway, and other
configuration parameters automatically from a DHCP server, without manual
configuration. DHCP is defined in :rfc:`2131`.

This showcase demonstrates DHCP address assignment in INET using the
:ned:`DhcpServer` and :ned:`DhcpClient` application modules. It includes
three configurations:

- **BasicDHCP**: A standard DORA (Discover–Offer–Request–Acknowledge) exchange
  where three clients obtain addresses from a server.
- **LeaseRenewal**: A short lease time causes clients to renew their leases
  during the simulation.
- **ClientReboot**: A client is shut down and restarted mid-simulation,
  demonstrating DHCP re-acquisition after a reboot.

| Verified with INET version: ``4.6``
| Source files location: `inet/showcases/general/dhcp <https://github.com/inet-framework/inet/tree/master/showcases/general/dhcp>`__

The Model
---------

The Network
~~~~~~~~~~~

The network consists of a DHCP server (``dhcpServer``), an Ethernet switch
(``switch``), and three DHCP clients (``client[0..2]``), all connected via
100 Mbps Ethernet links. An :ned:`Ipv4NetworkConfigurator` assigns a static
IP address (192.168.1.1/24) only to the server; client interfaces are left
unconfigured so they can obtain their addresses via DHCP. An
:ned:`IntegratedCanvasVisualizer` displays the acquired addresses on the
canvas.

.. figure:: media/network.png
   :width: 80%
   :align: center

.. literalinclude:: ../DhcpShowcase.ned
   :language: ned

DHCP Operation
~~~~~~~~~~~~~~

DHCP uses a four-message exchange known as DORA:

1. **Discover** — The client broadcasts a DHCPDISCOVER message to find
   available servers.
2. **Offer** — The server responds with a DHCPOFFER containing an available
   IP address and lease parameters.
3. **Request** — The client broadcasts a DHCPREQUEST indicating which offer
   it accepts.
4. **Acknowledge** — The server confirms the lease with a DHCPACK.

After receiving the DHCPACK, the client enters the **BOUND** state and
configures its interface with the leased address. The :ned:`DhcpClient`
module implements the full DHCP client state machine
(INIT → SELECTING → REQUESTING → BOUND → RENEWING → REBINDING) as
defined in :rfc:`2131`.

Each lease has a lifetime. At time T1 (by default, half the lease time),
the client transitions from BOUND to **RENEWING** and sends a unicast
DHCPREQUEST to the server to extend the lease. If that fails, at time T2
(by default, 87.5% of the lease time), the client enters **REBINDING** and
broadcasts the request.

Configuration
~~~~~~~~~~~~~

The ``[General]`` section sets up Ethernet bitrate, enables the interface
table visualizer, configures computed checksums for PCAP recording, and
attaches a :ned:`PcapRecorder` to the server:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :end-before: [Config BasicDHCP]

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
addresses in the 192.168.1.11–192.168.1.60 range (since the first 10
addresses are reserved). The interface table visualizer displays the
acquired address and prefix length next to each host.

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
at t=30s and restart it at t=60s. When the client comes back up, it
performs a new DORA exchange to obtain an address. The lease time is set
to 120 seconds, and ``**.hasStatus = true`` enables lifecycle management
on all hosts.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config ClientReboot]

The scenario script:

.. literalinclude:: ../scenario.xml
   :language: xml

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
At t=60s, the client restarts and performs a new DORA exchange. The other
two clients remain unaffected throughout.

.. figure:: media/client_reboot.png
   :width: 100%
   :align: center

   Client shutdown and restart with DHCP re-acquisition.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`DhcpShowcase.ned <../DhcpShowcase.ned>`, :download:`scenario.xml <../scenario.xml>`

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
