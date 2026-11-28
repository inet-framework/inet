// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.mqtt;
import inet.common.SimpleModule;
import inet.applications.contract.IApp;

// MQTT 3.1.1 broker with in-memory sessions. Broker crash loses stored state.
simple MqttBroker extends SimpleModule like IApp
{
    parameters:
        @class(MqttBroker);
        string localAddress = default(""); // Empty listens on all local addresses.
        int localPort = default(1883); // TCP port, 1..65535.
        string userName = default(""); // Empty accepts anonymous clients.
        string password = default(""); // Exact comparison; no cryptographic authentication.
        double connectTimeout @unit(s) = default(5s); // Positive timeout to receive CONNECT.
        double stopOperationTimeout @unit(s) = default(5s);
        @lifecycleSupport;
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[connectionChanged](type=long);
        @display("i=block/dispatch");
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
