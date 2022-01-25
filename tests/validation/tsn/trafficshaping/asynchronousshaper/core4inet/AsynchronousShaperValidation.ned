//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.validation.tsn.trafficshaping.asynchronousshaper.core4inet;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network AsynchronousShaperValidation extends TsnNetworkBase
{
    submodules:
        S0: TsnDevice {
            @display("p=400,150");
        }
        S5: TsnDevice {
            @display("p=400,300");
        }
        S6: TsnDevice {
            @display("p=400,450");
        }
        S7: TsnDevice {
            @display("p=400,600");
        }
        R: TsnDevice {
            @display("p=800,375");
        }
        switch: TsnSwitch {
            @display("p=600,375");
        }
    connections:
        S0.ethg++ <--> Eth100M <--> switch.ethg++;
        S5.ethg++ <--> Eth100M <--> switch.ethg++;
        S6.ethg++ <--> Eth100M <--> switch.ethg++;
        S7.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> R.ethg++;
}
