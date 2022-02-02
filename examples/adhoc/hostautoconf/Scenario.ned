//
// Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.adhoc.hostautoconf;

import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network Scenario
{
    parameters:
        int hosts;
        @display("bgb=650,450");
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,200;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100;is=s");
        }
        host[hosts]: AdhocHost {
            @display("p=300,150");
        }
}
