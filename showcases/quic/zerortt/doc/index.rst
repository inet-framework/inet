QUIC 0-RTT Connection Resumption
=================================

Goals
-----

This showcase demonstrates QUIC's 0-RTT (Zero Round-Trip Time) connection resumption
feature, one of the protocol's most significant performance advantages over TCP and
even TLS 1.3. The 0-RTT mechanism allows clients that have previously connected to
a server to resume their connection and send application data immediately in the
first flight of packets, without waiting for a handshake to complete.

Traditional connection establishment requires multiple round trips:

- **TCP**: 1 RTT for TCP handshake + 1-2 RTT for TLS handshake = 2-3 RTT total
- **TLS 1.3 (1-RTT)**: 1 RTT for combined TCP+TLS handshake
- **QUIC 0-RTT**: 0 RTT - application data sent immediately with connection request

This dramatic reduction in connection latency is particularly valuable for:

- **Short-lived connections**: Web requests, API calls, microservices communication
- **Mobile networks**: Where RTT can be 100-200ms or higher
- **Frequently accessed services**: Users returning to the same website or application

However, 0-RTT also introduces security considerations, as data sent in 0-RTT packets
lacks forward secrecy and may be vulnerable to replay attacks. This showcase explores
both the performance benefits and the security aspects of 0-RTT connections.

In this simulation, we demonstrate:

- **Successful 0-RTT resumption**: Client reconnects using a valid token from a
  previous session
- **Invalid token handling**: How QUIC gracefully falls back to 1-RTT when tokens
  are malformed or invalid
- **Security validation**: Server verification of 0-RTT tokens and rejection of
  invalid attempts
- **Performance comparison**: Latency differences between 1-RTT and 0-RTT connections

| Verified with INET version: ``4.6.0``
| Source files location: `inet/showcases/quic/zerortt <https://github.com/inet-framework/inet/tree/master/showcases/quic/zerortt>`__

The Model
---------

The Network
~~~~~~~~~~~

The network topology is simple and deliberately minimal to focus on the 0-RTT
mechanism without interference from complex routing or congestion:

.. figure:: media/network.png
   :width: 70%
   :align: center

The network consists of:

- **Client**: QUIC-enabled host with ``QuicZeroRttClient`` application
- **Server**: QUIC-enabled host with ``QuicDiscardServer`` application
- **Two routers**: Providing network separation with realistic latency
- **Link delay**: 1ms per link (2ms total RTT between client and server)

This simple topology allows us to clearly observe the timing differences between
0-RTT and 1-RTT connection establishment without confounding factors.

How 0-RTT Works in QUIC
~~~~~~~~~~~~~~~~~~~~~~~~

The 0-RTT process involves multiple stages:

**Initial Connection (1-RTT handshake)**:

1. Client sends Initial packet with ClientHello
2. Server responds with Initial packet containing ServerHello and Handshake data
3. Server sends NEW_TOKEN frame with an encrypted resumption token
4. Connection completes after 1-RTT
5. Client stores the token for future use

**Subsequent Connection (0-RTT resumption)**:

1. Client sends Initial packet with early data using stored token
2. Client immediately sends 0-RTT packets containing application data
3. Server validates token and processes early data
4. Handshake completes in parallel with data transmission
5. Connection transitions to 1-RTT security level

.. figure:: media/zerortt_sequence.png
   :width: 80%
   :align: center

   *Placeholder: Sequence diagram comparing 1-RTT vs 0-RTT connection establishment*

Configuration
~~~~~~~~~~~~~

The server must explicitly enable 0-RTT token generation:

.. code-block:: ini

   **.server.quic.sendZeroRttTokenAsServer = true

The client application is configured with ``QuicZeroRttClient``, which implements
the logic for storing tokens from initial connections and attempting 0-RTT resumption
on subsequent connections:

.. code-block:: ini

   **.client.app[0].typename = "QuicZeroRttClient"
   **.client.app[0].localPort = 1001
   **.client.app[0].connectPort = 1000
   **.client.app[0].connectAddress = "server"

For testing error conditions, the client can be configured to send invalid tokens:

.. code-block:: ini

   **.client.app[0].invalidClientTokenString = false  # Malformed token format
   **.client.app[0].sendInvalidToken = false          # Wrong token content

Scenarios
---------

Scenario 1: Successful 0-RTT Resumption
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[General]`` (default configuration)

This baseline scenario demonstrates the complete 0-RTT workflow:

1. **First connection**: Client connects to server using standard 1-RTT handshake
2. **Token exchange**: Server sends NEW_TOKEN frame to client
3. **Connection close**: First connection terminates
4. **Resumption**: Client reconnects using stored token with 0-RTT early data

**Expected behavior**: The second connection should complete with application data
being sent before the handshake finishes, saving one full round trip.

.. figure:: media/zerortt_timing_success.png
   :width: 100%
   :align: center

   *Placeholder: Timeline showing first connection (1-RTT) vs second connection (0-RTT)*

The chart should clearly show that in the 0-RTT connection, application data
transmission begins immediately, while in the initial 1-RTT connection, there's
a delay waiting for handshake completion.

