//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.mobility;

import inet.mobility.contract.IMobility;

//
// A host for demonstrating mobility models only -- it contains
// no protocol layers at all.
//
module MobileHost
{
    parameters:
        @networkNode();
        @display("i=device/pocketpc");
        @statistic[coordX](title="coord X"; source="xCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
        @statistic[coordY](title="coord Y"; source="yCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
        @statistic[coordZ](title="coord Z"; source="zCoord(mobilityPos(mobilityStateChanged))"; record=vector; interpolationmode=linear);
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            @display("p=100,100;is=s");
        }
}
