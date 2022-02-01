// Hierarchical Token Bucket Implementation for OMNeT++ & INET Framework
// Copyright (C) 2021 Marija Gajić (NTNU), Marcin Bosk (TUM), Susanna Schwarzmann (TU Berlin), Stanislav Lange (NTNU), and Thomas Zinner (NTNU)
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

package inet.examples.htb;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network htbEvaluation
{
    parameters:
        @display("bgb=710,417");
        int nVID = default(1); // Number of video clients
        int nLVD = default(1); // Number of live video clients
        int nFDO = default(1); // Number of file download clients
        int nSSH = default(1); // Number of SSH clients
        int nVIP = default(1); // Number of VoIP clients
    types:
        channel C extends DatarateChannel
        {
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=10,10;is=s");
        }

        hostFDO[nFDO]: StandardHost {
            @display("p=140,203");
        }

        serverFDO: StandardHost {
            @display("p=565,203");
        }

    connections:
        hostFDO[0].pppg++ <--> connFIX0: C <--> serverFDO.pppg++;
}

network htbEvaluationEth
{
    parameters:
        @display("bgb=710,417");
        int nVID = default(1); // Number of video clients
        int nLVD = default(1); // Number of live video clients
        int nFDO = default(1); // Number of file download clients
        int nSSH = default(1); // Number of SSH clients
        int nVIP = default(1); // Number of VoIP clients
    types:
        channel C extends DatarateChannel
        {
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=10,10;is=s");
        }

        hostFDO[nFDO]: StandardHost {
            @display("p=140,203");
        }

        serverFDO: StandardHost {
            @display("p=565,203");
        }

    connections:
        hostFDO[0].ethg++ <--> connFIX0: C <--> serverFDO.ethg++;
}

