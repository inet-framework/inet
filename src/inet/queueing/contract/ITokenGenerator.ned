//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Token generators produce tokens at specific rates or based on events,
// which can be used by other modules for traffic shaping, rate limiting,
// and access control.
//
moduleinterface ITokenGenerator
{
    parameters:
        @display("i=block/star");
}
