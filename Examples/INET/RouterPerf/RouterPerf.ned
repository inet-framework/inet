//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
    "BurstHost",
    "FlatNetworkConfigurator";


module RouterPerfNetwork
    parameters:
        nodeNo : numeric const;
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router BurstHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=60,160;i=block/cogwheel";
        sender: BurstHost[nodeNo];
            display: "p=100,317,row;i=device/pc3,yellow,10";
        recip: BurstHost[nodeNo];
            display: "p=100,68,row;i=device/pc3";
        router: Router;
            display: "p=172,182;i=abstract/router";
    connections nocheck:
        for i=0..nodeNo-1 do
            sender[i].out++ --> datarate 1000000 --> router.in++;
            sender[i].in++ <-- datarate 1000000 <-- router.out++;

            recip[i].out++ --> datarate 1000000 --> router.in++;
            recip[i].in++ <-- datarate 1000000 <-- router.out++;
        endfor;

endmodule

network routerPerfNetwork : RouterPerfNetwork
endnetwork