Connection Establishment Latency
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. figure:: media/connection_latency_comparison.png
   :width: 100%
   :align: center

   *Placeholder: Bar chart comparing connection establishment time: 1-RTT (~2ms) vs 0-RTT (~0ms)*

With a 2ms round-trip time in our network, the 0-RTT connection eliminates one
complete RTT, providing a 50% reduction in connection establishment latency compared
to standard 1-RTT QUIC.

Application Data Arrival Time
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. figure:: media/data_arrival_time.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing when first application data arrives at server:
   1-RTT connection vs 0-RTT connection*

The most critical metric for user experience is when the first application data
arrives at the server. With 0-RTT, this happens essentially at network speed,
while 1-RTT requires waiting for the handshake round trip.

Scenario 2: Invalid Token String Format
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config invalidClientTokenString]``

This scenario tests QUIC's robustness when the client attempts to use a malformed
token (e.g., corrupted during storage or transmission):

.. code-block:: ini

   [invalidClientTokenString]
   **.client.app[0].invalidClientTokenString = true

**Expected behavior**: The server should reject the malformed token and fall back
to a standard 1-RTT handshake. The connection should still succeed, but without
the 0-RTT benefit.

.. figure:: media/zerortt_invalid_format.png
   :width: 100%
   :align: center

   *Placeholder: Sequence diagram showing token rejection and fallback to 1-RTT*

This demonstrates QUIC's graceful degradation—connection failures due to invalid
tokens don't break connectivity, they simply revert to the standard handshake.

Scenario 3: Invalid Token Content
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configuration: ``[Config sendInvalidToken]``

This scenario tests what happens when the client sends a token with valid format
but incorrect content (e.g., expired token, token for different server, or replay
attack attempt):

.. code-block:: ini

   [sendInvalidToken]
   **.client.app[0].sendInvalidToken = true

**Expected behavior**: The server's cryptographic verification of the token should
fail, and the connection should fall back to 1-RTT. This protects against various
attacks while maintaining connectivity.

.. figure:: media/zerortt_invalid_content.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing token validation failure and connection recovery*

Results
-------

Performance Comparison
~~~~~~~~~~~~~~~~~~~~~~

The following table summarizes the connection establishment performance across
different scenarios:

.. figure:: media/performance_table.png
   :width: 80%
   :align: center

   *Placeholder: Table showing connection metrics for each scenario*

Expected values (with 2ms RTT):

- **Initial 1-RTT connection**: ~2-3ms to send first application data
- **0-RTT resumption**: ~0-1ms to send first application data (immediate)
- **Failed 0-RTT (fallback)**: ~2-3ms (same as initial connection)

Token Exchange Analysis
~~~~~~~~~~~~~~~~~~~~~~~

.. figure:: media/token_exchange.png
   :width: 100%
   :align: center

   *Placeholder: Packet capture view showing NEW_TOKEN frame in first connection
   and token usage in resumption*

The NEW_TOKEN frame sent by the server contains:

- Encrypted session resumption information
- Validation data for server-side verification
- Expiration information

Token Validation Success Rate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Across multiple simulation runs with different token configurations:

.. figure:: media/token_validation_rate.png
   :width: 100%
   :align: center

   *Placeholder: Bar chart showing validation success rate for valid tokens (~100%)
   vs invalid tokens (0%)*

Packet Count Comparison
~~~~~~~~~~~~~~~~~~~~~~~

.. figure:: media/packet_count.png
   :width: 100%
   :align: center

   *Placeholder: Comparison of packet count during connection establishment:
   1-RTT vs 0-RTT*

0-RTT doesn't necessarily reduce the total number of packets, but it changes their
timing—application data packets are sent earlier, overlapping with the handshake
rather than waiting for it to complete.

Security Considerations
~~~~~~~~~~~~~~~~~~~~~~~

The showcase also highlights important security considerations for 0-RTT:

.. figure:: media/security_tradeoffs.png
   :width: 100%
   :align: center

   *Placeholder: Diagram illustrating security properties of 0-RTT data vs 1-RTT data*

**0-RTT security properties**:

- ✓ Authentication: Verified through token
- ✓ Encryption: Data is encrypted
- ✗ Forward secrecy: Not yet established
- ✗ Replay protection: Must be handled at application layer

**Best practices for 0-RTT usage**:

- Only use for idempotent operations (GET requests, not POST)
- Implement application-layer replay protection for sensitive operations
- Consider the security/performance tradeoff for your use case

Real-World Impact
~~~~~~~~~~~~~~~~~

To contextualize the results, consider the impact with realistic network latencies:

.. figure:: media/realworld_impact.png
   :width: 100%
   :align: center

   *Placeholder: Chart showing latency savings across different RTT scenarios
   (LAN: 2ms, WiFi: 20ms, Mobile: 100ms, Satellite: 500ms)*

With mobile network latencies of 100-200ms, 0-RTT can save 100-200ms of perceived
loading time on every connection resumption—a significant UX improvement.

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`zerortt.ned <../zerortt.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/quic/zerortt`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/quic/zerortt && inet'

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

Use `this page <https://github.com/inet-framework/inet-showcases/issues/TBD>`__ in
the GitHub issue tracker for commenting on this showcase.
