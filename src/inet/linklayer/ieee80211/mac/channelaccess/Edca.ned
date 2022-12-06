//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.channelaccess;

import inet.linklayer.ieee80211.mac.contract.ICollisionController;
import inet.linklayer.ieee80211.mac.originator.NonQosRecoveryProcedure;

//
// Implements EDCA (Enhanced Distributed Channel Access) for IEEE 802.11.
// The implementation allows for a configurable number of access categories,
// not just four as defined by the standard.
//
module Edca
{
    parameters:
        int numEdcafs = default(4);
        string rxModule;
        string rateSelectionModule;

        *.rateSelectionModule = "^." + this.rateSelectionModule;
        *.rxModule = "^." + this.rxModule;
        *.collisionControllerModule = "^.collisionController";

        edcaf[0].accessCategory = default("AC_BK");
        edcaf[1].accessCategory = default("AC_BE");
        edcaf[2].accessCategory = default("AC_VI");
        edcaf[3].accessCategory = default("AC_VO");

        @class("Edca");
        @display("i=block/star");
    submodules:
        edcaf[numEdcafs]: Edcaf {
            parameters:
                @display("p=100,100,row,150");
        }
        collisionController: <default("EdcaCollisionController")> like ICollisionController {
            parameters:
                @display("p=100,200");
        }
        mgmtAndNonQoSRecoveryProcedure: NonQosRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edcaf[1]"; // best effort
                rtsPolicyModule = "^.^.rtsPolicy";
                @display("p=400,200");
        }
}

