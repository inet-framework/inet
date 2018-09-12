//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ieee80211.mac.coordinationfunction;

import inet.linklayer.ieee80211.mac.channelaccess.Dcaf;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.originator.NonQoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorMacDataService;
import inet.linklayer.ieee80211.mac.originator.RtsPolicy;
import inet.linklayer.ieee80211.mac.protectionmechanism.OriginatorProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.RateSelection;
import inet.linklayer.ieee80211.mac.recipient.CtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientMacDataService;

//
// IEEE 802.11 Distributed Coordination Function
//
module Dcf
{
    parameters:
        int maxQueueSize = default(100); // -1 means unlimited

        string rxModule;
        string txModule;
        string rateControlType = default("");
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Dcf);
        @display("i=block/queue");
        @signal[NF_LINK_BREAK](type=inet::ieee80211::Ieee80211DataOrMgmtFrame);
        @signal[NF_PACKET_DROP](type=inet::ieee80211::Ieee80211Frame);

    submodules:
        channelAccess: Dcaf {
            parameters:
                @display("p=150,100");
        }
        originatorMacDataService: OriginatorMacDataService {
            parameters:
                @display("p=150,200");
        }
        recipientMacDataService: RecipientMacDataService {
            parameters:
                @display("p=150,300");
        }
        rateSelection: RateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=400,100");
        }
        rateControl: <rateControlType> like IRateControl if rateControlType != "" {
            parameters:
                @display("p=400,200");
        }
        recoveryProcedure: NonQoSRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.channelAccess";
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=400,300");
        }
        originatorProtectionMechanism: OriginatorProtectionMechanism {
            parameters:
                @display("p=400,400");
        }
        originatorAckPolicy: OriginatorAckPolicy {
            parameters:
                @display("p=650,100");
        }
        recipientAckPolicy: RecipientAckPolicy {
            parameters:
                @display("p=650,200");
        }
        rtsPolicy: RtsPolicy {
            parameters:
                @display("p=650,300");
        }
        ctsPolicy: CtsPolicy {
            parameters:
                @display("p=650,400");
        }
}
