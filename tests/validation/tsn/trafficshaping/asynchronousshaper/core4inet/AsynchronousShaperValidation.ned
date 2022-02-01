//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
