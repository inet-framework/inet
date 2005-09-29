//
// Copyright (C) 2005 Wei Yang, Ng
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

channel ethernetline
	delay 0.1us;
endchannel

module DemoNetworkEth
	parameters:
		n: numeric const;
	submodules:
		configurator: FlatNetworkConfigurator6;
			parameters:
				moduleTypes = "Router6 StandardHost6",
				nonIPModuleTypes = "EtherHub";
			display: "i=block/cogwheel";
        r1: Router6;
            display: "i=abstract/router";
        r2: Router6;
            display: "i=abstract/router";
        cli: StandardHost6[n];
        	display: "i=device/laptop";
        srv: StandardHost6[n];
        	display: "i=device/server_l";
	connections nocheck:
		for i=0..n-1 do
			cli[i].ethOut++ --> ethernetline --> r1.ethIn++;
			cli[i].ethIn++ <-- ethernetline <-- r1.ethOut++;
			srv[i].ethOut++ --> ethernetline --> r2.ethIn++;
			srv[i].ethIn++ <-- ethernetline <-- r2.ethOut++;
		endfor
		
		r1.ethOut++ --> ethernetline --> r2.ethIn++;
		r1.ethIn++ <-- ethernetline <-- r2.ethOut++;
		
endmodule

network demoNetworkEth : DemoNetworkEth
endnetwork