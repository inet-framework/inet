// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.tests.mqttsn;
simple MqttSnScriptedPeer like inet.applications.contract.IApp
{
    parameters:
        @class(MqttSnScriptedPeer);
        int localPort = default(1884);
        string remoteAddress = default("");
        int remotePort = default(1884);
        xml script = default(xml("<script/>"));
        @signal[scenarioComplete](type=long);
    gates:
        input socketIn;
        output socketOut;
}
