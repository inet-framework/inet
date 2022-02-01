//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.contract;

//
// The module interface for mobility models.
//
moduleinterface IMobility
{
    parameters:
        @display("i=block/cogwheel");
        @signal[mobilityStateChanged];
}

