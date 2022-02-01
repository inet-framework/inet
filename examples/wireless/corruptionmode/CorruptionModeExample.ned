//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.corruptionmode;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadioMedium;

network CorruptionModeExample
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: ApskScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        sourceHost: AdhocHost {
            parameters:
                @display("p=250,150");
        }
        destinationHost: AdhocHost {
            parameters:
                @display("p=400,150");
        }
}
