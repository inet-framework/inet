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
    "StandardHost2",
    "FlatNetworkConfigurator";


channel fiberrouterline
    delay normal(0.000001,0.0000001);
    datarate 512*1000000;
endchannel

module FlatNet
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypesToProcess = "Router StandardHost2",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=cogwheel";
        rte: Router[57];
            parameters:
                routingFile="";
            display: "i=ball2_vs,gold,100";
    connections nocheck:
        rte[0].out++ --> fiberrouterline --> rte[1].in++;
        rte[0].in++ <-- fiberrouterline <-- rte[1].out++;

        rte[1].out++ --> fiberrouterline --> rte[2].in++;
        rte[1].in++ <-- fiberrouterline <-- rte[2].out++;

        rte[1].out++ --> fiberrouterline --> rte[4].in++;
        rte[1].in++ <-- fiberrouterline <-- rte[4].out++;

        rte[3].out++ --> fiberrouterline --> rte[4].in++;
        rte[3].in++ <-- fiberrouterline <-- rte[4].out++;

        rte[4].out++ --> fiberrouterline --> rte[5].in++;
        rte[4].in++ <-- fiberrouterline <-- rte[5].out++;

        rte[4].out++ --> fiberrouterline --> rte[7].in++;
        rte[4].in++ <-- fiberrouterline <-- rte[7].out++;

        rte[5].out++ --> fiberrouterline --> rte[6].in++;
        rte[5].in++ <-- fiberrouterline <-- rte[6].out++;

        rte[5].out++ --> fiberrouterline --> rte[10].in++;
        rte[5].in++ <-- fiberrouterline <-- rte[10].out++;

        rte[6].out++ --> fiberrouterline --> rte[7].in++;
        rte[6].in++ <-- fiberrouterline <-- rte[7].out++;

        rte[6].out++ --> fiberrouterline --> rte[9].in++;
        rte[6].in++ <-- fiberrouterline <-- rte[9].out++;

        rte[7].out++ --> fiberrouterline --> rte[8].in++;
        rte[7].in++ <-- fiberrouterline <-- rte[8].out++;

        rte[7].out++ --> fiberrouterline --> rte[12].in++;
        rte[7].in++ <-- fiberrouterline <-- rte[12].out++;

        rte[9].out++ --> fiberrouterline --> rte[11].in++;
        rte[9].in++ <-- fiberrouterline <-- rte[11].out++;

        rte[10].out++ --> fiberrouterline --> rte[11].in++;
        rte[10].in++ <-- fiberrouterline <-- rte[11].out++;

        rte[10].out++ --> fiberrouterline --> rte[13].in++;
        rte[10].in++ <-- fiberrouterline <-- rte[13].out++;

        rte[11].out++ --> fiberrouterline --> rte[12].in++;
        rte[11].in++ <-- fiberrouterline <-- rte[12].out++;

        rte[11].out++ --> fiberrouterline --> rte[14].in++;
        rte[11].in++ <-- fiberrouterline <-- rte[14].out++;

        rte[12].out++ --> fiberrouterline --> rte[15].in++;
        rte[12].in++ <-- fiberrouterline <-- rte[15].out++;

        rte[13].out++ --> fiberrouterline --> rte[18].in++;
        rte[13].in++ <-- fiberrouterline <-- rte[18].out++;

        rte[14].out++ --> fiberrouterline --> rte[15].in++;
        rte[14].in++ <-- fiberrouterline <-- rte[15].out++;

        rte[14].out++ --> fiberrouterline --> rte[22].in++;
        rte[14].in++ <-- fiberrouterline <-- rte[22].out++;

        rte[15].out++ --> fiberrouterline --> rte[16].in++;
        rte[15].in++ <-- fiberrouterline <-- rte[16].out++;

        rte[16].out++ --> fiberrouterline --> rte[17].in++;
        rte[16].in++ <-- fiberrouterline <-- rte[17].out++;

        rte[16].out++ --> fiberrouterline --> rte[23].in++;
        rte[16].in++ <-- fiberrouterline <-- rte[23].out++;

        rte[18].out++ --> fiberrouterline --> rte[19].in++;
        rte[18].in++ <-- fiberrouterline <-- rte[19].out++;

        rte[18].out++ --> fiberrouterline --> rte[21].in++;
        rte[18].in++ <-- fiberrouterline <-- rte[21].out++;

        rte[19].out++ --> fiberrouterline --> rte[20].in++;
        rte[19].in++ <-- fiberrouterline <-- rte[20].out++;

        rte[19].out++ --> fiberrouterline --> rte[26].in++;
        rte[19].in++ <-- fiberrouterline <-- rte[26].out++;

        rte[20].out++ --> fiberrouterline --> rte[21].in++;
        rte[20].in++ <-- fiberrouterline <-- rte[21].out++;

        rte[20].out++ --> fiberrouterline --> rte[24].in++;
        rte[20].in++ <-- fiberrouterline <-- rte[24].out++;

        rte[20].out++ --> fiberrouterline --> rte[25].in++;
        rte[20].in++ <-- fiberrouterline <-- rte[25].out++;

        rte[21].out++ --> fiberrouterline --> rte[22].in++;
        rte[21].in++ <-- fiberrouterline <-- rte[22].out++;

        rte[21].out++ --> fiberrouterline --> rte[23].in++;
        rte[21].in++ <-- fiberrouterline <-- rte[23].out++;

        rte[21].out++ --> fiberrouterline --> rte[24].in++;
        rte[21].in++ <-- fiberrouterline <-- rte[24].out++;

        rte[23].out++ --> fiberrouterline --> rte[24].in++;
        rte[23].in++ <-- fiberrouterline <-- rte[24].out++;

        rte[24].out++ --> fiberrouterline --> rte[28].in++;
        rte[24].in++ <-- fiberrouterline <-- rte[28].out++;

        rte[25].out++ --> fiberrouterline --> rte[27].in++;
        rte[25].in++ <-- fiberrouterline <-- rte[27].out++;

        rte[26].out++ --> fiberrouterline --> rte[32].in++;
        rte[26].in++ <-- fiberrouterline <-- rte[32].out++;

        rte[27].out++ --> fiberrouterline --> rte[29].in++;
        rte[27].in++ <-- fiberrouterline <-- rte[29].out++;

        rte[27].out++ --> fiberrouterline --> rte[30].in++;
        rte[27].in++ <-- fiberrouterline <-- rte[30].out++;

        rte[27].out++ --> fiberrouterline --> rte[32].in++;
        rte[27].in++ <-- fiberrouterline <-- rte[32].out++;

        rte[28].out++ --> fiberrouterline --> rte[29].in++;
        rte[28].in++ <-- fiberrouterline <-- rte[29].out++;

        rte[28].out++ --> fiberrouterline --> rte[31].in++;
        rte[28].in++ <-- fiberrouterline <-- rte[31].out++;

        rte[29].out++ --> fiberrouterline --> rte[31].in++;
        rte[29].in++ <-- fiberrouterline <-- rte[31].out++;

        rte[30].out++ --> fiberrouterline --> rte[36].in++;
        rte[30].in++ <-- fiberrouterline <-- rte[36].out++;

        rte[31].out++ --> fiberrouterline --> rte[33].in++;
        rte[31].in++ <-- fiberrouterline <-- rte[33].out++;

        rte[32].out++ --> fiberrouterline --> rte[34].in++;
        rte[32].in++ <-- fiberrouterline <-- rte[34].out++;

        rte[33].out++ --> fiberrouterline --> rte[37].in++;
        rte[33].in++ <-- fiberrouterline <-- rte[37].out++;

        rte[34].out++ --> fiberrouterline --> rte[35].in++;
        rte[34].in++ <-- fiberrouterline <-- rte[35].out++;

        rte[34].out++ --> fiberrouterline --> rte[40].in++;
        rte[34].in++ <-- fiberrouterline <-- rte[40].out++;

        rte[35].out++ --> fiberrouterline --> rte[36].in++;
        rte[35].in++ <-- fiberrouterline <-- rte[36].out++;

        rte[35].out++ --> fiberrouterline --> rte[39].in++;
        rte[35].in++ <-- fiberrouterline <-- rte[39].out++;

        rte[36].out++ --> fiberrouterline --> rte[37].in++;
        rte[36].in++ <-- fiberrouterline <-- rte[37].out++;

        rte[37].out++ --> fiberrouterline --> rte[38].in++;
        rte[37].in++ <-- fiberrouterline <-- rte[38].out++;

        rte[38].out++ --> fiberrouterline --> rte[39].in++;
        rte[38].in++ <-- fiberrouterline <-- rte[39].out++;

        rte[38].out++ --> fiberrouterline --> rte[42].in++;
        rte[38].in++ <-- fiberrouterline <-- rte[42].out++;

        rte[39].out++ --> fiberrouterline --> rte[40].in++;
        rte[39].in++ <-- fiberrouterline <-- rte[40].out++;

        rte[39].out++ --> fiberrouterline --> rte[41].in++;
        rte[39].in++ <-- fiberrouterline <-- rte[41].out++;

        rte[40].out++ --> fiberrouterline --> rte[43].in++;
        rte[40].in++ <-- fiberrouterline <-- rte[43].out++;

        rte[40].out++ --> fiberrouterline --> rte[44].in++;
        rte[40].in++ <-- fiberrouterline <-- rte[44].out++;

        rte[40].out++ --> fiberrouterline --> rte[52].in++;
        rte[40].in++ <-- fiberrouterline <-- rte[52].out++;

        rte[41].out++ --> fiberrouterline --> rte[42].in++;
        rte[41].in++ <-- fiberrouterline <-- rte[42].out++;

        rte[41].out++ --> fiberrouterline --> rte[43].in++;
        rte[41].in++ <-- fiberrouterline <-- rte[43].out++;

        rte[42].out++ --> fiberrouterline --> rte[43].in++;
        rte[42].in++ <-- fiberrouterline <-- rte[43].out++;

        rte[43].out++ --> fiberrouterline --> rte[46].in++;
        rte[43].in++ <-- fiberrouterline <-- rte[46].out++;

        rte[44].out++ --> fiberrouterline --> rte[45].in++;
        rte[44].in++ <-- fiberrouterline <-- rte[45].out++;

        rte[44].out++ --> fiberrouterline --> rte[47].in++;
        rte[44].in++ <-- fiberrouterline <-- rte[47].out++;

        rte[45].out++ --> fiberrouterline --> rte[46].in++;
        rte[45].in++ <-- fiberrouterline <-- rte[46].out++;

        rte[46].out++ --> fiberrouterline --> rte[49].in++;
        rte[46].in++ <-- fiberrouterline <-- rte[49].out++;

        rte[47].out++ --> fiberrouterline --> rte[48].in++;
        rte[47].in++ <-- fiberrouterline <-- rte[48].out++;

        rte[48].out++ --> fiberrouterline --> rte[50].in++;
        rte[48].in++ <-- fiberrouterline <-- rte[50].out++;

        rte[48].out++ --> fiberrouterline --> rte[51].in++;
        rte[48].in++ <-- fiberrouterline <-- rte[51].out++;

        rte[49].out++ --> fiberrouterline --> rte[50].in++;
        rte[49].in++ <-- fiberrouterline <-- rte[50].out++;

        rte[50].out++ --> fiberrouterline --> rte[53].in++;
        rte[50].in++ <-- fiberrouterline <-- rte[53].out++;

        rte[51].out++ --> fiberrouterline --> rte[52].in++;
        rte[51].in++ <-- fiberrouterline <-- rte[52].out++;

        rte[51].out++ --> fiberrouterline --> rte[53].in++;
        rte[51].in++ <-- fiberrouterline <-- rte[53].out++;

        rte[52].out++ --> fiberrouterline --> rte[53].in++;
        rte[52].in++ <-- fiberrouterline <-- rte[53].out++;

        rte[52].out++ --> fiberrouterline --> rte[54].in++;
        rte[52].in++ <-- fiberrouterline <-- rte[54].out++;

        rte[53].out++ --> fiberrouterline --> rte[56].in++;
        rte[53].in++ <-- fiberrouterline <-- rte[56].out++;

        rte[54].out++ --> fiberrouterline --> rte[55].in++;
        rte[54].in++ <-- fiberrouterline <-- rte[55].out++;

        rte[55].out++ --> fiberrouterline --> rte[56].in++;
        rte[55].in++ <-- fiberrouterline <-- rte[56].out++;
endmodule

network flatNet : FlatNet
endnetwork

