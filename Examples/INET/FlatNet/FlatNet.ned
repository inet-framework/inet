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


channel fiberline
    delay 1us;
    datarate 512*1000000;
endchannel

channel ethernetline
    delay 0.1us;
    datarate 10*1000000;
endchannel

module FlatNet
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=block/cogwheel";
        rte: Router[57];
            display: "i=misc/node_vs,green4";
        cli: StandardHost[2];
            display: "i=device/laptop_s";
        srv: StandardHost;
            display: "i=device/server_s";
    connections nocheck:
        cli[0].out++ --> ethernetline --> rte[50].in++;
        cli[0].in++ <-- ethernetline <-- rte[50].out++;
        cli[1].out++ --> ethernetline --> rte[8].in++;
        cli[1].in++ <-- ethernetline <-- rte[8].out++;

        srv.out++ --> fiberline --> rte[18].in++;
        srv.in++ <-- fiberline <-- rte[18].out++;

        rte[0].out++ --> fiberline --> rte[1].in++;
        rte[0].in++ <-- fiberline <-- rte[1].out++;

        rte[1].out++ --> fiberline --> rte[2].in++;
        rte[1].in++ <-- fiberline <-- rte[2].out++;

        rte[1].out++ --> fiberline --> rte[4].in++;
        rte[1].in++ <-- fiberline <-- rte[4].out++;

        rte[3].out++ --> fiberline --> rte[4].in++;
        rte[3].in++ <-- fiberline <-- rte[4].out++;

        rte[4].out++ --> fiberline --> rte[5].in++;
        rte[4].in++ <-- fiberline <-- rte[5].out++;

        rte[4].out++ --> fiberline --> rte[7].in++;
        rte[4].in++ <-- fiberline <-- rte[7].out++;

        rte[5].out++ --> fiberline --> rte[6].in++;
        rte[5].in++ <-- fiberline <-- rte[6].out++;

        rte[5].out++ --> fiberline --> rte[10].in++;
        rte[5].in++ <-- fiberline <-- rte[10].out++;

        rte[6].out++ --> fiberline --> rte[7].in++;
        rte[6].in++ <-- fiberline <-- rte[7].out++;

        rte[6].out++ --> fiberline --> rte[9].in++;
        rte[6].in++ <-- fiberline <-- rte[9].out++;

        rte[7].out++ --> fiberline --> rte[8].in++;
        rte[7].in++ <-- fiberline <-- rte[8].out++;

        rte[7].out++ --> fiberline --> rte[12].in++;
        rte[7].in++ <-- fiberline <-- rte[12].out++;

        rte[9].out++ --> fiberline --> rte[11].in++;
        rte[9].in++ <-- fiberline <-- rte[11].out++;

        rte[10].out++ --> fiberline --> rte[11].in++;
        rte[10].in++ <-- fiberline <-- rte[11].out++;

        rte[10].out++ --> fiberline --> rte[13].in++;
        rte[10].in++ <-- fiberline <-- rte[13].out++;

        rte[11].out++ --> fiberline --> rte[12].in++;
        rte[11].in++ <-- fiberline <-- rte[12].out++;

        rte[11].out++ --> fiberline --> rte[14].in++;
        rte[11].in++ <-- fiberline <-- rte[14].out++;

        rte[12].out++ --> fiberline --> rte[15].in++;
        rte[12].in++ <-- fiberline <-- rte[15].out++;

        rte[13].out++ --> fiberline --> rte[18].in++;
        rte[13].in++ <-- fiberline <-- rte[18].out++;

        rte[14].out++ --> fiberline --> rte[15].in++;
        rte[14].in++ <-- fiberline <-- rte[15].out++;

        rte[14].out++ --> fiberline --> rte[22].in++;
        rte[14].in++ <-- fiberline <-- rte[22].out++;

        rte[15].out++ --> fiberline --> rte[16].in++;
        rte[15].in++ <-- fiberline <-- rte[16].out++;

        rte[16].out++ --> fiberline --> rte[17].in++;
        rte[16].in++ <-- fiberline <-- rte[17].out++;

        rte[16].out++ --> fiberline --> rte[23].in++;
        rte[16].in++ <-- fiberline <-- rte[23].out++;

        rte[18].out++ --> fiberline --> rte[19].in++;
        rte[18].in++ <-- fiberline <-- rte[19].out++;

        rte[18].out++ --> fiberline --> rte[21].in++;
        rte[18].in++ <-- fiberline <-- rte[21].out++;

        rte[19].out++ --> fiberline --> rte[20].in++;
        rte[19].in++ <-- fiberline <-- rte[20].out++;

        rte[19].out++ --> fiberline --> rte[26].in++;
        rte[19].in++ <-- fiberline <-- rte[26].out++;

        rte[20].out++ --> fiberline --> rte[21].in++;
        rte[20].in++ <-- fiberline <-- rte[21].out++;

        rte[20].out++ --> fiberline --> rte[24].in++;
        rte[20].in++ <-- fiberline <-- rte[24].out++;

        rte[20].out++ --> fiberline --> rte[25].in++;
        rte[20].in++ <-- fiberline <-- rte[25].out++;

        rte[21].out++ --> fiberline --> rte[22].in++;
        rte[21].in++ <-- fiberline <-- rte[22].out++;

        rte[21].out++ --> fiberline --> rte[23].in++;
        rte[21].in++ <-- fiberline <-- rte[23].out++;

        rte[21].out++ --> fiberline --> rte[24].in++;
        rte[21].in++ <-- fiberline <-- rte[24].out++;

        rte[23].out++ --> fiberline --> rte[24].in++;
        rte[23].in++ <-- fiberline <-- rte[24].out++;

        rte[24].out++ --> fiberline --> rte[28].in++;
        rte[24].in++ <-- fiberline <-- rte[28].out++;

        rte[25].out++ --> fiberline --> rte[27].in++;
        rte[25].in++ <-- fiberline <-- rte[27].out++;

        rte[26].out++ --> fiberline --> rte[32].in++;
        rte[26].in++ <-- fiberline <-- rte[32].out++;

        rte[27].out++ --> fiberline --> rte[29].in++;
        rte[27].in++ <-- fiberline <-- rte[29].out++;

        rte[27].out++ --> fiberline --> rte[30].in++;
        rte[27].in++ <-- fiberline <-- rte[30].out++;

        rte[27].out++ --> fiberline --> rte[32].in++;
        rte[27].in++ <-- fiberline <-- rte[32].out++;

        rte[28].out++ --> fiberline --> rte[29].in++;
        rte[28].in++ <-- fiberline <-- rte[29].out++;

        rte[28].out++ --> fiberline --> rte[31].in++;
        rte[28].in++ <-- fiberline <-- rte[31].out++;

        rte[29].out++ --> fiberline --> rte[31].in++;
        rte[29].in++ <-- fiberline <-- rte[31].out++;

        rte[30].out++ --> fiberline --> rte[36].in++;
        rte[30].in++ <-- fiberline <-- rte[36].out++;

        rte[31].out++ --> fiberline --> rte[33].in++;
        rte[31].in++ <-- fiberline <-- rte[33].out++;

        rte[32].out++ --> fiberline --> rte[34].in++;
        rte[32].in++ <-- fiberline <-- rte[34].out++;

        rte[33].out++ --> fiberline --> rte[37].in++;
        rte[33].in++ <-- fiberline <-- rte[37].out++;

        rte[34].out++ --> fiberline --> rte[35].in++;
        rte[34].in++ <-- fiberline <-- rte[35].out++;

        rte[34].out++ --> fiberline --> rte[40].in++;
        rte[34].in++ <-- fiberline <-- rte[40].out++;

        rte[35].out++ --> fiberline --> rte[36].in++;
        rte[35].in++ <-- fiberline <-- rte[36].out++;

        rte[35].out++ --> fiberline --> rte[39].in++;
        rte[35].in++ <-- fiberline <-- rte[39].out++;

        rte[36].out++ --> fiberline --> rte[37].in++;
        rte[36].in++ <-- fiberline <-- rte[37].out++;

        rte[37].out++ --> fiberline --> rte[38].in++;
        rte[37].in++ <-- fiberline <-- rte[38].out++;

        rte[38].out++ --> fiberline --> rte[39].in++;
        rte[38].in++ <-- fiberline <-- rte[39].out++;

        rte[38].out++ --> fiberline --> rte[42].in++;
        rte[38].in++ <-- fiberline <-- rte[42].out++;

        rte[39].out++ --> fiberline --> rte[40].in++;
        rte[39].in++ <-- fiberline <-- rte[40].out++;

        rte[39].out++ --> fiberline --> rte[41].in++;
        rte[39].in++ <-- fiberline <-- rte[41].out++;

        rte[40].out++ --> fiberline --> rte[43].in++;
        rte[40].in++ <-- fiberline <-- rte[43].out++;

        rte[40].out++ --> fiberline --> rte[44].in++;
        rte[40].in++ <-- fiberline <-- rte[44].out++;

        rte[40].out++ --> fiberline --> rte[52].in++;
        rte[40].in++ <-- fiberline <-- rte[52].out++;

        rte[41].out++ --> fiberline --> rte[42].in++;
        rte[41].in++ <-- fiberline <-- rte[42].out++;

        rte[41].out++ --> fiberline --> rte[43].in++;
        rte[41].in++ <-- fiberline <-- rte[43].out++;

        rte[42].out++ --> fiberline --> rte[43].in++;
        rte[42].in++ <-- fiberline <-- rte[43].out++;

        rte[43].out++ --> fiberline --> rte[46].in++;
        rte[43].in++ <-- fiberline <-- rte[46].out++;

        rte[44].out++ --> fiberline --> rte[45].in++;
        rte[44].in++ <-- fiberline <-- rte[45].out++;

        rte[44].out++ --> fiberline --> rte[47].in++;
        rte[44].in++ <-- fiberline <-- rte[47].out++;

        rte[45].out++ --> fiberline --> rte[46].in++;
        rte[45].in++ <-- fiberline <-- rte[46].out++;

        rte[46].out++ --> fiberline --> rte[49].in++;
        rte[46].in++ <-- fiberline <-- rte[49].out++;

        rte[47].out++ --> fiberline --> rte[48].in++;
        rte[47].in++ <-- fiberline <-- rte[48].out++;

        rte[48].out++ --> fiberline --> rte[50].in++;
        rte[48].in++ <-- fiberline <-- rte[50].out++;

        rte[48].out++ --> fiberline --> rte[51].in++;
        rte[48].in++ <-- fiberline <-- rte[51].out++;

        rte[49].out++ --> fiberline --> rte[50].in++;
        rte[49].in++ <-- fiberline <-- rte[50].out++;

        rte[50].out++ --> fiberline --> rte[53].in++;
        rte[50].in++ <-- fiberline <-- rte[53].out++;

        rte[51].out++ --> fiberline --> rte[52].in++;
        rte[51].in++ <-- fiberline <-- rte[52].out++;

        rte[51].out++ --> fiberline --> rte[53].in++;
        rte[51].in++ <-- fiberline <-- rte[53].out++;

        rte[52].out++ --> fiberline --> rte[53].in++;
        rte[52].in++ <-- fiberline <-- rte[53].out++;

        rte[52].out++ --> fiberline --> rte[54].in++;
        rte[52].in++ <-- fiberline <-- rte[54].out++;

        rte[53].out++ --> fiberline --> rte[56].in++;
        rte[53].in++ <-- fiberline <-- rte[56].out++;

        rte[54].out++ --> fiberline --> rte[55].in++;
        rte[54].in++ <-- fiberline <-- rte[55].out++;

        rte[55].out++ --> fiberline --> rte[56].in++;
        rte[55].in++ <-- fiberline <-- rte[56].out++;
endmodule

network flatNet : FlatNet
endnetwork

