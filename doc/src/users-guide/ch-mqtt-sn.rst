MQTT-SN sensor messaging
========================

MQTT-SN v1.2 applications exchange compact messages over UDP and bridge them to an
MQTT 3.1.1 broker. Place ``MqttSnClient`` and ``MqttSnGateway`` in ordinary host
application slots. Configure the sensor's ``gatewayAddress`` and the gateway's
``brokerAddress``. The default UDP port is 1884; the broker TCP port is 1883.

The gateway uses a separate MQTT connection for each connected sensor and a shared
connection for connectionless publications and Wills. Use distinct sensor ClientIds
and a unique ``gatewayClientIdentifier`` for each gateway. The supplied MQTT broker
accepts 1–23 ASCII alphanumeric bytes for these identifiers. The sensor's identity is
preserved upstream, so another MQTT client using it can take over its connection.

Topics and workloads
--------------------

The sensor registers a normal topic before publishing using its two-octet id.
Wildcard subscriptions can require a gateway-originated registration before the
first concrete downlink message. Predefined ids require matching configuration at
both ends. ``topicKind=2`` denotes a two-octet short topic name; ``topicKind=1``
selects a predefined ``topicId``. Their namespaces are distinct from normal ids.

The XML workload uses nondecreasing ``at`` offsets from the first accepted CONNACK.
In connectionless mode its origin is application start. Binary payloads are hex:

.. code-block:: xml

   <mqttsn>
     <subscribe at="0s" filter="commands/+" qos="1"/>
     <publish at="1s" topic="sensors/value" qos="1" payload="00ff41"/>
     <sleep at="2s" duration="5s"/>
     <poll at="4s"/>
   </mqttsn>

Supported actions include ``subscribe``, ``unsubscribe``, ``publish``, ``sleep``,
``poll``, ``willTopic`` and ``willMessage``. The latter two independently replace
the stored Will topic/flags or binary payload. An empty Will topic deletes the Will.
Ordinary stop/start resumes the workload cursor; crash/start restarts the workload
and first uses a clean session to discard incompatible remote state.

Delivery and sleeping
---------------------

QoS 0 sends without acknowledgement. QoS 1 retransmits and can deliver duplicates.
QoS 2 tracks PUBLISH/PUBREC/PUBREL/PUBCOMP so a repeat does not cause another payload
handoff inside the retained session. The gateway terminates each hop independently;
gateway acceptance into its queue is not final subscriber delivery. QoS -1 permits
connectionless publishing with predefined or short topics and forwards at MQTT QoS 0.

``retryInterval`` and ``retryCount`` control datagram requests awaiting a reply;
the default is 10 seconds and three retransmissions after the original message.
Congestion responses defer publishing for ``congestionWait`` (default six minutes).
TCP owns loss recovery on an established MQTT connection.

QoS 0 error feedback carries no publication identifier. The model matches its
TopicId to the latest QoS 0 representation sent in that session. A predefined-topic
error cannot invalidate an unrelated normal-topic binding with the same number.
Late feedback after that number changes representation remains ambiguous.

Sleep preserves the session even with Clean Session true. The gateway buffers data
and suspends downlink retries while asleep. A poll carries ClientId; delivery and any
unfinished handshake finish before PINGRESP closes the drain. New application work
returns to active state through CONNECT. Application sleep does not switch off a
radio and is not by itself an energy model.

Discovery and failure limits
----------------------------

An empty ``gatewayAddress`` enables local IPv4 discovery on ``discoveryInterface``.
SEARCHGW/GWINFO use the configured broadcast address and TTL 1. Gateways advertise
only while their shared MQTT connection is active. Advertisement durations, keepalive
and sleep fields are whole seconds; local retry/search timers can be fractional.
IPv6 unicast requires a configured gateway; IPv6 discovery is not supported.

Switching gateways clears old normal bindings and uses a clean initial session,
then restores desired subscriptions. It cannot migrate the old gateway's exchange
memory. After per-sensor TCP failure the gateway waits for a fresh sensor CONNECT,
preventing competing gateways from continuously taking over the same ClientId.

State and queues are held in memory. A gateway or broker crash can lose accepted data;
exactly-once continuity and Will delivery across those failures are not promised.
Native ZigBee/IEEE 802.15.4 binding, forwarder encapsulation, aggregation, encryption
and durable storage are outside this profile. Initial forwarding queues have no
configurable capacity limit. MQTT-SN messages occupy one UDP datagram and are limited
to 65507 bytes in this binding; MQTT-SN itself performs no fragmentation.

Run the configurations in ``examples/mqttsn`` for wired QoS 0/1/2, a temporary
sensor-link error burst with QoS 1 recovery, connectionless
publishing, discovery, sleeping sensors and an existing Wi-Fi/IP transport model.
Signals distinguish sent/received control packets, session changes and delivered
application payloads. Use the NED declarations for parameter and statistic details.
