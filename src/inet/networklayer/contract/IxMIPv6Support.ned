//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.contract;

//
// Module interface for xMIPv6 (where x = F, H, F-H).
//
moduleinterface IxMIPv6Support
{
    parameters:
        bool isHomeAgent;
        bool isMobileNode;
        @display("i=block/classifier2");
    gates:
        input fromIPv6;
        output toIPv6;
}

