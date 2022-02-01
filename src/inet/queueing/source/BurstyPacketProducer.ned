//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;
import inet.queueing.scheduler.MarkovScheduler;

//
// This module mixes two different packet sources to generate bursty traffic.
//
module BurstyPacketProducer like IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out;
    submodules:
        normalProducer: ActivePacketSource {
            @display("p=100,100");
        }
        burstProducer: ActivePacketSource {
            @display("p=300,100");
        }
        scheduler: MarkovScheduler {
            @display("p=200,200");
        }
    connections allowunconnected:
        normalProducer.out --> scheduler.in++;
        burstProducer.out --> scheduler.in++;
        scheduler.out --> out;
}
