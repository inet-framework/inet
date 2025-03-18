//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Module interface for fragmentation policies that determine how packets are
// split into fragments. Implementations calculate the appropriate fragment
// sizes based on various constraints such as maximum transmission unit.
//
moduleinterface IFragmenterPolicy
{
    parameters:
        @display("i=block/star");
}
