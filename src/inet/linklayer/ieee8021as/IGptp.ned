//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee8021as;

import inet.applications.contract.IApp;

//
// Interface for gPTP module.
//
moduleinterface IGptp extends IApp
{
    parameters:
        string gptpNodeType;
        @display("i=block/app");
}

