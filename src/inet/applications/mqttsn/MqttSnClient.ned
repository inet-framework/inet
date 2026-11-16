// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.mqttsn;
import inet.common.SimpleModule;
import inet.applications.contract.IApp;

// MQTT-SN v1.2 over UDP. Discovery is limited to a configured IPv4 broadcast domain.
simple MqttSnClient extends SimpleModule like IApp
{
    parameters:
        @class(MqttSnClient);
        string localAddress = default("");
        int localPort = default(1884);
        string gatewayAddress = default(""); // Empty enables IPv4 discovery.
        int gatewayPort = default(1884);
        string clientIdentifier = default("sensor");
        bool cleanSession = default(true);
        int keepAlive @unit(s) = default(30s); // 0 disables active keepalive, maximum 65535s.
        bool connectionless = default(false); // QoS -1 publications only, with configured gateway.
        xml predefinedTopics = default(xml("<topics/>"));
        xml workload = default(xml("<mqttsn/>")); // Ordered at (seconds), topic/filter, qos, payload (hex), retain and duration actions.
        string willTopic = default("");
        string willPayload = default(""); // Hexadecimal bytes.
        int willQos = default(0);
        bool willRetain = default(false);
        double startTime @unit(s) = default(0s);
        double retryInterval @unit(s) = default(10s);
        int retryCount = default(3); // Retransmissions after the original request.
        double congestionWait @unit(s) = default(6min);
        string discoveryInterface = default("eth0");
        string interfaceTableModule;
        string broadcastAddress = default("255.255.255.255");
        double searchDelay @unit(s) = default(5s);
        double gatewayInfoDelay @unit(s) = default(5s);
        double gatewayInfoLifetime @unit(s) = default(60min);
        double searchResponseTimeout @unit(s) = default(5s);
        double searchRetryMax @unit(s) = default(60s);
        int missedAdvertisements = default(3);
        double stopOperationTimeout @unit(s) = default(5s);
        @lifecycleSupport;
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
        @signal[sessionStateChanged](type=long);
        @statistic[deliveredMessages](source=packetSentToUpper; record=count);
        @statistic[deliveredBytes](source=packetBytes(packetSentToUpper); record=sum);
        @display("i=block/wrxtx");
    gates:
        input socketIn;
        output socketOut;
}
