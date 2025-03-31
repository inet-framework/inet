//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation.base;

import inet.protocolelement.fragmentation.contract.IFragmenterPolicy;
import inet.queueing.base.PacketPusherBase;

//
// Base module for packet fragmenters that split packets into smaller fragments
// when they exceed the maximum allowed size. Uses a configurable fragmentation
// policy to determine fragment sizes.
//
module FragmenterBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        string fragmenterPolicyClass = default("");
        string fragmenterPolicyModule = default(".fragmenterPolicy");
        @display("i=block/fork");
        @class(FragmenterBase);
    submodules:
        fragmenterPolicy: <> like IFragmenterPolicy if typename != "" {
            parameters:
                @display("p=100,100");
        }
}
