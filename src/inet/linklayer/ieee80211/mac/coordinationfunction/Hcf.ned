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

import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.originator.NonQoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSMacDataService;
import inet.linklayer.ieee80211.mac.originator.QoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.QoSRtsPolicy;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.protectionmechanism.SingleProtectionMechanism;
import inet.linklayer.ieee80211.mac.rateselection.QoSRateSelection;
import inet.linklayer.ieee80211.mac.recipient.QoSCtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQoSAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQoSMacDataService;


//
// HCF -- Hybrid Coordination Function
//
module Hcf
{
    parameters:
        int numEdcafs = default(4);
        int maxQueueSize = default(100); // -1 means unlimited
        string rateControlType = default("");
        bool isBlockAckSupported = default(false);
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)

        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Hcf);
        @display("i=block/queue");
        @signal[NF_LINK_BREAK](type=inet::ieee80211::Ieee80211DataOrMgmtFrame);
        @signal[NF_PACKET_DROP](type=inet::ieee80211::Ieee80211Frame);

    submodules:
        edca: Edca {
            parameters:
                numEdcafs = numEdcafs;
                @display("p=150,100");
        }
        hcca: Hcca {
            parameters:
                @display("p=150,200");
        }
        originatorMacDataService: OriginatorQoSMacDataService {
            parameters:
                @display("p=150,300");
        }
        recipientMacDataService: RecipientQoSMacDataService {
            parameters:
                @display("p=150,400");
        }
        rateSelection: QoSRateSelection {
            parameters:
                rateControlModule = "^.rateControl";
                @display("p=400,100");
        }
        rateControl: <rateControlType> like IRateControl if rateControlType != "" {
            parameters:
                @display("p=400,200");
        }
        singleProtectionMechanism: SingleProtectionMechanism {
            parameters:
                @display("p=400,300");
        }
        edcaMgmtAndNonQoSRecoveryProcedure: NonQoSRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edca.edcaf[1]"; // best effort
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=700,300");
        }
        edcaDataRecoveryProcedures[numEdcafs]: QoSRecoveryProcedure {
            parameters:
                cwCalculatorModule = "^.edca.edcaf[" + string(index) + "]";
                rtsPolicyModule = "^.rtsPolicy";
                @display("p=700,400,column,100");
        }
        edcaTxopProcedures[numEdcafs]: TxopProcedure {
            parameters:
                @display("p=400,400,column,100");
        }
        originatorAckPolicy: OriginatorQoSAckPolicy {
            parameters:
                @display("p=1000,100");
        }
        recipientAckPolicy: RecipientQoSAckPolicy {
            parameters:
                @display("p=1000,200");
        }
        rtsPolicy: QoSRtsPolicy {
            parameters:
                @display("p=1000,300");
        }
        ctsPolicy: QoSCtsPolicy {
            parameters:
                @display("p=1000,400");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                originatorAckPolicyModule = "^.originatorAckPolicy";
                @display("p=1000,500");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy if isBlockAckSupported {
            parameters:
                @display("p=1000,600");
        }
}
