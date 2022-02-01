//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

simple EthernetPauseCommandProcessor
{
    parameters:
        //@class(EthernetPauseCommandProcessor);
        @display("i=block/inserter");
        @signal[pauseSent](type=long); // pause units
        @statistic[pauseSent](title="pause sent";record=count,sum,vector);
    gates:
        input in @labels(send);
        output out @labels(send);
}
