// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.applications.dtlsapp;

import inet.applications.contract.IApp;
import inet.applications.dtlsapp.DtlsSocketIo;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPassivePacketSink;
import inet.transportlayer.contract.IDtls;

// Generic packet source/sink over a modeled DTLS association.
module DtlsApp like IApp
{
    parameters:
        bool server = default(false);
        @application;
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
    submodules:
        dtls: <default("Dtls")> like IDtls {
            @display("p=350,300");
        }
        io: DtlsSocketIo {
            server = default(parent.server);
            @display("p=350,180");
        }
        source: <default(server ? "" : "ActivePacketSource")> like IActivePacketSource if typename != "" {
            @display("p=150,80");
        }
        sink: <default("PassivePacketSink")> like IPassivePacketSink if typename != "" {
            @display("p=550,80");
        }
    connections allowunconnected:
        source.out --> io.trafficIn if exists(source);
        io.trafficOut --> sink.in if exists(sink);
        socketIn --> dtls.socketIn;
        dtls.socketOut --> socketOut;
}
