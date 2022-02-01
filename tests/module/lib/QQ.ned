//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib;

//
// a simple queue for prevents "channel is currently busy with an ongoing transmission"
// on an output channel connected to "out" gate
//
simple QQ
{
    gates:
        input in;
        output out;
}

