//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.validation.tsn.trafficshaping.asynchronousshaper.icct;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network AsynchronousShaperValidation extends TsnNetworkBase
{
    submodules:
        N1: TsnDevice {
            @display("p=500,150");
        }
        N2: TsnDevice {
            @display("p=300,300");
        }
        N3: TsnDevice {
            @display("p=500,450");
        }
        N4: TsnDevice {
            @display("p=700,150");
        }
        N5: TsnDevice {
            @display("p=700,450");
        }
        N6: TsnDevice {
            @display("p=900,150");
        }
        N7: TsnDevice {
            @display("p=1100,300");
        }
        N8: TsnDevice {
            @display("p=900,450");
        }
        S1: TsnSwitch {
            @display("p=500,300");
        }
        S2: TsnSwitch {
            @display("p=700,300");
        }
        S3: TsnSwitch {
            @display("p=900,300");
        }
    connections:
        N1.ethg++ <--> Eth100M <--> S1.ethg++;
        N2.ethg++ <--> Eth100M <--> S1.ethg++;
        N3.ethg++ <--> Eth100M <--> S1.ethg++;
        N4.ethg++ <--> Eth100M <--> S2.ethg++;
        N5.ethg++ <--> Eth100M <--> S2.ethg++;
        N6.ethg++ <--> Eth100M <--> S3.ethg++;
        N7.ethg++ <--> Eth100M <--> S3.ethg++;
        N8.ethg++ <--> Eth100M <--> S3.ethg++;
        S1.ethg++ <--> Eth100M <--> S2.ethg++;
        S2.ethg++ <--> Eth100M <--> S3.ethg++;
}
