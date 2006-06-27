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
    "Router",
    "StandardHost",
    "FlatNetworkConfigurator";


channel ethernetline2
    delay 0.1us;
    datarate 100*1000000;
endchannel

channel gigabitline2
    delay 0.1us;
    datarate 1000*1000000;
endchannel

module NClients2
    parameters:
        numrouters: numeric const,
        hostsperrouter: numeric const;
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=block/cogwheel";

        r: Router[numrouters];
            display: "i=abstract/router";
        cli: StandardHost[numrouters*hostsperrouter];
            display: "i=device/laptop_vs";
        srv: StandardHost;
            display: "i=device/server_l";
    connections nocheck:
        for i=0..numrouters-1, j=0..hostsperrouter-1 do
            cli[i*hostsperrouter+j].out++ --> ethernetline2 --> r[i].in++;
            cli[i*hostsperrouter+j].in++ <-- ethernetline2 <-- r[i].out++;
        endfor

        for i=0..numrouters-2 do
            r[i].out++ --> gigabitline2 --> r[i+1].in++;
            r[i+1].out++ --> gigabitline2 --> r[i].in++;
        endfor
        r[numrouters-1].out++ --> ethernetline2 --> srv.in++;
        r[numrouters-1].in++ <-- ethernetline2 <-- srv.out++;
endmodule

network nClients2 : NClients2
    parameters:
        numrouters = input(3, "Number of routers"),
        hostsperrouter = input(5, "Hosts per router");
endnetwork

