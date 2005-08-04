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
    "FlatNetworkConfigurator6";


channel fiberline
    delay 1us;
    datarate 512*1000000;
endchannel

module NClientsPPP
    parameters:
        n: numeric const;
    submodules:
        configurator: FlatNetworkConfigurator6;
            parameters:
                moduleTypes = "Router6 StandardHost6",
                nonIPModuleTypes = "";
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
    connections nocheck:
        for i=0..n-1 do
            cli[i].out++ --> fiberline --> r1.in++;
            cli[i].in++ <-- fiberline <-- r1.out++;
        endfor

        r1.out++ --> fiberline --> r2.in++;
        r1.in++ <-- fiberline <-- r2.out++;

        r2.out++ --> fiberline --> r3.in++;
        r2.in++ <-- fiberline <-- r3.out++;

        r3.out++ --> fiberline --> srv.in++;
        r3.in++ <-- fiberline <-- srv.out++;

endmodule

network nClientsPPP : NClientsPPP
endnetwork

