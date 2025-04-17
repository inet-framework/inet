//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.common.SimpleModule;

simple EthernetPauseCommandProcessor extends SimpleModule
{
    parameters:
        @class(EthernetPauseCommandProcessor);
        //@class(EthernetPauseCommandProcessor);
        @display("i=block/inserter");
        @signal[pauseSent](type=long); // Pause units
        @statistic[pauseSent](title="pause sent";record=count,sum,vector);
    gates:
        input in @labels(send);
        output out @labels(send);
}
