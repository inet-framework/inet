//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


import
    "Router6",
    "StandardHost6",
    "EtherHub",
    "FlatNetworkConfigurator6";


channel fiberline
    delay 1us;
    datarate 512*1000000;
endchannel

channel ethernetline
    delay 0.1us;
endchannel

module NClientsEth
    parameters:
        n: numeric const;
    submodules:
        configurator: FlatNetworkConfigurator6;
            parameters:
                moduleTypes = "Router6 StandardHost6 EtherHub",
                nonIPModuleTypes = "EtherHub";
            display: "i=block/cogwheel";
        r1: Router6;
            display: "i=abstract/router";
        r2: Router6;
            display: "i=abstract/router";
        r3: Router6;
            display: "i=abstract/router";
        cli: StandardHost6[n];
            display: "i=device/laptop";
        srv: StandardHost6;
            display: "i=device/server_l";
        hub: EtherHub;
            display: "i=device/hub_s";
    connections nocheck:
        for i=0..n-1 do
            cli[i].ethOut++ --> ethernetline --> hub.in++;
            cli[i].ethIn++ <-- ethernetline <-- hub.out++;
        endfor

        hub.out++ --> ethernetline --> r1.ethIn++;
        hub.in++ <-- ethernetline <-- r1.ethOut++;

        r1.ethOut++ --> ethernetline --> r2.ethIn++;
        r1.ethIn++ <-- ethernetline <-- r2.ethOut++;

        r2.ethOut++ --> ethernetline --> r3.ethIn++;
        r2.ethIn++ <-- ethernetline <-- r3.ethOut++;

        r3.ethOut++ --> ethernetline --> srv.ethIn++;
        r3.ethIn++ <-- ethernetline <-- srv.ethOut++;

        // Mixing ethernet and ppp currently doesn't work, due to a bug in
        // FlatNetworkConfigurator6::determineGateIndex.
        //
        // r1.out++ --> fiberline --> r2.in++;
        // r1.in++ <-- fiberline <-- r2.out++;
        //
        // r2.out++ --> fiberline --> r3.in++;
        // r2.in++ <-- fiberline <-- r3.out++;
        //
        // r3.out++ --> fiberline --> srv.in++;
        // r3.in++ <-- fiberline <-- srv.out++;

endmodule

network nClientsEth : NClientsEth
endnetwork

