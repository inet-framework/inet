// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.mqttsn;
import inet.common.SimpleModule;
import inet.applications.contract.IApp;

// Per-sensor MQTT connections, independent hop QoS, in-memory Will and sleep queues.
simple MqttSnGateway extends SimpleModule like IApp
{
    parameters:
        @class(MqttSnGateway);
        string localAddress = default("");
        int localPort = default(1884);
        int gatewayId = default(1); // Unique within the broadcast domain, 0..255.
        string brokerAddress = default(""); // Required MQTT broker address.
        int brokerPort = default(1883);
        string gatewayClientIdentifier = default("gateway"); // Unique broker identifier for shared publications.
        string userName = default("");
        string password = default("");
        xml predefinedTopics = default(xml("<topics/>"));
        double retryInterval @unit(s) = default(10s);
        int retryCount = default(3);
        double congestionWait @unit(s) = default(6min);
        double connectTimeout @unit(s) = default(5s);
        double reconnectInterval @unit(s) = default(1s);
        int brokerKeepAlive @unit(s) = default(30s);
        double brokerPingTimeout @unit(s) = default(5s);
        bool advertise = default(true);
        int advertisementInterval @unit(s) = default(20min); // 1..65535 whole seconds.
        string discoveryInterface = default("eth0");
        string interfaceTableModule;
        string broadcastAddress = default("255.255.255.255");
        double stopOperationTimeout @unit(s) = default(5s);
        @lifecycleSupport;
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetSentToUpper](type=inet::Packet);
        @signal[sessionStateChanged](type=long);
        @display("i=block/dispatch");
    gates:
        input socketIn;
        output socketOut;
}
