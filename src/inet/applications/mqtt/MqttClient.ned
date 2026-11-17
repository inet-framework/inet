// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.mqtt;
import inet.common.SimpleModule;
import inet.applications.contract.IApp;

// MQTT 3.1.1 client. Workload offsets are measured from the first accepted CONNACK.
// Each subscribe/unsubscribe waits for its acknowledgement. Binary payloads use hex.
simple MqttClient extends SimpleModule like IApp
{
    parameters:
        @class(MqttClient);
        string localAddress = default(""); // Empty selects an unspecified local address.
        int localPort = default(-1); // -1 selects an ephemeral port.
        string connectAddress = default(""); // Empty leaves the client inactive.
        int connectPort = default(1883); // TCP server port, 1..65535.
        string clientIdentifier = default("client"); // Configure a unique identifier; initial profile accepts 1..23 ASCII alphanumeric bytes.
        bool cleanSession = default(true); // False preserves eligible exchanges across disconnects.
        int keepAlive @unit(s) = default(0s); // Integer seconds, 0 disables liveness pings, maximum 65535s.
        string userName = default(""); // Empty omits authentication unless password is provided.
        string password = default(""); // Sent even when empty if a username is configured. Plain TCP credentials are not encrypted.
        string willTopic = default(""); // Empty disables the connection Will.
        string willPayload = default(""); // Hexadecimal binary Will payload.
        int willQos = default(0); // 0, 1 or 2.
        bool willRetain = default(false);
        xml workload = default(xml("<mqtt/>")); // Ordered subscribe, unsubscribe, publish elements with at, filter/topic, qos, retain and payload attributes.
        double startTime @unit(s) = default(0s); // Nonnegative initial connection time.
        double reconnectInterval @unit(s) = default(1s); // Strictly positive delay between attempts.
        double connectTimeout @unit(s) = default(5s); // Positive bound including TCP and CONNACK.
        double pingResponseTimeout @unit(s) = default(5s); // Positive bound after PINGREQ.
        double stopOperationTimeout @unit(s) = default(5s);
        @lifecycleSupport;
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
        @signal[connectionChanged](type=long);
        @statistic[deliveredMessages](source=packetSentToUpper; record=count);
        @statistic[deliveredBytes](source=packetBytes(packetSentToUpper); record=sum);
        @display("i=block/wrxtx");
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
