MQTT
====

The MQTT applications model MQTT 3.1.1 publish/subscribe messaging over the
existing TCP/IP stack. ``MqttClient`` can publish, subscribe, or do both.
``MqttBroker`` accepts independent client connections and routes publications
through their topic filters. Install them in the usual host ``app[]`` slots.

Configure a client
------------------

Set ``connectAddress`` to the broker host, ``connectPort`` to its listening port
(1883 by default), and ``clientIdentifier`` to a unique nonempty identifier.
The initial broker policy accepts 1 to 23 ASCII alphanumeric characters.
``startTime`` controls the first connection attempt. The client sends CONNECT
after TCP establishment and waits for an accepted CONNACK before starting work.
An empty destination leaves the default client inactive. Configure a unique
identifier when enabling it; the default identifier is intended for a single client.

The ``workload`` XML parameter contains ordered ``subscribe``, ``unsubscribe``
and ``publish`` actions. ``at`` is a nonnegative time offset from the first
accepted CONNACK; equal offsets preserve XML order. Topics and filters are UTF-8.
Payload strings are hexadecimal binary bytes, so ``0041ff`` represents three
bytes, including a leading zero. For example::

    *.sensor.app[0].typename = "MqttClient"
    *.sensor.app[0].clientIdentifier = "sensor1"
    *.sensor.app[0].connectAddress = "broker"
    *.sensor.app[0].workload = xml("<mqtt><subscribe at='0s' filter='factory/+/command' qos='1'/><publish at='1s' topic='factory/line1/temperature' qos='1' payload='32332e35'/></mqtt>")

A subscription action waits for SUBACK or UNSUBACK before later workload
actions proceed. Refused SUBACK results are reported without an immediate retry
loop. If disconnected, due actions wait for reconnection. Desired subscriptions
are restored when the broker has no prior session; a resumed session reissues
only an interrupted subscription mutation. Replayed QoS exchanges continue
independently so they can free identifiers needed by restoration.

Delivery and stored state
-------------------------

QoS 0 sends once without an MQTT acknowledgement. QoS 1 retains the publication
until PUBACK and can deliver duplicates after recovery. QoS 2 uses the
PUBLISH/PUBREC/PUBREL/PUBCOMP exchange and suppresses repeated application
handoffs within a session. The publisher-to-broker and broker-to-subscriber
legs have independent identifiers and QoS state. A publisher acknowledgement
does not prove delivery to every subscriber.

``cleanSession=false`` preserves eligible session state across network
disconnection. Persistent subscribers receive queued QoS 1/2 messages after
reconnection; offline QoS 0 storage is disabled. Ordinary application stop/start
retains eligible state, whereas crash discards runtime state. A crashed client
first reconnects with Clean Session true to remove incompatible broker state.
Broker sessions and retained records are in memory and do not survive crash.

``retain=true`` on a publication stores its latest topic value at the broker
for future subscriptions. A zero-byte retained payload deletes the stored value
and is still forwarded to current matching subscribers. Retained records are
independent of client sessions. Overlapping live subscriptions produce one copy
at the highest matching granted QoS, capped by the incoming publication's QoS.

Liveness and shutdown
---------------------

``keepAlive`` is an integer number of seconds from 0 through 65535; zero
disables idle pings. A nonzero value causes an idle client to send PINGREQ and
the broker to disconnect after 1.5 times that interval without a complete
control packet. Client timing is measured at the TCP handoff, not completion
of transmission on the wire; prolonged TCP backpressure is a fidelity limit.
``pingResponseTimeout`` independently bounds the wait for a response (5s by
default), and ``connectTimeout`` bounds TCP establishment plus CONNACK (5s).
``reconnectInterval`` is a fixed positive retry delay (1s by default).

Set ``willTopic``, hexadecimal ``willPayload``, ``willQos`` and ``willRetain``
to register a connection Will. A valid DISCONNECT suppresses it; abnormal
closure, detected network failure, keepalive expiry and identifier takeover
cause the broker to publish it. A partition is detected later, not at partition
onset. Orderly client stop sends DISCONNECT and then closes TCP without further
MQTT control packets. Broker crash provides no Will-delivery guarantee.

Examples and measurements
-------------------------

``examples/mqtt`` contains basic QoS 0, ``Qos1``, ``Qos2``, ``Retained`` and
``PersistentReconnect`` configurations. Run a configuration with the standard
INET launcher from that directory, for example::

    inet --debug -u Cmdenv -c Qos2 -r 0

The applications emit standard INET packet signals at complete MQTT control
packet boundaries; clients emit ``packetSentToUpper`` for application payload
handoff. ``deliveredMessages`` counts those handoffs and ``deliveredBytes`` sums
payload bytes, including duplicate deliveries. Divide by the stated observation
interval for delivered-payload throughput. MQTT/TCP traffic and distinct
application data are different quantities; the initial measurements do not
deduplicate QoS 1 deliveries or compute general end-to-end delay.

Supported scope
---------------

The model includes actual MQTT bytes and all three QoS levels. It omits MQTT 5.0,
MQTT-SN, WebSockets, TLS, broker bridges/clustering, external authentication
services, durable storage and CPU/disk timing. A configured broker
``userName``/``password`` pair enables deterministic exact-match admission;
anonymous access is the default. Credentials on plain TCP are not secure.
There is no configurable broker capacity model: finite experiments must fit
host memory. Malformed wire input is rejected at the protocol boundary rather
than silently acknowledged.
