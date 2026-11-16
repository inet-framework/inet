// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.dtlsapp;

import inet.common.SimpleModule;

// Adapter between generic packet sources/sinks and a DTLS engine.
simple DtlsSocketIo extends SimpleModule
{
    parameters:
        @class(dtls::DtlsSocketIo);
        @display("i=block/socket");
        string dtlsModule = default("^.dtls");
        bool server = default(false);
        bool resumeSession = default(false); // best-effort use of a cached ticket on open
        string destAddress = default("");
        int destPort = default(-1);
        @lifecycleSupport();
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
    gates:
        input trafficIn;
        output trafficOut;
}
