//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Traffic conditioners connect one input to one output, operating in passive
// mode on input and active mode on output. They can drop, delay, and reorder
// packets without modifying packet contents.
//
moduleinterface ITrafficConditioner extends IPassivePacketSink, IActivePacketSource
{
    parameters:
        @omittedTypename(OmittedTrafficConditioner);
        @display("i=block/downarrow");
}
